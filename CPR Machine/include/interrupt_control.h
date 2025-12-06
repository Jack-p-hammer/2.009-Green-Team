#pragma once

// Zeroing failure
void displayAlignmentConfirmation();
bool checkUserAlignmentConfirmation();
// TODO: Handle repeated entries to state having different behavior?

// Paused
void displayPauseMessage();
bool checkPauseCommand();

// Kneeling failure
void displayKneelFailureMessage();
bool isKneelingFailure(); // Returns true if the system is currently in kneeling failure state

// Abort
void displayAbortMessage();
void updateAbort(); // Return motor to zero position ASAP