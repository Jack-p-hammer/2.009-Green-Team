#include <Arduino.h>
#include "control_scheme.h"
#include "compression_control.h"
#include "zeroing_control.h"
#include "sensors.h"
#include "HMI_utils.h"
#include "interrupt_control.h"
#include <Moteus.h>
#include <ACAN2517FD.h>


void setup() {
  // Do everything that needs to occur on power up
  initializeMotor();
  initializeSensors();
  HMI_util_setup();
  // TODO: Does anything else need to happen here?
}

void loop() {
  // Main State Machine
  // State changes into failure states handled by error detection code
  // State changes into success states handled in main switch case
  // FIXME: if this turns out to be a bad idea
  // if(currentState != ABORT) {
  //   DPRINT(currentState); DPRINT(" | "); DPRINTLN(linearPos);
  // }
  long timer = millis();
    // Get current time once for all buttons
  GreenNow = millis();
  PauseNow = millis();
  
  DPRINT(">");
  DPRINT("State:"); DPRINT(currentState);
  DPRINT(", Zero:"); DPRINT(linearZeroPos*39.37);
  DPRINT(", Setpoint:"); DPRINT((computeCompressionSetpoint()+ linearZeroPos)*39.37);
  DPRINT(", LinearPos:"); DPRINT(linearPos*39.37);


  switch (currentState) {
    case START_UP:  
      showScreen(startBmpFile);
      playAudio(startWavFile);

      if(checkUserConfirmation()) {
        // User has pressed start, get started
        prevState = currentState;
        currentState = BATTERY_CHECK;
      }

      break;

    case BATTERY_CHECK:
      // Immediately check battery state
      if(!verifyBatteryPercentage()) {
        // Handle low battery scenario
        prevState = currentState;
        currentState = ABORT;
      } else {
        // Battery good, move to startup
        prevState = currentState;
        currentState = CUT_CLOTHING;
      }
      break;

    case CUT_CLOTHING:  
      displayCutClothingInstructions();

      if(checkUserConfirmation()) {
        // User has pressed start, get started
        prevState = currentState;
        currentState = UNFOLD;
      }

      break;
    
      
    case UNFOLD:  
      displayUnfoldInstructions();

      if(checkUserConfirmation()) {
        // User has pressed start, get started
        prevState = currentState;
        currentState = ALIGNMENT;
      }

      break;


    case ALIGNMENT:
      displayAlignmentInstructions();

      if(checkUserConfirmation()) {
        // User has confirmed alignment, move to zeroing
        prevState = currentState;
        currentState = ZEROING_PREP;
      }
      break;
    
    case ZEROING_PREP:
      // Prepare for zeroing, then move to zeroing state
      displayZeroingInstructions();

      if(checkUserConfirmation()) {
        // User has confirmed alignment, move to zeroing
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
        DPRINTLN("ZEROING COMPLETE");
        prevState = COMPRESSIONS;
        currentState = COMPRESSIONS;
      }
      
      if(currentState == ZEROING) {
        prevState = currentState;
      }
      break;
    case WAIT_FOR_COMPRESSION_CONFIRMATION:
      if(displayCompressionConfirmation()) {
        // User has confirmed, move to compressions
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

      if(currentState == COMPRESSIONS) {
        prevState = currentState;
      }
      break;
    case PAUSED:
      displayPauseMessage();

      // If not paused, resume compressions
      if(!checkPauseCommand()) {
        prevState = currentState;
        currentState = COMPRESSIONS;
      }

      break;
    case KNEEL_FAILURE:
      // Command zero setpoint, don't necessarily need to get there before re-kneel
      returnToCompressionZero(); 
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

      if(prevState != currentState) {
        DPRINTLN("ABORT");
        prevState = currentState;
      }
      
      // No getting out of this one, don't change state
      // TODO: Handle reset after use
      break;
  }

  DPRINTLN(millis() - timer);
  Serial.println(millis() - timer);
}
