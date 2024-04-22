#include <SPI.h>
#include <Wire.h>
#include "SSD1306Wire.h"

class OledDisplay {
    private:
      SSD1306Wire displayOled;

    public:
      OledDisplay(short sda_pin, short scl_pin)
      : displayOled(0x3c, sda_pin, scl_pin, GEOMETRY_128_32)
      {
        //Initialising the UI will init the display too.
        displayOled.init();
        displayOled.flipScreenVertically();
        displayOled.setTextAlignment(TEXT_ALIGN_LEFT);
        displayOled.setFont(ArialMT_Plain_10);
      }

      void printText(String message){
        displayOled.clear();
        displayOled.drawString(0, 0, message);
        displayOled.display();
      }

      int x, minX;
      void printScrollText(String message)
      {
        while(true){
          
        }

        displayOled.clear();
        displayOled.setFont(ArialMT_Plain_10);
        displayOled.drawString(x, 0, message);

        displayOled.display();
        x=x+8; // scroll speed, make more positive to slow down the scroll
        if(x < minX) x= displayOled.width();
      }
};