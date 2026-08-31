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
- Checks that sensing initialization fails gracefully when the pigpio connection is unavailable.
- Checks that sensing initialization succeeds with the fake hardware stack in place.
- Checks that sensing initialization fails gracefully when the I2C bus cannot be created.
- Checks that the current sensor helper functions behave safely as placeholders.

### Actuation tests
- Checks that the current motor placeholder functions report normal operation.
- Checks that those placeholder functions return enum values rather than raw values.

### Main-state tests
- Checks that the main flow advances through the early startup states when the next button is pressed.
- Checks that initialization errors are routed to the abort state.
- Checks that the fatal-error set includes the expected terminal conditions.