#include <FastLED.h>
#define NUM_LEDS 150
#define DATA_PIN 3
CRGB leds[NUM_LEDS];


#include "HX711-multi.h"


int highT = 4000;
int lowT = -4000;
int hist = 2000;

int stepCounter = 0;

int ledLight = 13;

bool isOn = false;
bool pIsOn = false;

// Pins to the load cell amp
#define CLK 8      // clock pin to the load cell amp
//#define DOUT1 9    // data pin to the first lca
//#define DOUT2 9    // data pin to the second lca
//#define DOUT3 9    // data pin to the third lca

#define BOOT_MESSAGE "MIT_ML_SCALE V0.8"

#define TARE_TIMEOUT_SECONDS 4

byte DOUTS[] = {9,10}; //,9,9,9,9

#define CHANNEL_COUNT sizeof(DOUTS)/sizeof(byte)

long int results[CHANNEL_COUNT];

HX711MULTI scales(CHANNEL_COUNT, DOUTS, CLK);

void setup() {
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  Serial.begin(115200);
  //  Serial.println(BOOT_MESSAGE);
  Serial.flush();
  pinMode(11, OUTPUT);
  pinMode(ledLight, OUTPUT);

  tare();
}


void tare() {
  bool tareSuccessful = false;

  unsigned long tareStartTime = millis();
  while (!tareSuccessful && millis() < (tareStartTime + TARE_TIMEOUT_SECONDS * 1000)) {
    tareSuccessful = scales.tare(20, 10000); //reject 'tare' if still ringing
  }
}

void sendRawData() {
  scales.read(results);
  for (int i = 0; i < scales.get_count(); ++i) {
    ;

    if (!isOn) {
      if (-results[i] > highT || -results[i] < lowT) {
        isOn = true;
        stepCounter++;
      }
    } else {
      if (-results[i] < highT - hist && -results[i] > lowT + hist ) {
        isOn = false;
      }
    }




    Serial.print( -results[i] + i * 20);
    Serial.print( (i != scales.get_count() - 1) ? "\t" : "\n");


    //    digitalWrite(ledLight, isOn);

  }
  //  delay(10);
}

void loop() {

  sendRawData(); //this is for sending raw data, for where everything else is done in processing

  //  //on serial data (any data) re-tare
  //  if (Serial.available()>0) {
  //    while (Serial.available()) {
  //      Serial.read();
  //      Serial.print("tareee\n");
  //    }
  //    tare();
  //  }

  //    delay(100);

  if (isOn) {
    
    CRGB cl;
    int num = stepCounter % 3;
    if (num == 0) {
        cl = CRGB::Red;
      } else if (num == 1) {
        cl = CRGB::Green;
      } else if (num == 2) {
        cl = CRGB::Blue;
      }
      
    for (int i = 11; i < NUM_LEDS; i++) {
      leds[i] = cl;

    }

    FastLED.show();

  } else {
    for (int i = 11; i < NUM_LEDS; i++) {

      leds[i] = CRGB::Black;

    }

    FastLED.show();
  }

}
