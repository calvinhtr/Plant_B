#include <FastLED.h>

#define LED_PIN 7
#define NUM_LEDS 60

CRGB leds[NUM_LEDS];

void setup() {
  // put your setup code here, to run once:
  FastLED.addLeds<WS2812, LED_PIN, GRB> (leds, NUM_LEDS);
  for (int i = 0; i <= 59; i++) {
    leds[i] = CRGB(5,0,0);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  // for (int i = 0; i <= 39; i++) {
  //   leds[i] = CRGB(0,50,0);
  // }
  // leds[0] = CRGB(255,0,0); 
  FastLED.show();
}
