#include <math.h>
// Pin setup
const int dirPin1 = 4;   // H Bridge Dir Pin 1
const int dirPin2 = 5;  // H Bridge Dir Pin 2
const int pwmPin = 6;   // PWM pin (must support analogWrite)

// Control parameters
const int maxSpeed = 100;   // Max speed (0–255)
const int waveDelay = 60;   // Delay between steps (ms)
const float wavePeriod = 500.0; // Full sine wave period in ms (adjust for slower/faster oscillation)
bool direction = true;
unsigned long lastActTime = millis();
unsigned long currentTime = millis();

void setup() {
  pinMode(dirPin1, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("Starting sine wave motor control...");
}
void loop() {
  // Get current time in milliseconds
  currentTime = millis();
  if(currentTime - lastActTime >= waveDelay) {
    lastActTime = currentTime;

    // Compute sine wave position (radians)
    float angle = (2 * PI * currentTime) / wavePeriod;

    // Sine output oscillates between -1 and 1
    float sineVal = sin(angle);

    // Map sine value to speed and direction separately
    int speedValue = abs(int(sineVal * maxSpeed));
    // Save prev direction for debug
    bool prevDir = direction;
    // true = forward, false = reverse
    direction = (sineVal >= 0);       

    // Apply to motor
    // Direction
    if(sineVal >= 0 ) {
      digitalWrite(dirPin1, HIGH);
      digitalWrite(dirPin2, LOW);
    } else {
      digitalWrite(dirPin1, LOW);
      digitalWrite(dirPin2, HIGH);
    }
    // Speed
    analogWrite(pwmPin, speedValue);

    // Print for debugging (optional)
    // if(prevDir != direction) {
    //   Serial.print("Changing Directions :");
    //   Serial.print(direction ? "FWD " : "REV ");
    //   Serial.print("Speed: ");
    //   Serial.print(speedValue);
    //   Serial.print(" ");
    //   Serial.println(sineVal);
    // }
    // Serial.print("Dir: ");
    // Serial.print(direction ? "FWD " : "REV ");
    // Serial.print("Speed: ");
    // Serial.println(speedValue);
    // delay(waveDelay);
  }
}