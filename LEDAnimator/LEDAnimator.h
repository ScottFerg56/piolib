#ifndef _LEDANIMATOR_H
#define _LEDANIMATOR_H

#include <Arduino.h>
#define FASTLED_INTERNAL // avoid spurious warnings about SPI!
#include "FastLED.h"

struct ANIM
{
    const uint32_t	*pixelData;	    // packed pixel data
    const uint32_t	*colorTable;	// colors indexed by pixel data
    uint8_t			numPixels;      // number of pixels in a frame
    uint8_t			numFrames;      // number of frames in animation
    uint8_t         bitsPerPixel;   // number of bits per pixel in pixel data (must factor cleanly into 32 bit data words)
};

template<template<uint8_t DATA_PIN, EOrder RGB_ORDER> class CHIPSET, uint8_t DATA_PIN, EOrder RGB_ORDER> class LEDAnimator
{
public:
    CRGB* leds;     // working storage for LED colors
    int NumLeds;    // number of LEDs

    LEDAnimator(int nLeds)
    {
        NumLeds = nLeds;
    }

    void Init()
    {
        leds = new CRGB[NumLeds];
        FastLED.addLeds<CHIPSET, DATA_PIN, RGB_ORDER>(leds, NumLeds);
    }

    void ChangeIterations(uint16_t iters) { Iterations = iters; }
    void ChangeBrightness(uint8_t brightness) { maxBrightness = brightness; }

protected:
    ANIM*       pAnim;  		// current animation
    uint16_t	msPerFrame;	  	// ms per frame for current animation
    uint16_t    Iterations;     // number of iterations for current animation
    uint16_t	pixelInx;		// index of current pixel [0..numPixels*numFrames)
    uint16_t    dataInx;        // index of current pixelData
    uint32_t    dataWord;       // current pixel data word
    uint32_t    dataMsk;        // mask into dataWord for current pixel
    uint16_t    bitsLeft;       // number of unprocessed bits in dataWord
    uint32_t	prevFrameTime;	// time of last frame update
    uint8_t     maxBrightness;  // max brightness for animation
    uint8_t     currBrightness; // current brightness state (while fading)
    uint8_t     fadeInInc;      // brightness increment (+) for fade in
    uint8_t     fadeOutInc;     // brightness decrement (+) for fade out
    uint8_t     fadeInCnt;      // number of increments for fade in
    uint8_t     fadeOutCnt;     // number of increments for fade out
    uint8_t     fadeCnt;        // current count for current fade operation in progress
    uint32_t    fadeDelay;      // ms delay between fade increments

    enum AnimStates             // current state of an animation frame
    {
        FrameStart,             // starting
        FrameEnd,               // frame complete
        FadingIn,               // fading in (multiple increments)
        FadingOut,              // fading out (multiple increments)
        FadingLast,             // last fade phase complete (last increment with fadeDelay)
    };

    AnimStates AnimState;       // state of current animation frame

public:
    // start an animation
    void SetAnim(ANIM* panim, uint16_t msperframe, uint8_t brightness, uint16_t iterations, uint8_t fadeIn = 0, uint8_t fadeOut = 0)
    {
        maxBrightness = brightness;
        pAnim = panim;
        Iterations = iterations;
        msPerFrame = msperframe;
        fadeInInc = fadeIn;
        fadeOutInc = fadeOut;
        if (pAnim == nullptr)
            return;
        fadeInCnt = fadeInInc == 0 ? 0 : maxBrightness / fadeInInc;
        fadeOutCnt = fadeOutInc == 0 ? 0 : maxBrightness / fadeOutInc;
        fadeCnt = 0;
        fadeDelay = 0;
        // calculate fadeDelay
        if (fadeInCnt + fadeOutCnt > 0)
        {
            fadeDelay = msPerFrame / (fadeInCnt + fadeOutCnt);
            if (fadeDelay < 20)
                fadeDelay = 20;
        }
        AnimState = FrameStart;
        // mask starts with first pixel in LS bits
        dataMsk = (1 << pAnim->bitsPerPixel) - 1;
        // initialize pixel data access for first pixel in tthe first frame
        pixelInx = 0;
        dataInx = 0;
        bitsLeft = 32;
        dataWord = pAnim->pixelData[dataInx];
        // set to start on next Loop call
        prevFrameTime = millis() + msperframe;
    }

protected:
    // get the color of the current pixel and advance to next
    uint32_t NextPixel()
    {
        // data word is always pre-shifted to mask off LS bits for color index
        uint32_t v = dataWord & dataMsk;
        // index into the color table
        uint32_t c = pAnim->colorTable[v];
        // advance the pixel index and check for end of animation
        // (there may be unused bitsLeft in dataWord)
        if (++pixelInx >= pAnim->numPixels * pAnim->numFrames)
        {
            // reset to start for next (possible) iteration
            pixelInx = 0;
            dataInx = 0;
            bitsLeft = 32;
            dataWord = pAnim->pixelData[dataInx];
        }
        else
        {
            // advance to the next pixel
            // drop bit count
            // bitsPerPixel must factor cleanly into 32 bit data words: 1, 2, 4, 8, 16, 32
            bitsLeft -= pAnim->bitsPerPixel;
            if (bitsLeft > 0)
            {
                // more bits in the word, so shift 'em down
                dataWord >>= pAnim->bitsPerPixel;
            }
            else
            {
                // go to the next pixelData word
                dataInx++;
                bitsLeft = 32;
                dataWord = pAnim->pixelData[dataInx];
            }
        }
        return c;
    }

public:
    // called from main 'loop' function
    // check for an animation frame to process
    bool Loop()
    {
        if (pAnim == nullptr || pAnim->pixelData == nullptr)
            return false;

        uint32_t t = millis();
        // using fadeDelay if currently processing a fade operation
        uint32_t ms = AnimState > FrameEnd ? fadeDelay : msPerFrame;

        if (t - prevFrameTime >= ms)
        {
            prevFrameTime = t;


            switch (AnimState)
            {
            case FrameStart:
            case FrameEnd:
            case FadingLast:    // just so last fading operation can have a shorter fadeDelay
                if (AnimState == FrameEnd || AnimState == FadingLast)
                {
                    AnimState = FrameStart;
                    if (pixelInx == 0)
                    {
                        // pixelInx zero AFTER doing a frame
                        // means we reached the end of the animation
                        if (Iterations != 0 && --Iterations == 0)
                        {
                            // clear (reset) for next animation (but don't show)
                            for (uint8_t i = 0; i < pAnim->numPixels; i++)
                                leds[i] = 0;
                            FastLED.setBrightness(maxBrightness);
                            pAnim = nullptr;
                            return false;
                        }
                    }
                }

                // set pixel colors for the next frame
                for (uint8_t i = 0; i < pAnim->numPixels; i++)
                {
                    uint32_t c = NextPixel();
                    leds[i] = c;
                }
                // kill the brightness if we're fading in
                FastLED.setBrightness(fadeInCnt > 0 ? 0 : maxBrightness);
                FastLED.show();

                if (fadeInCnt > 0)
                {
                    // start frame with fade in operation
                    fadeCnt = fadeInCnt;
                    currBrightness = 0;
                    AnimState = FadingIn;
                    return true;
                }

                if (fadeOutCnt > 0)
                {
                    // start frame with fade out operation
                    fadeCnt = fadeOutCnt;
                    currBrightness = maxBrightness;
                    AnimState = FadingOut;
                    return true;
                }

                AnimState = FrameEnd;
                break;
            
            case FadingIn:
                // another increment done
                --fadeCnt;
                currBrightness += fadeInInc;
                if (fadeCnt == 0)
                {
                    // fade in complete
                    currBrightness = maxBrightness;
                    if (fadeOutCnt > 0)
                    {
                        // finish frame with fade out operation
                        fadeCnt = fadeOutCnt;
                        AnimState = FadingOut;
                    }
                    else
                    {
                        // not fading out, let this frame finish with a fadeDelay, then end
                        AnimState = FadingLast;
                    }
                }
                FastLED.setBrightness(currBrightness);
                FastLED.show();
                break;
            
            case FadingOut:
                // another increment done
                --fadeCnt;
                currBrightness -= fadeOutInc;
                if (fadeCnt == 0)
                {
                    FastLED.setBrightness(0);
                    // fade out complete
                    currBrightness = maxBrightness;
                    // let this frame finish with a fadeDelay, then end
                    AnimState = FadingLast;
                }
                else
                {
                    FastLED.setBrightness(currBrightness);
                }
                FastLED.show();
                break;
            }
        }
        return true;
    }

};

#endif // _LEDANIMATOR_H
