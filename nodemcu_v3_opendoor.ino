#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <FormsGoogle.h>
#include "DHT.h"

#define PIN_SHOCK  A0
#define PIN_REED    4
#define PIN_BUZZER  5
#define PIN_DHT     0
#define DHTTYPE DHT11
#define SENSIBILITY 15
#define DELAY_LOOP 100
#define VALUE_OPEN_DOOR   "isOpen"

int humidityValue = 0;
int temperatureValue = 0;
const char* ssid = "TIGO-B13F";
const char* password = "2NB112101391";
uint8_t DHTPin = PIN_DHT;
boolean openDoor = false;
int bufferedTock[3];
boolean cleanBuffered = false;
boolean shock = false;
String params[] = {"entry.802876412="//Campo humidity
                ,"entry.1605107394="//campo temperature
                ,"entry.1627871440="//campo value isOpen
                ,"entry.428488488="//campo shock
                };//
String paramsSend[4];
const String keyConnect =  "1FAIpQLSdal5EO0E42S8dvgnGJzjhsj88afyhHqbBa9fTRU5FXZh-5Qw";
bool sendData = false;

WiFiClientSecure client;
FormsGoogle form;

const char fingerprint[] PROGMEM = "1C 9A D3 68 40 3A 92 46 DC D1 51 A2 71 CF 71 67 5A AC F5 5B";//docs.google.com

ESP8266WiFiMulti WiFiMulti;
DHT dht(DHTPin,DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.println("Setup");
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(PIN_REED,INPUT);
  pinMode(PIN_BUZZER,OUTPUT);
  pinMode(PIN_DHT,INPUT);
  wiFiConnect();
  dht.begin();
  form.setFingerPrint(fingerprint);
  form.setKeyConnect(keyConnect);
  //form.activeDebugMode();
}

void loop() {
  readSeed();
  readShock();
  readHumidity();
  readTemperature();
  writeBuzzer();
  writeSheetGoogle();
  clearBuffered();
  delay(DELAY_LOOP);
}

 void readSeed(){
  
    int value = digitalRead(PIN_REED);
    if(value == HIGH){
      if(!openDoor){
        Serial.println("open door");
        sendData = true;
      }
      digitalWrite(LED_BUILTIN,LOW);
      openDoor = true;
      paramsSend[2] = params[2] + VALUE_OPEN_DOOR; 
    }else{
      if(openDoor){
        Serial.println("close door");
        sendData = true;
      }
      digitalWrite(LED_BUILTIN,HIGH);
      openDoor = false;
      paramsSend[2] = params[2] + ""; 
    }
 }

 void readShock(){
    int value = analogRead(PIN_SHOCK);
    if(value > SENSIBILITY){
      for(int i = 0; i < sizeof(bufferedTock);i++){
        if(bufferedTock[i] == 0){
          paramsSend[3] = params[3] + value; 
          bufferedTock[i] = value;
          break;
        }
      }
    }else{
      paramsSend[3] = params[3] + 0; 
    }
 }

 void writeBuzzer(){
    int j = 0;
    for(int i = 0; i < sizeof(bufferedTock);i++){
      if(bufferedTock[i] != 0){
        j++;
      }
    }
   if(j > 2){
      sendData = true;
      Serial.print("Buzzer");
      digitalWrite(PIN_BUZZER,HIGH);
      delay(1500);
      digitalWrite(PIN_BUZZER,LOW);
      cleanBuffered = true;
   }
 }

 void clearBuffered(){
  if(cleanBuffered){
    for(int i = 0; i < sizeof(bufferedTock);i++){
       bufferedTock[i] = 0;
    }
    cleanBuffered = false;
  }
 }

 void writeSheetGoogle(){
  if(sendData){
    sendData = false; 
    form.addParams(paramsSend);
    if(form.send()){
      Serial.println("Enviado correctamente");
    }else{
      Serial.println("Error en el envio verifique activando modeDebug");
    }
    
  }
 }


void wiFiConnect(){
    WiFi.mode(WIFI_STA);
    //WiFiMulti.addAP(ssid,password);
    WiFi.begin(ssid,password);
   Serial.print("\r\nConnecting");
    while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(500);
    }
    Serial.println("");
    Serial.println("..Connected to WiFi..");
  }

  void readHumidity(){
   float h = dht.readHumidity();
    if(!isnan(h) ){
      paramsSend[0] = params[0] + (int)h ;
    }else{
      paramsSend[0] = params[0] + 0; 
    }
  }
  
  void readTemperature(){
    float t = dht.readTemperature();
    if(!isnan(t) ){
      paramsSend[1] = params[1] + (int)t; 
      
    }else{
      paramsSend[1] = params[1] + 0;
    }
  }
