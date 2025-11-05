const int sensorPin = A0;
const int V_inPin = A1;
const int samples = 1;  // number of readings to average

const int a = 1187.7440;
const int b = -548.7972;

void setup() {
  Serial.begin(115200);
}

void loop() {
  float ForceSum = 0;
  float VinSum = 0;

  // take 5 readings and average them
  for (int i = 0; i < samples; i++) {
    ForceSum += analogRead(sensorPin);
    VinSum += analogRead(V_inPin);

  }

  float ForceAnalog = ForceSum / samples;
  float VoltageInAnalog = VinSum / samples;

  float ForceVoltage = ForceAnalog * (5.0 / 1023.0);
  float VoltageIn = VoltageInAnalog * (5.0 / 1023.0);
  float ratio = ForceVoltage / VoltageIn;

  float force = a*ForceVoltage + b;

  Serial.print("Force(N):");
  Serial.println(force, 3);
  // Serial.print(" , ");
  // Serial.print(ForceVoltage, 3);
  // Serial.print(" , ");
  // Serial.print(VoltageIn, 3);
  // Serial.print(" , ");
  // Serial.println(ratio, 3);




  delay(50);  // overall update rate
}
