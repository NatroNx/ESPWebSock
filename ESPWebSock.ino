/*
 * ESP8266 Web httpServer with Web Socket to control an LED.
 *
 * The web httpServer keeps all clients' LED status up to date and any client may
 * turn the LED on or off.
 *
 * For example, clientA connects and turns the LED on. This changes the word
 * "LED" on the web page to the color red. When clientB connects, the word
 * "LED" will be red since the httpServer knows the LED is on.  When clientB turns
 * the LED off, the word LED changes color to black on clientA and clientB web
 * pages.
 *
 * References:
 *
 * https://github.com/Links2004/arduinoWebSockets
 *
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>




// Fill in your WiFi router SSID and password
const char* ssid = "Wlan07";
const char* password = "wlan_01!_2005!grojer_..";
MDNSResponder mdns;

ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
WebSocketsServer webSocket = WebSocketsServer(81);



const char INDEX_HTML[] =
  "<!DOCTYPE html>"
  "<html>"
  "<head>"
  "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
  "<title>ESP8266 WebSocket Demo</title>"
  "<style>"
  "\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\""
  "</style>"
  "<script>"
  "var websock;"
  "function start() {"
  "websock = new WebSocket('ws://10.0.0.67:81/');"
  "websock.onopen = function(evt) { console.log('websock open'); };"
  "websock.onclose = function(evt) { console.log('websock close'); };"
  "websock.onerror = function(evt) { console.log(evt); };"
  "websock.onmessage = function(evt) {"
  "console.log(evt);"
  "var e = document.getElementById('ledstatus');"
"if (evt.data === 'Bled|1023\\n') {"
"e.style.color = 'red';"
"}"
"else if (evt.data === 'Bled|0\n') {"
"e.style.color = 'black';"
"}"
  "else {"
  "console.log('unknown event');"
  "}"
  "};"
  "}"
"function buttonclick(e) {"
"websock.send('B' + e.id + '\n');"
  "}"
  "</script>"
  "</head>"
  "<body onload=\"javascript:start();\">"
  "<h1>Aquaduino with OTA Updater</h1>"
"<div id=\"ledstatus\"><b>LED</b></div>"
"<button id=\"led|1023\"  type=\"button\" onclick=\"buttonclick(this);\">On</button> "
"<button id=\"led|0\" type=\"button\" onclick=\"buttonclick(this);\">Off</button>"
  "</body>"
  "</html>";

// GPIO#0 is for Adafruit ESP8266 HUZZAH board. Your board LED might be on 13.
const int LEDPIN = 13;
// Current LED status
bool LEDStatus;

// Commands sent through Web Socket
const char LEDON[] = "led|1";
const char LEDOFF[] = "led|0";
String webSocketString = "999";

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        // Send the current LED status
        if (LEDStatus) {
          webSocket.sendTXT(num, LEDON, strlen(LEDON));


        }
        else {
          webSocket.sendTXT(num, LEDOFF, strlen(LEDOFF));
        }
      
      }
      break;
    case WStype_TEXT:
      {Serial.printf("[%u] get Text: %s\r\n", num, payload);
      String webSocketString = String((char *) &payload[0]);
      Serial.println("XXXXXXXXXXXX");

      Serial.print("Integer + 1: ");
      Serial.println(webSocketString.toInt()+1);
      
      
      Serial.println("text: ");
      Serial.println(webSocketString);
      if (webSocketString.startsWith("B")){
        String t1Val = (webSocketString.substring(webSocketString.indexOf("B") + 1, webSocketString.length()));
        if (t1Val.startsWith("led")){
          String t2Val = (t1Val.substring(t1Val.indexOf("|") + 1, t1Val.length()));
          writeLED(t2Val.toInt());}
        
      }
      else if (webSocketString.startsWith("S")) {
      String tVal = (webSocketString.substring(webSocketString.indexOf("S") + 1, webSocketString.length()));
      analogWrite(LEDPIN, tVal.toInt()); 
          char charBuf[50];
          tVal.toCharArray(charBuf, 50);
          webSocket.sendTXT(num, charBuf , 5);
      }
      else {
        analogWrite(LEDPIN, webSocketString.toInt());
        Serial.write("Something wrong");
       
        
      }
      // send data to all connected clients
      webSocket.broadcastTXT(payload, length);
      }
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      hexdump(payload, length);

      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
    default:
      Serial.printf("Invalid WStype [%d]\r\n", type);
      break;
  }
}

void handleRoot()
{
  httpServer.send(200, "text/html", INDEX_HTML);
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += httpServer.uri();
  message += "\nMethod: ";
  message += (httpServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += httpServer.args();
  message += "\n";
  for (uint8_t i = 0; i < httpServer.args(); i++) {
    message += " " + httpServer.argName(i) + ": " + httpServer.arg(i) + "\n";
  }
  httpServer.send(404, "text/plain", message);
}

void writeLED(int LEDon)
{
  analogWrite(LEDPIN, LEDon);
}

void setup()
{ Serial.begin(115200);

  pinMode(LEDPIN, OUTPUT);
  writeLED(false);




  Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] BOOT WAIT %d...\r\n", t);
    Serial.flush();
    delay(300);
  }
WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());


  Serial.print("Connect to http://");
  Serial.println(WiFi.localIP());
  httpUpdater.setup(&httpServer);
  HTTPUpdateConnect();
  httpServer.on("/", handleRoot);
  httpServer.onNotFound(handleNotFound);

  httpServer.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}


void HTTPUpdateConnect() {

  httpUpdater.setup(&httpServer);
  httpServer.begin();
}






void loop()
{
  webSocket.loop();
  httpServer.handleClient();
  delay(1);
}
