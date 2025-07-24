#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <DHT.h>
#include <math.h>  // per sqrt

#define DHTPIN    2
#define DHTTYPE   DHT11
DHT dht(DHTPIN, DHTTYPE);

Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(9600);
  while (!Serial) delay(10);
  dht.begin();
  if (!mpu.begin()) {
    Serial.println("Errore MPU6050");
    while (1) delay(10);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void loop() {
  // Leggi umidit� e temperatura
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Leggi i sensori MPU6050
  sensors_event_t accel, gyro, temp_mpu;
  bool mpu_ok = mpu.getEvent(&accel, &gyro, &temp_mpu);

  // Variabili accelerazione
  float ax = NAN, ay = NAN, az = NAN, a = NAN;
  if (mpu_ok) {
    ax = accel.acceleration.x;
    ay = accel.acceleration.y;
    az = accel.acceleration.z;
    a  = sqrt(ax*ax + ay*ay + az*az);
  }

  // Costruisci il JSON con controllo null
  String payload = "{";
  payload += "\"gearbox\":\"gb1\",";
  
  payload += "\"humidity\":";
  if (isnan(h)) payload += "null"; else payload += String(h, 2);
  payload += ",";
  
  payload += "\"temp_dht\":";
  if (isnan(t)) payload += "null"; else payload += String(t, 2);
  payload += ",";
  
  payload += "\"accel_x\":";
  if (isnan(ax)) payload += "null"; else payload += String(ax, 2);
  payload += ",";
  
  payload += "\"accel_y\":";
  if (isnan(ay)) payload += "null"; else payload += String(ay, 2);
  payload += ",";
  
  payload += "\"accel_z\":";
  if (isnan(az)) payload += "null"; else payload += String(az, 2);
  payload += ",";
  
  payload += "\"accel\":";
  if (isnan(a)) payload += "null"; else payload += String(a, 2);
  
  payload += "}";

  Serial.println(payload);
  delay(10000);  // ogni 10 secondi
}
