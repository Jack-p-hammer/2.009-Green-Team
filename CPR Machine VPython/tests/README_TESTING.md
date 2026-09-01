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
- Checks that HMI startup sets up the display and GPIO pins correctly.
- Checks that HMI initialization fails gracefully when pygame startup, display
  setup, or GPIO setup fails.
- Checks that button/laser helpers write the expected LED and PWM states, and
  report a daemon failure if the underlying pigpio call fails.
- Checks that button readers respond to fake GPIO input and report a daemon
  failure if the underlying pigpio call fails.
- Checks that pump_events() succeeds normally and warns (without raising) if
  the pygame event queue fails.
- Checks that set_image/set_audio/set_image_audio play the right prompt, stop
  playback for audio-less states, and report the right error code (unknown
  image vs. pygame failure) when the underlying pygame call fails.
- Checks that the audio-finished helper reports elapsed time against the
  current prompt's length, not the pygame mixer's busy state.

### Sensing tests
- Checks that sensor initialization fails gracefully when pigpio, the I2C bus,
  the ToF sensor, or the IMU is unavailable.
- Checks that sensor initialization captures the absolute zero positions from
  the rotary encoder, ToF sensor, and force sensor, both with a lightweight
  fake controller and through the real actuation.init_motor() -> MoteusThread
  path.
- Checks that the individual sensor read helpers (force, ToF, IMU) report the
  configured hardware values.
- Checks that read_sensors() validates readings against mode-appropriate
  limits: normal operation, zeroing finished, force/accel out of range, and
  rotary/ToF position disagreement.
- Checks that zero_position() captures the current position once zeroing
  finishes, and fails if zeroing hasn't finished yet.
- Checks that battery_check() reports normal operation, low battery, or a
  motor failure if the voltage read itself fails.

### Actuation tests
- Checks that init_motor() creates the shared motor controller on success and
  reports an init failure if the underlying moteus controller can't be
  constructed.
- Checks that init_zeroing()/init_compressions() record their start times.
- Checks that zeroing() fails on timeout, motor error, or exceeding the
  extension limit, and succeeds when sensors are healthy.
- Checks that computeCompressionSetpoint() follows the expected trapezoidal
  profile across a full compression cycle.
- Checks that compressions()/pause_compressions() report normal operation, a
  sensor failure, or a motor failure depending on sensor/motor state.
- Checks that abort_compressions() succeeds without requiring sensing to have
  been initialized.

### Main-state tests
- Checks that ERROR_STATE_MAP routes each recoverable error to the correct
  state, and that FATAL_ERRORS never overlaps it.
- Checks that a fatal error during startup (HMI init failure, unknown image,
  pigpio daemon failure) aborts the motor and exits the process.
- Checks that a non-fatal error (e.g. motor init failure) routes to the ABORT
  state and runs its setup instead of exiting.
- Checks that each state's setup runs exactly once and the machine advances
  through startup when the Next button is held down.