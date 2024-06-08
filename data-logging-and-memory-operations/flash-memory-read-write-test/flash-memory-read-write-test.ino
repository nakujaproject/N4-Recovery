/**
 * Author: Edwin Mwiti 
 * Email: emwiti658@gmail.com
 * Date: 6th June 2024
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

// change the CS_PIN to the pin being used as chip select
#define CS_PIN 5
#define FLASH_LED 4

SerialFlashFile file;
const char* filename = "test.csv";
float filesize = 4096;

// test data to write
struct fake_data {
  float ax;
  float ay;
  int alt;
};

char data_buffer[100]; // to hold csv formatted data

struct fake_data dummy_data;
struct fake_dats* p_dummy_data;

bool initFlash() {
  if(!SerialFlash.begin(CS_PIN)) {
    return false;
  }
  return true;
}

void writeToFile(struct fake_data* d) {
  // try opening file 
  file = SerialFlash.open(filename);
  if(file) {
    Serial.println("File ready for R/W");
    
  }

  // close file after writing
  
}

void initFlashLED() {
  pinMode(FLASH_LED, OUTPUT);
}

void setup() {
  Serial.begin(115200);
  initFlashLED();

  if(initFlash()) {
    Serial.println("Ready to go");
  } else Serial.println("Filed memory init");

  //create file 
  Serial.print("Creating file...");
  Serial.println(filename);
  if(!SerialFlash.exists(filename) ) {
    int s = SerialFlash.createErasable(filename,filesize);
    if(s) {
      Serial.println("File created");
    } else {
      Serial.println("Failed to create file");
    }
  } else {
    Serial.println("File already exists");
  }
  randomSeed(analogRead(13));

  // generate fake data 
  p_dummy_data.ax = random(0, 56);
  p_dummy_data.ay = random(0, 10);
  p_dummy_data.alt = random(0, 1500);

  sprintf(data_buffer,
          "%.2f, .2f, %d", 
          p_dummy_data.ax,
          p_dummy_data.ay,
          p_dummy_data.alt);

  Serial.println(data_buffer);

  // write to file 
  
}


void loop() {
  

}
