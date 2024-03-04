#include <FastLED.h>

#define LED_PIN 6
#define NUM_LEDS 60

CRGB leds[NUM_LEDS];

void setup() {
  // pinMode(LED_PIN, OUTPUT);
  // pinMode(1, OUTPUT);
  // put your setup code here, to run once:
  FastLED.addLeds<WS2812, LED_PIN, GRB> (leds, NUM_LEDS);
  for (int i = 0; i <= 59; i++) {
    leds[i] = CRGB(0,0,0);
  }
}

void loop() {
  // digitalWrite(1, HIGH);
  // delay(1000);
  // digitalWrite(1,LOW);
  // delay(1000);
  // put your main code here, to run repeatedly:
  for (int i = 0; i <= 39; i++) {
    leds[i] = CRGB(250,0,75);
  }
  // leds[0] = CRGB(255,0,0); 
  FastLED.show();

}
