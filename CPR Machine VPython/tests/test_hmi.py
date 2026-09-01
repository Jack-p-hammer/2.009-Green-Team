"""Tests for HMI.py: screen/audio display, button/LED GPIO, and laser control.

Adapted to the real current HMI.py API: init_HMI/pump_events/set_image/
set_audio/set_image_audio, enable/disable_next_button, enable/
disable_pause_button, enable/disable_lasers, next_button_pressed/
pause_button_pressed (each returning an (ErrorCode, bool) tuple), and
audio_finished (elapsed-time based, not pygame.mixer.get_busy()).

One item from the team's test inventory isn't covered here because the
underlying HMI.py functionality doesn't exist, not because a test is
missing: "audio_finished handles missing files gracefully" (expects
ERROR_UNKNOWN_AUDIO) -- no such error code exists in Enums.error_codes, and
audio_finished() returns a plain bool. It never touches pygame or loads a
file, so it can't fail this way; loading the audio file is set_audio()'s
job, and its failure path is covered by
test_set_audio_plays_prompt_and_reports_failures (which returns the
existing ERROR_PYGAME_FAILURE, not ERROR_UNKNOWN_AUDIO).

There's also no function that takes a duty cycle argument to accept or
reject arbitrary values (enable_lasers()/disable_lasers() always use the
fixed LASER_PWM_DUTY_CYCLE constant), so "accepts/rejects duty cycles
0/0.5/1 vs -0.1/1.5" isn't directly testable either. What test_lasers_
respond_to_enable_disable checks instead is the hardware analog: that the
raw value handed to hardware_PWM() is exactly LASER_PWM_DUTY_CYCLE *
LASER_PWM_SCALE, so HMI.py's configured duty cycle and scale are faithfully
reflected in what actually reaches the hardware.
"""
from Enums.error_codes import ErrorCode


# -------------------- init_HMI --------------------

def test_init_hmi_sets_up_display_and_gpio(hardware):
    """Startup should configure the fullscreen display and every GPIO pin."""
    import HMI

    assert HMI.init_HMI(hardware.pi) == ErrorCode.NORMAL_OPERATION

    # Buttons are inputs with pull-ups
    assert hardware.pi.modes[HMI.NEXT_BTN_PIN] == 0   # pigpio.INPUT
    assert hardware.pi.modes[HMI.PAUSE_BTN_PIN] == 0
    assert hardware.pi.pulls[HMI.NEXT_BTN_PIN] == 2   # pigpio.PUD_UP
    assert hardware.pi.pulls[HMI.PAUSE_BTN_PIN] == 2

    # LEDs and the laser pin are outputs, pulled down
    for pin in (HMI.NEXT_ENABLE_PIN, HMI.PAUSE_ENABLE_PIN, HMI.LASER_PIN):
        assert hardware.pi.modes[pin] == 1   # pigpio.OUTPUT
        assert hardware.pi.pulls[pin] == 3   # pigpio.PUD_DOWN

    # Laser PWM starts off
    assert hardware.pi.pwm_calls[-1] == (HMI.LASER_PIN, 0, 0)


def test_init_hmi_fails_when_pygame_init_fails(hardware):
    """A pygame/mixer init failure is treated as the dedicated fatal error."""
    hardware.pygame.init_error = RuntimeError("no audio device")
    import HMI

    assert HMI.init_HMI(hardware.pi) == ErrorCode.ERROR_PYGAME_INIT_FAILURE


def test_init_hmi_fails_when_display_init_fails(hardware):
    """A display setup failure (Info/set_mode/set_caption) is a regular init failure."""
    hardware.pygame.display_error = RuntimeError("no display")
    import HMI

    assert HMI.init_HMI(hardware.pi) == ErrorCode.ERROR_INIT_FAILURE


def test_init_hmi_fails_when_gpio_setup_fails(hardware):
    """A pigpio failure while configuring buttons/LEDs/laser is an init failure."""
    hardware.pi.raise_on_set_mode = RuntimeError("gpio daemon unavailable")
    import HMI

    assert HMI.init_HMI(hardware.pi) == ErrorCode.ERROR_INIT_FAILURE


# -------------------- button / LED helpers --------------------

def test_enable_disable_next_button_writes_expected_pin_states(hardware):
    """Enabling/disabling the Next button should write its LED/button-gate pin."""
    import HMI
    HMI.init_HMI(hardware.pi)

    assert HMI.enable_next_button() == ErrorCode.NORMAL_OPERATION
    assert hardware.pi.writes[-1] == (HMI.NEXT_ENABLE_PIN, 1)

    assert HMI.disable_next_button() == ErrorCode.NORMAL_OPERATION
    assert hardware.pi.writes[-1] == (HMI.NEXT_ENABLE_PIN, 0)


def test_enable_disable_pause_button_writes_expected_pin_states(hardware):
    """Enabling/disabling the Pause button should write its LED/button-gate pin."""
    import HMI
    HMI.init_HMI(hardware.pi)

    assert HMI.enable_pause_button() == ErrorCode.NORMAL_OPERATION
    assert hardware.pi.writes[-1] == (HMI.PAUSE_ENABLE_PIN, 1)

    assert HMI.disable_pause_button() == ErrorCode.NORMAL_OPERATION
    assert hardware.pi.writes[-1] == (HMI.PAUSE_ENABLE_PIN, 0)


def test_button_led_write_failure_reports_pi_daemon_error(hardware):
    """A pigpio write failure while toggling a button LED surfaces as a daemon error."""
    hardware.pi.raise_on_write = RuntimeError("daemon gone")
    import HMI
    HMI.init_HMI(hardware.pi)

    assert HMI.enable_next_button() == ErrorCode.ERROR_PI_DAEMON_FAILURE


def test_button_enable_disable_is_idempotent(hardware):
    """Calling enable/disable twice in a row should just re-assert the same pin state."""
    import HMI
    HMI.init_HMI(hardware.pi)

    HMI.enable_next_button()
    HMI.enable_next_button()
    HMI.enable_pause_button()
    HMI.enable_pause_button()
    assert hardware.pi.writes[-4:] == [
        (HMI.NEXT_ENABLE_PIN, 1), (HMI.NEXT_ENABLE_PIN, 1),
        (HMI.PAUSE_ENABLE_PIN, 1), (HMI.PAUSE_ENABLE_PIN, 1),
    ]

    HMI.disable_next_button()
    HMI.disable_next_button()
    HMI.disable_pause_button()
    HMI.disable_pause_button()
    assert hardware.pi.writes[-4:] == [
        (HMI.NEXT_ENABLE_PIN, 0), (HMI.NEXT_ENABLE_PIN, 0),
        (HMI.PAUSE_ENABLE_PIN, 0), (HMI.PAUSE_ENABLE_PIN, 0),
    ]


# -------------------- button readers --------------------

def test_button_readers_respond_to_fake_gpio_input(hardware):
    """Button readers should reflect whatever the fake GPIO pins report."""
    import HMI
    HMI.init_HMI(hardware.pi)

    hardware.pi.reads[HMI.NEXT_BTN_PIN] = 0
    error, pressed = HMI.next_button_pressed()
    assert error == ErrorCode.NORMAL_OPERATION
    assert pressed is False

    hardware.pi.reads[HMI.NEXT_BTN_PIN] = 1
    error, pressed = HMI.next_button_pressed()
    assert error == ErrorCode.NORMAL_OPERATION
    assert pressed is True

    hardware.pi.reads[HMI.PAUSE_BTN_PIN] = 1
    error, pressed = HMI.pause_button_pressed()
    assert error == ErrorCode.NORMAL_OPERATION
    assert pressed is True


def test_button_reader_failure_reports_pi_daemon_error(hardware):
    """A pigpio read failure surfaces as a daemon error rather than raising."""
    hardware.pi.raise_on_read = RuntimeError("daemon gone")
    import HMI
    HMI.init_HMI(hardware.pi)

    error, pressed = HMI.next_button_pressed()
    assert error == ErrorCode.ERROR_PI_DAEMON_FAILURE
    assert pressed is False


def test_both_buttons_pressed_simultaneously(hardware):
    """Next and Pause are read independently, so both can report pressed at once."""
    import HMI
    HMI.init_HMI(hardware.pi)
    hardware.pi.reads[HMI.NEXT_BTN_PIN] = 1
    hardware.pi.reads[HMI.PAUSE_BTN_PIN] = 1

    next_error, next_pressed = HMI.next_button_pressed()
    pause_error, pause_pressed = HMI.pause_button_pressed()

    assert (next_error, next_pressed) == (ErrorCode.NORMAL_OPERATION, True)
    assert (pause_error, pause_pressed) == (ErrorCode.NORMAL_OPERATION, True)


# -------------------- laser control --------------------

def test_lasers_respond_to_enable_disable(hardware):
    """Enabling lasers should set a nonzero PWM frequency/duty cycle; disabling zeroes both.

    There's no function that takes a duty cycle argument to test accepting/
    rejecting arbitrary values (enable_lasers()/disable_lasers() always use
    the fixed LASER_PWM_DUTY_CYCLE constant), so this checks the hardware
    analog instead: that the raw value actually handed to hardware_PWM()
    correctly reflects HMI.py's configured duty cycle and scale.
    """
    import HMI
    HMI.init_HMI(hardware.pi)

    assert HMI.enable_lasers() == ErrorCode.NORMAL_OPERATION
    pin, frequency, duty_cycle = hardware.pi.pwm_calls[-1]
    assert pin == HMI.LASER_PIN
    assert frequency == HMI.LASER_PWM_FREQUENCY
    assert 0 <= duty_cycle <= HMI.LASER_PWM_SCALE
    assert duty_cycle == int(HMI.LASER_PWM_DUTY_CYCLE * HMI.LASER_PWM_SCALE)

    assert HMI.disable_lasers() == ErrorCode.NORMAL_OPERATION
    assert hardware.pi.pwm_calls[-1] == (HMI.LASER_PIN, 0, 0)


def test_laser_pwm_failure_reports_pi_daemon_error(hardware):
    """A pigpio PWM failure while (de)activating the laser is a daemon error."""
    hardware.pi.raise_on_hardware_pwm = RuntimeError("pwm channel busy")
    import HMI
    HMI.init_HMI(hardware.pi)

    assert HMI.enable_lasers() == ErrorCode.ERROR_PI_DAEMON_FAILURE
    assert HMI.disable_lasers() == ErrorCode.ERROR_PI_DAEMON_FAILURE


# -------------------- pump_events --------------------

def test_pump_events_normal_and_failure(hardware):
    """pump_events() should succeed normally and warn (not raise) if pygame fails."""
    import HMI
    HMI.init_HMI(hardware.pi)

    assert HMI.pump_events() == ErrorCode.NORMAL_OPERATION

    hardware.pygame.event_pump_error = RuntimeError("event queue broken")
    assert HMI.pump_events() == ErrorCode.WARNING_PYGAME_PUMP_FAILURE


# -------------------- screen / audio --------------------

def test_set_image_success_and_failure(hardware):
    """set_image() should succeed for a normal prompt and report ERROR_UNKNOWN_IMAGE on failure."""
    import HMI
    HMI.init_HMI(hardware.pi)

    assert HMI.set_image(HMI.Image.UNFOLD) == ErrorCode.NORMAL_OPERATION

    hardware.pygame.image_load_error = RuntimeError("missing asset")
    assert HMI.set_image(HMI.Image.UNFOLD) == ErrorCode.ERROR_UNKNOWN_IMAGE


def test_set_audio_plays_prompt_and_reports_failures(hardware):
    """set_audio() should start playback on the shared channel, looping, and surface failures."""
    import HMI
    HMI.init_HMI(hardware.pi)

    assert HMI.set_audio(HMI.AudioPrompt.UNFOLD) == ErrorCode.NORMAL_OPERATION
    assert hardware.pygame.channel.sound is not None
    assert hardware.pygame.channel.loops == -1

    hardware.pygame.audio_load_error = RuntimeError("missing asset")
    assert HMI.set_audio(HMI.AudioPrompt.ALIGNMENT) == ErrorCode.ERROR_PYGAME_FAILURE


def test_set_audio_with_no_prompt_stops_channel(hardware):
    """States with no audio prompt (empty path) should stop whatever was playing."""
    import HMI
    HMI.init_HMI(hardware.pi)

    HMI.set_audio(HMI.AudioPrompt.UNFOLD)
    assert hardware.pygame.channel.sound is not None

    assert HMI.set_audio(HMI.AudioPrompt.PAUSE) == ErrorCode.NORMAL_OPERATION
    assert hardware.pygame.channel.sound is None


def test_set_image_audio_short_circuits_on_image_failure(hardware):
    """set_image_audio() should not attempt to play audio if the image failed to load."""
    import HMI
    HMI.init_HMI(hardware.pi)
    hardware.pygame.image_load_error = RuntimeError("missing asset")

    error = HMI.set_image_audio(HMI.Image.UNFOLD, HMI.AudioPrompt.UNFOLD)

    assert error == ErrorCode.ERROR_UNKNOWN_IMAGE
    assert hardware.pygame.channel.sound is None


# -------------------- audio_finished --------------------

def test_audio_finished_reports_elapsed_time_against_prompt_length(hardware, monkeypatch):
    """audio_finished() should flip true once one full loop of the prompt has elapsed.

    HMI.py times this with time.monotonic() (not pygame.time.get_ticks()), so
    the fake clock here patches HMI's own time.monotonic directly.
    """
    import HMI
    HMI.init_HMI(hardware.pi)

    fake_now = {"t": 0.0}
    monkeypatch.setattr(HMI.time, "monotonic", lambda: fake_now["t"])

    hardware.pygame.sound_length_sec = 2.0
    HMI.set_audio(HMI.AudioPrompt.UNFOLD)

    assert HMI.audio_finished() is False

    fake_now["t"] = 1.5  # < 2.0s prompt length
    assert HMI.audio_finished() is False

    fake_now["t"] = 2.0  # exactly one full loop elapsed
    assert HMI.audio_finished() is True


def test_audio_finished_true_when_no_prompt_is_set(hardware):
    """States with no audio prompt should report finished immediately."""
    import HMI
    HMI.init_HMI(hardware.pi)

    HMI.set_audio(HMI.AudioPrompt.PAUSE)  # empty-path prompt

    assert HMI.audio_finished() is True


def test_audio_finished_before_any_prompt_has_ever_played(hardware):
    """audio_finished() polled immediately after init_HMI(), before set_audio() has
    ever been called, should report True (no prompt loaded means nothing to wait on).

    `_audio_length` defaults to 0.0 at module load, and audio_finished()
    treats any zero-length prompt -- including "none queued yet" -- as
    finished; confirmed as the intended behavior (true-on-loop-finish is the
    more natural default) rather than a gap to fix.
    """
    import HMI
    HMI.init_HMI(hardware.pi)

    assert HMI.audio_finished() is True
