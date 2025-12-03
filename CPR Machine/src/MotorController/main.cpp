#include <Arduino.h>
#include "control_scheme.h"
#include "compression_control.h"
#include "zeroing_control.h"
#include "sensors.h"
#include "HMI_utils.h"
#include "interrupt_control.h"
#include <Moteus.h>
#include <ACAN2517FD.h>

long prepTimer = millis();

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
      showScreen(startUpBmpFile);
      playAudio(startUpWavFile);

      if(nextButtonLoop()) {
        // User has pressed start, get started
        prevState = currentState;
        currentState = BATTERY_CHECK;
      }

      // Now that one loop has passed, update prevState
      if(currentState == START_UP) {
        prevState = currentState;
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
      
      // Now that one loop has passed, update prevState
      if(currentState == BATTERY_CHECK) {
        prevState = currentState;
      }

      break;

    case CUT_CLOTHING:  
      showScreen(cutClothingBmpFile);
      playAudio(cutClothingWavFile);

      if(nextButtonLoop()) {
        // User has pressed start, get started
        prevState = currentState;
        currentState = UNFOLD;
      }

      // Now that one loop has passed, update prevState
      if(currentState == CUT_CLOTHING) {
        prevState = currentState;
      }

      break;
    
    case UNFOLD:  
      showScreen(unfoldBmpFile);
      playAudio(unfoldWavFile);

      if(nextButtonLoop()) {
        // User has pressed start, get started
        prevState = currentState;
        currentState = ALIGNMENT;
      }

      // Now that one loop has passed, update prevState
      if(currentState == UNFOLD) {
        prevState = currentState;
      }

      break;

    case ALIGNMENT:
      showScreen(alignmentBmpFile);
      playAudio(alignmentWavFile);

      if(nextButtonLoop()) {
        // User has confirmed alignment, move to zeroing
        prevState = currentState;
        currentState = ZEROING_PREP;
      }

      // Now that one loop has passed, update prevState
      if(currentState == ALIGNMENT) {
        prevState = currentState;
      }

      break;
    
    case ZEROING_PREP:
      // Prepare for zeroing, then move to zeroing state
      showScreen(zeroingPrepBmpFile);
      playAudio(zeroingPrepWavFile);

      if(nextButtonLoop()) {
        // User has confirmed alignment, move to zeroing
        prevState = currentState;
        currentState = ZEROING;
      }

      // Now that one loop has passed, update prevState
      if(currentState == ZEROING_PREP) {
        prevState = currentState;
      }

      break;
    
    case ZEROING:
      showScreen(zeroingBmpFile);
      //playAudio(zeroingWavFile);
      // Initialize zeroing state, with error handling
      // Only do this when we switch states
      if(prevState != currentState) {
        initializeZeroing();
      }

      if(updateZeroing()) {
        // updateZeroing returned true, indicating zeroing is complete
        // Error cases handled in zeroing control
        DPRINTLN("ZEROING COMPLETE");
        prevState = currentState; //I changed this from COMPRESSIONS
        currentState = COMPRESSION_PREP;
      }
      
      // Now that one loop has passed, update prevState
      if(currentState == ZEROING) {
        prevState = currentState;
      }

      break;

    case COMPRESSION_PREP:
      showScreen(compressionPrepBmpFile);
      playAudio(compressionPrepWavFile);
      
      if(prevState != currentState) {
        prepTimer = millis();
      }

      if(millis() - prepTimer >= 10000) {
        prevState = currentState;
        currentState = COMPRESSIONS;
      }
      // if (playWav1.isPlaying()) {
      //   audioWasPlaying = true;
      // }

      // // Trigger when playback *finishes*
      // if (audioWasPlaying && !playWav1.isPlaying()) {
      //     audioWasPlaying = false;  // reset
      //     prevState = currentState;
      //     currentState = COMPRESSIONS;
      // } 


      
      // Now that one loop has passed, update prevState
      if(currentState == COMPRESSION_PREP) {
        prevState = currentState;
      }

      break; 

    case COMPRESSIONS:
      showScreen(compressionsBmpFile);
      playAudio(compressionsWavFile);

      if(prevState != currentState) {
        initializeCompressions();
      }

      // Error handling handled within updateCompressions()
      updateCompressions();

      // Check for pause command
      if(pauseButtonLoop()) {
        prevState = currentState;
        currentState = PAUSED;
      }

      // Now that one loop has passed, update prevState
      if(currentState == COMPRESSIONS) {
        prevState = currentState;
      }

      break;

    case PAUSED:
      showScreen(pausedBmpFile);
      playAudio(pausedWavFile);


      // If not paused, resume compressions
      if(nextButtonLoop()) {
        prevState = currentState;
        currentState = COMPRESSIONS;
      }

      // Now that one loop has passed, update prevState
      if(currentState == PAUSED) {
        prevState = currentState;
      }

      break;

    case KNEEL_FAILURE:
      // Command zero setpoint, don't necessarily need to get there before re-kneel
      returnToCompressionZero(); 
      showScreen(kneelFailureBmpFile);
      playAudio(kneelFailureWavFile);

      // If they re-kneel, get back to rib breaking
      if(!isKneelingFailure()) {
        prevState = currentState;
        currentState = COMPRESSIONS;
      }

      // Now that one loop has passed, update prevState
      if(currentState == KNEEL_FAILURE) {
        prevState = currentState;
      }

      break;

    case ABORT:
      // Shit's fucked, get plunger out of there and tell people to do manual compressions
      showScreen(abortBmpFile);
      playAudio(abortWavFile);
      updateAbort();

      if(prevState != currentState) {
        DPRINTLN("ABORT");

        // Update prevState now, no backing out of this one
        prevState = currentState;
      }
      
      // No getting out of this one, don't change state
      // TODO: Handle reset after use
      break;
  }

  DPRINTLN(millis() - timer);
  Serial.println(millis() - timer);
}
