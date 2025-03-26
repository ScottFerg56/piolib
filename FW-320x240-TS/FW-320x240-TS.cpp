#include <Adafruit_STMPE610.h>
#include <lvgl.h>
#if LV_USE_TFT_ESPI
#define DISABLE_ALL_LIBRARY_WARNINGS
#include <TFT_eSPI.h>
#endif

#ifndef CALIBRATING_TS
#define CALIBRATING_TS 0
#endif

#define STMPE_CS 32
#define SD_CS    14

Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_WIDTH * TFT_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

#if CALIBRATING_TS
int16_t calib[4] = {2000, 2000, 2000, 2000};  // touch screen calibration
#else
int16_t calib[4] = {437,3803,335,3751}; // touch screen calibration
#endif

bool wasPressed = false;

/*Read the touchpad*/
void my_touchpad_read(lv_indev_t* indev, lv_indev_data_t* data)
{
    lv_display_t * disp = lv_indev_get_display(indev);
    // active rotation
    auto rot = lv_display_get_rotation(disp);
    // these res values change with rotation (the internal hor_res and ver_res stick to the rotation 0 values)
    auto hor_res = lv_display_get_horizontal_resolution(disp);
    auto ver_res = lv_display_get_vertical_resolution(disp);

    bool touched = !ts.bufferEmpty();
    if (!touched)
    {
        data->state = LV_INDEV_STATE_RELEASED;
        if (!wasPressed)
            return;
        wasPressed = false;
    }
    else
    {
        TS_Point p = ts.getPoint();

#if CALIBRATING_TS
        // TS in portrait mode has X top->bottom and Y left->right
        //   when the USB port is at top left
        int16_t u = p.x;
        int16_t v = p.y;
        bool chg = false;
        if (u < calib[0]) { calib[0] = u; chg = true; }
        if (u > calib[1]) { calib[1] = u; chg = true; }
        if (v < calib[2]) { calib[2] = v; chg = true; }
        if (v > calib[3]) { calib[3] = v; chg = true; }
        if (chg)
        {
            Serial.print("{");
            Serial.print(calib[0]); Serial.print(",");
            Serial.print(calib[1]); Serial.print(",");
            Serial.print(calib[2]); Serial.print(",");
            Serial.print(calib[3]);
            Serial.println("}");
        }
#endif

        /*
        TFT_WIDTH and TFT_HEIGHT are the default portrait dimensions, 240x320
        When the USB port is at top right, portrait, the TS origin is at top right,
        with X left along the short side and Y down along the long side.
        Scale input TS coordintes using the calibration #'s.

        NOTE:
        The LV_DISPLAY_ROTATION_0 case is the coordinate system we should use
        for the return coordinates regardless of rotation.
        
        Here is the transform LVGL applies to the coordinates in lvgl\src\indev\lv_indev.c:indev_pointer_proc() after
        calling us:
        
            if(disp->rotation == LV_DISPLAY_ROTATION_180 || disp->rotation == LV_DISPLAY_ROTATION_270) {
                data->point.x = disp->hor_res - data->point.x - 1;
                data->point.y = disp->ver_res - data->point.y - 1;
                }
            if(disp->rotation == LV_DISPLAY_ROTATION_90 || disp->rotation == LV_DISPLAY_ROTATION_270) {
                int32_t tmp = data->point.y;
                data->point.y = data->point.x;
                data->point.x = disp->ver_res - tmp - 1;
                }

        The second part rotates the touch point 90 degrees,
        but in the opposite direction from the rotation of display.
        */

        int16_t x = 0, y = 0;
        switch (rot)
        {
        case LV_DISPLAY_ROTATION_0:     // portrait (USB top right)
        case LV_DISPLAY_ROTATION_180:   // portrait (USB bottom left)
            // flip the X axis for conversion from the TS system to the ROT_0 system
            // thess cases are not subject to the 'wrong' 90 degree rotation in LVGL upon our return
            x = map(p.x, calib[0], calib[1], TFT_WIDTH-1, 0);
            y = map(p.y, calib[2], calib[3], 0, TFT_HEIGHT-1);
            break;
        
        case LV_DISPLAY_ROTATION_90:    // landscape (USB top left)
        case LV_DISPLAY_ROTATION_270:   // landscape (USB bottom right)
            // this inversion of both axes results in a 180 degree rotation
            // which compensates for the 'wrong' 90 degree rotation in LVGL upon our return
            x = map(p.x, calib[0], calib[1], 0, TFT_WIDTH-1);
            y = map(p.y, calib[2], calib[3], TFT_HEIGHT-1, 0);
            break;

        default:
            Serial.print("ROT NOT IMPLEMENTED");
            break;
        }
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = constrain(x, 0, TFT_WIDTH-1);
        data->point.y = constrain(y, 0, TFT_HEIGHT-1);
        wasPressed = true;
    }
}

bool setupFW_320x240_TS()
{
    if (!ts.begin())
        Serial.println("Touchscreen init FAILED");

    lv_display_t* disp = lv_tft_espi_create(TFT_WIDTH, TFT_HEIGHT, draw_buf, sizeof(draw_buf));
    if (!disp)
        return false;

    lv_indev_t* indev = lv_indev_create();
    if (!indev)
        return false;
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
    lv_indev_set_read_cb(indev, my_touchpad_read);
    return true;
}
