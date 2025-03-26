#pragma once
#include "LightFX.h"

class FXLedSegBase : public FXSegmentBase
{
public:
    FXLedSegBase(const uint8_t* pins, uint8_t length, bool setMode = true) : Pins(pins), Length(length)
    {
        if (setMode)
        {
            for (size_t i = 0; i < Length; i++)
                pinMode(Pins[i], OUTPUT);
        }
    }
    FXLedSegBase(uint8_t pin, bool setMode = true) : Pins(nullptr), Pin(pin), Length(1)
    {
        if (setMode)
            pinMode(Pin, OUTPUT);
    }
    virtual uint32_t GetLength() { return Length; };
    virtual uint32_t GetPixelColor(uint16_t pixel)
    {
        return digitalRead(Pins ? Pins[pixel] : Pin) ? 0xFFFFFF : 0;
    }
    virtual void SetPixelColor(uint16_t pixel, uint32_t color)
    {
        digitalWrite(Pins ? Pins[pixel] : Pin, color != 0 ? HIGH : LOW);
    }
    virtual void Show() {}
protected:
    const uint8_t* Pins;
    uint8_t Pin;
    uint8_t Length;
};
