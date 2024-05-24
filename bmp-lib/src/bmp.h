#ifndef BMP_H
#define BMP_H

#include <Arduino.h>
#include <Wire.h>

#define BMP_ADDRESS 0x77 
// EEPROM registers addresses
// Note: these registers are 16-bit
#define AC1_H       0XAA
#define AC1_L       0XAB
#define AC2_H       0XAC
#define AC2_L       0XAD
#define AC3_H       0xAE
#define AC3_L       0xAF
#define AC4_H       0XB0
#define AC4_L       0XB1
#define AC5_H       0xB2
#define AC5_L       0xB3
#define AC6_H       0XB4
#define AC6_L       0XB5
#define B1_H        0xB6
#define B1_L        0xB7
#define B2_H        0XB8
#define B2_L        0XB9
#define MB_H        0xBA
#define MB_L        0xBB
#define MC_H        0xBC
#define MC_L        0xBD
#define MD_H        0XBE
#define MD_L        0XBF

class BMP180 {
    private:
    uint8_t _address;

    public:
    BMP180(uint8_t address); // constructor

    long temperature;
    long pressure;
    long altitude;

    // callibration params
    short ac1, ac2, ac3;
    unsigned short ac4, ac5, ac6;
    short b1, b2, mb, mc, md;

    void init();
    void bmp180_get_cal_param();
    long bmp180_get_ut();
    long bmp180_get_up();
    long bmp180_get_temperature();
    long bmp180_calc_pressure();

};

#endif