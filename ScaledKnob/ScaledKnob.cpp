#include <Arduino.h>
#include <Adafruit_seesaw.h>
#include <seesaw_neopixel.h>
#include "ScaledKnob.h"

ScaledKnob::ScaledKnob(int encoder, int button, float minValue, float maxValue, float increment)
{
    Encoder = encoder;
    Button = button;
    MinValue = minValue;
    MaxValue = maxValue;
    Increment = increment;
}

void ScaledKnob::Init(Adafruit_seesaw* pss, seesaw_NeoPixel* pssxpixel, float initValue)
{
    pSS = pss;
    pSSPixel = pssxpixel;
    pSS->pinMode(Button, INPUT_PULLUP);
    delay(10);
    pSS->setGPIOInterrupts((uint32_t)1 << Button, 1);
    pSS->enableEncoderInterrupt(Encoder);
    SetValue(initValue);
}

void ScaledKnob::Sample()
{
    int32_t delta = pSS->getEncoderDelta(Encoder);
    SetValue(Value + Increment * delta);
}

void ScaledKnob::SetColor(uint32_t c)
{
    pSSPixel->setPixelColor(Encoder, c);
    pSSPixel->show();
}

void ScaledKnob::SetColor(uint8_t r, uint8_t g, uint8_t b)
{
    SetColor(pSSPixel->Color(r, g, b));
}

ScaledKnob::Presses ScaledKnob::Pressed()
{
    bool new_pressed = !pSS->digitalRead(Button);
    if (LastPressed != new_pressed)
    {
        LastPressed = new_pressed;
        if (LastPressed)
        return Presses::Press;
        return Presses::Release;
    }
    else if (LastPressed)
        return Presses::Hold;
    else
        return Presses::Off;
}

void ScaledKnob::SetValue(float value)
{
    Value = constrain(value, MinValue, MaxValue);
}

float ScaledKnob::GetValue()
{
    return Value;
}
