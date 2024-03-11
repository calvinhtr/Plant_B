#include <FastLED.h>

#define LED_PIN 1
#define NUM_LEDS 9
int lightSensor = A6;
int waterMoisture = A4;
int waterMoistActivate = 14;

CRGB leds[NUM_LEDS];

void setup() {
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
}

void loop() {
  // digitalWrite(1, HIGH);
  // delay(1000);
  // digitalWrite(1,LOW);
  // delay(1000);
  // put your main code here, to run repeatedly:
  for (int i = 0; i <= NUM_LEDS-1; i++) {
    leds[i] = CRGB(50,0,50);
  }
  // leds[0] = CRGB(255,0,0); 
  FastLED.show();
  int lightReading = analogRead(lightSensor);
  Serial.println(String(lightReading));

}

int readMoisture() {
	digitalWrite(waterMoistActivate, HIGH);	// Turn the sensor ON
	delay(10);							// wait 10 milliseconds
	moistureVal = analogRead(waterMoisture);		// Read the analog value form sensor
	int percentageHumidity = map(moistureVal, wet, dry, 100, 0); //Convert reading into percentage
  
  digitalWrite(waterLevelActivate, LOW);		// Turn the sensor OFF
	return percentageHumidity;							// send current reading
}

