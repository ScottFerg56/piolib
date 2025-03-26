#pragma once
#include "LightFX.h"
#include "StdListFX.h"

// Static Color0 fill.
class FXStatic : public FXEffect
{
public:
    uint16_t Run()
    {
        if (CallCounter == 0)
            Fill(Params->Color0);
        // don't really need to come back!
        return 60000;
    }
};

// Blink base class.
class FXBlinkBase : public FXEffect
{
protected:
    //
    // Blink/strobe function
    // Alternate between Color1 and Color2
    // if (strobe == true) then create a strobe effect
    //
    uint16_t Blink(uint32_t color1, uint32_t color2, bool strobe)
    {
        uint32_t color = (((CallCounter & 1) == 0) == !Params->Reverse) ? color1 : color2;
        Fill(color);

        if ((CallCounter & 1) == 0)
            return strobe ? 20 : (Params->Speed / 2);
        else
            return strobe ? Params->Speed - 20 : (Params->Speed / 2);
    }
};

// Blink with 50% duty cycle between Color0 and Color1.
class FXBlink : public FXBlinkBase
{
public:
    uint16_t Run()
    { return Blink(Params->Color0, Params->Color1, false); }
};

// Short strobe flash of Color0 over Color1.
class FXStrobe : public FXBlinkBase
{
public:
    uint16_t Run()
    { return Blink(Params->Color0, Params->Color1, true); }
};

// Blinking with cycling rainbow colors alternating with Color1.
class FXBlinkRainbow : public FXBlinkBase
{
public:
    uint16_t Run()
    { return Blink(ColorWheel(CallCounter & 0xFF), Params->Color1, false); }
};

// Wipe base class.
class FXWipeBase : public FXEffect
{
protected:
    //
    // Wipe Color0 on then erase with Color1 in the same direction.
    // 'rev' causes erase to happen in the reverse direction.
    // 
    uint16_t Wipe(uint32_t color1, uint32_t color2, bool rev)
    {
        // wipe on; erase off
        if (Forward)
        {
            Segment->SetPixelColor(RevInxIf(Step), color1);
        }
        else
        {
            Segment->SetPixelColor(RevInxIf(Step, Params->Reverse == rev), color2);
        }

        CycleStepFlip();
        return Params->Speed / (Length() * 2);
    }
};

// Wipe Color0 on then erase with Color1 in the same direction.
class FXWipe : public FXWipeBase
{
public:
    uint16_t Run()
    { return Wipe(Params->Color0, Params->Color1, false); }
};

// Wipe Color0 on then erase with Color1 in the reverse direction.
class FXWipeRev : public FXWipeBase
{
public:
    uint16_t Run()
    { return Wipe(Params->Color0, Params->Color1, true); }
};

// Wipe random color on then erase with Color1 in the same direction.
class FXWipeRandom : public FXWipeBase
{
public:
    uint16_t Run()
    {
        if (CallCounter == 0)
            WheelInx = 0;
        if (Step == 0)
            WheelInx = GetRandomWheelIndex(WheelInx);
        auto color = ColorWheel(WheelInx);
        return Wipe(color, Params->Color1, false) * 2;
    }
protected:
    uint8_t WheelInx;
};

// Runs a single Color0 pixel back and forth over Color1.
class FXScan : public FXEffect
{
public:
    uint16_t Run()
    {
        if (CallCounter == 0)
            Fill(Params->Color1);
        Segment->SetPixelColor(Step, Params->Color1);
        CycleStepFlip();
        Segment->SetPixelColor(Step, Params->Color0);
        return (Params->Speed / (Length() * 2));
    }
};

// Runs two Color0 pixels back and forth in opposite directions over Color1.
class FXDualScan : public FXEffect
{
public:
    uint16_t Run()
    {
        if (CallCounter == 0)
            Fill(Params->Color1);
        Segment->SetPixelColor(Step, Params->Color1);
        Segment->SetPixelColor(RevInx(Step), Params->Color1);
        CycleStepFlip();
        Segment->SetPixelColor(Step, Params->Color0);
        Segment->SetPixelColor(RevInx(Step), Params->Color0);
        return (Params->Speed / (Length() * 2));
    }
};

// Does the "standby-breathing" of well known i-Devices.
class FXBreathe : public FXEffect
{
public:
    uint16_t Run()
    {
        //                         0    1    2   3   4   5   6    7   8   9  10  11   12   13   14   15   16   // step
        uint16_t delaySteps[] = {  7,   9,  13, 15, 16, 17, 18, 930, 19, 18, 15, 13,   9,   7,   4,   5,  10}; // magic numbers for breathing light
        uint8_t  lumSteps[]   = {150, 125, 100, 75, 50, 25, 16,  15, 16, 25, 50, 75, 100, 125, 150, 220, 255}; // even more magic numbers!

        if (CallCounter == 0)
            lum = lumSteps[0] + 1;

        if (Step < 8)
            lum--;
        else
            lum++;

        // update index of current delay when target luminance is reached, start over after the last step
        if (lum == lumSteps[Step])
            CycleStep(sizeof(lumSteps) / sizeof(lumSteps[0]));

        Fill(LumScale(Params->Color0, lum));

        return delaySteps[Step];
    }

protected:
    int lum;
};

// Lights all lights in one random color each iteration.
class FXRandomColor : public FXEffect
{
public:
    uint16_t Run()
    {
        Step = GetRandomWheelIndex(Step);
        Fill(ColorWheel(Step));
        return Params->Speed;
    }
};

// Sets random lights to random colors.
class FXSingleDynamic : public FXEffect
{
public:
    uint16_t Run()
    {
        if (CallCounter == 0)
            Fill(Params->Color0);
        Segment->SetPixelColor(random(Length()), ColorWheel(random(256)));
        return Params->Speed;
    }
};

// Changes all lights each cycle to new random colors.
class FXMultiDynamic : public FXEffect
{
public:
    uint16_t Run()
    {
        for (uint16_t i = 0; i < Length(); i++)
            Segment->SetPixelColor(i, ColorWheel(random(256)));
        return Params->Speed;
    }
};

// Cycles all lights at once through a rainbow.
class FXRainbow : public FXEffect
{
public:
    uint16_t Run()
    {
        Fill(ColorWheel(Step));
        CycleStep(256);
        return (Params->Speed / 256);
    }
};

// Cycles a rainbow over all lights.
class FXRainbowCycle : public FXEffect
{
public:
    uint16_t Run()
    {
        for (uint16_t i = 0; i < Length(); i++)
        {
            uint32_t color = ColorWheel(((i * 256 / Length()) + Step) & 0xFF);
            Segment->SetPixelColor(i, color);
        }
        CycleStep(256);
        return Params->Speed / 256;
    }
};

// Fades the lights on and (almost) off again.
class FXFade : public FXEffect
{
public:
    uint16_t Run()
    {
        int lum = Step - 31;
        lum = 63 - (abs(lum) * 2);
        lum = map(lum, 0, 64, 25, 255);
        Fill(LumScale(Params->Color0, lum));
        CycleStep(65);
        return Params->Speed / 64;
    }
};

// Theater chase base class
class FXTheaterChaseBase : public FXEffect
{
public:
    uint16_t TheaterChase(uint32_t color1, uint32_t color2)
    {
        CallCounter = CallCounter % 3;
        for (uint16_t i = 0; i < Length(); i++)
            Segment->SetPixelColor(RevInxIf(i), ((i % 3) == CallCounter) ? color1 : color2);
        return Params->Speed / Length();
    }
};

// Theatre-style crawling lights; Color0 over Color1.
class FXTheaterChase : public FXTheaterChaseBase
{
public:
    uint16_t Run()
    { return TheaterChase(Params->Color0, Params->Color1); }
};

// Theatre-style crawling lights; rainbow over Color1.
class FXTheaterChaseRainbow : public FXTheaterChaseBase
{
public:
    uint16_t Run()
    {
        CycleStep(256);
        return TheaterChase(ColorWheel(Step), Params->Color1);
    }
};

// Running lights effect with smooth sine transition.
class FXRunningLights : public FXEffect
{
public:
    uint16_t Run()
    {
        float radPerLed = (2.0 * 3.14159) / Length();
        for (uint16_t i = 0; i < Length(); i++)
        {
            int lum = map((int)(sin((i + Step) * radPerLed) * 128), -128, 128, 0, 255);
            auto clr = LumScale(Params->Color0, lum);
            Segment->SetPixelColor(RevInxIf(i), clr);
        }
        CycleStep(Length());
        return Params->Speed / Length();
    }
};

// Twinkle base class.
class FXTwinkleBase : public FXEffect
{
protected:
    uint16_t Twinkle(uint32_t color1, uint32_t color2)
    {
        if (Step == 0)
        {
            Fill(color2);
            uint16_t min_leds = max(1u, Length() / 5); // make sure, at least one light is on
            uint16_t max_leds = max(1u, Length() / 2); // make sure, at least one light is on
            Step = random(min_leds, max_leds);
        }

        Segment->SetPixelColor(random(Length()), color1);

        Step--;
        return Params->Speed / Length();
    }
};

// Blink several lights on, reset, repeat; Color0 over Color1.
class FXTwinkle : public FXTwinkleBase
{
public:
    uint16_t Run()
    { return Twinkle(Params->Color0, Params->Color1); }
};

// Blink several lights on, reset, repeat; random colors over Color1.
class FXTwinkleRandom : public FXTwinkleBase
{
public:
    uint16_t Run()
    { return Twinkle(ColorWheel(random(256)), Params->Color1); }
};

// Twinkle base class.
class FXTwinkleFadeBase : public FXEffect
{
public:
    uint16_t TwinkleFade(uint32_t color)
    {
        Fade();
        if (random(3) == 0)
            Segment->SetPixelColor(random(Length()), color);
        return Params->Speed / 8;
    }
};

// Blink several lights on in Color0, fading to black.
class FXTwinkleFade : public FXTwinkleFadeBase
{
public:
    uint16_t Run()
    { return TwinkleFade(Params->Color0); }
};

// Blink several lights on in random colors, fading to black.
class FXTwinkleFadeRandom : public FXTwinkleFadeBase
{
public:
    uint16_t Run()
    { return TwinkleFade(ColorWheel(random(256))); }
};

// Blinks one light at a time; Color0 over Color1.
class FXSparkle : public FXEffect
{
public:
    uint16_t Run()
    {
        if (CallCounter == 0)
        {
            Fill(Params->Color1);
            Step = random(Length());
        }
        Segment->SetPixelColor(Step, Params->Color1);
        Step = random(Length());
        Segment->SetPixelColor(Step, Params->Color0);
        return Params->Speed / Length();
    }
};

class FXChaseBase : public FXEffect
{
protected:
    /*
    * Color chase function.
    *   color1 = background color
    *   color2 and color3 = colors of two adjacent leds
    */
    uint16_t Chase(uint32_t color1, uint32_t color2, uint32_t color3)
    {
        uint16_t a = Step;
        uint16_t b = (a + 1) % Length();
        uint16_t c = (b + 1) % Length();
        Segment->SetPixelColor(RevInxIf(a), color1);
        Segment->SetPixelColor(RevInxIf(b), color2);
        Segment->SetPixelColor(RevInxIf(c), color3);

        CycleStep(Length());
        return Params->Speed / Length();
    }
};

// Color0 running on Color1.
class FXChase : public FXChaseBase
{
public:
    uint16_t Run()
    { return Chase(Params->Color1, Params->Color0, Params->Color0); }
};

// Color0 running on a solid rainbow.
class FXChaseRainbow : public FXChaseBase
{
public:
    uint16_t Run()
    {
        uint8_t color_sep = 256 / Length();
        uint8_t color_index = CallCounter & 0xFF;
        uint32_t color = ColorWheel(((Step * color_sep) + color_index) & 0xFF);
        return Chase(color, Params->Color0, Params->Color0);
    }
};

// Color0 flashes running on Color1.
class FXChaseFlash : public FXEffect
{
public:
    uint16_t Run()
    {
        const static uint8_t flash_count = 4;
        uint8_t flash_step = CallCounter % ((flash_count * 2) + 1);

        Fill(Params->Color1);

        uint16_t delay = Params->Speed / Length();
        if (flash_step < (flash_count * 2))
        {
            if (flash_step % 2 == 0)
            {
                uint16_t n = Step;
                uint16_t m = (Step + 1) % Length();
                Segment->SetPixelColor(RevInxIf(n), Params->Color0);
                Segment->SetPixelColor(RevInxIf(m), Params->Color0);
                delay = 20;
            }
            else
            {
                delay = 30;
            }
        }
        else
        {
            CycleStep(Length());
        }
        return delay;
    }
};

class FXRunningBase : public FXEffect
{
public:
    // Alternating pixels running function.
    uint16_t Running(uint32_t color1, uint32_t color2)
    {
        for (uint16_t i = 0; i < Length(); i++)
        {
            if ((i + Step) % 4 < 2)
                Segment->SetPixelColor(RevInxIf(i), color1);
            else
                Segment->SetPixelColor(RevInxIf(i, !Params->Reverse), color2);
        }
        CycleStep(4);
        return Params->Speed / Length();
    }
};

// Alternating color0/color1 pixels running.
class FXRunning : public FXRunningBase
{
public:
    uint16_t Run()
    { return Running(Params->Color0, Params->Color1); }
};

// Cylon; Color0 over black.
class FXCylon : public FXEffect
{
public:
    uint16_t Run()
    {
        Fade();
        Segment->SetPixelColor(Step, Params->Color0);
        CycleStepFlip();
        return (Params->Speed / (Length() * 2));
    }
};

// Firing comets from one end in Color0.
class FXComet : public FXEffect
{
public:
    uint16_t Run()
    {
        Fade();
        Segment->SetPixelColor(RevInxIf(Step), Params->Color0);
        CycleStepWrap();
        return Params->Speed / Length();
    }
};

// Fireworks base class.
class FXFireWorksBase : public FXEffect
{
public:
    uint16_t Fireworks(uint32_t color)
    {
        Fade();
        for (uint16_t i = 1; i < Length() - 1; i++)
        {
            uint32_t prevInx = Segment->GetPixelColor(i - 1);
            uint32_t thisInx = Segment->GetPixelColor(i);
            uint32_t nextInx = Segment->GetPixelColor(i + 1);
            uint32_t prevX = (prevInx >> 2) & 0x3F3F3F;
            uint32_t thisX =  thisInx;
            uint32_t nextX = (nextInx >> 2) & 0x3F3F3F;
            Segment->SetPixelColor(i, prevX + thisX + nextX);
        }
        for (uint16_t i = 0; i < max(1u, Length() / 20); i++)
        {
            if (random(10) == 0)
                Segment->SetPixelColor(random(Length()), color);
        }
        return Params->Speed / Length();
    }
};

// Fireworks in Color0.
class FXFireWorks : public FXFireWorksBase
{
public:
    uint16_t Run()
    { return Fireworks(Params->Color0); }
};

// Fireworks in random colors.
class FXFireworksRandom : public FXFireWorksBase
{
public:
    uint16_t Run()
    { return Fireworks(ColorWheel(random(256))); }
};

// Fire flicker base class.
class FXFireFlickerBase : public FXEffect
{
public:
    uint16_t FireFlicker(int rev_intensity)
    {
        auto r = R(Params->Color0);
        auto g = G(Params->Color0);
        auto b = B(Params->Color0);
        byte lum = max(r, max(g, b)) / rev_intensity;
        for (uint16_t i = 0; i < Length(); i++)
        {
            int flicker = random(0, lum);
            auto clr = RGBtoInt(max(r - flicker, 0), max(g - flicker, 0), max(b - flicker, 0));
            Segment->SetPixelColor(i, clr);
        }
        return Params->Speed / Length();
    }
};

// Random flickering in Color0.
class FXFireFlicker : public FXFireFlickerBase
{
public:
    uint16_t Run()
    { return FireFlicker(3); }
};

// Random flickering in Color0, more intesity.
class FXFireFlickerIntense : public FXFireFlickerBase
{
public:
    uint16_t Run()
    { return FireFlicker(1.7); }    // UNDONE: float value to int arg??
};

// List standard effects constructors for factory registration.
FXFactory::effectsItem stdEffectsList [] =
{
    { FX_STATIC,                []() { return (FXEffect*) new FXStatic();               } },
    { FX_BLINK,                 []() { return (FXEffect*) new FXBlink();                } },
    { FX_STROBE,                []() { return (FXEffect*) new FXStrobe();               } },
    { FX_BLINK_RAINBOW,         []() { return (FXEffect*) new FXBlinkRainbow();         } },
    { FX_WIPE,                  []() { return (FXEffect*) new FXWipe();                 } },
    { FX_WIPE_REV,              []() { return (FXEffect*) new FXWipeRev();              } },
    { FX_WIPE_RANDOM,           []() { return (FXEffect*) new FXWipeRandom();           } },
    { FX_SCAN,                  []() { return (FXEffect*) new FXScan();                 } },
    { FX_DUAL_SCAN,             []() { return (FXEffect*) new FXDualScan();             } },
    { FX_BREATH,                []() { return (FXEffect*) new FXBreathe();              } },
    { FX_RANDOM_COLOR,          []() { return (FXEffect*) new FXRandomColor();          } },
    { FX_SINGLE_DYNAMIC,        []() { return (FXEffect*) new FXSingleDynamic();        } },
    { FX_MULTI_DYNAMIC,         []() { return (FXEffect*) new FXMultiDynamic();         } },
    { FX_RAINBOW,               []() { return (FXEffect*) new FXRainbow();              } },
    { FX_RAINBOW_CYCLE,         []() { return (FXEffect*) new FXRainbowCycle();         } },
    { FX_FADE,                  []() { return (FXEffect*) new FXFade();                 } },
    { FX_THEATER_CHASE,         []() { return (FXEffect*) new FXTheaterChase();         } },
    { FX_THEATER_CHASE_RAINBOW, []() { return (FXEffect*) new FXTheaterChaseRainbow();  } },
    { FX_RUNNING_LIGHTS,        []() { return (FXEffect*) new FXRunningLights();        } },
    { FX_TWINKLE,               []() { return (FXEffect*) new FXTwinkle();              } },
    { FX_TWINKLE_RANDOM,        []() { return (FXEffect*) new FXTwinkleRandom();        } },
    { FX_TWINKLE_FADE,          []() { return (FXEffect*) new FXTwinkleFade();          } },
    { FX_TWINKLE_FADE_RANDOM,   []() { return (FXEffect*) new FXTwinkleFadeRandom();    } },
    { FX_SPARKLE,               []() { return (FXEffect*) new FXSparkle();              } },
    { FX_CHASE,                 []() { return (FXEffect*) new FXChase();                } },
    { FX_CHASE_RAINBOW,         []() { return (FXEffect*) new FXChaseRainbow();         } },
    { FX_CHASE_FLASH,           []() { return (FXEffect*) new FXChaseFlash();           } },
    { FX_RUNNING,               []() { return (FXEffect*) new FXRunning();              } },
    { FX_CYLON,                 []() { return (FXEffect*) new FXCylon();                } },
    { FX_COMET,                 []() { return (FXEffect*) new FXComet();                } },
    { FX_FIREWORKS,             []() { return (FXEffect*) new FXFireWorks();            } },
    { FX_FIREWORKS_RANDOM,      []() { return (FXEffect*) new FXFireworksRandom();      } },
    { FX_FIRE_FLICKER,          []() { return (FXEffect*) new FXFireFlicker();          } },
    { FX_FIRE_FLICKER_INTENSE,  []() { return (FXEffect*) new FXFireFlickerIntense();   } },
};
