#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <MAX30105.h>
#include "heartRate.h"
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>

#define FIREBASE_HOST "https://tabletdispenser-3fb84-default-rtdb.firebaseio.com/"  // e.g., "your-project-id.firebaseio.com"
#define FIREBASE_AUTH "AIzaSyCJZQQwzHW_4iL6GRf6J-AP-gJB9J6fQ0Y"  // your Firebase database secret
#define WIFI_SSID "OnePlus 7"
#define WIFI_PASSWORD "e212878f1206"

// Initialize sensors
Adafruit_MPU6050 mpu;
MAX30105 particleSensor;

const byte RATE_SIZE = 4; 
byte rates[RATE_SIZE]; 
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

const double VCC = 3.3;
const double R2 = 10000;
const double adc_resolution = 1023;
const double A = 0.001129148;
const double B = 0.000234125;
const double C = 0.0000000876741;

// Firebase object
FirebaseData firebaseData;

void setup(void) {
  Serial.begin(115200);

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Initialize MAX30105
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    while (1);
  }
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);

  // Initialize WiFi and Firebase
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected to Wi-Fi");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // Read MPU6050 values
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Read MAX30105 values
  long irValue = particleSensor.getIR();
  if (checkForBeat(irValue)) {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;
      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++) {
        beatAvg += rates[x];
      }
      beatAvg /= RATE_SIZE;
    }
  }

  // Read temperature values
  double adc_value = analogRead(A0);
  double Vout = (adc_value * VCC) / adc_resolution;
  double Rth = (VCC * R2 / Vout) - R2;
  double temperature = (1 / (A + (B * log(Rth)) + (C * pow((log(Rth)), 3)))) - 273.15;

  // Print values to Serial Monitor
  Serial.print("Accel: ");
  Serial.print(a.acceleration.x); Serial.print(", ");
  Serial.print(a.acceleration.y); Serial.print(", ");
  Serial.print(a.acceleration.z); Serial.print(" | ");
  
  Serial.print("Gyro: ");
  Serial.print(g.gyro.x); Serial.print(", ");
  Serial.print(g.gyro.y); Serial.print(", ");
  Serial.print(g.gyro.z); Serial.print(" | ");

  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);
  
  Serial.print(" | Temp: ");
  Serial.print(temperature);
  Serial.println(" °C");

  // Send data to Firebase
  Firebase.setFloat(firebaseData, "/sensors/accelerometer/x", a.acceleration.x);
  Firebase.setFloat(firebaseData, "/sensors/accelerometer/y", a.acceleration.y);
  Firebase.setFloat(firebaseData, "/sensors/accelerometer/z", a.acceleration.z);
  
  Firebase.setFloat(firebaseData, "/sensors/gyroscope/x", g.gyro.x);
  Firebase.setFloat(firebaseData, "/sensors/gyroscope/y", g.gyro.y);
  Firebase.setFloat(firebaseData, "/sensors/gyroscope/z", g.gyro.z);

  Firebase.setInt(firebaseData, "/sensors/heartRate/IR", irValue);
  Firebase.setFloat(firebaseData, "/sensors/heartRate/BPM", beatsPerMinute);
  Firebase.setFloat(firebaseData, "/sensors/heartRate/AvgBPM", beatAvg);

  Firebase.setFloat(firebaseData, "/sensors/temperature", temperature);

  delay(1000);  // Delay for 1 second before the next loop
}
