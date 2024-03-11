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

// Define calibration values
int val = 0;
int moistureVal = 0;
int dry = 794;
int wet = 305;

String input;
int split;
String command;
String comm_val;

RTCZero rtc;
/* Change these values to set the current initial time */
const byte secInit = 0;
const byte minInit = 0;
const byte hourInit = 0;

/* Change these values to set the current initial date */
const byte dayInit = 10;
const byte monthInit = 3;
const byte yearInit = 24;

void setup() {
  // put your setup code here, to run once:
  pinMode(pump, OUTPUT);
  pinMode(waterMoistActivate, OUTPUT);
  pinMode(lightSensor, INPUT);
  pinMode(waterMoisture, INPUT);

  // Initialize pump and power to water moisture sensor to off
  digitalWrite(pump, HIGH);
  digitalWrite(waterMoistActivate, HIGH);

  FastLED.addLeds<WS2812, LED_PIN, GRB> (leds, NUM_LEDS);

  // Initialize time
  rtc.begin();
  // Set the time
  rtc.setHours(hourInit);
  rtc.setMinutes(minInit);
  rtc.setSeconds(secInit);

  // Set the date
  rtc.setDay(dayInit);
  rtc.setMonth(monthInit);
  rtc.setYear(yearInit);

  // Connect to WIFI
  // while ( status != WL_CONNECTED) {
  //   Serial.print("Attempting to connect to SSID: ");
  //   Serial.println(ssid);
  //   // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  //   status = WiFi.begin(ssid, pass);
  //   // wait 10 seconds for connection:
  //   delay(10000);
  // }
  Serial.begin(9600); 
}

// Default control system values (changeable via user input)
int waterAmount = 50; // Amount of water to dispense per watering session in mL
int waterInterval = 2; // Number of days before watering again - should be 7 but setting to 2 for demo
int waterThreshold = 300; // The moisture level which will trigger watering
int checkInterval = 5; // How long between loops. Currently in seconds (1 second = 1 hour). Change later to be every 15/30 minutes
int lightHoursPerDay = 8; // Number of hours of light needed per day - should be 16, but setting to 8 for demo
int lightThreshold = 600; // Threshold for light intensity required for plant
int lightR = 150; // RGB value for red
int lightG = 0; // RGB value for green
int lightB = 30; // RGB value for blue
int startLight = 6; // Time to start lighting plant

// Helper variables for control system
int numHoursLightToday = 0;
int daysPassedSinceLastWater = 999;
int lastHour;
int lastDay;

// Sensor readings
int lightReading;
int moistureReading;

void loop() {
  // Read inputs from user first
  if(Serial.available()){
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

  if (currHour >= startLight && currHour < startLight + lightHoursPerDay) {
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
  if (currHour >= startLight + lightHoursPerDay || currHour < startLight) {
    for (int i = 0; i <= NUM_LEDS-1; i++) {
        leds[i] = CRGB(0,0,0);
      }
      FastLED.show();
  }

  // If time is between 5AM to 10AM, and the sufficient number of days have passed since last watering, and the moisture reading is below threshold, water
  if (currHour >= 6 && currHour <= 10 && daysPassedSinceLastWater >= waterInterval && moistureReading > waterThreshold) {
    pumpMl(waterAmount);
    daysPassedSinceLastWater = 0;
  }
  delay(checkInterval * 1000);
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
	delay(2000);							// wait 10 milliseconds
	moistureVal = analogRead(waterMoisture);		// Read the analog value form sensor
	// int percentageHumidity = map(moistureVal, wet, dry, 100, 0); //Convert reading into percentage
  // Serial.println("Raw moisture: " + String(moistureVal));
  digitalWrite(waterMoistActivate, LOW);		// Turn the sensor OFF
	return moistureVal;							// send current reading
}