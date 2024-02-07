#include <SPI.h> // What is used to communicate with the WiFi chip
#include <WiFiNINA.h> // Wifi library fro Arduino MKR1000 and WiFi shield
#include "arduino_secrets.h"
#include <utility/wifi_drv.h>

// Define sensors and motors
int waterLevel = A0;
int waterMoisture = A1;
int light = A2;
int ledR = A3;
int ledG = A4;
int ledB = A5;
int pump =  10; 


char ssid[] = SECRET_SSID;      // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS; //status of wifi

WiFiServer server(80); //declare server object and spedify port, 80 is port used for internet

void setup() {
  pinMode(10, OUTPUT);
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();
  printWifiStatus();
}

void loop() {
  // Serial.println(String(analogRead(A0)));
  WiFiClient client = server.available();

  if (client) {                             
    String currentLine = "";
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
       // Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            String var = String(analogRead(A0));
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json;charset=utf-8");
            client.println("Server: Arduino");
            client.println("Connection: close");
            client.println();
            client.println("[{\"Light Level\":" + String(analogRead(A0)) + " }]");
            client.println();
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /G")) {
          analogWrite(A3, 0);
          analogWrite(A4, 50);
          analogWrite(A5, 0);
          WiFiDrv::analogWrite(25, 0);
          WiFiDrv::analogWrite(26, 50);
          WiFiDrv::analogWrite(27, 0);
          // digitalWrite(lControl, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /P")) {
          analogWrite(A3, 50);
          analogWrite(A4, 0);
          analogWrite(A5, 50);
          WiFiDrv::analogWrite(25, 50);
          WiFiDrv::analogWrite(26, 0);
          WiFiDrv::analogWrite(27, 50);
          // digitalWrite(lControl, LOW);                // GET /L turns the LED off
        }
        if (currentLine.endsWith("GET /R")) {
          analogWrite(A3, 50);
          analogWrite(A4, 0);
          analogWrite(A5, 0);
          WiFiDrv::analogWrite(25, 50);
          WiFiDrv::analogWrite(26, 0);
          WiFiDrv::analogWrite(27, 0);
          // digitalWrite(lControl, LOW);                // GET /L turns the LED off
        }
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(10, HIGH);           
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(10, LOW);           
        }
      }
    }
    // close the connection:
    client.stop();
   // Serial.println("client disconnected");
  }
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}