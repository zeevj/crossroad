#include <FastLED.h>

#define TOTAL_NUM_LEDS 150
#define STEP_NUM 7

struct Step
{
    int fromLed;
    int toLed;
    int highThreshold = 4000;
    int lowThreshold = -4000;
    int histeressis = 2000;
    bool stepDetected;
    bool stepsCounter;
};

class Effects
{
private:
    //pointer to leds array from main
    CRGB *leds;
    Step *steps;
    uint8_t gHue = 0;

public:
    Effects(CRGB *_leds)
    {
        leds = _leds;
    }

    void setSteps(Step *_steps)
    {
        steps = _steps;
    }

    void lightBoard(uint8_t data, CRGB color, int duration)
    {
        for (int i = 0; i < STEP_NUM; i++)
        {
            bool isLightStep = bitRead(data, i);
            for (int led = steps[i].fromLed; led < steps[i].toLed; led++)
            {
                leds[led] = (isLightStep ? color : CRGB::Black);
            }
        }
        FastLED.show();
        delay(duration);
    }

    void lightBoardEscelate(CRGB color, int duration, int times)
    {
        for (int round = 0; round < times; round++)
        {
            int cnt = 0;
            uint8_t step = 1 << cnt;
            for (int i = 0; i < STEP_NUM; i++)
            {
                lightBoard(step, color, duration);
                cnt = (cnt + 1) % 7;
                step = 1 << cnt;
            }
        }
    }

    void lightBoardDescelate(CRGB color, int duration, int times)
    {
        for (int round = 0; round < times; round++)
        {
            int cnt = STEP_NUM;
            uint8_t step = 1 << cnt;
            for (int i = 0; i < STEP_NUM; i++)
            {
                lightBoard(step, color, duration);
                cnt = (cnt - 1) % 7;
                step = 1 << cnt;
            }
        }
    }

    void bpm(int a)
    {
        // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
        uint8_t BeatsPerMinute = 62;
        CRGBPalette16 palette = PartyColors_p;
        uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
        for (int i = 0; i < TOTAL_NUM_LEDS; i++)
        {
            leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
        }
    }
};
