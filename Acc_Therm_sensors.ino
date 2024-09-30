#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>

// Firebase configuration and authentication
FirebaseConfig config;
FirebaseAuth auth;

#define WIFI_SSID "OnePlus 7"
#define WIFI_PASSWORD "e212878f1206"

// Initialize MPU6050
Adafruit_MPU6050 mpu;

// Thermistor parameters
const double VCC = 3.3;
const double R2 = 10000;  // 10k ohm series resistor
const double adc_resolution = 1023;  // 10-bit ADC resolution

const double A = 0.001129148;  // Steinhart-Hart coefficients
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

  // Initialize WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected to Wi-Fi");

  // Firebase configuration
  config.host = "tabletdispenser-3fb84-default-rtdb.firebaseio.com";
  config.api_key = "YOUR_FIREBASE_API_KEY";  // Replace with your Firebase API key

  // Optional: if using Firebase authentication with user credentials
  // auth.user.email = "YOUR_USER_EMAIL";
  // auth.user.password = "YOUR_USER_PASSWORD";

  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // Read MPU6050 values
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Read temperature sensor values
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

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.println(" °C");

  // Send data to Firebase
  Firebase.setFloat(firebaseData, "/sensors/accelerometer/x", a.acceleration.x);
  Firebase.setFloat(firebaseData, "/sensors/accelerometer/y", a.acceleration.y);
  Firebase.setFloat(firebaseData, "/sensors/accelerometer/z", a.acceleration.z);

  Firebase.setFloat(firebaseData, "/sensors/gyroscope/x", g.gyro.x);
  Firebase.setFloat(firebaseData, "/sensors/gyroscope/y", g.gyro.y);
  Firebase.setFloat(firebaseData, "/sensors/gyroscope/z", g.gyro.z);

  Firebase.setFloat(firebaseData, "/sensors/temperature", temperature);

  delay(1000);  // Delay for 1 second before the next loop
}
