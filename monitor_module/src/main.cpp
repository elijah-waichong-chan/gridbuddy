#include "screen.h"
#include "measurement.h"
#include "wireless.h"
#include "Display_ST7789.h"
#include "LVGL_Driver.h"

void setup() {
  Serial.begin(115200);
  LCD_Init();
  Lvgl_Init();
  wireless_Init();
  Measurement_Init();
  startScreen();
  delay(1000);
}

void loop() 
{
  Measure();
  SendMessage();
  ButtonAction();
  Timer_Loop();
}
