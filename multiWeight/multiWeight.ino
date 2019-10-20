#include <FastLED.h>
#include "HX711-multi.h"
#include "effects.cpp"

#define CLK 8 // clock pin to the load cell amp
#define TARE_TIMEOUT_SECONDS 4
byte DOUTS[] = {9, 10, 11, 12, 13, 14, 15}; //data from each pressure amplifier
#define CHANNEL_COUNT sizeof(DOUTS) / sizeof(byte)
long int results[CHANNEL_COUNT];

#define NUM_LEDS 150
#define DATA_PIN 5

#define RUN_FOR_N_MILLISECONDS(N) for (uint32_t start = millis(); (millis() - start) < N;)

CRGB leds[NUM_LEDS];

Step steps[CHANNEL_COUNT];

//////// THREADS

QueueHandle_t queue_1;

TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;

Effects e = Effects(leds);
Parameters p = Parameters(); //FIXME
    

void tare(HX711MULTI *scales)
{
  bool tareSuccessful = false;
  unsigned long tareStartTime = millis();
  while (!tareSuccessful && millis() < (tareStartTime + TARE_TIMEOUT_SECONDS * 1000))
  {
    tareSuccessful = scales->tare(20, 10000); //reject 'tare' if still ringing
  }
}

void calibrateThresholds(HX711MULTI *scales)
{
  for (int j = 0; j < scales->get_count(); j++)
  {
    steps[j].highThreshold = -10000;
    steps[j].lowThreshold = 10000;
  }

  const int numberOfSamples = 80 * 4;
  const float extraThresh = 0.4;

  for (int i = 0; i < numberOfSamples; i++)
  {
    scales->read(results);
    for (int j = 0; j < scales->get_count(); j++)
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
    }

    delay(1000 / 80);
  }
}

void detectSteps(HX711MULTI *scales)
{
  for (int i = 0; i < scales->get_count(); ++i)
  {
    int scaleResult = -results[i];
    if (!steps[i].stepDetected)
    {
      if (scaleResult > steps[i].highThreshold || scaleResult < steps[i].lowThreshold)
      {
        steps[i].stepDetected = true;
        steps[i].stepsCounter++;
      }
    }
    else
    {
      if (scaleResult < steps[i].highThreshold - steps[i].histeressis && scaleResult > steps[i].lowThreshold + steps[i].histeressis)
      {
        steps[i].stepDetected = false;
      }
    }
    //
    Serial.print(scaleResult + i * 20);
    Serial.print((i != scales->get_count() - 1) ? "\t" : "\n");
  }
}

const int stepModes = 3;

void renderLeds(HX711MULTI *scales)
{
  for (int i = 0; i < scales->get_count(); ++i)
  {
    if (steps[i].stepDetected)
    {
      CRGB cl;
      int stp = steps[i].stepsCounter % stepModes;

      switch (stp)
      {
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

      for (int j = steps[i].fromLed; j < steps[i].toLed; j++)
      {
        leds[j] = cl;
      }
    }
    else
    {
      for (int j = steps[i].fromLed; j < steps[i].toLed; j++)
      {
        leds[j] = CRGB::Black;
      }
    }
  }
  FastLED.show();
}

void Task1code(void *pvParameters)
{
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
  int counter = 0;
  int a[] = {0, 1};

p.setInterval(100);
p.setColor(CRGB::Red);
  
  const int FPS = 30;
  const unsigned long frameTimeIntervalMs = 1000 / FPS;
  unsigned long currentFrameTimeMs = 0;

  for (;;)
  {

    uint8_t data = 0;
    /*
    if (xQueueReceive(queue_1, &data, portMAX_DELAY) == pdPASS) {
      FastLED.clear();
      for (int i = 0; i < CHANNEL_COUNT; i++) {
        bool isStepOn = bitRead(data, i); //(i == counter); //
        for (int led = steps[i].fromLed; led < steps[i].toLed; led++)
        {
          leds[led] = (isStepOn ? CRGB::Red : CRGB::Black );
        }
      }
      counter = (counter + 1) % 8;
      FastLED.show();
    }
    */

  /*  if (Serial.available())
    {
      char inByte = ' ';
      inByte = Serial.read(); // read the incoming data
      Serial.println(inByte);
      if (inByte == '1')
      {
      }
    } */

    e.sinelon(&p);


    if (millis() > (currentFrameTimeMs + frameTimeIntervalMs))
    {
      currentFrameTimeMs = millis();
      FastLED.show();
    }
  }
}

void Task2code(void *pvParameters)
{
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  ///HX711MULTI scales(CHANNEL_COUNT, DOUTS, CLK);

  //TODO --
  //tare();
  //calibrateThresholds();

  int cnt = 0;

  const bool mockStepDetection = true;

  for (;;)
  {
    //scales.read(results);
    //detectSteps();
    /*
      0b00000001 - first step is on
      0b00010001 - first and fifth are on
      0b01111111 - all are on
    */

    //fake step detection

    if (mockStepDetection)
    {
      for (int i = 0; i < CHANNEL_COUNT; i++)
      {
        steps[i].stepDetected = (cnt == i);
      }
    }
    else
    {
      ///detectSteps(&scales);
    }

    uint8_t data = 1 << cnt;
    for (int i = 0; i < CHANNEL_COUNT; i++)
    {
      bitWrite(data, i, steps[i].stepDetected);
    }
    xQueueSend(queue_1, &data, portMAX_DELAY);

    cnt = (cnt + 1) % 7;

    const TickType_t xDelay = 100 / portTICK_PERIOD_MS;
    vTaskDelay(xDelay);
  }
}

void Task3code(void *pvParameters)
{
}

void startThreads()
{

  xTaskCreatePinnedToCore(
      Task1code, /* Task function. */
      "Task1",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task1,    /* Task handle to keep track of created task */
      0          /* pin task to core 0 */
  );

  xTaskCreatePinnedToCore(
      Task2code, /* Task function. */
      "Task2",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Task2,    /* Task handle to keep track of created task */
      1          /* pin task to core 1 */
  );

  // xTaskCreatePinnedToCore(
  //     Task3code, /* Task function. */
  //     "Task3",   /* name of task. */
  //     10000,     /* Stack size of task */
  //     NULL,      /* parameter of the task */
  //     1,         /* priority of the task */
  //     &Task3,    /* Task handle to keep track of created task */
  //     1          /* pin task to core 1 */
  // );
}

//================== Step Detection =============

void initSteps()
{

  steps[0].fromLed = 0;
  steps[0].toLed = 10;

  steps[1].fromLed = 11;
  steps[1].toLed = 20;

  steps[2].fromLed = 21;
  steps[2].toLed = 40;

  steps[3].fromLed = 41;
  steps[3].toLed = 75;

  steps[4].fromLed = 76;
  steps[4].toLed = 95;

  steps[5].fromLed = 96;
  steps[5].toLed = 120;

  steps[6].fromLed = 121;
  steps[6].toLed = 150;
}

void setup()
{
  initSteps();
  e.setSteps(steps);

  Serial.begin(115200);
  Serial.flush();

  queue_1 = xQueueCreate(1, sizeof(uint8_t));

  startThreads();

  uint8_t data = 0b00000011;
  xQueueSend(queue_1, &data, portMAX_DELAY);
}

void loop()
{
}
