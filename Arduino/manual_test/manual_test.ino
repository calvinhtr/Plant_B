#include <FastLED.h>

#define LED_PIN 1
#define NUM_LEDS 9
int lightSensor = A6;
int waterMoisture = A4;
int waterMoistActivate = 12;
int pump =  10;

int moistureVal = 0;
int dry = 520;
int wet = 240;

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(9600);
  // pinMode(LED_PIN, OUTPUT);
  // pinMode(1, OUTPUT);
  // put your setup code here, to run once:
  FastLED.addLeds<WS2812, LED_PIN, GRB> (leds, NUM_LEDS);
  for (int i = 0; i <= NUM_LEDS-1; i++) {
    leds[i] = CRGB(0,0,0);
  }
  // FastLED.addLeds<WS2812, LED_PIN, GRB> (leds, NUM_LEDS);
  // for (int i = 0; i <= NUM_LEDS-1; i++) {
  //   leds[i] = CRGB::Red;
  // }
  FastLED.show();
  pinMode(waterMoistActivate, OUTPUT);
  digitalWrite(waterMoistActivate, LOW);
  pinMode(pump, OUTPUT);
  digitalWrite(pump, HIGH);
  pinMode(waterMoisture, INPUT);
}

void loop() {
  digitalWrite(pump, LOW);
  Serial.println("ON");
  delay(1000);
  
  digitalWrite(pump,HIGH);
  Serial.println("OFF");
  delay(1000);
  // Serial.println(String(analogRead(lightSensor)));
  // put your main code here, to run repeatedly:
  // for (int i = 0; i <= NUM_LEDS-1; i++) {
  //   leds[i] = CRGB(50,0,50);
  // } 
  // FastLED.show();
  // int lightReading = analogRead(lightSensor);
  // Serial.println("Light: " + String(lightReading));
  // int moistureReading = readMoisture();
  // Serial.println("Moisture: " + String(moistureReading));
  // delay(1000);

  
  // digitalWrite(waterMoistActivate, HIGH);	// Turn the sensor ON
  // delay(10);
  // moistureVal = analogRead(waterMoisture);		// Read the analog value form sensor
  
  // digitalWrite(waterMoistActivate, LOW);
  // int percentageHumidity = map(moistureVal, wet, dry, 100, 0); //Convert reading into percentage
  // Serial.println("Raw moisture: " + String(percentageHumidity));
  // delay(1000);
}

int readMoisture() {
	digitalWrite(waterMoistActivate, HIGH);	// Turn the sensor ON
	delay(1000);							// wait 10 milliseconds
	moistureVal = analogRead(waterMoisture);		// Read the analog value form sensor
  Serial.println("Raw moisture: " + String(moistureVal));
	int percentageHumidity = map(moistureVal, wet, dry, 100, 0); //Convert reading into percentage
  
  digitalWrite(waterMoistActivate, LOW);		// Turn the sensor OFF
	return percentageHumidity;							// send current reading
}

