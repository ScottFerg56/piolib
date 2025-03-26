#pragma once
#include "LightFX.h"

class FXStripBase
{
public:
    virtual void Begin() = 0;
    virtual void Show() = 0;
    virtual void Clear() = 0;
    virtual void SetPixelColor(uint16_t pixel, uint32_t color) = 0;
    virtual uint32_t GetPixelColor(uint16_t pixel) = 0;
};

class FXStripSegBase : public FXSegmentBase
{
public:
    FXStripSegBase(FXStripBase* strip) : Strip(strip) { }
    void SetStrip(FXStripBase* strip) { Strip = strip; }
    virtual uint32_t GetLength() { return Length; }
    virtual void Show() { Strip->Show(); }

    virtual uint32_t GetPixelColor(uint16_t pixel)
    {
        if (pixel >= Length)
        {
            FXLOGE("pixel overrun");
            pixel = 0;
        }
        return Strip->GetPixelColor(Index(pixel));
    }

    virtual void SetPixelColor(uint16_t pixel, uint32_t color)
    {
        if (pixel >= Length)
        {
            FXLOGE("pixel overrun");
            pixel = 0;
        }
        Strip->SetPixelColor(Index(pixel), color);
    }
protected:
    FXStripBase* Strip;
    uint16_t Length;
    virtual uint16_t Index(uint16_t pixel) = 0;
};

class FXStripSegRange : public FXStripSegBase
{
public:
    FXStripSegRange(uint16_t start, uint16_t stop, FXStripBase* strip) : FXStripSegBase(strip), Start(start)
    {
        Length = stop-start+1;
        if (!strip)
            FXLOGE("FXStripBase is null!");
    }
    FXStripSegRange(uint16_t start, FXStripBase* strip) : FXStripSegBase(strip), Start(start)
    {
        Length = 1;
        if (!strip)
            FXLOGE("FXStripBase is null!");
    }
protected:
    uint16_t Index(uint16_t pixel) { return Start + pixel; }
private:
    uint16_t Start;
};

#define PMAP_END 0xFFFF

class FXStripSegMapped : public FXStripSegBase
{
public:
    FXStripSegMapped(const uint16_t* pmap, FXStripBase* strip) : FXStripSegBase(strip), Pmap(pmap)
    {
        for (uint8_t i = 0; ; i++)
        {
            if (Pmap[i] == PMAP_END)
            {
                Length = i;
                return;
            }
        }
    }

protected:
    uint16_t Index(uint16_t pixel) { return Pmap[pixel]; }
private:
    const uint16_t* Pmap;
};
