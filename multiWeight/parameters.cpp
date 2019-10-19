
#include <FastLED.h>
class Parameters
{
private:
    unsigned long startTime = millis();
    unsigned long currentTime = millis();
    unsigned long interval;
    CRGB color;

public:
    Parameters() {}

    void setInterval(unsigned long _interval)
    {
        interval = _interval;
    }

    void setColor(CRGB _color)
    {
        color = _color;
    }

    void setStartTime(unsigned long _startTime)
    {
        startTime = _startTime;
    }

    void setCurrentTime(unsigned long _currentTime)
    {
        currentTime = _currentTime;
    }

    unsigned long getStartTime()
    {
        return startTime;
    }

    unsigned long getCurrentTime()
    {
        return currentTime;
    }

    unsigned long getInterval()
    {
        return interval;
    }

    CRGB getColor()
    {
        return color;
    }
};
