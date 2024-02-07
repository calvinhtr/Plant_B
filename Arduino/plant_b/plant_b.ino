#include <SPI.h> // What is used to communicate with the WiFi chip
#include <WiFiNINA.h> // Wifi library fro Arduino MKR1000 and WiFi shield
#include "arduino_secrets.h"
#include <utility/wifi_drv.h>
#include <FastLED.h>

// Define sensors and motors
int waterLevel = A5;
int waterMoisture = A4;
int waterLevelActivate = 10;
int lightSensor = A6;
int pump =  7; 
int val = 0;

#define LED_PIN 5
#define NUM_LEDS 20

CRGB leds[NUM_LEDS];


char ssid[] = SECRET_SSID;      // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS; //status of wifi


bool autoVar = false;

WiFiServer server(80); //declare server object and spedify port, 80 is port used for internet

void setup() {
  // Initialize pump output
  pinMode(pump, OUTPUT);
  pinMode(waterLevelActivate, OUTPUT);

  // Initialize pump to off
  digitalWrite(pump, HIGH);

  // Connect to WIFI
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
  digitalWrite(waterLevelActivate, LOW);
}

void loop() {
  WiFiClient client = server.available();
  
  if (client) {            
    String currentLine = "";
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        // Serial.println("client has been availabled");   
        char c = client.read();             // read a byte, then
       // Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:  
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json;charset=utf-8");
            client.println("Server: Arduino");
            client.println("Connection: close");
            client.println();
            client.println("[{\"Light Level\":" + String(analogRead(lightSensor)) + " }]");
            client.println();
            break;
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
            // Serial.println("c is nothing, currentline cleared"); 
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
          // Serial.println("c is slash n"); 
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /motorOff")) {
          digitalWrite(pump, HIGH);
        }
        if (currentLine.endsWith("GET /motorOn")) {
          digitalWrite(pump, LOW);
          // Serial.println("motor is onning");
        }
        if (currentLine.endsWith("GET /ledR")) {
          FastLED.addLeds<WS2812, LED_PIN, GRB> (leds, NUM_LEDS);
          for (int i = 0; i <= NUM_LEDS-1; i++) {
            leds[i] = CRGB::Red;
          }
          FastLED.show();
        }
        if (currentLine.endsWith("GET /ledG")) {
          FastLED.addLeds<WS2812, LED_PIN, GRB> (leds, NUM_LEDS);
          for (int i = 0; i <= NUM_LEDS-1; i++) {
            leds[i] = CRGB::Green;
          }
          FastLED.show();
        }
        if (currentLine.endsWith("GET /ledB")) {
          FastLED.addLeds<WS2812, LED_PIN, GRB> (leds, NUM_LEDS);
          for (int i = 0; i <= NUM_LEDS-1; i++) {
            leds[i] = CRGB::Blue;
          }
          FastLED.show();
        }
        if (currentLine.endsWith("GET /ledOff")) {
          FastLED.addLeds<WS2812, LED_PIN, GRB> (leds, NUM_LEDS);
          for (int i = 0; i <= NUM_LEDS-1; i++) {
            leds[i] = CRGB(0,0,0);
          }
          FastLED.show();
        }
        if (currentLine.endsWith("GET /autoOn")) {
          autoVar = true;
        }
        if (currentLine.endsWith("GET /autoOff")) {
          autoVar = false;
          FastLED.addLeds<WS2812, LED_PIN, GRB> (leds, NUM_LEDS);
          for (int i = 0; i <= NUM_LEDS-1; i++) {
            leds[i] = CRGB(0,0,0);
          }
          FastLED.show();
          digitalWrite(pump, HIGH);

        }
      } 
    }
    // close the connection:
    client.stop();
   // Serial.println("client disconnected");
  }

  if (autoVar) {
    int lightReading = analogRead(lightSensor);
    int moistureReading = readMoisture();
    // Serial.println(moistureReading);
    if (lightReading < 500) {
      FastLED.addLeds<WS2812, LED_PIN, GRB> (leds, NUM_LEDS);
      for (int i = 0; i <= NUM_LEDS-1; i++) {
        leds[i] = CRGB(5,0,0);
      }
    } else {
      FastLED.addLeds<WS2812, LED_PIN, GRB> (leds, NUM_LEDS);
      for (int i = 0; i <= NUM_LEDS-1; i++) {
        leds[i] = CRGB(0,0,0);
      }
    }
    FastLED.show();
    if (moistureReading < 100) {
      digitalWrite(pump, LOW);
    }
    else {
      digitalWrite(pump, HIGH);
    }
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

//This is a function used to get the reading
int readMoisture() {
	digitalWrite(waterLevelActivate, HIGH);	// Turn the sensor ON
	delay(10);							// wait 10 milliseconds
	val = analogRead(waterLevel);		// Read the analog value form sensor
	digitalWrite(waterLevelActivate, LOW);		// Turn the sensor OFF
	return val;							// send current reading
}
