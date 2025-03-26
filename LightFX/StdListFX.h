#pragma once

// Unique IDs for the standard effects.
enum FX_LIST
{                              // color0  color1  reverse
    FX_STATIC,                 //   x                      Static Color0 fill.
    FX_BLINK,                  //   x       x       x      Blink with 50% duty cycle between Color0 and Color1.
    FX_STROBE,                 //   x       x       x      Short strobe flash of Color0 over Color1.
    FX_BLINK_RAINBOW,          //           x              Blinking with cycling rainbow colors alternating with Color1.
    FX_WIPE,                   //   x       x       x      Wipe Color0 on then erase with Color1 in the same direction.
    FX_WIPE_REV,               //   x       x       x      Wipe Color0 on then erase with Color1 in the reverse direction.
    FX_WIPE_RANDOM,            //           x       x      Wipe random color on then erase with Color1 in the same direction.
    FX_SCAN,                   //   x       x              Runs a single Color0 pixel back and forth over Color1.
    FX_DUAL_SCAN,              //   x       x              Runs two Color0 pixels back and forth in opposite directions over Color1.
    FX_BREATH,                 //   x                      Does the "standby-breathing" of well known i-Devices.
    FX_RANDOM_COLOR,           //                          Sets random lights to random colors.
    FX_SINGLE_DYNAMIC,         //                          Changes one random light after the other to another random color.
    FX_MULTI_DYNAMIC,          //                          Changes all lights each cycle to new random colors.
    FX_RAINBOW,                //                          Cycles all lights at once through a rainbow.
    FX_RAINBOW_CYCLE,          //                          Cycles a rainbow over all lights.
    FX_FADE,                   //   x                      Fades the lights on and (almost) off again.
    FX_THEATER_CHASE,          //   x       x       x      Theatre-style crawling lights; Color0 over Color1.
    FX_THEATER_CHASE_RAINBOW,  //           x       x      Theatre-style crawling lights; rainbow over Color1.
    FX_RUNNING_LIGHTS,         //   x               x      Running lights effect with smooth sine transition.
    FX_TWINKLE,                //   x       x              Blink several lights on, reset, repeat; Color0 over Color1.
    FX_TWINKLE_RANDOM,         //           x              Blink several lights on, reset, repeat; random colors over Color1.
    FX_TWINKLE_FADE,           //   x                      Blink several lights on in Color0, fading to black.
    FX_TWINKLE_FADE_RANDOM,    //                          Blink several lights on in random colors, fading to black.
    FX_SPARKLE,                //   x       x              Blinks one light at a time; Color0 over Color1.
    FX_CHASE,                  //   x       x       x      Color0 running on Color1.
    FX_CHASE_RAINBOW,          //   x               x      Color0 running on a solid rainbow.
    FX_CHASE_FLASH,            //   x       x       x      Color0 flashes running on Color1.
    FX_RUNNING,                //   x               x      Alternating color0/color1 pixels running.
    FX_CYLON,                  //   x               x      Cylon; Color0 over black.
    FX_COMET,                  //   x               x      Firing comets from one end in Color0.
    FX_FIREWORKS,              //   x                      Fireworks in Color0.
    FX_FIREWORKS_RANDOM,       //                   x      Fireworks in random colors.
    FX_FIRE_FLICKER,           //   x                      Random flickering in Color0.
    FX_FIRE_FLICKER_INTENSE,   //   x                      Random flickering in Color0, more intesity.
    FX_COUNT
};
