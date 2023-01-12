#include <ESPmDNS.h>
#include <HTTPClient.h>
#include "WiFi.h"

const char* ssid = "router";
const char* password = "Vlad lives here!";

void sendEvent() {
  
  HTTPClient http;

  http.begin("http://fallserver.local/?action=0");
  int httpCode = http.GET();

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

void setup() {
  Serial.begin(9600);
  Serial1.begin(115200);
  Serial.print("Test"); 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
  
}

void loop() {
  if (Serial1.available()) {
    byte inByte = Serial1.read();
    Serial.print(inByte);
  }
}
