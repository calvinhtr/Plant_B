#include <SPI.h> // What is used to communicate with the WiFi chip
#include <WiFiNINA.h> // Wifi library fro Arduino MKR1000 and WiFi shield
#include "arduino_secrets.h"
#include <utility/wifi_drv.h>
#include <FastLED.h>
#include <RTCZero.h>

// Define sensors and motors
int waterLevel = A5;
int waterMoisture = A4;
int waterLevelActivate = 10;
int waterMoistActivate = 14;
int lightSensor = A6;
int pump =  7; 
int val = 0;
int moistureVal = 0;
int dry = 794;
int wet = 305;
int countNewline = 0;

#define LED_PIN 6
#define NUM_LEDS 9

CRGB leds[NUM_LEDS];

RTCZero rtc;

char ssid[] = SECRET_SSID;      // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS; //status of wifi

bool autoVar = false;

WiFiServer server(80); //declare server object and spedify port, 80 is port used for internet

void setup() {
  // Initialize pump output
  pinMode(pump, OUTPUT);
  pinMode(waterMoistActivate, OUTPUT);

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
  digitalWrite(waterMoistActivate, LOW);

  // Get time
  rtc.begin();

  unsigned long epoch;
  int numberOfTries = 0, maxTries = 6;
  do {
    epoch = WiFi.getTime();
    numberOfTries++;
  } while ((epoch == 0) && (numberOfTries < maxTries));

  if (numberOfTries == maxTries) {
    Serial.print("NTP unreachable!!");
    while (1);
  }

  else {
    Serial.print("Epoch received: ");
    Serial.println(epoch);
    rtc.setEpoch(epoch);
  }
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
            client.println("[{\"Light Level\": " + String(analogRead(lightSensor)) + ", \"Moisture Level\": " + String(readMoisture()) + " }]");
            client.println();
            break;
          }
          else { 
            // Put all code to check at end here
            if (currentLine.startsWith("Referer")) {
              if (currentLine.indexOf("/printText") > 0) {
                int index = currentLine.indexOf("ext/");
                int indexHTTP = currentLine.indexOf("HTTP/1.1");
                Serial.println(currentLine.substring(index + 4, indexHTTP));
              }
              else if (currentLine.indexOf("/pumpVar") > 0) {
                int index = currentLine.indexOf("Var/");
                int indexHTTP = currentLine.indexOf("HTTP/1.1");
                int pumpVal = currentLine.substring(index + 4, indexHTTP).toInt();
                pumpMl(pumpVal);
              }
            }
            // Clear on newline because info is irrelevant
            currentLine = "";
          }
        }
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
        // Define other endpoints
        if (currentLine.endsWith("GET /motorOff")) {
          digitalWrite(pump, HIGH);
        }
        if (currentLine.endsWith("GET /motorOff")) {
          digitalWrite(pump, HIGH);
        }
        if (currentLine.endsWith("GET /motorOn")) {
          digitalWrite(pump, LOW);
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
          // Turn off LEDs and pump 
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
    if (moistureReading < 50) {
      digitalWrite(pump, LOW);
    }
    else {
      digitalWrite(pump, HIGH);
    }
  }
}

void pumpMl(int ml) {
  digitalWrite(pump, LOW);
  delay(ml*32);
  digitalWrite(pump, HIGH);
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
int readWaterLevel() {
	digitalWrite(waterLevelActivate, HIGH);	// Turn the sensor ON
	delay(10);							// wait 10 milliseconds
	val = analogRead(waterLevel);		// Read the analog value form sensor
	digitalWrite(waterLevelActivate, LOW);		// Turn the sensor OFF
	return val;							// send current reading
}

int readMoisture() {
	digitalWrite(waterMoistActivate, HIGH);	// Turn the sensor ON
	delay(10);							// wait 10 milliseconds
	moistureVal = analogRead(waterMoisture);		// Read the analog value form sensor
	int percentageHumidity = map(moistureVal, wet, dry, 100, 0); //Convert reading into percentage
  
  digitalWrite(waterLevelActivate, LOW);		// Turn the sensor OFF
	return percentageHumidity;							// send current reading
}
