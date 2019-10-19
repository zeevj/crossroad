#include <FastLED.h>

#define TOTAL_NUM_LEDS 150
#define STEP_NUM 7
#define FRAMES_PER_SECOND 120
#define MAX_BRIGHTNESS 255
#define MIN_BRIGHTNESS 0
#define CLEAR()          \
    FastLED.clear(true); \
    FastLED.show();

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
    fract8 chanceOfGlitter = 255;
    uint32_t delay = (1000 / FRAMES_PER_SECOND);

    int curr_steps = 0;
    int step_incr = 8;

public:
    Effects(CRGB *_leds)
    {
        leds = _leds;
    }

    void setSteps(Step *_steps)
    {
        steps = _steps;
    }

    void lightsBeat(CRGB color, int bpm)
    {
        for (int i = 0; i < TOTAL_NUM_LEDS; i++)
        {
            leds[i] = color;
        }
        FastLED.setBrightness(curr_steps);
        FastLED.show();
        FastLED.delay(bpm);
        curr_steps += step_incr;

        if (curr_steps > MAX_BRIGHTNESS)
        {
            curr_steps = MAX_BRIGHTNESS;
            step_incr *= -1;
        }
        else if (curr_steps < MIN_BRIGHTNESS)
        {
            curr_steps = MIN_BRIGHTNESS;
            step_incr *= -1;
        }
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
        FastLED.delay(duration);
    }

    bool lightBoard(uint8_t data, CRGB color)
    {
        for (int i = 0; i < STEP_NUM; i++)
        {
            bool isLightStep = bitRead(data, i);
            for (int led = steps[i].fromLed; led < steps[i].toLed; led++)
            {
                leds[led] = (isLightStep ? color : CRGB::Black);
            }
        }
    }


    void lightBoardEscelate(CRGB color, int duration)
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

    void lightBoardEscelate(CRGB color, unsigned long durationMs, unsigned long startTimeMs, unsigned long timeStampMs)
    {
        float numFromZeroToOne = float(timeStampMs - startTimeMs) / float(durationMs);
        int stepToLight = int(numFromZeroToOne * STEP_NUM);
        uint8_t step = 0;

        for (int i = 0; i < STEP_NUM; i++) {
            bitWrite(step, i, i <= stepToLight);
        }
        lightBoard(step, color);
    }


    void lightBoardDescelate(CRGB color, int duration)
    {
        int cnt = STEP_NUM - 1;
        uint8_t step = 1 << cnt;
        for (int i = 0; i < STEP_NUM; i++)
        {
            lightBoard(step, color, duration);
            cnt = (cnt - 1) % 7;
            step = 1 << cnt;
        }
    }

    void showLeds()
    {
        FastLED.setBrightness(MAX_BRIGHTNESS);
        // send the 'leds' array out to the actual LED strip
        FastLED.show();
        // insert a delay to keep the framerate modest
        FastLED.delay(delay);
        FastLED.clear(true);
        FastLED.show();
    }

    void bpm(uint8_t BeatsPerMinute)
    {
        // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
        CRGBPalette16 palette = PartyColors_p;
        uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);

        EVERY_N_MILLISECONDS(20) { gHue++; }
        for (int i = 0; i < TOTAL_NUM_LEDS; i++)
        {
            leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
        }
        showLeds();
    }

    void glitter()
    {
        CLEAR()
        if (random8() < chanceOfGlitter)
        {
            for (int i = 0; i < (TOTAL_NUM_LEDS / 7); i++)
            {
                leds[random16(TOTAL_NUM_LEDS)] += CRGB::White;
            }
        }
        showLeds();
    }

    void confetti()
    {
        // random colored speckles that blink in and fade smoothly
        fadeToBlackBy(leds, TOTAL_NUM_LEDS, 10);
        for (int i = 0; i < (TOTAL_NUM_LEDS / 7); i++)
        {
            int pos = random16(TOTAL_NUM_LEDS);
            leds[pos] += CHSV(gHue + random8(64 * i), 200, 255);
        }
        showLeds();
    }

    void juggle()
    {
        // eight colored dots, weaving in and out of sync with each other
        fadeToBlackBy(leds, TOTAL_NUM_LEDS, 20);
        byte dothue = 0;
        for (int i = 0; i < 8; i++)
        {
            leds[beatsin16(i + 7, 0, TOTAL_NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
            dothue += 32;
        }
        showLeds();
    }

    void sinelon(int bpm)
    {
        for (int i = 0; i < STEP_NUM; i++)
        {
            // a colored dot sweeping back and forth, with fading trails
            fadeToBlackBy(leds, TOTAL_NUM_LEDS, 20);
            int pos = beatsin16(bpm, steps[i].fromLed, steps[i].toLed);
            leds[pos] += CHSV(gHue, 255, 192);
        }
        showLeds();
    }

    void runOnLeds(int numOfLeds, int loopTimeMs, int timeStampMs) {
        float numFromZeroToOne = float(timeStampMs % loopTimeMs) / float(loopTimeMs);
        int currentLed = int(numOfLeds * numFromZeroToOne);
        for (int i = 0; i < numOfLeds; i++)
        {
            leds[pos] = (currentLed == i ? CRGB::Red : CRGB::Black);
        }
    }
};
