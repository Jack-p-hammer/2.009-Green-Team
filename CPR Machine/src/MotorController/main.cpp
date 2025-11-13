#include <Arduino.h>
#include "control_scheme.h"
#include "compression_control.h"
#include "zeroing_control.h"
#include "sensors.h"
#include <Moteus.h>
#include <ACAN2517FD.h>

// General FIXME: This will not build until all state functions declared
// in headers are given definitions

void setup() {
  // Do everything that needs to occur on power up
  initializeMotor();
  initializeSensors();
  // TODO: Does anything else need to happen here?
}

void loop() {
  // Main State Machine
  // State changes into failure states handled by error detection code
  // State changes into success states handled in main switch case
  // FIXME: if this turns out to be a bad idea
  switch (currentState) {
    // TODO: Implement state machine cases
    case START_UP:  
      if(!verifyBatteryPercentage()) {
        // Handle low battery scenario
        prevState = currentState;
        currentState = ABORT;
      }

      displaySetupInstructions();

      if(checkUserStartConfirmation()) {
        // User has pressed start, get started
        prevState = currentState;
        currentState = ZEROING;
      }

      break;
    case ZEROING:
      // Initialize zeroing state, with error handling
      // Only do this when we switch states
      if(prevState != currentState) {
        initializeZeroing();
      }

      if(updateZeroing()) {
        // updateZeroing returned true, indicating zeroing is complete
        // Error cases handled in zeroing control
        prevState = currentState;
        currentState = WAIT_FOR_COMPRESSION_CONFIRMATION;
      }

      break;
    case ZERO_FAILED:
        // TODO: Handle different behavior for repeated zeroing failures
        // Hence, nothing for now
    case WAIT_FOR_COMPRESSION_CONFIRMATION:
      displayCompressionConfirmation();

      // If user confirms compression start, start compressions (shocker)
      if(checkUserCompressionConfirmation()) {
        prevState = currentState;
        currentState = COMPRESSIONS;
      }

      break;
    case COMPRESSIONS:
      if(prevState != currentState) {
        initializeCompressions();
      }

      // Error handling handled within updateCompressions()
      updateCompressions();

      // Check for pause command
      if(checkPauseCommand()) {
        prevState = currentState;
        currentState = PAUSED;
      }

      break;
    case PAUSED:
      displayPauseMessage();

      // If not paused, resume compressions
      if(!isPaused()) {
        prevState = currentState;
        currentState = COMPRESSIONS;
      }

      break;
    case KNEEL_FAILURE:
      // Command zero setpoint, don't necessarily need to get there before re-kneel
      returnToZero(); 
      displayKneelFailureMessage();

      // If they re-kneel, get back to rib breaking
      if(!isKneelingFailure()) {
        prevState = currentState;
        currentState = COMPRESSIONS;
      }

      break;
    case ABORT:
      // Shit's fucked, get plunger out of there and tell people to do manual compressions
      displayAbortMessage();
      updateAbort();
      
      // No getting out of this one, don't change state
      // TODO: Handle reset after use
      break;
  }
}
