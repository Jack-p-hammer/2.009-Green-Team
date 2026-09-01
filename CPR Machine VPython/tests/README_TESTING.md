# CPR Machine Code Tests
This folder contains simple tests for the parts of the project that can be checked without the real hardware attached.

## Why these tests exist
The machine depends on sensors, buttons, LEDs, and a motor controller. Those pieces are hard to test on a laptop, so these tests use small fake versions of the hardware interfaces instead.

## How to run the tests
From the project root, run:

```bash
python -m pytest -q
```

If you only want to run the tests in this folder, that same command will do it.

## Notes for future contributors
- These tests use fake hardware.
- If new hardware logic is added, add or update tests in this folder.


## Current test inventory
### HMI tests
- Startup GPIO setup: given healthy fake GPIO, init_HMI() sets the Next/Pause
  button pins to input+pullup, the Next/Pause LED and laser pins to
  output+pulldown, and leaves the laser PWM off.
- Startup failure paths: given an injected pygame/mixer failure, init_HMI()
  returns the dedicated fatal ERROR_PYGAME_INIT_FAILURE; given a display or
  GPIO failure, it returns the recoverable ERROR_INIT_FAILURE (never raises
  either way).
- Button/LED writes: given a healthy pi, enable/disable_next/pause_button()
  write HIGH/LOW to the right pin; calling either twice in a row is
  idempotent, and a pigpio write exception returns ERROR_PI_DAEMON_FAILURE.
- Button reads: given HIGH/LOW on the button pins (including both pressed at
  once), next/pause_button_pressed() report the matching bool; a pigpio read
  exception returns ERROR_PI_DAEMON_FAILURE.
- Laser control: given a healthy pi, enable_lasers()/disable_lasers() write a
  nonzero/zero PWM duty cycle to the laser pin, with the raw value equal to
  LASER_PWM_DUTY_CYCLE * LASER_PWM_SCALE (the hardware analog of a
  duty-cycle check, since there's no parameterized function to pass
  arbitrary duty cycles into); a pigpio PWM exception returns
  ERROR_PI_DAEMON_FAILURE.
- pump_events(): given a healthy or broken pygame event queue, returns
  NORMAL_OPERATION or WARNING_PYGAME_PUMP_FAILURE (never raises).
- set_image/set_audio/set_image_audio: given a valid prompt they play it
  (looped); given an injected pygame load failure they return
  ERROR_UNKNOWN_IMAGE or ERROR_PYGAME_FAILURE, and set_image_audio() skips
  audio entirely if the image failed; an audio-less state stops playback.
- audio_finished(): given a prompt of known length, False before that much
  time has elapsed and True at/after it; also True for an audio-less state
  and before any prompt has ever been played (confirmed intended:
  true-on-loop-finish is the natural default when nothing is queued).
- Not yet coverable: the team's inventory also lists an ERROR_UNKNOWN_AUDIO
  code for a missing audio file -- no such code exists in error_codes.py,
  and audio_finished() (a plain bool, no file access) isn't the function
  that would ever load one, so there's nothing to test against (see
  summary).

### Sensing tests
- init_sensors() failure paths: given an unavailable pigpio daemon, I2C bus,
  ToF sensor, or IMU, returns ERROR_INIT_FAILURE (never raises).
- init_sensors() success: given healthy fake hardware (one test goes through
  the real actuation.init_motor() -> MoteusThread path), returns
  NORMAL_OPERATION and snapshots the rotary/ToF/force readings as absolute
  zero.
- Individual sensor reads: read_force_sensor()/read_ToF_sensor()/read_IMU()
  return whatever raw ADC bytes/ToF range/IMU acceleration the fakes were
  given.
- read_sensors() validation: given readings within limits, NORMAL_OPERATION;
  given force past the zeroing threshold, ZEROING_FINISHED; given IMU
  orientation beyond tolerance, rotary/ToF disagreement, or an invalid
  control mode, ERROR_SENSOR_FAILURE.
- read_sensors() mid-operation failures: given the I2C bus, ToF sensor, or
  IMU raising after a previously successful init, ERROR_SENSOR_FAILURE
  (never raises out of read_sensors()).
- zero_position(): given zeroing finished, captures the zeroed position and
  returns NORMAL_OPERATION; given zeroing still in progress,
  ERROR_SENSOR_FAILURE.
- battery_check(): given a healthy, low, or unreadable voltage, returns
  NORMAL_OPERATION, ERROR_LOW_BATTERY, or ERROR_MOTOR_FAILURE respectively.

### Actuation tests
- init_motor()/get_motor_controller(): given a healthy or failing moteus
  controller construction, returns NORMAL_OPERATION (and the same controller
  instance thereafter) or ERROR_INIT_FAILURE.
- init_zeroing()/init_compressions(): record their start times.
- zeroing(): given a timeout, a motor error, or exceeding the extension
  limit, returns the matching error; given healthy sensors,
  NORMAL_OPERATION.
- computeCompressionSetpoint(): follows the expected trapezoidal profile
  (flat, ramp, plateau, ramp, repeat) across a full 0.56s compression cycle.
- compressions()/pause_compressions(): given healthy state, a sensor
  failure, or a motor failure, return NORMAL_OPERATION,
  ERROR_SENSOR_FAILURE, or ERROR_MOTOR_FAILURE respectively; given an IMU
  kneel failure, pass it through unchanged instead of converting it to
  ERROR_SENSOR_FAILURE.
- abort_compressions(): succeeds without requiring sensing to have been
  initialized.

### Main-state tests
- ERROR_STATE_MAP: every recoverable error routes to ABORT except the IMU
  kneel failure, which routes to KNEEL_FAILURE; FATAL_ERRORS is exactly
  {EXIT_UNKNOWN, ERROR_PYGAME_INIT_FAILURE, ERROR_PI_DAEMON_FAILURE,
  ERROR_UNKNOWN_IMAGE} and never overlaps ERROR_STATE_MAP.
- Fatal-error fast exit: an HMI init failure, an unknown image, or a pigpio
  daemon failure during startup aborts the motor and raises SystemExit(1).
- State chain: holding Next advances STARTUP through UNFOLD_CUT_CLOTHES,
  ALIGNMENT, ZEROING_PREP, and into ZEROING; zeroing finished advances
  ZEROING to COMPRESSION_PREP and holds it there otherwise; audio finished
  advances COMPRESSION_PREP to COMPRESSION and holds it there otherwise.
- Compression and pause: the pause button, not Next, moves COMPRESSION to
  PAUSE; Next returns PAUSE to COMPRESSION_PREP; Next returns KNEEL_FAILURE
  to ALIGNMENT.
- Error-driven routing: a battery failure in STARTUP, a zeroing failure or
  sensor failure in ZEROING, and a sensor failure in COMPRESSION or PAUSE
  all route to ABORT; an IMU kneel failure in ZEROING, COMPRESSION, or PAUSE
  routes to KNEEL_FAILURE.
- Kneel-failure setup: tolerates a repeated IMU kneel failure from
  pause_compressions() and still shows its screen, instead of re-routing
  into itself forever; any other error from that call still routes to
  ABORT.