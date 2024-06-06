#include <SPI.h>
#include <SerialFlash.h>

#define CS_PIN 5

void getChipDetails() {
  unsigned char buf[256];
  unsigned long address, count, chip_size, block_size;

  Serial.println();
  Serial.println(F("Reading chip ID: "));
  SerialFlash.readID(buf);
  Serial.print(F("JECEC ID:\t"));
  Serial.print(buf[0], HEX);
  Serial.print(' ');
  Serial.print(buf[1], HEX);
  Serial.print(' ');
  Serial.println(buf[2], HEX);
  Serial.print(F("Memory size:\t"));
  chip_size =  SerialFlash.capacity(buf);
  Serial.print(chip_size);
  Serial.println(F("\tBytes"));

  Serial.print(chip_size/(1024*1024));
  Serial.println(F(" MB"));
  
  
}

void setup() {
  Serial.begin(115200);
  while(!Serial) ; // wait for serial 
  delay(100);

  if(!SerialFlash.begin(CS_PIN)) {
    while (1) {
      Serial.println("Unable to access SPI Flash chip");
      delay(1000);
    }
  }

  Serial.println("Flash chip found");

  // get chip details 
  getChipDetails();
  

}

void loop() {
  // put your main code here, to run repeatedly:

}
