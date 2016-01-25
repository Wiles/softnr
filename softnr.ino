/* 
 * softnr prototype code. 
 * Logs current reading to thingspeak.
 */

#include "ESP8266WiFi.h"
#include "DHT.h"
#include "gpio.h"

const char* ssid     = "";
const char* password = "";

const char* host = "184.106.153.149";
const char* thingspeakKey = "";

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

int value = 0;

void loop() {
  delay(5000);
  ++value;

  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
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
  Serial.println(reading);
  // We now create a URI for the request
  String url = "/update?key=" + thingspeakKey;
  url += "&field1=";
  url += distance;
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(10);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
}
