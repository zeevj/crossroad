#include <FastLED.h>
#include "parameters.cpp"

#define TOTAL_NUM_LEDS 150
#define STEP_NUM 7
#define FRAMES_PER_SECOND 120
#define MAX_BRIGHTNESS 255
#define MIN_BRIGHTNESS 0

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
    unsigned long startTime = millis();
    int stepCountUp = 0;
    int stepCountDown = STEP_NUM - 1;

public:
    Effects(CRGB *_leds)
    {
        leds = _leds;
        EVERY_N_MILLISECONDS(20) { gHue++; } //FIXME - leave as is ? 
    }

    void setSteps(Step *_steps)
    {
        steps = _steps;
    }

    void lightsBeat(Parameters *params)
    {
        float numFromZeroToOne = float(params->getCurrentTime() - params->getStartTime()) / float(params->getInterval());
        for (int i = 0; i < TOTAL_NUM_LEDS; i++)
        {
            leds[i] = params->getColor();
        }
        FastLED.setBrightness(curr_steps);
        if (numFromZeroToOne >= 1)
        {
            curr_steps += step_incr;
            params->setStartTime(params->getCurrentTime());
        }

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
        params->setCurrentTime(millis());
    }

    void lightBoard(uint8_t data, CRGB color)
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

    bool lightBoardEscelate(Parameters *params)
    {
        float numFromZeroToOne = float(params->getCurrentTime() - params->getStartTime()) / float(params->getInterval());
        uint8_t step = 1 << stepCountUp;
        if (numFromZeroToOne >= 1)
        {
            stepCountUp = (stepCountUp + 1) % 7;
            step = 1 << stepCountUp;
            params->setStartTime(params->getCurrentTime());
        }
        lightBoard(step, params->getColor());
        params->setCurrentTime(millis());
    }

    bool lightBoardDescelate(Parameters *params)
    {
        float numFromZeroToOne = float(params->getCurrentTime() - params->getStartTime()) / float(params->getInterval());
        if (numFromZeroToOne >= 1)
        {
            stepCountDown = (stepCountDown - 1) % 7;
            uint8_t step = 1 << stepCountDown;
            if (stepCountDown == 0)
            {
                stepCountDown = 7;
            }
            params->setStartTime(params->getCurrentTime());
            lightBoard(step, params->getColor());
        }
        params->setCurrentTime(millis());
    }

    void bpm(Parameters *params)
    {
        // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
        CRGBPalette16 palette = PartyColors_p;
        uint8_t beat = beatsin8(params->getInterval(), 64, 255);

        for (int i = 0; i < TOTAL_NUM_LEDS; i++)
        {
            leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
        }
    }

    void glitter(Parameters *params)
    {
        for (int i = 0; i < (TOTAL_NUM_LEDS); i++)
        {
            leds[i] = CRGB::Black;
        }
        if (random8() < chanceOfGlitter)
        {
            for (int i = 0; i < (TOTAL_NUM_LEDS / 7); i++)
            {
                leds[random16(TOTAL_NUM_LEDS)] += CRGB::White;
            }
        }
    }

    void confetti(Parameters *params)
    {
        // random colored speckles that blink in and fade smoothly
        fadeToBlackBy(leds, TOTAL_NUM_LEDS, 10);
        for (int i = 0; i < (TOTAL_NUM_LEDS / 7); i++)
        {
            int pos = random16(TOTAL_NUM_LEDS);
            leds[pos] += CHSV(gHue + random8(64 * i), 200, 255);
        }
    }

    void juggle(Parameters *params)
    {
        // eight colored dots, weaving in and out of sync with each other
        fadeToBlackBy(leds, TOTAL_NUM_LEDS, 20);
        byte dothue = 0;
        for (int i = 0; i < 8; i++)
        {
            leds[beatsin16(i + 7, 0, TOTAL_NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
            dothue += 32;
        }
    }

    void sinelon(Parameters *params)
    {
        for (int i = 0; i < STEP_NUM; i++)
        {
            // a colored dot sweeping back and forth, with fading trails
            fadeToBlackBy(leds, TOTAL_NUM_LEDS, 20);
            int pos = beatsin16(params->getInterval(), steps[i].fromLed, steps[i].toLed);
            leds[pos] += CHSV(gHue, 255, 192);
        }
    }
};
