/**
 * Author: Edwin Mwiti Maingi
 * Email: emwiti658@gmail.com
 * 
 * This file implements readin and writing from a SPI flash
 * chip
 * We read random data from the serial, put it into a buffer and dump it into a file in 
 * the SPI flash memory
 * 
 * We use the ESP32-Devkit module here with the CS pin as pin 5
 * 
 * Functions for formatting the SPI flash are also included, with an LED 
 * to indicate when the flash is formatting
 * The led blinks at 20 Hz when formatting and then remains lit continously 
 * after formatting is done
 * 
 */

#include <SPI.h>
#include <SerialFlash.h>

// file with a size of 4KB
String filename = "log.bin";
uint32_t file_size = 4096; // size in bytes

#define CS_PIN 5

bool initFlash() {
  if(!SerialFlash.begin(CS_PIN)) {
    return false;
  }

  return true;
  
}

void formatFlash() {
  uint8_t id[5];
  SerialFlash.readID(id);
  SerialFlash.eraseAll();
}

void setup() {
  Serial.begin(115200);
  //wait for serial
  while(!Serial);
  delay(100);

  int flash_start_flag = initFlash();

  // start by formatting the flash memory
  

  // create a file
  if(flash_start_flag) {
    // TODO: update system check table
    SerialFlash.create(filename, filesize);
  }
   
}

void loop() {
  // put your main code here, to run repeatedly:

}
