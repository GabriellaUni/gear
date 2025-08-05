#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <DHT.h>
#include <math.h>

// ——— DHT11 ———
#define DHTPIN   2
#define DHTTYPE  DHT11
DHT dht(DHTPIN, DHTTYPE);

#define DHTPIN1  4
#define DHTTYPE1 DHT11
DHT dht1(DHTPIN1, DHTTYPE1);

// ——— MPU6050 ———
Adafruit_MPU6050 mpu0;
Adafruit_MPU6050 mpu1;

void setup() {
  Serial.begin(9600);
  while (!Serial) delay(10);

  dht.begin();
  dht1.begin();

  Wire.begin();
  if (!mpu0.begin(0x68, &Wire)) {
    Serial.println("Error MPU0 (0x68)");
    while (1) delay(10);
  }
  if (!mpu1.begin(0x69, &Wire)) {
    Serial.println("Error MPU1 (0x69)");
    while (1) delay(10);
  }

  mpu0.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu0.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu0.setFilterBandwidth(MPU6050_BAND_21_HZ);
  mpu1.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu1.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu1.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void loop() {
  // — lettura DHT11 —
  float h   = dht.readHumidity();
  float t   = dht.readTemperature();
  float h1  = dht1.readHumidity();
  float t1  = dht1.readTemperature();

  // — lettura MPU0 —
  sensors_event_t accel, gyro, temp_mpu;
  bool ok0 = mpu0.getEvent(&accel, &gyro, &temp_mpu);
  float ax= NAN, ay= NAN, az= NAN, a= NAN;
  float gx= NAN, gy= NAN, gz= NAN, g= NAN, tmp0= NAN;
  if (ok0) {
    ax   = accel.acceleration.x;
    ay   = accel.acceleration.y;
    az   = accel.acceleration.z;
    a    = sqrt(ax*ax + ay*ay + az*az);

    gx   = gyro.gyro.x;
    gy   = gyro.gyro.y;
    gz   = gyro.gyro.z;
    g    = sqrt(gx*gx + gy*gy + gz*gz);

    tmp0 = temp_mpu.temperature;
  }

  // — lettura MPU1 —
  sensors_event_t accel1, gyro1, temp_mpu1;
  bool ok1 = mpu1.getEvent(&accel1, &gyro1, &temp_mpu1);
  float ax1= NAN, ay1= NAN, az1= NAN, a1= NAN;
  float gx1= NAN, gy1= NAN, gz1= NAN, g1= NAN, tmp1= NAN;
  if (ok1) {
    ax1  = accel1.acceleration.x;
    ay1  = accel1.acceleration.y;
    az1  = accel1.acceleration.z;
    a1   = sqrt(ax1*ax1 + ay1*ay1 + az1*az1);

    gx1  = gyro1.gyro.x;
    gy1  = gyro1.gyro.y;
    gz1  = gyro1.gyro.z;
    g1   = sqrt(gx1*gx1 + gy1*gy1 + gz1*gz1);

    tmp1 = temp_mpu1.temperature;
  }

  // — build JSON payload —
  String payload = "{";
  payload += "\"gearbox\":\"gb1\",";
  payload += "\"humidity\":"   + String(isnan(h)   ? NAN : h,  2) + ",";
  payload += "\"temp_dht\":"   + String(isnan(t)   ? NAN : t,  2) + ",";
  payload += "\"humidity1\":"  + String(isnan(h1)  ? NAN : h1, 2) + ",";
  payload += "\"temp_dht1\":"  + String(isnan(t1)  ? NAN : t1, 2) + ",";

  // MPU0 accel
  payload += "\"accel_x\":"    + String(isnan(ax)  ? NAN : ax,  2) + ",";
  payload += "\"accel_y\":"    + String(isnan(ay)  ? NAN : ay,  2) + ",";
  payload += "\"accel_z\":"    + String(isnan(az)  ? NAN : az,  2) + ",";
  payload += "\"accel\":"      + String(isnan(a)   ? NAN : a,   2) + ",";

  // MPU0 gyro + modulo
  payload += "\"gyro_x\":"     + String(isnan(gx)  ? NAN : gx,  2) + ",";
  payload += "\"gyro_y\":"     + String(isnan(gy)  ? NAN : gy,  2) + ",";
  payload += "\"gyro_z\":"     + String(isnan(gz)  ? NAN : gz,  2) + ",";
  payload += "\"gyro\":"       + String(isnan(g)   ? NAN : g,   2) + ",";
  payload += "\"temp_mpu\":"   + String(isnan(tmp0)? NAN : tmp0,2) + ",";

  // MPU1 accel
  payload += "\"accel1_x\":"   + String(isnan(ax1) ? NAN : ax1, 2) + ",";
  payload += "\"accel1_y\":"   + String(isnan(ay1) ? NAN : ay1, 2) + ",";
  payload += "\"accel1_z\":"   + String(isnan(az1) ? NAN : az1, 2) + ",";
  payload += "\"accel1\":"     + String(isnan(a1)  ? NAN : a1,  2) + ",";

  // MPU1 gyro + modulo
  payload += "\"gyro1_x\":"    + String(isnan(gx1) ? NAN : gx1, 2) + ",";
  payload += "\"gyro1_y\":"    + String(isnan(gy1) ? NAN : gy1, 2) + ",";
  payload += "\"gyro1_z\":"    + String(isnan(gz1) ? NAN : gz1, 2) + ",";
  payload += "\"gyro1\":"      + String(isnan(g1)  ? NAN : g1,  2) + ",";
  payload += "\"temp_mpu1\":"  + String(isnan(tmp1)? NAN : tmp1,2);

  payload += "}";

  Serial.println(payload);
  delay(5000);
}

