#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <Arduino.h>
#include <WebSocketsServer.h>
#include <Hash.h>


#define BLUEPIN 13

#define SSID_ME "Wlan07"
#define PW_ME "wlan_01!_2005!grojer_.."
#define HOST_ME "ESP8266"




//Globals
WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
const char* host = HOST_ME;
const char* ssid     = SSID_ME;
const char* password = PW_ME;
unsigned long lastTimeHost;
unsigned long lastTimeSent;
String text="z10";
int zInt;
int zOld;

// WebSOcket Events
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:
      break;


    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        // Send the current LED status
 
          webSocket.sendTXT(num, text);
      }
      
    
      break;


      
    case WStype_TEXT:
      {
        text = String((char *) &payload[0]);

        if (text.startsWith("u")) {
          String zVal = (text.substring(text.indexOf("u") + 1, text.length()));
          int zInt = zVal.toInt();
          analogWrite(BLUEPIN, zInt);
          webSocket.broadcastTXT(text);
        }

        if (text.startsWith("z")) {
          String zVal = (text.substring(text.indexOf("z") + 1, text.length()));
          int zInt = zVal.toInt();
          analogWrite(BLUEPIN, zInt);
        }
     

        

       }
      break;

    case WStype_BIN:
      hexdump(payload, length);
      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
  }
}


// Wifi Connection
void WifiConnect() {

 // WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println(WiFi.localIP());

}

// WebSocket Connection
void WebSocketConnect() {
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}



// HTTP updater connection
void HTTPUpdateConnect() {
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  Serial.print("Connect to ");
  Serial.println(WiFi.localIP());

}




void setup() {

  Serial.begin(115200);
  pinMode(BLUEPIN, OUTPUT);
  WifiConnect();
  WebSocketConnect();

  HTTPUpdateConnect();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WifiConnect();
    WebSocketConnect();
  }
  else {

    webSocket.loop();
    if (millis() - lastTimeHost > 1) {
      httpServer.handleClient();
      lastTimeHost = millis();
    }

  }
}

