#include <Adafruit_NeoPixel.h>
#include <ESPmDNS.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <HTTPClient.h>
#include "pitches.h"

const char* ssid = "router";
const char* password = "Vlad lives here!";
const char* hostMail = "maker.ifttt.com";
const String hostSMS = "https://api-mapper.clicksend.com/http/v2/send.php?";
const String usernameSMS = "w.g.taborsky@student.tue.nl";
const String apiKeyMail = "bIykChiyswu7wz2Xj6M-xw";
const String apiKeySMS = "57701511-AF35-51C9-85F1-B96D8169373B";
//const String phoneNumber = "+31620521628";
const String phoneNumber = "+61411111111";
AsyncWebServer server(80);

#define LEDPIN 25
#define N_LEDS 12
#define SOUNDPIN 26
#define BUTTONPIN 13
#define SOUNDCHANNEL 0

Adafruit_NeoPixel pixels(N_LEDS, LEDPIN, NEO_GRB + NEO_KHZ800);

uint32_t red = pixels.Color(255, 0, 0); //Red color for led ring
uint32_t green = pixels.Color(0, 255, 0); //Green color for led ring
uint32_t off = pixels.Color(0,0,0); //Turn led off
const int ledDelay = 20; //Delay for led blinking

int alarmStatus = 0; //State of the alarm 1 rings 2 stops ringing 0 armed
bool buttonState = 1; //State of the button

int alarm_melody[] = {NOTE_D4, NOTE_F4, NOTE_A4, NOTE_C4}; //Notes for the melody of the alarm
int good_melody[] = {NOTE_A5, NOTE_C5}; //Notes for the sound of false positive alarm
int melodyLength = 4; //Length of alarm_melody array
int goodLength = 2; //Length of good_melody array
int noteDuration = 1000; //Duration of a note
int notePause = 100; //Pause between notes
int alarmTime = 120E3; //Time before alarm automatically turns off

void IRAM_ATTR ISR(){ //Function that is called if button is pushed
  buttonState = 0;
}

void setup() {
  Serial.begin(115200);
  
  pinMode(LEDPIN, OUTPUT);
  pinMode(SOUNDPIN, OUTPUT);
  pinMode(BUTTONPIN, INPUT_PULLUP);
  attachInterrupt(BUTTONPIN, ISR, FALLING);

  ledcSetup(SOUNDCHANNEL,1E5,12);
  ledcAttachPin(SOUNDPIN,0);
  
  pixels.begin();
  pixels.show();

  WiFi.begin(ssid, password);

  int connectionTries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    ++ connectionTries;
    delay(1000);
    Serial.println("Connecting to WiFi..");
    if(connectionTries >= 10){
      ESP.restart();
    }
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

void startEvent() {
  alarmStatus = 1;
}

void updateBase(bool value) { //Updates firebase true or false
  HTTPClient http;
  http.begin("https://fallarm-2dee8-default-rtdb.europe-west1.firebasedatabase.app/event/.json");
  http.addHeader("Content-Type", "application/json");
  
  int httpResponseCode;

  Serial.println(value);
  
  if (value == false) {
    httpResponseCode = http.POST("{\"action\":false}");
  } 
  else {
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

void sendMail(String action) { //Sends a request for the email
  
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

void sendSMS(String action) { //Sends a request for the email
  
  HTTPClient http;

  if(action == "Fall"){
    http.begin(hostSMS + "method = http" + "&username=" + usernameSMS + "&key=" + apiKeySMS + "&to=" + phoneNumber + "&message=Elderly has fallen!"); //Specify the URL
    int httpCode = http.GET(); //Make the request
    if(httpCode == 200){
      Serial.println("SMS fall send");
      Serial.println(httpCode);
    }
    else Serial.println("SMS fall failed");
  }

  if(action == "Fallok"){
    http.begin(hostSMS + "method = http" + "&username=" + usernameSMS + "&key=" + apiKeySMS + "&to=" + phoneNumber + "&message=Elderly is oke!"); //Specify the URL
    int httpCode = http.GET(); //Make the request
    if(httpCode == 200){
      Serial.println("SMS ok send");
      Serial.println(httpCode);
    }
    else Serial.println("SMS ok failed");
  }

  http.end();
}

void light_led(uint32_t color){
   for(int i=0; i<=N_LEDS; ++i){
    pixels.setPixelColor(i, color);
    pixels.show();
    delay(ledDelay);
   }
   pixels.clear();
   pixels.show();
}

void alarm(){
  for (int thisNote = 0; thisNote < melodyLength; thisNote++) {
     ledcWriteTone(SOUNDCHANNEL, alarm_melody[thisNote]);
     
     for(int delayUpdate = 0; delayUpdate < noteDuration; delayUpdate += N_LEDS*ledDelay){
        light_led(red);
     }
     
     ledcWrite(SOUNDCHANNEL, 0);
     delay(ledDelay);
   }
}

void false_alarm(){
  for (int thisNote = 0; thisNote < goodLength; thisNote++) {
     ledcWriteTone(SOUNDCHANNEL, good_melody[thisNote]);
     
     for(int delayUpdate = 0; delayUpdate < noteDuration; delayUpdate += N_LEDS*ledDelay){
        light_led(green);
     }
     
     ledcWrite(SOUNDCHANNEL, 0);
     delay(ledDelay);
   }
}

void event() { //What happens when a fall is detected

  int startTime = millis();

  Serial.println("alarm start");

  updateBase(true); //Update firebase
  
  sendMail("alarm"); //Sends email to caregiver
  sendSMS("Fall"); //Sends sms to caregiver

  buttonState = 1;

  while(buttonState) { //Buzzer and intercepts button input
    alarm();

    if (millis() >= startTime + alarmTime){
      Serial.println("Alarm stopped");
      alarmStatus = 0;
      return;
    }
  }
  
  Serial.println("alarm turned off");

  false_alarm();

  updateBase(false); //False to firebase

  sendMail("alarmok"); //Email that it is oke
  sendSMS("Fallok"); //Sms that it is oke
  
  Serial.println("back to normal");
  alarmStatus = 0;
}
