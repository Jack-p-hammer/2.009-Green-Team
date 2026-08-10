"""Shared hardware abstractions for the CPR machine test suite.

This module fakes out every external hardware library the app touches
(pigpio, board/busio, the Adafruit sensor drivers, and moteus) so that
sensing.py, actuation.py, and moteus_thread.py can be imported and exercised
on a laptop with no hardware attached.

There are two layers of fakes, used for two different kinds of tests:

- Hardware-level fakes (`HardwareHarness` / `install_fake_hardware_modules` /
  the `hardware` fixture) stub out the external libraries only. The real
  sensing.py / actuation.py / moteus_thread.py source runs unmodified on top
  of them. Use this layer to test the logic inside those modules.

- Whole-module fakes (`install_fake_main_modules`) replace sensing.py,
  actuation.py, and HMI.py entirely with lightweight stand-ins matching their
  public API. Use this layer to test main.py's state machine in isolation,
  without depending on sensing/actuation/HMI internals.

HMI.py is not yet implemented, so there is no hardware-level fake for pygame
here. Once HMI.py is fleshed out, add a pygame fake alongside these.

sensing.py, actuation.py, and moteus_thread.py form a one-way dependency
chain (actuation -> sensing -> moteus_thread), not a cycle: the shared
MoteusThread instance is created in actuation.py and passed into
sensing.init_sensors() as a parameter, so no module needs to reach back into
one that imports it. Any of the three can be imported first in a test.
"""
import sys
import types
from pathlib import Path
from typing import Optional

import pytest

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src"
SRC_STR = str(SRC.resolve())
if SRC_STR not in sys.path:
    sys.path.insert(0, SRC_STR)

from Enums.error_codes import ErrorCode

# Modules from src/ that need to be re-imported fresh once fake hardware/main
# modules are (re)installed, so they pick up the fakes instead of any
# previously cached real/fake versions.
_APP_MODULES = ["sensing", "actuation", "moteus_thread", "HMI", "main"]

# Shared keys used by both the fake moteus.Register namespace and the values
# dict FakeMoteusController hands back, so `result.values[moteus.Register.X]`
# lookups succeed by identity.
_MOTEUS_REGISTERS = types.SimpleNamespace(
    MODE="mode", FAULT="fault", POSITION="position", VELOCITY="velocity", VOLTAGE="voltage",
)


def _clear_app_modules():
    for name in _APP_MODULES:
        sys.modules.pop(name, None)


def _fake_module(name: str, **attrs) -> types.ModuleType:
    """Build a types.ModuleType and populate it via setattr.

    Static type checkers (pyright/Pylance) reject `mod.attr = value` on a bare
    ModuleType since it has no declared attributes; setattr sidesteps that
    while still producing a real module object, which is what sys.modules is
    typed to hold.
    """
    mod = types.ModuleType(name)
    for key, value in attrs.items():
        setattr(mod, key, value)
    return mod


# -------------------- Hardware-level fakes --------------------

class FakePi:
    """Stand-in for a pigpio.pi() instance."""

    def __init__(self, connected=True):
        self.connected = connected
        self.modes = {}
        self.pulls = {}
        self.writes = []
        self.reads = {}

    def set_mode(self, pin, mode):
        self.modes[pin] = mode

    def set_pull_up_down(self, pin, pull):
        self.pulls[pin] = pull

    def write(self, pin, value):
        self.writes.append((pin, value))

    def read(self, pin):
        return self.reads.get(pin, 0)


class FakeI2C:
    """Stand-in for the busio.I2C bus shared by the ToF sensor, IMU, and ADC."""

    def __init__(self):
        self.adc_bytes = bytearray(2)
        self.reads = []

    def readfrom_into(self, address, buffer):
        self.reads.append(address)
        n = min(len(buffer), len(self.adc_bytes))
        buffer[:n] = self.adc_bytes[:n]


class FakeVL6180X:
    """Stand-in for the adafruit_vl6180x.VL6180X time-of-flight sensor."""

    def __init__(self):
        self.range = 0


class FakeBNO08X_I2C:
    """Stand-in for the adafruit_bno08x BNO08X_I2C IMU."""

    def __init__(self):
        self.enabled_features = []
        self.acceleration = (0.0, 0.0, 9.81)

    def enable_feature(self, feature):
        self.enabled_features.append(feature)


class FakeMoteusResult:
    """Stand-in for the moteus.Result returned by a query-mode command."""

    def __init__(self, values: dict):
        self.values = values


class FakeMoteusController:
    """Stand-in for moteus.Controller, the object MoteusThread talks to.

    Configure `mode`/`fault`/`position`/`velocity`/`voltage` to control what
    the next query returns, or set `raise_on_set_position` to an exception
    instance to simulate a communication failure. Every call is recorded in
    `commands` for assertions.
    """

    def __init__(self):
        self.controller_id: Optional[int] = None
        self.commands: list[dict] = []
        self.mode = 0
        self.fault = 0
        self.position = 0.0
        self.velocity = 0.0
        self.voltage = 24.0
        self.raise_on_set_position: Optional[Exception] = None

    async def set_position(self, **kwargs) -> FakeMoteusResult:
        if self.raise_on_set_position is not None:
            raise self.raise_on_set_position
        self.commands.append(kwargs)
        return FakeMoteusResult({
            _MOTEUS_REGISTERS.MODE: self.mode,
            _MOTEUS_REGISTERS.FAULT: self.fault,
            _MOTEUS_REGISTERS.POSITION: self.position,
            _MOTEUS_REGISTERS.VELOCITY: self.velocity,
            _MOTEUS_REGISTERS.VOLTAGE: self.voltage,
        })


class HardwareHarness:
    """One place to configure and inspect all fake hardware for a test.

    Set the `*_connect_error` attributes to an exception instance to make the
    corresponding init step fail; leave them None (the default) for a
    successful init. The `pi`/`i2c`/`tof`/`imu`/`moteus_controller` attributes
    are the actual fake objects the app code will read from and write to, so
    tests can configure sensor readings on them directly (e.g.
    `harness.tof.range = 42`).
    """

    def __init__(self):
        self.pi = FakePi()
        self.i2c = FakeI2C()
        self.tof = FakeVL6180X()
        self.imu = FakeBNO08X_I2C()
        self.moteus_controller = FakeMoteusController()

        self.i2c_connect_error: Optional[Exception] = None
        self.tof_connect_error: Optional[Exception] = None
        self.imu_connect_error: Optional[Exception] = None
        self.moteus_connect_error: Optional[Exception] = None


def _make_pigpio_module(harness: HardwareHarness) -> types.ModuleType:
    return _fake_module(
        "pigpio",
        INPUT=0,
        OUTPUT=1,
        PUD_UP=2,
        PUD_DOWN=3,
        pi=lambda: harness.pi,
    )


def _make_board_module() -> types.ModuleType:
    return _fake_module("board", SCL="SCL", SDA="SDA")


def _make_busio_module(harness: HardwareHarness) -> types.ModuleType:
    def I2C(scl, sda):
        if harness.i2c_connect_error is not None:
            raise harness.i2c_connect_error
        return harness.i2c

    return _fake_module("busio", I2C=I2C)


def _make_vl6180x_module(harness: HardwareHarness) -> types.ModuleType:
    def VL6180X(i2c):
        if harness.tof_connect_error is not None:
            raise harness.tof_connect_error
        return harness.tof

    return _fake_module("adafruit_vl6180x", VL6180X=VL6180X)


def _make_bno08x_modules(harness: HardwareHarness) -> tuple[types.ModuleType, types.ModuleType]:
    def BNO08X_I2C(i2c):
        if harness.imu_connect_error is not None:
            raise harness.imu_connect_error
        return harness.imu

    bno_i2c_mod = _fake_module("adafruit_bno08x.i2c", BNO08X_I2C=BNO08X_I2C)
    bno_mod = _fake_module("adafruit_bno08x", BNO_REPORT_ACCELEROMETER="accel", i2c=bno_i2c_mod)
    return bno_mod, bno_i2c_mod


def _make_moteus_module(harness: HardwareHarness) -> types.ModuleType:
    def Controller(*, id=1, **_kwargs):
        if harness.moteus_connect_error is not None:
            raise harness.moteus_connect_error
        harness.moteus_controller.controller_id = id
        return harness.moteus_controller

    return _fake_module("moteus", Register=_MOTEUS_REGISTERS, Controller=Controller)


def install_fake_hardware_modules(harness: Optional[HardwareHarness] = None) -> HardwareHarness:
    """Install fake pigpio/board/busio/adafruit/moteus modules into sys.modules.

    Pass a pre-configured HardwareHarness (e.g. with a `*_connect_error` set)
    to simulate a failure from the very first import, or omit it to get a
    harness that behaves as fully healthy hardware until a test configures it
    otherwise. Also clears any cached sensing/actuation/moteus_thread/HMI/main
    imports so the next `import` of those picks up the fakes installed here.
    """
    harness = harness or HardwareHarness()

    sys.modules["pigpio"] = _make_pigpio_module(harness)
    sys.modules["board"] = _make_board_module()
    sys.modules["busio"] = _make_busio_module(harness)
    sys.modules["adafruit_vl6180x"] = _make_vl6180x_module(harness)
    bno_mod, bno_i2c_mod = _make_bno08x_modules(harness)
    sys.modules["adafruit_bno08x"] = bno_mod
    sys.modules["adafruit_bno08x.i2c"] = bno_i2c_mod
    sys.modules["moteus"] = _make_moteus_module(harness)

    _clear_app_modules()

    return harness


@pytest.fixture
def hardware():
    """Provide a fresh HardwareHarness with fake hardware modules installed.

    Usage:
        def test_something(hardware):
            hardware.pi.connected = False
            import actuation, sensing
            actuation.init_motor()
            assert sensing.init_sensors(actuation.get_motor_controller()) == ErrorCode.ERROR_INIT_FAILURE

    Note that sensing.init_sensors() takes the shared MoteusThread instance as
    a parameter (it zeroes the rotary encoder by reading from it directly), so
    call `actuation.init_motor()` first and pass `actuation.get_motor_controller()`
    in, matching what main.py does.
    """
    harness = install_fake_hardware_modules()
    yield harness

    # Stop any motor command thread a test started via actuation.init_motor(),
    # so it doesn't keep spinning at 100Hz in the background during later tests.
    actuation_mod = sys.modules.get("actuation")
    if actuation_mod is not None:
        controller = getattr(actuation_mod, "_motor_controller", None)
        if controller is not None:
            controller.stop()

    _clear_app_modules()


# -------------------- Whole-module fakes for main.py tests --------------------

def install_fake_main_modules(monkeypatch) -> dict:
    """Install lightweight fake sensing/actuation/HMI modules for main.py tests.

    These replace the whole module (not just the underlying hardware
    libraries), so main.py's state machine can be tested without depending on
    the real sensing/actuation logic. HMI.py is still unimplemented, so this
    fake only covers the surface main.py currently calls; revisit once HMI.py
    is built out.

    Returns a dict of the installed fake modules so a test can further
    customize individual functions. Use setattr (not direct assignment) so
    static type checkers don't flag it the same way direct assignment on a
    ModuleType is flagged above, e.g.:
        modules = install_fake_main_modules(monkeypatch)
        setattr(modules["HMI"], "next_button_pressed", lambda: True)
    """
    monkeypatch.delitem(sys.modules, "main", raising=False)

    sensing_mod = _fake_module(
        "sensing",
        init_sensors=lambda motor_controller: ErrorCode.NORMAL_OPERATION,
        get_pi=lambda: FakePi(),
        zero_position=lambda: ErrorCode.NORMAL_OPERATION,
        battery_check=lambda: ErrorCode.NORMAL_OPERATION,
    )
    monkeypatch.setitem(sys.modules, "sensing", sensing_mod)

    actuation_mod = _fake_module(
        "actuation",
        init_motor=lambda: ErrorCode.NORMAL_OPERATION,
        get_motor_controller=lambda: None,
        init_zeroing=lambda: ErrorCode.NORMAL_OPERATION,
        zeroing=lambda: ErrorCode.NORMAL_OPERATION,
        init_compressions=lambda: ErrorCode.NORMAL_OPERATION,
        compressions=lambda: ErrorCode.NORMAL_OPERATION,
        pause_compressions=lambda: ErrorCode.NORMAL_OPERATION,
        abort_compressions=lambda: ErrorCode.NORMAL_OPERATION,
    )
    monkeypatch.setitem(sys.modules, "actuation", actuation_mod)

    hmi_image = types.SimpleNamespace(
        STARTUP="startup",
        UNFOLD="unfold",
        ALIGNMENT="alignment",
        ZEROING_PREP="zeroing_prep",
        ZEROING="zeroing",
        COMPRESSION_PREP="compression_prep",
        COMPRESSION="compression",
        PAUSE="pause",
        ABORT="abort",
        KNEEL_FAILURE="kneel_failure",
    )
    hmi_audio_prompt = types.SimpleNamespace(
        STARTUP="startup_prompt",
        UNFOLD="unfold_prompt",
        ALIGNMENT="alignment_prompt",
        ZEROING_PREP="zeroing_prep_prompt",
        ZEROING="zeroing_prompt",
        COMPRESSION_PREP="compression_prep_prompt",
        COMPRESSION="compression_prompt",
        PAUSE="",
        ABORT="",
        KNEEL_FAILURE="",
    )
    hmi_mod = _fake_module(
        "HMI",
        Image=hmi_image,
        AudioPrompt=hmi_audio_prompt,
        init_HMI=lambda pi_instance: ErrorCode.NORMAL_OPERATION,
        set_screen_audio=lambda image, prompt: None,
        enable_next_button=lambda: None,
        disable_next_button=lambda: None,
        enable_pause_button=lambda: None,
        disable_pause_button=lambda: None,
        enable_lasers=lambda: None,
        disable_lasers=lambda: None,
        next_button_pressed=lambda: False,
        pause_button_pressed=lambda: False,
        audio_finished=lambda: True,
    )
    monkeypatch.setitem(sys.modules, "HMI", hmi_mod)

    return {"sensing": sensing_mod, "actuation": actuation_mod, "HMI": hmi_mod}
