#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "WiFi.h"
#include <HTTPClient.h>
#include <Wire.h>
#include <math.h>

Adafruit_MPU6050 mpu;

const char* ssid = "router";
const char* password = "Vlad lives here!";
float avg = 0;
void sendEvent() {
  
  HTTPClient http;

  http.begin("http://192.168.203.239/?action=0"); //Specify the URL
  int httpCode = http.GET();                                        //Make the request

  if (httpCode > 0) { //Check for the returning code

    String payload = http.getString();
    Serial.println(httpCode);
    Serial.println(payload);
  }

  else {
    Serial.println("Error on HTTP request");
  }

  http.end(); //Free the resources
}

void setup(void) {
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("Adafruit MPU6050 test!");
  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);

  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println(WiFi.localIP());

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_94_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }

  Serial.println("");
  delay(100);
}

//void loop() {
//  /* Get new sensor events with the readings */
//  sensors_event_t a, g, temp;
//  mpu.getEvent(&a, &g, &temp);
//  float newval = 0;
//  float oldval = a.acceleration.x+a.acceleration.y+a.acceleration.z;
//  while(1) {
//    mpu.getEvent(&a, &g, &temp);
//    newval = a.acceleration.x+a.acceleration.y+a.acceleration.z;
//    Serial.println(newval);
//    if (newval - oldval > 0.1 or oldval - newval > 0.1) {
//      Serial.println("event");
//    }
//    oldval = newval;
//  }
//}

void loop() {
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  if (avg == 0) avg = sqrt((a.acceleration.x*a.acceleration.x) + (a.acceleration.y*a.acceleration.y) + (a.acceleration.z*a.acceleration.z)) - 9.81;
  /* Print out the values */
//  Serial.print("Acceleration X: ");
//  Serial.print(a.acceleration.x);
//  Serial.print(", Y: ");
//  Serial.print(a.acceleration.y);
//  Serial.print(", Z: ");
//  Serial.print(a.acceleration.z);
//  Serial.println(" m/s^2");
  double maxus = 0;
  for( int i = 0; i <= 100; ++i) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    double vector = sqrt((a.acceleration.x*a.acceleration.x) + (a.acceleration.y*a.acceleration.y) + (a.acceleration.z*a.acceleration.z)) - 9.81;
    if (abs(vector) > maxus) maxus = vector;
  }
  //if (abs(maxus) > 4) sendEvent();
  avg = (99 * avg + maxus)/100;
  if (millis() > 10000) {
    digitalWrite(15, HIGH);
    if (abs(avg - maxus) > 0.14) sendEvent();
  }
  //Serial.print("Vector: ");
  Serial.println(maxus);
}
