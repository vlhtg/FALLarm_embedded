#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>


char ssid[64] = "REPLACE_WITH_YOUR_SSID";
char password[64] = "REPLACE_WITH_YOUR_PASSWORD";

WiFiServer server(80);
String header;

boolean readWiFiDetails() {
  int i;
  for(i = 0; i < 64; ++i) {
    ssid[64] = EEPROM.readChar(i);
  }
  for(i = 64; i < 127; ++i) {
    password[64] = EEPROM.readChar(i);
  }
  return EEPROM.readBool(128);
}

void writeWiFiDetails() {
  int i;
  for(i = 0; i < 64; ++i) {
    EEPROM.writeChar(i, ssid[i]);
  }
  for(i = 64; i < 128; ++i) {
    EEPROM.writeChar(i, password[i]);
  }
  EEPROM.writeBool(128, false);
}

void connectionSettings() {
  Serial.println("Enter SSID: ");
  String ssidString = Serial.readString();
  Serial.println("Enter password: ");
  String passwordString = Serial.readString();

  ssidString.toCharArray(ssid, 64);
  passwordString.toCharArray(password, 64);
  writeWiFiDetails();
}

void WiFiSetup() {
  if(!digitalRead(12) || readWiFiDetails()) {
    connectionSettings();
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting ");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.print("Device IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Strength: ");
  Serial.println(WiFi.RSSI());
}

void buttonsSetup() {
  Serial.begin(9600);
  pinMode(12, OUTPUT);
}
void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}