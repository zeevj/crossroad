//https://cdn.sparkfun.com/datasheets/Sensors/ForceFlex/hx711_english.pdf

//================== HX711-multi =============
#include "HX711-multi.h"
#define CLK 25      // clock pin to the load cell amp
#define TARE_TIMEOUT_SECONDS 4
byte DOUTS[] = {18}; //data from each pressure amplifier
#define CHANNEL_COUNT sizeof(DOUTS)/sizeof(byte)
long int results[CHANNEL_COUNT];
HX711MULTI scales(CHANNEL_COUNT, DOUTS, CLK);

//================== Fast LED =============
#include <FastLED.h>
#define NUM_LEDS 150 * 2
#define DATA_PIN 5
CRGB leds[NUM_LEDS];

int led = 2;

struct Step {
  int fromLed;
  int toLed;
  int highThreshold = 4000;
  int lowThreshold = -4000;
  int histeressis = 2000;
  bool stepDetected;
  bool stepsCounter;
};

Step steps[CHANNEL_COUNT];


//================== Step Detection =============
//int highThreshold = 4000;
//int lowThreshold = -4000;
//int histeressis = 2000;

bool stepsDetected[CHANNEL_COUNT];
int stepsValue[CHANNEL_COUNT];
int stepsAvgValue[CHANNEL_COUNT];
int stepsCounter[CHANNEL_COUNT];
const int stepModes = 3;



bool isNewValWayOverUnder() {
  //
}

void printBorders(int i) {
      Serial.print( stepsValue[i] );
      Serial.print( "\t" );
      Serial.print(steps[i].highThreshold );
      Serial.print( "\t" );
      Serial.print(steps[i].highThreshold - steps[i].histeressis );
      Serial.print( "\t" );
      Serial.print(steps[i].lowThreshold );
      Serial.print( "\t" );
      Serial.print(stepsAvgValue[i]);
      Serial.print( "\t" );
      Serial.println(steps[i].lowThreshold + steps[i].histeressis );

      
}


void calibrateThresholds()
{
  for (int j = 0; j < scales.get_count(); j++)
  {
    steps[j].highThreshold = 2000;
    steps[j].lowThreshold = -2000;
  }

  const int numberOfSamples = 80 * 4;
  const float extraThresh = 0.1;

  for (int i = 0; i < numberOfSamples; i++)
  {
    scales.read(results);
    for (int j = 0; j < scales.get_count(); j++)
    {
      int scaleResult = -results[j];

      if (steps[j].highThreshold < scaleResult)
      {
        steps[j].highThreshold = scaleResult + (scaleResult * extraThresh);
      }

      if (steps[j].lowThreshold > scaleResult)
      {
        steps[j].lowThreshold = scaleResult - (scaleResult * extraThresh);
      }
      steps[j].histeressis = abs(steps[j].highThreshold - steps[j].lowThreshold) * 0.25;

      printBorders(j);
    }

    delay(1000 / 80);
  }

//  Serial.print("hight threshold: ");
//  Serial.println(steps[0].highThreshold);
//  Serial.print("low threhsold: ");
//  Serial.println(steps[0].lowThreshold);
//  Serial.print("histeressis: ");
//  Serial.println(steps[0].histeressis);

  
}


void setup() {

  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  steps[0].fromLed = 0;
  steps[0].toLed = 124;

  //  steps[0].fromLed = 160;
  //  steps[0].toLed = 220;

  Serial.begin(115200);
  Serial.flush();

  //Serial.println("STTTTTARTING");
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
  tare();
  calibrateThresholds();
  digitalWrite(led, LOW);
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

    float scaleResult = float(-results[i]);
    float lerpVal = 0.6;
    if (abs(scaleResult - stepsValue[i]) > 10000) {
      lerpVal = 0.99;
    }
//    Serial.println(abs(scaleResult - stepsValue[i]) );
    stepsValue[i] = stepsValue[i] * lerpVal + scaleResult * (1 - lerpVal);

    float avgLerp = 0.999;
    stepsAvgValue[i] = stepsAvgValue[i] * avgLerp + stepsValue[i] * (1-avgLerp);

    if (!stepsDetected[i]) {
      if (stepsValue[i] > steps[i].highThreshold || stepsValue[i] < steps[i].lowThreshold   ) {
        stepsDetected[i] = true;
        stepsCounter[i]++;
      }
    } else {
      if (stepsValue[i] < (steps[i].highThreshold - (steps[i].histeressis ) ) && stepsValue[i] > (steps[i].lowThreshold + (steps[i].histeressis  )) ) {
        stepsDetected[i] = false;
      }
    }
    

    //
//    Serial.print( stepsValue[i] + i * 20);
       printBorders(i);
    
//    Serial.print( (i != scales.get_count() - 1) ? "\t" : "\n");
  }
}

void renderLeds() {
  for (int i = 0; i < scales.get_count(); ++i) {
    if (stepsDetected[i]) {
      CRGB cl;
      int stp = stepsCounter[i] % stepModes;

      switch (stp) {
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

      for (int j = steps[i].fromLed; j < steps[i].toLed; j++) {
        leds[j] = cl;
      }
    } else {
      for (int j = steps[i].fromLed; j < steps[i].toLed; j++) {
        leds[j] = CRGB::Black;
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
