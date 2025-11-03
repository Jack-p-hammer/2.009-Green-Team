const int sensorPin = A0;
const int V_inPin = A1;
const float V_out = 2.5;


void setup() {
  Serial.begin(115200);
}

void loop() {
  float ForceAnalog = analogRead(sensorPin);
  float ForceVoltage = ForceAnalog * (5.0 / 1023.0);
  float VoltageIn = analogRead(V_inPin);
  VoltageIn = VoltageIn * (5.0 / 1023.0);
  float ratio = ForceVoltage / VoltageIn;
  Serial.print(ForceVoltage, 4);
  Serial.print(" , ");
  Serial.println(ratio, 4);
  delay(50);
}
