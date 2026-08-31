"""Tests for main.py: the top-level CPR-machine state machine.

main.main() only returns via sys.exit() when it hits a FATAL_ERRORS code; any
other error just re-routes the state machine and the loop continues forever
by design (see CPRState.ABORT: "Halt -- only a power cycle exits this
state"). So tests that want to observe non-fatal routing use
_run_main_for_ticks() below to bail out after a bounded number of
iterations, instead of letting main() run forever.

Note: calling main.main() runs its real configure_logging(), which creates a
new (small, mostly empty) log file under <repo root>/Logs/ each time -- a
pre-existing side effect of importing/running main.py, not something these
tests add.
"""
import pytest

from conftest import install_fake_main_modules
from Enums.error_codes import ErrorCode
from Enums.states import CPRState


class _StopMainLoop(Exception):
    """Sentinel raised to break out of main()'s otherwise-infinite loop."""


def _run_main_for_ticks(monkeypatch, main, max_ticks):
    """Run main.main() for at most max_ticks loop iterations, then return.

    Patches time.sleep (main's per-tick pacing call) to count calls and raise
    _StopMainLoop once max_ticks is reached, so a test can inspect what the
    fakes recorded without needing main() to actually exit.
    """
    ticks = {"count": 0}

    def fake_sleep(seconds):
        ticks["count"] += 1
        if ticks["count"] >= max_ticks:
            raise _StopMainLoop()

    monkeypatch.setattr(main.time, "sleep", fake_sleep)
    try:
        main.main()
    except _StopMainLoop:
        pass


def _recording(return_value=ErrorCode.NORMAL_OPERATION):
    """Build a fake that records each call's args and returns a fixed value."""
    calls = []

    def fn(*args, **kwargs):
        calls.append(args)
        return return_value

    fn.calls = calls
    return fn


# -------------------- static routing tables --------------------

def test_error_state_map_routes_expected_errors_to_states(monkeypatch):
    """Every recoverable error routes to ABORT except the kneel failure, which routes
    to KNEEL_FAILURE so the operator can re-confirm alignment before resuming."""
    install_fake_main_modules(monkeypatch)
    import main

    assert main.ERROR_STATE_MAP[ErrorCode.ERROR_INIT_FAILURE] == CPRState.ABORT
    assert main.ERROR_STATE_MAP[ErrorCode.ERROR_SENSOR_FAILURE] == CPRState.ABORT
    assert main.ERROR_STATE_MAP[ErrorCode.ERROR_ZEROING_FAILURE] == CPRState.ABORT
    assert main.ERROR_STATE_MAP[ErrorCode.ERROR_LOW_BATTERY] == CPRState.ABORT
    assert main.ERROR_STATE_MAP[ErrorCode.ERROR_MOTOR_FAILURE] == CPRState.ABORT
    assert main.ERROR_STATE_MAP[ErrorCode.ERROR_PYGAME_FAILURE] == CPRState.ABORT
    assert main.ERROR_STATE_MAP[ErrorCode.ERROR_IMU_KNEEL_FAILURE] == CPRState.KNEEL_FAILURE


def test_fatal_errors_contains_expected_terminal_codes(monkeypatch):
    """Only truly unrecoverable conditions should end the process outright."""
    install_fake_main_modules(monkeypatch)
    import main

    assert main.FATAL_ERRORS == {
        ErrorCode.EXIT_UNKNOWN,
        ErrorCode.ERROR_PYGAME_INIT_FAILURE,
        ErrorCode.ERROR_PI_DAEMON_FAILURE,
        ErrorCode.ERROR_UNKNOWN_IMAGE,
    }
    assert ErrorCode.NORMAL_OPERATION not in main.FATAL_ERRORS
    # A fatal error should end the process outright, not get silently routed
    # to a state -- so the two tables should never overlap.
    assert main.FATAL_ERRORS.isdisjoint(main.ERROR_STATE_MAP.keys())


# -------------------- fatal-error fast exit --------------------

def test_main_exits_on_hmi_init_failure(monkeypatch):
    """A pygame init failure during startup should abort the motor and exit(1)."""
    modules = install_fake_main_modules(monkeypatch)
    setattr(modules["HMI"], "init_HMI", lambda pi_instance: ErrorCode.ERROR_PYGAME_INIT_FAILURE)
    abort = _recording()
    setattr(modules["actuation"], "abort_compressions", abort)

    import main
    with pytest.raises(SystemExit) as exc_info:
        main.main()

    assert exc_info.value.code == 1
    assert len(abort.calls) == 1  # the shutdown sequence aborts the motor before exiting


def test_main_exits_on_unknown_image(monkeypatch):
    """An unrecognized image path during startup is fatal, not routed to ABORT state."""
    modules = install_fake_main_modules(monkeypatch)
    setattr(modules["HMI"], "set_image_audio", lambda image, prompt: ErrorCode.ERROR_UNKNOWN_IMAGE)

    import main
    with pytest.raises(SystemExit):
        main.main()


def test_main_exits_on_pi_daemon_failure(monkeypatch):
    """A pigpio daemon failure anywhere is fatal -- GPIO state can't be trusted afterward."""
    modules = install_fake_main_modules(monkeypatch)
    setattr(modules["HMI"], "enable_next_button", lambda: ErrorCode.ERROR_PI_DAEMON_FAILURE)

    import main
    with pytest.raises(SystemExit):
        main.main()


# -------------------- non-fatal error routing --------------------

def test_main_routes_init_failure_to_abort_state(monkeypatch):
    """A non-fatal init failure should route to ABORT and run its setup, not exit."""
    modules = install_fake_main_modules(monkeypatch)
    setattr(modules["actuation"], "init_motor", lambda: ErrorCode.ERROR_INIT_FAILURE)
    set_image_audio = _recording()
    setattr(modules["HMI"], "set_image_audio", set_image_audio)

    import main
    _run_main_for_ticks(monkeypatch, main, max_ticks=3)

    assert (main.HMI.Image.ABORT, main.HMI.AudioPrompt.ABORT) in set_image_audio.calls


# -------------------- startup happy path --------------------

def test_main_advances_from_startup_when_next_pressed(monkeypatch):
    """With the Next button held down, each state's setup should run once and advance."""
    modules = install_fake_main_modules(monkeypatch)
    setattr(modules["HMI"], "next_button_pressed", lambda: (ErrorCode.NORMAL_OPERATION, True))
    set_image_audio = _recording()
    setattr(modules["HMI"], "set_image_audio", set_image_audio)

    import main
    _run_main_for_ticks(monkeypatch, main, max_ticks=3)

    assert (main.HMI.Image.STARTUP, main.HMI.AudioPrompt.STARTUP) in set_image_audio.calls
    assert (main.HMI.Image.UNFOLD, main.HMI.AudioPrompt.UNFOLD) in set_image_audio.calls
