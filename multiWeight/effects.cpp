#include <FastLED.h>
#include "parameters.cpp"

#define TOTAL_NUM_LEDS 745
#define FRAMES_PER_SECOND 120
#define MAX_BRIGHTNESS 100
#define MIN_BRIGHTNESS 0

struct Step
{
    int fromLed;
    int toLed;
    int highThreshold = 4000;
    int lowThreshold = -4000;
    int histeressis = 2000;
    bool stepDetected;
    bool pstepDetected;
    int stepsValue;
    int stepsAvgValue;
    int stepsCounter;
    unsigned long hideEffectUntilTime = 0;
    bool ledIsntalClockWise = true;

    int getLedNumber(int num)
    {
        if (ledIsntalClockWise)
        {
            return fromLed + num;
        }
        else
        {
            return toLed - num;
        }
    }

    int getLedInLedsArray(int num)
    {
        if (ledIsntalClockWise)
        {
            return num;
        }
        else
        {
            return toLed + fromLed - num;
        }
    }
    int getTotalLeds()
    {
        return toLed - fromLed;
    }
};

class Effects
{
private:
    //pointer to leds array from main
    CRGB *leds;
    Step *steps;
    int stepsSize = 0;
    int stepNum;

    uint8_t gHue = 0;
    fract8 chanceOfGlitter = 255;
    int curr_steps = 0;
    int step_incr = 8;
    unsigned long startTime = millis();
    int stepCountUp = 0;
    int stepCountDown = stepNum - 1;

    int redStart = 0;
    int redEnd = redStart + 34;

    int greenStart = redEnd + 1;
    int greenEnd = greenStart + 34;

    // 1 - red , 2 - running red , 3 - running green , 4 green
    int playSign = 1;

public:
    Effects(CRGB *_leds, int _stepNum)
    {
        leds = _leds;
        stepNum = _stepNum;
        EVERY_N_MILLISECONDS(20) { gHue++; } //FIXME - leave as is ?
    }

    void setSteps(Step *_steps, int numberOfSteps)
    {
        steps = _steps;
        stepsSize = numberOfSteps;
    }

    void runEffect(int e, Parameters *params)
    {
        if (e > 0)
        {
            FastLED.setBrightness(MAX_BRIGHTNESS);
        }

        switch (e)
        {
        case 0:
            lightsBeat(params);
            break;
        case 1:
            lightBoardEscelate(params);
            break;
        case 2:
            lightBoardDescelate(params);
            break;
        case 3:
            bpm(params);
            break;
        case 4:
            glitter(params);
            break;
        case 5:
            confetti(params);
            break;
        case 6:
            juggle(params);
            break;
        case 7:
            sinelon(params);
            break;
        case 8:
            paint(params);
            break;
        case 9:
            //3 - running green
            playSign = 3;
            break;
        case 10:
            //2 - running red
            playSign = 2;
            break;
        case 11:
            // 1 - red
            playSign = 1;
            break;
        default:
            lightsBeat(params);
            break;
        }
        playSigns();
        addSteps();
    }

    void playSigns()
    {
        // 1 - red , 2 - running red , 3 - running green , 4 green
        switch (playSign)
        {
        case 1:
            red();
            break;
        case 2:
            redRun();
            break;
        case 3:
            greenRun();
            break;
        case 4:
            green();
            break;
        }
    }

    void lightsBeat(Parameters *params)
    {
        float numFromZeroToOne = float(params->getCurrentTime() - params->getStartTime()) / float(params->getInterval());
        for (int i = greenEnd + 1; i < TOTAL_NUM_LEDS; i++)
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

    void addSteps()
    {
        uint8_t step;
        for (int i = 0; i < stepNum; i++)
        {
            if (millis() < steps[i].hideEffectUntilTime )
            {
                for (int led = steps[i].fromLed; led < steps[i].toLed; led++)
                {
                    leds[led] = CRGB::Green;
                }
            }
        }
    }

    void lightBoard(uint8_t data, CRGB color)
    {
        for (int i = 0; i < stepNum; i++)
        {
            bool isLightStep = bitRead(data, i);
            for (int led = steps[i].fromLed; led < steps[i].toLed; led++)
            {
                leds[led] = (isLightStep ? CRGB(color.r, color.g, color.b) : CRGB::Black);
            }
        }
    }

    void lightBoardEscelate(Parameters *params)
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

    void lightBoardDescelate(Parameters *params)
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

        for (int i = greenEnd + 1; i < TOTAL_NUM_LEDS; i++)
        {
            leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
        }
    }

    void glitter(Parameters *params)
    {
        for (int i = greenEnd + 1; i < (TOTAL_NUM_LEDS); i++)
        {
            leds[i] = CRGB::Black;
        }
        if (random8() < chanceOfGlitter)
        {
            for (int i = greenEnd + 1; i < (TOTAL_NUM_LEDS - 70); i++)
            {
                int position = random16(TOTAL_NUM_LEDS);
                if (position <= greenEnd)
                {
                    position = position + greenEnd + 1;
                }
                leds[position] += CRGB::White;
            }
        }
    }

    void confetti(Parameters *params)
    {
        // random colored speckles that blink in and fade smoothly
        fadeToBlackBy(leds, TOTAL_NUM_LEDS, 10);
        for (int i = greenEnd + 1; i < (TOTAL_NUM_LEDS / 7); i++)
        {
            int position = random16(TOTAL_NUM_LEDS);
            if (position <= greenEnd)
            {
                position = position + greenEnd + 1;
            }
            leds[position] += CHSV(gHue + random8(64 * i), 200, 255);
        }
    }

    void juggle(Parameters *params)
    {
        // eight colored dots, weaving in and out of sync with each other
        fadeToBlackBy(leds, TOTAL_NUM_LEDS, 20);
        byte dothue = 0;
        for (int i = 0; i < stepsSize; i++)
        {
            int position = beatsin16(i + 7, 0, TOTAL_NUM_LEDS - 1);
            if (position <= greenEnd)
            {
                position = position + greenEnd + 1;
            }
            leds[position] |= CHSV(dothue, 200, 255);
            dothue += 32;
        }
    }

    void sinelon(Parameters *params)
    {
        for (int i = 0; i < stepsSize; i++)
        {
            // a colored dot sweeping back and forth, with fading trails
            fadeToBlackBy(leds, TOTAL_NUM_LEDS, 20);
            int pos = beatsin16(params->getInterval(), steps[i].fromLed, steps[i].toLed);
            if (pos <= greenEnd)
            {
                pos = greenEnd + 1;
            }
            leds[pos] += CHSV(gHue, 255, 192);
        }
    }

    void paint(Parameters *params)
    {
        for (int stepNum = 0; stepNum < 7; stepNum++)
        {
            //hack: use interval param to select the step
            //int stepNum = min(params->getInterval(),(unsigned long)stepsSize) - 1;
            //int stepNum = 0;
            for (int i = steps[stepNum].fromLed; i < steps[stepNum].toLed; i++)
            {
                leds[i] = params->getColor();
                //leds[i] = CRGB(params->getColor().r, params->getColor().g, params->getColor().b);
            }
        }
    }

    void green()
    {
        for (int i = redStart; i <= redEnd; i++)
        {
            leds[i] = CRGB::Black;
        }
        for (int i = greenStart; i <= greenEnd; i++)
        {
            leds[i] = CRGB::Green;
        }
    }

    void greenRun()
    {
        fadeToBlackBy(leds, 35, 20);
        int pos = beatsin16(60, greenStart, greenEnd);
        if (pos > greenEnd)
        {
            pos = greenEnd;
        }
        else if (pos < greenStart)
        {
            pos = greenStart;
        }

        leds[pos] += CRGB::Green;
    }

    void red()
    {
        for (int i = redStart; i < redEnd + 1; i++)
        {
            leds[i] = CRGB::Red;
        }
        for (int j = greenStart; j < greenEnd + 1; j++)
        {
            leds[j] = CRGB::Black;
        }
    }

    void redRun()
    {
        /* fadeToBlackBy(leds, 35, 20);
        int pos = beatsin16(60, redStart, redEnd);
        if (pos >= redEnd){
            pos = redStart;
        }
        leds[pos] += CRGB::Red; */

        for (int i = redStart; i < redEnd; i++)
        {
            if (i == cntctn){
                leds[i] = CRGB::Red;
                leds[i-1] = CRGB::Red;
                leds[i-2] = CRGB::Red;
            }else
            {
                leds[i] = CRGB::Black;
            }
        }
        cntctn = (cntctn + 1) % 35;
    }

    int cntctn;
    void snake2(Parameters *params)
    {

        for (int i = steps[0].fromLed; i < steps[stepsSize - 1].toLed; i++)
        {
            leds[i] = (i == cntctn) ? params->getColor() : CRGB::Black;
        }
        cntctn = (cntctn + 1) % TOTAL_NUM_LEDS;
    }

    void snake(Parameters *params)
    {

        for (int i = 0; i < stepsSize; i++)
        {
            for (int j = steps[i].fromLed; j < steps[i].toLed; j++)
            {
                if (cntctn == steps[i].getLedInLedsArray(j))
                {
                    leds[j] = params->getColor();
                }
                else
                {
                    leds[j] = CRGB::Black;
                }
            }
            //leds[i] = (i == cntctn) ? params->getColor(): CRGB::Black;
        }
        cntctn = (cntctn + 1) % TOTAL_NUM_LEDS;
    }
};
