/*
 * softnr prototype code. 
 * Logs current reading to thingspeak.
 */

#include "ESP8266WiFi.h"
#include "gpio.h"

#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>

const char* thingspeakHost = "184.106.153.149";
const char* iftttHost = "maker.ifttt.com";
const int httpPort = 80;

String thingspeakKey;
String iftttKey;
String iftttTrigger;

int height = 80;
int triggerLevel = 40;

int belowTriggerCount = 0;
int requiredReadingCount = 6;
boolean triggerSent = false;

void setup() {

    WiFiManager wifiManager;
    
    wifiManager.resetSettings();

    wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    WiFiManagerParameter thingspeakParam("thingspeak", "Thingspeak key", "", 40);
    wifiManager.addParameter(&thingspeakParam);
    WiFiManagerParameter iftttParam("iftttKey", "IFTTT key", "", 80);
    wifiManager.addParameter(&iftttParam);
    WiFiManagerParameter iftttTriggerParam("iftttTigger", "IFTTT trigger", "", 40);
    wifiManager.addParameter(&iftttTriggerParam);
    WiFiManagerParameter heightParam("height", "Height of water softener (cm)", "", 40);
    wifiManager.addParameter(&heightParam);
    WiFiManagerParameter triggerParam("trigger", "Trigger level (cm)", "", 40);
    wifiManager.addParameter(&triggerParam);
    
    wifiManager.autoConnect("softnr config");

    thingspeakKey = thingspeakParam.getValue(); 
    iftttKey = iftttParam.getValue();
    iftttTrigger = iftttTriggerParam.getValue();
    height = (String(heightParam.getValue())).toInt();
    triggerLevel = (String(triggerParam.getValue())).toInt();
    delay(5000);
}

void loop() {
  int reading = analogRead(A0);
  if (reading == 0) {
    reading = 1;
  }

  int distance = 27.0 / (reading * (3.3 / 1023.0));
  if (distance > 80) {
    distance = 80;
  }

  int remaining = height - distance;

  if (remaining < triggerLevel) {
     ++belowTriggerCount;
  } else {
    belowTriggerCount = 0;
    triggerSent = false;
  }

  if ((belowTriggerCount >= requiredReadingCount)) {
    if (iftttKey.length() > 0 && iftttTrigger.length() > 0) {
      if (triggerSent == false) {
        triggerSent = true;
        WiFiClient iftttClient;
        if (iftttClient.connect(iftttHost, httpPort)) {
          if (thingspeakKey.length() > 0) {
            // We now create a URI for the request
            String url = "/trigger/" + iftttTrigger;
            url += "/with/key/";
            url += iftttKey;
            
            // This will send the request to the server
            iftttClient.print(String("GET ") + url + " HTTP/1.1\r\n" +
                         "Host: " + iftttHost + "\r\n" + 
                         "Connection: close\r\n\r\n");
          }
        }
      }
    }
  }

  // Log reading to Thingspeak
  if (thingspeakKey.length() > 0) {
    
    WiFiClient client;
    if (client.connect(thingspeakHost, httpPort)) {
      // We now create a URI for the request
      String url = "/update?key=" + thingspeakKey;
      url += "&field1=";
      url += remaining;
      
      // This will send the request to the server
      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + thingspeakHost + "\r\n" + 
                   "Connection: close\r\n\r\n");
    }
  }
  
  delay(60 * 60 * 1000);
}

