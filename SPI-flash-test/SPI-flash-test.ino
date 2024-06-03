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
  

}
