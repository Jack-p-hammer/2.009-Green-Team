"""Tests for actuation.py: motor init, zeroing, compressions, pause, and abort.

Most tests here rely on the real actuation -> sensing -> moteus_thread chain
via the `hardware` fixture. Behavior that depends on the motor controller's
latest queried state (motor error, position, battery voltage) requires a
short sleep after configuring hardware.moteus_controller, since that state is
only updated once MoteusThread's background command-loop thread ticks -- see
the `hardware` fixture's docstring in conftest.py.
"""
import pytest

from Enums.error_codes import ErrorCode


# -------------------- init_motor / get_motor_controller --------------------

def test_init_motor_success_creates_motor_controller(hardware):
    import actuation

    assert actuation.init_motor() == ErrorCode.NORMAL_OPERATION
    assert actuation.get_motor_controller() is not None


def test_init_motor_failure_when_moteus_construction_fails(hardware):
    hardware.moteus_connect_error = RuntimeError("no CAN adapter")
    import actuation

    assert actuation.init_motor() == ErrorCode.ERROR_INIT_FAILURE


def test_get_motor_controller_returns_same_instance_after_init(hardware):
    import actuation
    actuation.init_motor()

    controller = actuation.get_motor_controller()
    assert actuation.get_motor_controller() is controller


# -------------------- init_zeroing / init_compressions --------------------

def test_init_zeroing_sets_start_time(hardware):
    import time
    import actuation

    before = time.monotonic()
    assert actuation.init_zeroing() == ErrorCode.NORMAL_OPERATION
    after = time.monotonic()

    assert before <= actuation.zeroing_start_time <= after


def test_init_compressions_sets_start_time(hardware):
    import time
    import actuation

    before = time.monotonic()
    assert actuation.init_compressions() == ErrorCode.NORMAL_OPERATION
    after = time.monotonic()

    assert before <= actuation.compression_start_time <= after


# -------------------- zeroing --------------------

def test_zeroing_fails_on_timeout(hardware):
    """The timeout check runs before touching the motor controller at all."""
    import actuation
    actuation.init_zeroing()
    actuation.zeroing_start_time -= 9999  # simulate far more than ZEROING_TIMEOUT_SEC elapsed

    assert actuation.zeroing() == ErrorCode.ERROR_ZEROING_FAILURE


def test_zeroing_fails_on_motor_error(hardware):
    import time
    import actuation
    actuation.init_motor()
    actuation.init_zeroing()

    hardware.moteus_controller.raise_on_set_position = RuntimeError("comm lost")
    time.sleep(0.05)  # let the background command loop record the failure

    assert actuation.zeroing() == ErrorCode.ERROR_MOTOR_FAILURE


def test_zeroing_fails_on_max_extension(hardware):
    import time
    import actuation
    actuation.init_motor()
    actuation.init_zeroing()

    hardware.moteus_controller.position = actuation.EXTENSION_STROKE_LIMIT_M + 1.0
    time.sleep(0.05)

    assert actuation.zeroing() == ErrorCode.ERROR_ZEROING_FAILURE


def test_zeroing_succeeds_when_sensors_healthy(hardware):
    import actuation
    import sensing
    actuation.init_motor()
    sensing.init_sensors(actuation.get_motor_controller())
    actuation.init_zeroing()

    assert actuation.zeroing() == ErrorCode.NORMAL_OPERATION


# -------------------- computeCompressionSetpoint (pure) --------------------

def test_compute_compression_setpoint_piecewise_profile(hardware, monkeypatch):
    """The trapezoidal waveform should stay flat, ramp up, plateau, ramp down, and repeat."""
    import actuation

    fake_time = {"t": 0.0}
    monkeypatch.setattr(actuation.time, "monotonic", lambda: fake_time["t"])
    actuation.init_compressions()  # compression_start_time = 0.0
    peak = actuation.COMPRESSION_DEPTH_CM / 100.0

    fake_time["t"] = 0.05  # 0.00-0.12s: still at the top
    assert actuation.computeCompressionSetpoint() == pytest.approx(0.0)

    fake_time["t"] = 0.18  # 0.12-0.24s: ramping down toward full compression
    assert 0.0 < actuation.computeCompressionSetpoint() < peak

    fake_time["t"] = 0.28  # 0.24-0.323s: fully compressed
    assert actuation.computeCompressionSetpoint() == pytest.approx(peak)

    fake_time["t"] = 0.50  # 0.323-0.56s: ramping back up
    assert 0.0 < actuation.computeCompressionSetpoint() < peak

    fake_time["t"] = 0.56 + 0.05  # cycle repeats every 0.56s
    assert actuation.computeCompressionSetpoint() == pytest.approx(0.0)


# -------------------- compressions --------------------

def test_compressions_normal_operation(hardware):
    import actuation
    import sensing
    actuation.init_motor()
    sensing.init_sensors(actuation.get_motor_controller())

    assert actuation.compressions() == ErrorCode.NORMAL_OPERATION


def test_compressions_fails_on_sensor_error(hardware):
    import actuation
    import sensing
    actuation.init_motor()
    sensing.init_sensors(actuation.get_motor_controller())
    hardware.imu.acceleration = (0.0, 0.0, 20.0)  # exceeds the compression accel limit

    assert actuation.compressions() == ErrorCode.ERROR_SENSOR_FAILURE


def test_compressions_fails_on_motor_error(hardware):
    import time
    import actuation
    import sensing
    actuation.init_motor()
    sensing.init_sensors(actuation.get_motor_controller())

    hardware.moteus_controller.raise_on_set_position = RuntimeError("comm lost")
    time.sleep(0.05)

    assert actuation.compressions() == ErrorCode.ERROR_MOTOR_FAILURE


# -------------------- pause_compressions --------------------

def test_pause_compressions_normal_operation(hardware):
    import actuation
    import sensing
    actuation.init_motor()
    sensing.init_sensors(actuation.get_motor_controller())

    assert actuation.pause_compressions() == ErrorCode.NORMAL_OPERATION


def test_pause_compressions_fails_on_sensor_error(hardware):
    import actuation
    import sensing
    actuation.init_motor()
    sensing.init_sensors(actuation.get_motor_controller())
    hardware.imu.acceleration = (0.0, 0.0, 20.0)

    assert actuation.pause_compressions() == ErrorCode.ERROR_SENSOR_FAILURE


def test_pause_compressions_fails_on_motor_error(hardware):
    import time
    import actuation
    import sensing
    actuation.init_motor()
    sensing.init_sensors(actuation.get_motor_controller())

    hardware.moteus_controller.raise_on_set_position = RuntimeError("comm lost")
    time.sleep(0.05)

    assert actuation.pause_compressions() == ErrorCode.ERROR_MOTOR_FAILURE


# -------------------- abort_compressions --------------------

def test_abort_compressions_does_not_require_sensing_init(hardware):
    """Abort intentionally skips sensor reads to return to zero as fast as possible."""
    import actuation
    actuation.init_motor()
    # Deliberately skip sensing.init_sensors() -- abort must not depend on it.

    assert actuation.abort_compressions() == ErrorCode.NORMAL_OPERATION
