#pragma once
#include <Arduino.h>

class Metronome
{
public:
	uint32_t	PeriodMS;

	Metronome(uint32_t periodMS) : PeriodMS(periodMS), LastTime(millis()) {}

	bool Test()
    {
        uint32_t t = millis();
        if (t - LastTime <= PeriodMS)
            return false;
        LastTime = t;
        return true;
    }
    
	operator bool() { return Test(); }

private:
	uint32_t	LastTime;
};
