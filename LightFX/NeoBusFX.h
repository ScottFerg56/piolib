#pragma once
#include "StripFX.h"
#include <NeoPixelBus.h>

template<typename T_PIXEL_METHOD> class FXNeoBus : public FXStripBase
{
public:
    FXNeoBus(T_PIXEL_METHOD& strip) : Strip(strip) {}
    void Begin()
    {
        Strip.Begin();
        Clear();
        Show();
    }
    void Show() { Strip.Show(); }
    void Clear() { Strip.ClearTo((RgbColor) (HtmlColor(0))); }
    void SetPixelColor(uint16_t pixel, uint32_t color)
    {
        if (pixel >= Strip.PixelCount())
        {
            FXLOGE("strip pixel overrun");
            pixel = 0;
        }
        Strip.SetPixelColor(pixel, (RgbColor)(HtmlColor(color)));
    }
    uint32_t GetPixelColor(uint16_t pixel)
    {
        if (pixel >= Strip.PixelCount())
        {
            FXLOGE("strip pixel overrun");
            pixel = 0;
        }
    	auto color = Strip.GetPixelColor(pixel);
        uint32_t c = color.R << 16 | color.G << 8 | color.B;
        return c;
    }
private:
    T_PIXEL_METHOD& Strip;
};
