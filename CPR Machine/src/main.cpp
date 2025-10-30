#include <Arduino.h>
#include <math.h>

// put function declarations here:
int myFunction(int, int);

// Pin setup
const int dirPin1 = 4;   // Direction control
const int dirPin2 = 5;
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
    // Map sine value to speed and direction
    int speedValue = abs(int(sineVal * maxSpeed));   // 0–maxSpeed
    bool prevDir = direction;
    direction = (sineVal >= 0);            // true = forward, false = reverse
    // Apply to motor
    if(sineVal >= 0 ) {
      digitalWrite(dirPin1, HIGH);
      digitalWrite(dirPin2, LOW);
    } else {
      digitalWrite(dirPin1, LOW);
      digitalWrite(dirPin2, HIGH);
    }

    // digitalWrite(dirPin, HIGH);
    analogWrite(pwmPin, speedValue);
    
    // Print for debugging (optional)
    if(prevDir != direction) {
      Serial.print("Changing Directions :");
      Serial.print(direction ? "FWD " : "REV ");
      Serial.print("Speed: ");
      Serial.print(speedValue);
      Serial.print(" ");
      Serial.println(sineVal);
    }
  }
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}