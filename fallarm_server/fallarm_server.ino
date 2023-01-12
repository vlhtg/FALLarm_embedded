#include <Adafruit_NeoPixel.h>
#include <ESPmDNS.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <HTTPClient.h>

const char* ssid = "router";
const char* password = "Vlad lives here!";
const char* host = "maker.ifttt.com";
const char* apiKey = "bIykChiyswu7wz2Xj6M-xw";

AsyncWebServer server(80);
#define LEDPIN    16
#define INLEDPIN  15
#define BUTTONPIN 21
#define NUMPIXELS 12
#define BUZZPIN   34


Adafruit_NeoPixel pixels(NUMPIXELS, LEDPIN, NEO_GRB + NEO_KHZ400);

int alarmStatus = 0;

void updateBase(bool value) {
  HTTPClient http;
  http.begin("https://fallarm-2dee8-default-rtdb.europe-west1.firebasedatabase.app/event.json");
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode;
  if (value == false) {
    httpResponseCode = http.POST("{\"action\":false}");
  } else {
    httpResponseCode = http.POST("{\"action\":true}");
  }
  if(httpResponseCode>0){
    String response = http.getString();
    Serial.println(httpResponseCode);   //Print return code
    Serial.println(response);           //Print request answer
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
  
   http.end();  //Free resources
}

void send(String action) {
  
  HTTPClient http;

  http.begin("https://maker.ifttt.com/trigger/" + action + "/with/key/bIykChiyswu7wz2Xj6M-xw"); //Specify the URL
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

void event() {

  Serial.println("alarm start");

  send("alarm");

  for(int i = 0; i < 12; ++i) {
    pixels.setPixelColor(i, pixels.Color(256, 0, 0));
    pixels.show();
    delay(50);
  }

  pixels.show();

  while(digitalRead(BUTTONPIN)) {
    analogWrite(BUZZPIN, 127);
    digitalWrite(INLEDPIN, HIGH);
    delay(50);
    analogWrite(BUZZPIN, 0);
    digitalWrite(INLEDPIN, LOW);
    delay(50);
  }
  Serial.println("alarm turned off");

  send("alarmok");

  for(int j = 0; j < 5; ++j) {
    for(int i = 0; i < 12; ++i) {
      pixels.setPixelColor(i, pixels.Color(256, 0, 0));
      pixels.show();
      delay(50);
    }
    pixels.show();
    delay(1000);
    pixels.clear();
    pixels.show();
    delay(200);
  }
  Serial.println("back to normal");
  alarmStatus = 0;
}

void startEvent() {
  alarmStatus = 1;
}


void setup() {
  pinMode(BUZZPIN, OUTPUT);
  pinMode(BUTTONPIN, INPUT_PULLUP);
  pinMode(INLEDPIN, OUTPUT);
  pixels.begin();
  Serial.begin(115200);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  if(!MDNS.begin("fallserver")) {
     Serial.println("Error starting mDNS");
     return;
  }
  Serial.println(WiFi.localIP());
 
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
 
    int paramsNr = request->params();
    Serial.println(paramsNr);

    AsyncWebParameter* p = request->getParam(0);
    String name = p->name();
    String val = p->value();
    if(name == "action" && val == "0") {
      startEvent();
    }

    request->send(200, "text/plain", "message received");
  });
 
  server.begin();
}

void loop() {
  if (alarmStatus == 1) {
    event();
  }
}
