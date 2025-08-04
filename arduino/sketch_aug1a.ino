#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <math.h>

Adafruit_MPU6050 mpu0, mpu1;

void setup() {
  Serial.begin(9600);
  while (!Serial) delay(10);
  Wire.begin();
  // First sensor at 0x68
  if (!mpu0.begin(0x68, &Wire)) {
    Serial.println("Error: MPU (0x68) not found!");
    while (1) delay(10);
  }
  // Second sensor at 0x69
  if (!mpu1.begin(0x69, &Wire)) {
    Serial.println("Error: MPU1 (0x69) not found!");
    while (1) delay(10);
  }
  // Same configuation for both
  mpu0.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu0.setGyroRange         (MPU6050_RANGE_500_DEG);
  mpu0.setFilterBandwidth   (MPU6050_BAND_21_HZ);
  mpu1.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu1.setGyroRange         (MPU6050_RANGE_500_DEG);
  mpu1.setFilterBandwidth   (MPU6050_BAND_21_HZ);
}

void loop() {
  // 1) Read both sensors
  sensors_event_t accel0, gyro0, temp0;
  sensors_event_t accel1, gyro1, temp1;
  mpu0.getEvent(&accel0, &gyro0, &temp0);
  mpu1.getEvent(&accel1, &gyro1, &temp1);
  // 2) First sensor
  float ax = accel0.acceleration.x;
  float ay = accel0.acceleration.y;
  float az = accel0.acceleration.z;
  float a = sqrt(ax*ax + ay*ay + az*az);
  // 3) Second sensors
  float ax1 = accel1.acceleration.x;
  float ay1 = accel1.acceleration.y;
  float az1 = accel1.acceleration.z;
  float a1 = sqrt(ax1*ax1 + ay1*ay1 + az1*az1);
  // 6) Build the JSON
  String payload = "{";
    // First sensor
    payload += "\"gearbox\":\"gb1\",";
    payload += "\"accel_x\":" + String(ax,2) + ",";
    payload += "\"accel_y\":" + String(ay,2) + ",";
    payload += "\"accel_z\":" + String(az,2) + ",";
    payload += "\"accel\":" + String(a,2) + ",";
    // Second sensor
    payload += "\"accel1_x\":" + String(ax1,2) + ",";
    payload += "\"accel1_y\":" + String(ay1,2) + ",";
    payload += "\"accel1_z\":" + String(az1,2) + ",";
    payload += "\"accel1\":" + String(a1,2);
  payload += "}";

  Serial.println(payload);
  delay(5000);
}




