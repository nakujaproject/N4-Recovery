/**
 * Author: Edwin Mwiti
 * Email: emwiti658@gmail.com
 * Date: 7/6/2024
 * 
 * This file implements the onboard logging functions for the flight computer 
 * We use the SerialFlash library by Paul, link to the library is provided in the references
 * section
 * 
 * If you are using more than one SPI flash memory, create an instance for each SPI flash memory
 * 
*/

#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <SerialFlash.h>

class Logger {
    public:
        uint8_t _cs_pin;         // Chip select pin for the SPI flash memory
        uint8_t _flash_led;      // this LED flashes at a 10Hz frequency when the SPI flash chip is foramtting
        char _filename[15];      // filename of the log file
        uint32_t _file_size;    // how large do you want the file 

    private:
        uint8_t _flash_delay = 100; // 100ms delay gives a frequency of 20Hz
        uint8_t _file_pointer = 0;   // pointer to the start of the file- to be used when reading the file 

        Logger(uint8_t CS_PIN); // constructor
        bool loggerInit();
        void loggerFormat();
        bool loggerTest();
        void loggerWrite(struct* data);
        void loggerRead(uint8_t file_pointer, char buffer);
};


#endif