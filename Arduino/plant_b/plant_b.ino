#include <SPI.h> // What is used to communicate with the WiFi chip
#include <WiFiNINA.h> // Wifi library fro Arduino MKR1000 and WiFi shield
#include "arduino_secrets.h"
#include <utility/wifi_drv.h>
#include <FastLED.h>
#include <RTCZero.h>

// Define sensors and motors
int waterMoisture = A4;
int waterMoistActivate = 12;
int lightSensor = A6;
int pump =  10; 
#define LED_PIN 1
#define NUM_LEDS 9

CRGB leds[NUM_LEDS];

// Calibration values
int val = 0;
int moistureVal = 0;
int dry = 794;
int wet = 305;

RTCZero rtc;

/* Change these values to set the current initial time */
const byte secInit = 0;
const byte minInit = 0;
const byte hourInit = 0;

/* Change these values to set the current initial date */
const byte dayInit = 11;
const byte monthInit = 3;
const byte yearInit = 24;

char ssid[] = SECRET_SSID;      // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS; //status of wifi

bool autoVar = false;

WiFiServer server(80); //declare server object and spedify port, 80 is port used for internet

void setup() {
  Serial.begin(9600);

  // Initialize pump output
  pinMode(pump, OUTPUT);
  pinMode(waterMoistActivate, OUTPUT);
  pinMode(lightSensor, INPUT);
  pinMode(waterMoisture, INPUT);

  // Initialize pump and water moisture sensor to off
  digitalWrite(pump, HIGH);
  digitalWrite(waterMoistActivate, LOW);

  FastLED.addLeds<WS2812, LED_PIN, GRB> (leds, NUM_LEDS);

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

  // Begin timer
  rtc.begin();

  // Set the time
  rtc.setHours(hourInit);
  rtc.setMinutes(minInit);
  rtc.setSeconds(secInit);

  // Set the date
  rtc.setDay(dayInit);
  rtc.setMonth(monthInit);
  rtc.setYear(yearInit);

  // Get current time from WiFi
  // unsigned long epoch;
  // int numberOfTries = 0, maxTries = 6;
  // do {
  //   epoch = WiFi.getTime();
  //   numberOfTries++;
  // } while ((epoch == 0) && (numberOfTries < maxTries));

  // if (numberOfTries == maxTries) {
  //   Serial.print("NTP unreachable - Setting default datetime");
  //   // Set the time and date to default vals
  //   rtc.setHours(hourInit);
  //   rtc.setMinutes(minInit);
  //   rtc.setSeconds(secInit);

  //   rtc.setDay(dayInit);
  //   rtc.setMonth(monthInit);
  //   rtc.setYear(yearInit);
  // } else {
  //   Serial.print("Epoch received: ");
  //   Serial.println(epoch);
  //   rtc.setEpoch(epoch);
  // }
}

// Default control system values (changeable via user input)
int waterAmount = 50; // Amount of water to dispense per watering session in mL
int waterInterval = 2; // Number of days before watering again - should be 7 but setting to 2 for demo
int moistureThreshold = 300; // The moisture level which will trigger watering
int checkInterval = 1; // How long between loops. Currently in seconds (1 second = 1 hour). Change later to be every 15/30 minutes
int lightHoursPerDay = 8; // Number of hours of light needed per day - should be 16, but setting to 8 for demo
int lightThreshold = 600; // Threshold for light intensity required for plant
int lightStart = 6; // Time to start lighting plant
int lightR = 250; // RGB value for red
int lightG = 0; // RGB value for green
int lightB = 83; // RGB value for blue


// Helper variables for control system
int numHoursLightToday = 0;
int daysPassedSinceLastWater = 999;
int lastHour;
int lastDay;

// Sensor readings
int lightReading;
int moistureReading;

// Initialize vars for reading Serial input
String input;
int split;
String command;
String comm_val;

void loop() {
  // In case device disconnects from WiFi
  if (status != WL_CONNECTED) {
    while (status != WL_CONNECTED) {
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
  
  WiFiClient client = server.available();

  // Read inputs from user first
  if (Serial.available()) {
    // Parse input command
    input = Serial.readStringUntil('\n');
    split = input.indexOf(' ');
    command = input.substring(0, split);
    comm_val = input.substring(split+1);
      
    if (command.equals("time")) {
      set_time(comm_val);
    }
    else if (command.equals("day")) {
      set_day(comm_val);
    }
    else if (command.equals("currtime")) {
      printTime();
    }
    else if (command.equals("pump")) {
      pumpMl(comm_val.toInt());
    }
    else {
      Serial.println("Invalid command");
    }
  }
  
  if (client) {            
    String currentLine = "";
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client, 
        char c = client.read();             // read a byte, then
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:  
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Access-Control-Allow-Origin: *");
            client.println("");
            // client.println("Server: Arduino");
            // client.println("Connection: close");
            // client.println();
            client.println("{\"Light Level\": " + String(analogRead(lightSensor)) + ", \"Moisture Level\": " + String(readMoisture()) + "}");
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
              else if (currentLine.indexOf("/setR") > 0) {
                int index = currentLine.indexOf("etR/");
                int indexHTTP = currentLine.indexOf("HTTP/1.1");
                lightR = currentLine.substring(index + 4, indexHTTP).toInt();
                Serial.println("Red value of RGB set to " + String(lightR));
              }
              else if (currentLine.indexOf("/setG") > 0) {
                int index = currentLine.indexOf("etG/");
                int indexHTTP = currentLine.indexOf("HTTP/1.1");
                lightG = currentLine.substring(index + 4, indexHTTP).toInt();
                Serial.println("Green value of RGB set to " + String(lightG));
              }
              else if (currentLine.indexOf("/setB") > 0) {
                int index = currentLine.indexOf("etB/");
                int indexHTTP = currentLine.indexOf("HTTP/1.1");
                lightB = currentLine.substring(index + 4, indexHTTP).toInt();
                Serial.println("Blue value of RGB set to " + String(lightB));
              }
              else if (currentLine.indexOf("/setWaterAmt") > 0) {
                int index = currentLine.indexOf("Amt/");
                int indexHTTP = currentLine.indexOf("HTTP/1.1");
                waterAmount = currentLine.substring(index + 4, indexHTTP).toInt();
                Serial.println("Water amount set to " + String(waterAmount) + " mL");
              }
              else if (currentLine.indexOf("/setWaterInt") > 0) {
                int index = currentLine.indexOf("Int/");
                int indexHTTP = currentLine.indexOf("HTTP/1.1");
                waterInterval = currentLine.substring(index + 4, indexHTTP).toInt();
                Serial.println("Water interval set to " + String(waterInterval) + " days");
              }
              else if (currentLine.indexOf("/setMoistThresh") > 0) {
                int index = currentLine.indexOf("esh/");
                int indexHTTP = currentLine.indexOf("HTTP/1.1");
                moistureThreshold = currentLine.substring(index + 4, indexHTTP).toInt();
                Serial.println("Moisture threshold set to " + String(moistureThreshold));
              }
              else if (currentLine.indexOf("/setLightHours") > 0) {
                int index = currentLine.indexOf("urs/");
                int indexHTTP = currentLine.indexOf("HTTP/1.1");
                lightHoursPerDay = currentLine.substring(index + 4, indexHTTP).toInt();
                Serial.println("Light hours per day set to " + String(lightHoursPerDay) + " hours");
              }
              else if (currentLine.indexOf("/setLightThresh") > 0) {
                int index = currentLine.indexOf("esh/");
                int indexHTTP = currentLine.indexOf("HTTP/1.1");
                lightThreshold = currentLine.substring(index + 4, indexHTTP).toInt();
                Serial.println("Light threshold set to " + String(lightThreshold));
              }
              else if (currentLine.indexOf("/setLightStart") > 0) {
                int index = currentLine.indexOf("art/");
                int indexHTTP = currentLine.indexOf("HTTP/1.1");
                lightStart = currentLine.substring(index + 4, indexHTTP).toInt();
                Serial.println("Light start set to " + String(lightStart));
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
          for (int i = 0; i <= NUM_LEDS-1; i++) {
            leds[i] = CRGB::Red;
          }
          FastLED.show();
        }
        if (currentLine.endsWith("GET /ledG")) {
          for (int i = 0; i <= NUM_LEDS-1; i++) {
            leds[i] = CRGB::Green;
          }
          FastLED.show();
        }
        if (currentLine.endsWith("GET /ledB")) {
          for (int i = 0; i <= NUM_LEDS-1; i++) {
            leds[i] = CRGB::Blue;
          }
          FastLED.show();
        }
        if (currentLine.endsWith("GET /ledOff")) {
          for (int i = 0; i <= NUM_LEDS-1; i++) {
            leds[i] = CRGB(0,0,0);
          }
          FastLED.show();
        }
        if (currentLine.endsWith("GET /pumpVar50")) {
          pumpMl(50);
        }
        if (currentLine.endsWith("GET /pumpVar100")) {
          pumpMl(100);
        }
        if (currentLine.endsWith("GET /pumpVar300")) {
          pumpMl(300);
        }
        if (currentLine.endsWith("GET /autoOn")) {
          autoVar = true;
        }
        if (currentLine.endsWith("GET /autoOff")) {
          autoVar = false;
          // Turn off LEDs and pump 
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
  }

  if (autoVar) {
    // Retrieve time, if WiFi is connected. If not, use time on clock
    int currHour = rtc.getHours();
    int currMin = rtc.getMinutes();
    int currSec = rtc.getSeconds();
    int currDay = rtc.getDay();

    // Read sensors
    lightReading = analogRead(lightSensor);
    moistureReading = readMoisture();

    // Temporarily print readings to screen instead of logging
    Serial.println("****** SENSOR READINGS ******");
    Serial.println("Light level: " + String(lightReading) + ", Moisture level: " + String(moistureReading) + "\n");

    Serial.println("****** CONTROL SYSTEM READINGS ******");
    printTime();
    Serial.println("daysPassedSinceLastWater: " + String(daysPassedSinceLastWater) + "\n");

    // Perform dailies between 12:00AM and 12:30AM
    if (lastDay != currDay) {
      numHoursLightToday = 0;
      daysPassedSinceLastWater += 1;
      lastDay = currDay;
    }

    if (currHour >= lightStart && currHour < lightStart + lightHoursPerDay) {
      if (lightReading < lightThreshold) {
        for (int i = 0; i <= NUM_LEDS-1; i++) {
          leds[i] = CRGB(lightR,lightG,lightB);
        }
        FastLED.show();
      }
      else {
        for (int i = 0; i <= NUM_LEDS-1; i++) {
          leds[i] = CRGB(0,0,0);
        }
        FastLED.show();
      }
    }
    if (currHour >= lightStart + lightHoursPerDay || currHour < lightStart) {
      for (int i = 0; i <= NUM_LEDS-1; i++) {
        leds[i] = CRGB(0,0,0);
      }
      FastLED.show();
    }

    // If time is between 5AM to 10AM, and the sufficient number of days have passed since last watering, and the moisture reading is below threshold, water
    if (currHour >= 6 && currHour <= 10 && daysPassedSinceLastWater >= waterInterval && moistureReading > moistureThreshold) {
      pumpMl(waterAmount);
      daysPassedSinceLastWater = 0;
    }
    delay(checkInterval * 1000);
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

// Input is an int representing new time: HHMM
void set_time(String input) {
  String setHour = input.substring(0,2);
  String setMin = input.substring(2);

  // Set the time
  rtc.setHours(setHour.toInt());
  rtc.setMinutes(setMin.toInt());
}

// Input is an int representing new time: HHMM
void set_day(String input) {
  // Set the time
  rtc.setDay(input.toInt());
}

void print2digits(int number) {
  if (number < 10) {
    Serial.print("0"); // print a 0 before if the number is < than 10
  }
  Serial.print(number);
}

void printTime() {
  Serial.println("");
  Serial.print("Date: ");
  print2digits(rtc.getMonth()); //retrieve day 
  Serial.print("/");
  print2digits(rtc.getDay()); //retrieve month
  Serial.print("/");
  print2digits(rtc.getYear()); //retrieve year

  Serial.println("");
  Serial.print("Time: ");
  print2digits(rtc.getHours()); //retrieve hours
  Serial.print(":");
  print2digits(rtc.getMinutes()); //retrieve minutes
  Serial.print(":");
  print2digits(rtc.getSeconds()); //retrieve seconds
  Serial.print("\n");
}

void pumpMl(int ml) {
  digitalWrite(pump, LOW);
  delay(ml*32);
  digitalWrite(pump, HIGH);
}

int readMoisture() {
	digitalWrite(waterMoistActivate, HIGH);	// Turn the sensor ON
	delay(1000);							// wait 10 milliseconds
	moistureVal = analogRead(waterMoisture);		// Read the analog value form sensor
	// int percentageHumidity = map(moistureVal, wet, dry, 100, 0); //Convert reading into percentage
  // Serial.println("Raw moisture: " + String(moistureVal));
  digitalWrite(waterMoistActivate, LOW);		// Turn the sensor OFF
	return moistureVal;							// send current reading
}