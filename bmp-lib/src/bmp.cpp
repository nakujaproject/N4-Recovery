#include "bmp.h"

////////////// procedure to read BMP /////////////////
// 1. start
// 2. start temperature measurement
// 3. read UT
// 4. start pressure measurement
// 5. wait 
// 6. Read UP
// 7. Calculate pressure and temperature in physical units 
// 8. goto 2

/**
 * Constructor
*/
BMP180::BMP180(uint8_t address) {
    this->_address = address;
}

/**
 * Init BMP180
*/
void BMP180::init() {
    Wire.begin(static_cast<int>(SDA), static_cast<int>(SCL));
    Wire.beginTransmission(this->_address);
    Wire.endTransmission(true);
    delay(10);
}

/**
 * Get callibation parameters
*/
void BMP180::bmp180_get_cal_param() {
    Wire.beginTransmission(this->_address);
    Wire.write(AC1_H);
    Wire.endTransmission(true);

    Wire.requestFrom(this->_address, 2, false);
    this->ac1 = Wire.read()<<8|Wire.read();
    
}

