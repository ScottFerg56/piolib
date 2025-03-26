#pragma once

#include <Arduino.h>
#include <map>
#include <vector>

#ifndef FXLOGE
#define FXLOGE(format, ...) do {} while(0)
#define FXLOGW(format, ...) do {} while(0)
#define FXLOGI(format, ...) do {} while(0)
#define FXLOGD(format, ...) do {} while(0)
#define FXLOGV(format, ...) do {} while(0)
#endif

class FXSegmentBase
{
public:
    virtual uint32_t GetLength() = 0;
    virtual uint32_t GetPixelColor(uint16_t pixel) = 0;
    virtual void SetPixelColor(uint16_t pixel, uint32_t color) = 0;
    virtual void Show() = 0;
};

class FXParams
{
public:
    FXParams(bool on, uint32_t color0, uint32_t color1, uint16_t speed, bool reverse) : On(on), Color0(color0), Color1(color1), Speed(speed), Reverse(reverse) {}
    bool On;
    uint32_t Color0;
    uint32_t Color1;
    uint16_t Speed;
    bool Reverse;
};

/// @brief Effects base class.
/// @remarks remark text
class FXEffect
{
public:
    FXParams* Params;

    void Init(FXSegmentBase* segment, FXParams* params)
    {
        if (!segment)
            FXLOGE("null segment");
        Segment = segment;
        Params = params;
        Reset();
    }

    void Init(FXSegmentBase* segment, FXEffect* effect)
    {
        if (!effect)
        {
            FXLOGE("null effect");
            Segment = segment;
            return;
        }
        Init(segment, effect->Params);
    }

    void DoRun(unsigned long now)
    {
        if (now > NextTime)
        {
            if (!Params->On)
            {
                NextTime = now + 60000;
                return;
            }
            uint16_t delay = Run();
            NextTime = now + max((int)delay, 10);
            CallCounter++;
            Segment->Show();
        }
    }

    void Reset()
    {
        CallCounter = 0;
        Step = 0;
        Pass = 0;
        Forward = true;
        NextTime = 0;
        if (!Params->On)
        {
            Fill(0);
            Segment->Show();
        }
    }

    void Fill(uint32_t color)
    {
        for (uint16_t i = 0; i < Length(); i++)
        {
            Segment->SetPixelColor(i, color);
        }
    }

protected:
    virtual uint16_t Run() = 0;

    uint32_t Length() { return Segment->GetLength(); }

    void CycleStep(uint32_t modulus)
    {
        if (++Step >= modulus)
            Step = 0;
    }

    void CycleStepWrap()
    {
        if (Step >= Length() - 1)
        {
            Step = 0;
            ++Pass;
        }
        else
        {
            ++Step;
        }
    }

    void CycleStepFlip()
    {
        if (Forward)
        {
            if (Step >= Length() - 1)
            {
                Step = Length() - 1;
                Forward = false;
                ++Pass;
            }
            else
            {
                ++Step;
            }
        }
        else
        {
            if (Step <= 0)
            {
                Step = 0;
                Forward = true;
                ++Pass;
            }
            else
            {
                --Step;
            }
        }
    }

    uint32_t RGBtoInt(uint8_t r, uint8_t g, uint8_t b)
    {
        return (((uint32_t)r << 16L) | ((uint32_t)g << 8L) | (uint32_t)b);
    }

    uint8_t R(uint32_t clr) { return (clr >> 16) & 0xFF; }
    uint8_t G(uint32_t clr) { return (clr >>  8) & 0xFF; }
    uint8_t B(uint32_t clr) { return (clr      ) & 0xFF; }

    uint32_t LumScale(uint32_t clr, uint32_t num, uint32_t den = 255)
    {
        uint8_t r = (R(clr) * num) / den;
        uint8_t g = (G(clr) * num) / den;
        uint8_t b = (B(clr) * num) / den;
        return RGBtoInt(r, g, b);
    }

    uint32_t RevInx(uint32_t inx) { return Length() - inx - 1; }
    uint32_t RevInxIf(uint32_t inx) { return Params->Reverse ? RevInx(inx) : inx; }
    uint32_t RevInxIf(uint32_t inx, bool rev) { return rev ? RevInx(inx) : inx; }

    //
    // Input value 0 to 255 returns a color value from the color wheel.
    // The colors transition r -> g -> b -> back to r
    //
    uint32_t ColorWheel(uint8_t pos)
    {
        pos = 255 - pos;
        uint32_t ret_hex;
        if (pos < 85)
        {
            ret_hex = ((uint32_t)(255 - pos * 3) << 16) | ((uint32_t)(0) << 8) | (pos * 3);
        }
        else if (pos < 170)
        {
            pos -= 85;
            ret_hex = ((uint32_t)(0) << 16) | ((uint32_t)(pos * 3) << 8) | (255 - pos * 3);
        }
        else
        {
            pos -= 170;
            ret_hex = ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8) | (0);
        }
        return ret_hex;
    }

    //
    // Returns a new, random wheel index with a minimum distance of 42 from pos.
    //
    uint8_t GetRandomWheelIndex(uint8_t pos)
    {
        uint8_t r = 0;
        uint8_t x = 0;
        uint8_t y = 0;
        uint8_t d = 0;

        while(d < 42)
        {
            r = random(256);
            x = abs(pos - r);
            y = 255 - x;
            d = min(x, y);
        }
        return r;
    }

    // Fades the current segment toward black by dividing each pixel's intensity by 2.
    void Fade()
    {
        for (uint16_t i = 0; i < Length(); i++)
        {
            uint32_t clr = LumScale(Segment->GetPixelColor(i), 1, 2);
            Segment->SetPixelColor(i, clr);
        }
    };

    FXSegmentBase* Segment;
    uint32_t CallCounter;
    uint16_t Step;
    uint16_t Pass;
    bool Forward;
    unsigned long NextTime;
};

class FXFactory
{
public:
    using CreateFn = FXEffect* (*)();

    bool RegisterEffect(uint8_t id, CreateFn createFn)
    {
        return FXMap.insert({id, createFn}).second;
    }
    
    struct effectsItem { uint8_t id; FXFactory::CreateFn fn; };

    void RegisterEffects(effectsItem list[], size_t len)
    {
        for (uint8_t i = 0; i < len; i++)
            RegisterEffect(list[i].id, list[i].fn);
    }

    FXEffect* CreateEffect(uint8_t id, FXSegmentBase* segment, FXParams* params)
    {
        auto it = FXMap.find(id);
        if (it != FXMap.end())
        {
            auto effect = it->second();
            if (effect)
            {
                effect->Init(segment, params);
                return effect;
            }
        }
        FXLOGE("create effect failed: %d", id);
        return nullptr;
    }

private:
    std::map<uint8_t, CreateFn> FXMap;
};

class FXServer
{
public:
    void AddSegment(uint8_t id, FXSegmentBase* segment, FXEffect* effect)
    {
        if (!effect)
        {
            FXLOGE("null effect: %d", id);
            return;
        }
        if (!segment)
        {
            FXLOGE("null segment: %d", id);
            return;
        }
        effect->Reset();
        std::tuple<FXSegmentBase*, FXEffect*> tuple = std::make_tuple(segment, effect);
        Segments.insert({id, tuple});
    }

    FXEffect* SetEffect(uint8_t id, FXEffect* effect)
    {
        if (!effect)
        {
            FXLOGE("null effect: %d", id);
            return nullptr;
        }
        auto tuple = Segments[id];
        auto oldEffect = std::get<1>(tuple);
        auto segment = std::get<0>(tuple);
        Segments[id] = std::make_tuple(segment, effect);
        effect->Reset();
        effect->Init(segment, oldEffect);
        return oldEffect;
    }

    FXEffect* GetEffect(uint8_t id)
    {
        return std::get<1>(Segments[id]);
    }

    void Start()
    {
        Running = true;
    }

    void Stop()
    {
        if (!Running)
            return;
        Running = false;
        for (const auto& pair : Segments)
        {
            auto effect = std::get<1>(pair.second);
            if (effect)
                effect->Params->On = false;
            effect->Reset();
        }
    }

    using ShowFn = void (*)();

    void Run()
    {
        unsigned long now = millis(); // Be aware, millis() rolls over every 49 days
        bool dirty = false;
        for (const auto& pair : Segments)
        {
            auto tuple = pair.second;
            auto effect = std::get<1>(tuple);
            if (!effect)
            {
                FXLOGE("null effect %d", pair.first);
                continue;
            }
            effect->DoRun(now);
        }
    }

private:
    std::map<uint8_t, std::tuple<FXSegmentBase*, FXEffect*>> Segments;
    bool Running;
};
