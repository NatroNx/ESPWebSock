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

#include <ESP8266HTTPUpdateServer.h>




// Fill in your WiFi router SSID and password
const char* ssid = "Wlan07";
const char* password = "wlan_01!_2005!grojer_..";

ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
WebSocketsServer webSocket = WebSocketsServer(81);




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
  httpServer.send(200, "text/html", "<html><head><script>var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);connection.onopen = function () {  connection.send('Connect ' + new Date()); }; connection.onerror = function (error) {    console.log('WebSocket Error ', error);};connection.onmessage = function (e) {  console.log('Server: ', e.data);};function sendRGB() {  var r = parseInt(document.getElementById('r').value).toString(16);  var g = parseInt(document.getElementById('g').value).toString(16);  var b = parseInt(document.getElementById('b').value).toString(16);  if(r.length < 2) { r = '0' + r; }   if(g.length < 2) { g = '0' + g; }   if(b.length < 2) { b = '0' + b; }   var rgb = '#'+r+g+b;    console.log('RGB: ' + rgb); connection.send(rgb); }</script></head><body>LED Control:<br/><br/>R: <input id=\"r\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" onchange=\"sendRGB();\" /><br/>G: <input id=\"g\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" onchange=\"sendRGB();\" /><br/>B: <input id=\"b\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" onchange=\"sendRGB();\" /><br/></body></html>");
    });
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

}
