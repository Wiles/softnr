/*
 * softnr prototype code. 
 * Logs current reading to thingspeak.
 */

#include "ESP8266WiFi.h"
#include "gpio.h"

#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>

const char* host = "184.106.153.149";
String thingspeakKey;

void setup() {

    WiFiManager wifiManager;
    
    wifiManager.resetSettings();

    WiFiManagerParameter custom_param("thingspeak", "Thingspeak Key", "", 40);
    wifiManager.addParameter(&custom_param);
    
    wifiManager.autoConnect("AutoConnectAP");

    thingspeakKey = custom_param.getValue(); 
}

void loop() {
  delay(5000);

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    return;
  }
  int reading = analogRead(A0);
  if (reading == 0) {
    reading = 1;
  }

  int distance = 27.0 / (reading * (3.3 / 1023.0));
  if (distance > 80) {
    distance = 80;
  }

  // We now create a URI for the request
  String url = "/update?key=" + thingspeakKey;
  url += "&field1=";
  url += distance;
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(10);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
  }
}

