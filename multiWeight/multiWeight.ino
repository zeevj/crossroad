//https://cdn.sparkfun.com/datasheets/Sensors/ForceFlex/hx711_english.pdf

//================== HX711-multi =============
#include "HX711-multi.h"
#define CLK 8      // clock pin to the load cell amp
#define TARE_TIMEOUT_SECONDS 4
byte DOUTS[] = {9,10}; //data from each pressure amplifier 
#define CHANNEL_COUNT sizeof(DOUTS)/sizeof(byte)
long int results[CHANNEL_COUNT];
HX711MULTI scales(CHANNEL_COUNT, DOUTS, CLK);

//================== Fast LED =============
#include <FastLED.h>
#define NUM_LEDS 150 * 2
#define DATA_PIN 3
CRGB leds[NUM_LEDS];

struct Step {
  int fromLed;
  int toLed;
};

Step steps[CHANNEL_COUNT];


//================== Step Detection =============
int highThreshold = 4000;
int lowThreshold = -4000;
int histeressis = 2000;

bool stepsDetected[CHANNEL_COUNT];
bool stepsCounter[CHANNEL_COUNT];
const int stepModes = 3;

void setup() {


  steps[0].fromLed = 11;
  steps[0].toLed = 150;

  steps[1].fromLed = 160;
  steps[1].toLed = 220;

  Serial.begin(115200);
  Serial.flush();
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
  tare();
}

void tare() {
  bool tareSuccessful = false;
  unsigned long tareStartTime = millis();
  while (!tareSuccessful && millis() < (tareStartTime + TARE_TIMEOUT_SECONDS * 1000)) {
    tareSuccessful = scales.tare(20, 10000); //reject 'tare' if still ringing
  }
}

void detectSteps() {
  for (int i = 0; i < scales.get_count(); ++i) {
    int scaleResult = -results[i];
    if (!stepsDetected[i]) {
      if (scaleResult > highThreshold || scaleResult < lowThreshold) {
        stepsDetected[i] = true;
        stepsCounter[i]++;
      }
    } else {
      if (scaleResult < highThreshold - histeressis && scaleResult > lowThreshold + histeressis ) {
        stepsDetected[i] = false;
      }
    }

    Serial.print( scaleResult + i * 20);
    Serial.print( (i != scales.get_count() - 1) ? "\t" : "\n");
  }
}

void renderLeds() {
  for (int i = 0; i < scales.get_count(); ++i) {
    if (stepsDetected[i]) {
      CRGB cl;
      int stp = stepsCounter[i] % stepModes;

      switch(stp) {
        case 0:
          cl = CRGB::Red;
          break;
        case 1:
          cl = CRGB::Green;
          break;
        case 2:
          cl = CRGB::Blue;
          break;
      }
      
      for (int i = steps[i].fromLed; i < steps[i].toLed; i++) {
        leds[i] = cl;
      }
    } else {
      for (int i = steps[i].fromLed; i < steps[i].toLed; i++) {
        leds[i] = CRGB::Black;
      }
    }
  }
  FastLED.show();
}

void loop() {
  scales.read(results);
  detectSteps();
  renderLeds();
}