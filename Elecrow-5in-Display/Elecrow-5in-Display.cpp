#include <Wire.h>
#include <SPI.h>
#include <PCA9557.h>
#include <lvgl.h>

/*******************************************************************************
   Config the display panel and touch panel in gfx_conf.h
 ******************************************************************************/
#include "gfx_conf.h"

#define DRAW_BUF_SIZE (screenWidth * screenHeight / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];
static lv_display_t* disp;

PCA9557 Out;    //for touch timing init

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t * px_map)
{
   uint32_t w = ( area->x2 - area->x1 + 1 );
   uint32_t h = ( area->y2 - area->y1 + 1 );
   tft.pushImageDMA(area->x1, area->y1, w, h,(lgfx::rgb565_t*)px_map);

   lv_disp_flush_ready( disp );
}

void my_touchpad_read(lv_indev_t *indev_driver, lv_indev_data_t *data)
{
   uint16_t touchX, touchY;
   bool touched = tft.getTouch( &touchX, &touchY);
   if( !touched )
   {
      data->state = LV_INDEV_STATE_REL;
   }
   else
   {
      data->state = LV_INDEV_STATE_PR;

      /*Set the coordinates*/
      data->point.x = touchX;
      data->point.y = touchY;
   }
}

bool setupElecrow_5in_Display()
{
  //GPIO init
#if defined (CrowPanel_50) || defined (CrowPanel_70)
  pinMode(38, OUTPUT);
  digitalWrite(38, LOW);
  pinMode(17, OUTPUT);
  digitalWrite(17, LOW);
  pinMode(18, OUTPUT);
  digitalWrite(18, LOW);
  pinMode(42, OUTPUT);
  digitalWrite(42, LOW);

  //touch timing init
  Wire.begin(19, 20);
  Out.reset();
  Out.setMode(IO_OUTPUT);
  Out.setState(IO0, IO_LOW);
  Out.setState(IO1, IO_LOW);
  delay(20);
  Out.setState(IO0, IO_HIGH);
  delay(100);
  Out.setMode(IO1, IO_INPUT);

#elif defined (CrowPanel_43)
  pinMode(20, OUTPUT);
  digitalWrite(20, LOW);
  pinMode(19, OUTPUT);
  digitalWrite(19, LOW);
  pinMode(35, OUTPUT);
  digitalWrite(35, LOW);
  pinMode(38, OUTPUT);
  digitalWrite(38, LOW);
  pinMode(0, OUTPUT);//TOUCH-CS
#endif

  //Display Prepare
  tft.begin();
  //tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  delay(200);

  disp = lv_display_create(screenWidth, screenHeight);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER); /*Touchpad should have POINTER type*/
  lv_indev_set_read_cb(indev, my_touchpad_read);

  tft.fillScreen(TFT_BLACK);
  return true;
}
