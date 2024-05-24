#include <Arduino.h>
#include "bmp.h"

// create a BMP180 object 
BMP180 altimeter(0x77);

void setup() {
    Serial.begin(115200);
    delay(100);

    altimeter.init();

    altimeter.bmp180_get_cal_param();
    Serial.println(altimeter.ac1);
}

void loop() {

}