#include <SPI.h>
#include <SparkFun_SPI_SerialFlash.h>

const byte PIN_FLASH_CS = 5;
SFE_SPI_FLASH myFlash;

void setup() {
  Serial.begin(115200);

  // start the SPI flash
  if(myFlash.begin(PIN_FLASH_CS) == false) {
    Serial.println(F("SPI flash not detected..."));
  } 
  
  Serial.println(F("Flash memory found"));
  sfe_flash_manufacturer_e mfgID = myFlash.getManufacturerID();
  if (mfgID != SFE_FLASH_MFG_UNKNOWN)
  {
    Serial.print(F("Manufacturer: "));
    Serial.println(myFlash.manufacturerIDString(mfgID));
  }
  else
  {
    uint8_t unknownID = myFlash.getRawManufacturerID(); // Read the raw manufacturer ID
    Serial.print(F("Unknown manufacturer ID: 0x"));
    if (unknownID < 0x10) Serial.print(F("0")); // Pad the zero
    Serial.println(unknownID, HEX);
  }

  Serial.print(F("Device ID: 0x"));
  Serial.println(myFlash.getDeviceID(), HEX);
}

void loop() {
  Serial.println();
  Serial.println("r) read HEX values, 1k bytes");
  Serial.println("v) view ASCII values, 1K bytes");
  Serial.println("d) dump raw values, 1k bytes");
  Serial.println("w) write test values, 1k bytes");
  Serial.println("e) erase entire chip");

  while(Serial.available()) {
    Serial.read();
  } 

  // wait for character
  while( Serial.available() == 0); // wait for character

  byte choice = Serial.read();

  // read HEX values
  if(choice == 'r') {
    Serial.println("Read raw values for 1024 bytes");

    for(int x=0; x<0x0400; x++) {
      if(x % 16 == 0) {
        Serial.println();
        Serial.print("0x");
        if(x < 0x100) Serial.print("0");
        if(x < 0x10) Serial.print("0");
        Serial.print(x, HEX);
        Serial.print(": ");
      }

      byte val = myFlash.readByte(x);
      if(val < 0x10) Serial.print("0");
      Serial.print(val, HEX);
      Serial.print(" ");
    }

    Serial.println();

  // view ASCII values
  } else if (choice == 'v') {
    Serial.println("View ASCII for 1024 bytes");

    for(int x=0; x< 0x400;x++){
      if(x%16 == 0) Serial.println();
      byte val = myFlash.readByte(x);
      if(isAlphaNumeric(val)) 
        Serial.write(val);
    }

  // dump raw values   
  } else if (choice == 'd') {
    for(int x = 0; x < 0x400; x++) {
      byte val = myFlash.readByte(x);
      Serial.write(val);
    }

    // erase entire chip
  } else if(choice == 'e') {
    Serial.println("Erasing entire chip");
    myFlash.erase();

    // write HEX values to the first 1024 bytes 
  } else if(choice == 'w') {
    Serial.println("Writing test values to the first 1024 bytes");
    uint8_t myVal[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    for(int x=0; x<0x400; x+=4) {
      myFlash.writeBlock(x, myVal, 4); // address, pointer, size
    }
  } else {
    Serial.print("Unknown choice: ");
    Serial.write(choice);
    Serial.println();
  }
  
}
