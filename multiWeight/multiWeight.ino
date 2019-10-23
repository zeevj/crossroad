#include <FastLED.h>
#include "HX711-multi.h"
#include "effects.cpp"

#define CLK_PIN 27 // clock pin to the load cell amp
#define TARE_TIMEOUT_SECONDS 4
byte DOUTS[] = {18}; //, 10, 11, 12, 13, 14, 15}; //data from each pressure amplifier
#define CHANNEL_COUNT sizeof(DOUTS) / sizeof(byte)
long int results[CHANNEL_COUNT];

#define NUM_LEDS 150
#define DATA_PIN 5

#define RUN_FOR_N_MILLISECONDS(N) for (uint32_t start = millis(); (millis() - start) < N;)

CRGB leds[NUM_LEDS];
int led = 2;

Step steps[CHANNEL_COUNT];

//////// THREADS

QueueHandle_t queue_1;

TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;

Effects effect = Effects(leds, CHANNEL_COUNT);
int effectNum = 0;

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
    steps[j].highThreshold = 3000;
    steps[j].lowThreshold = -3000;
  }

  const int numberOfSamples = 80 * 4;
  const float extraThresh = 0.1;

  for (int i = 0; i < numberOfSamples; i++)
  {
    scales->read(results);
    int lastResult;

    for (int j = 0; j < scales->get_count(); j++)
    {
      int scaleResult = -results[j];

      if (abs(scaleResult - lastResult) > 10000) {
        // maybe its a spike, dont calculate it in the threshold
      } else {
        if (steps[j].highThreshold < scaleResult) {
          steps[j].highThreshold = scaleResult + (scaleResult * extraThresh);
        }

        if (steps[j].lowThreshold > scaleResult) {
          steps[j].lowThreshold = scaleResult - (scaleResult * extraThresh);
        }
        steps[j].histeressis = abs(steps[j].highThreshold - steps[j].lowThreshold) * 0.25;
      }
      lastResult = scaleResult;
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

void detectSteps(HX711MULTI *scales)
{
  for (int i = 0; i < scales->get_count(); ++i)
  {

    float scaleResult = float(-results[i]);
    float lerpVal = 0.6;
    if (abs(scaleResult - steps[i].stepsValue) > 10000)
    {
      lerpVal = 0.99;
    }
    //    Serial.println(abs(scaleResult - stepsValue[i]) );
    steps[i].stepsValue = steps[i].stepsValue * lerpVal + scaleResult * (1 - lerpVal);

    float avgLerp = 0.999;
    steps[i].stepsAvgValue = steps[i].stepsAvgValue * avgLerp + steps[i].stepsValue * (1 - avgLerp);

    if (!steps[i].stepDetected)
    {
      if (steps[i].stepsValue > steps[i].highThreshold || steps[i].stepsValue < steps[i].lowThreshold)
      {
        steps[i].stepDetected = true;
        steps[i].stepsCounter++;
      }
    }
    else
    {
      if (steps[i].stepsValue < (steps[i].highThreshold - (steps[i].histeressis)) && steps[i].stepsValue > (steps[i].lowThreshold + (steps[i].histeressis)))
      {
        steps[i].stepDetected = false;
      }
    }

    //
    //    Serial.print( stepsValue[i] + i * 20);
    printBorders(i);

    //    Serial.print( (i != scales->get_count() - 1) ? "\t" : "\n");
  }
}

const int stepModes = 3;

void printBorders(int i)
{
  Serial.print(steps[i].stepsValue);
  Serial.print("\t");
  Serial.print(steps[i].highThreshold);
  Serial.print("\t");
  Serial.print(steps[i].highThreshold - steps[i].histeressis);
  Serial.print("\t");
  Serial.print(steps[i].lowThreshold);
  Serial.print("\t");
  Serial.print(steps[i].stepsAvgValue);
  Serial.print("\t");
  Serial.println(steps[i].lowThreshold + steps[i].histeressis);
}

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

const uint maxTokens = 10;

String tokens[maxTokens];

void processTokens(int numOfTokens, Parameters *params)
{
  Serial.print("token 0:");
  Serial.println(tokens[0]);
  if (numOfTokens > 1)
  {
    if (!tokens[0].equals("ef"))
    {
      return;
    }
  }

  //ef,effect_number,turn_on,time,reg,green,blue
  //ef,5,1,100,255,0,0
  effectNum = tokens[1].toInt();
  params->setInterval(tokens[3].toInt());
  params->setColor(CRGB(tokens[4].toInt(), tokens[5].toInt(), tokens[6].toInt()));

  for (int i = 1; i <= numOfTokens; i++)
  {
    Serial.print("token ");
    Serial.print(i);
    Serial.print(":");
    Serial.println(tokens[i]);
  }

  params->setStartTime(millis());
  params->setCurrentTime(millis());
}

void Task1code(void *pvParameters)
{
  pinMode(led, OUTPUT);
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
  int counter = 0;

  Parameters params = Parameters();
  params.setInterval(100);
  params.setColor(CRGB(0, 0, 255));

  const int FPS = 60;
  const unsigned long frameTimeIntervalMs = 1000 / FPS;
  unsigned long currentFrameTimeMs = 0;

  for (;;)
  {

    uint8_t data = 0;
    bool didGottMsg = false;
    if (xQueueReceive(queue_1, &data, (TickType_t)0) == pdPASS)
    {
      //digitalWrite(led, HIGH);
      didGottMsg = true;
      for (int i = 0; i < CHANNEL_COUNT; i++)
      {
        bool isStepOn = bitRead(data, i); //(i == counter); //
        if (isStepOn && (millis() > steps[i].hideEffectUntilTime))
        {
          steps[i].hideEffectUntilTime = millis() + 500;
          //Serial.print(" hideEffectUntilTime - ");
          //Serial.println(steps[i].hideEffectUntilTime);
        }
      }
      counter = (counter + 1) % 8;
    }

    String inData;
    int numberOfTokens = 0;

    //ef,effect_number,turn_on,time,reg,green,blue
    //ef,5,1,100,255,0,0
    while (Serial.available() > 0)
    {

      char recieved = Serial.read();

      // Process message when new line character is recieved
      if (recieved == ',')
      {
        tokens[numberOfTokens] = inData;
        tokens[numberOfTokens].replace(" ", "");
        inData = "";
        numberOfTokens++;
      }
      else if (recieved == '\n')
      {
        tokens[numberOfTokens] = inData;
        tokens[numberOfTokens].replace(" ", "");
        Serial.print("got message with: ");
        Serial.print(numberOfTokens);
        Serial.println(" tokens");

        processTokens(numberOfTokens, &params);
        inData = ""; // Clear recieved buffer
        numberOfTokens = 0;
      }
      else
      {
        inData += recieved;
      }
    }

    if (!didGottMsg)
    {
      effect.runEffect(effectNum, &params);
    }

    if (millis() > (currentFrameTimeMs + frameTimeIntervalMs))
    {
      currentFrameTimeMs = millis();
      FastLED.show();
      digitalWrite(led, LOW);
    }
  }
}

void Task2code(void *pvParameters)
{
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  HX711MULTI scales(CHANNEL_COUNT, DOUTS, CLK_PIN);

  //TODO --
  tare(&scales);
  calibrateThresholds(&scales);

  int cnt = 0;

  const bool mockStepDetection = false;

  uint8_t prevData = 0;

  for (;;)
  {
    scales.read(results);
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
      detectSteps(&scales);
    }

    uint8_t data = 1 << cnt;

    for (int i = 0; i < CHANNEL_COUNT; i++)
    {
      bitWrite(data, i, steps[i].stepDetected);
    }

    if (prevData != data) {
      xQueueSend(queue_1, &data, portMAX_DELAY);
    }

    cnt = (cnt + 1) % 7;

    const TickType_t xDelay = (1000 / 80) / portTICK_PERIOD_MS;
    vTaskDelay(xDelay);
    prevData = data;
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
  steps[0].toLed = 67;

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
  effect.setSteps(steps,CHANNEL_COUNT);

  Serial.begin(115200);
  Serial.flush();

  queue_1 = xQueueCreate(1, sizeof(uint8_t));

  startThreads();

  //uint8_t data = 0b00000011;
  // xQueueSend(queue_1, &data, portMAX_DELAY);
}

void loop()
{
}
