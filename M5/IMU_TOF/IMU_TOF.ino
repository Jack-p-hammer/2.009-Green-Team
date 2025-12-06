#include <Wire.h>
#include <Adafruit_BNO08x.h>
#include <Adafruit_VL53L0X.h>

// --- Global sensor objects ---
Adafruit_BNO08x bno = Adafruit_BNO08x();
Adafruit_VL53L0X tof = Adafruit_VL53L0X();

// Linear acceleration globals
float ax = 0;
float ay = 0;
float az = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  Serial.println("Starting BNO080 + VL53L0X...");

  Wire.begin();  // Start I2C

  // --- Initialize ToF ---
  if (!tof.begin()) {
    Serial.println("Failed to find VL53L0X!");
    while (1);
  }
  Serial.println("VL53L0X initialized!");

  // --- Initialize BNO080 IMU ---
  if (!bno.begin_I2C()) {
    Serial.println("Failed to find BNO080!");
    while (1);
  }
  Serial.println("BNO080 initialized!");

  // Enable linear acceleration at 100 Hz
  if (!bno.enableReport(SH2_LINEAR_ACCELERATION, 10000)) {
    Serial.println("Could not enable linear acceleration!");
    while (1);
  }

  delay(100);
}

void loop() {

  // ------ Read IMU ------
  sh2_SensorValue_t imu;
  if (bno.getSensorEvent(&imu)) {
    if (imu.sensorId == SH2_LINEAR_ACCELERATION) {
      ax = imu.un.linearAcceleration.x;
      ay = imu.un.linearAcceleration.y;
      az = imu.un.linearAcceleration.z;
    }
  }


  // ------ Read ToF ------
  VL53L0X_RangingMeasurementData_t measure;
  tof.rangingTest(&measure, false);  // 'false' = no debug print

  int distance_mm = -1;
  if (measure.RangeStatus == 0) {
    distance_mm = measure.RangeMilliMeter;
  }

  // ------ Print results ------
  Serial.print("AX:");
  Serial.print(",");
  Serial.print(ax, 3);
  Serial.print("AY:");
  Serial.print(",");
  Serial.print(ay, 3);
  Serial.print("AZ:");
  Serial.print(",");
  Serial.print(az, 3);
  Serial.print("TOF(mm):");
    Serial.print(",");
  Serial.println(distance_mm);

  delay(5);
}
