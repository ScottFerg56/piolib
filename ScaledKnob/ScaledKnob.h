#ifndef ScaledKnob_h
#define ScaledKnob_h

#include <Arduino.h>
#include <Adafruit_seesaw.h>
#include <seesaw_neopixel.h>

class ScaledKnob
{
protected:
  float Value = 0;
  bool LastPressed = false;
  float MinValue;
  float MaxValue;
  float Increment;

  Adafruit_seesaw* pSS;
  seesaw_NeoPixel* pSSPixel;
  int Encoder;
  int Button;

public:
  enum class Presses {Off, Press, Hold, Release};

  ScaledKnob(int encoder, int button, float minValue, float maxValue, float increment);
  void Init(Adafruit_seesaw* pss, seesaw_NeoPixel* pssxpixel, float initValue);
  void Sample();
  void SetColor(uint32_t c);
  void SetColor(uint8_t r, uint8_t g, uint8_t b);
  Presses Pressed();
  void SetValue(float value);
  float GetValue();
};

#endif