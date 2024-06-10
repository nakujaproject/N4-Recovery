#include <SPIMemory.h>

const uint32_t sample_interval = 10;
const byte PIN_FLASH_CS = 5; 

SPIFlash myFlash(PIN_FLASH_CS);

// test data to write
struct fake_data{
  uint32_t flightNumber;
  uint32_t recordNumber;
  uint32_t timestamp;
  float ax;
  float ay;
  uint16_t alt;
};
uint32_t prevFlightNumber = 0;

struct fake_data dummy_data;
struct fake_data* p_dummy_data = &dummy_data;
char data_buffer[500];

const uint32_t flashMemTop = 16777216L;

// next address to write
uint32_t nextFlashWriteAddress = 0; 

// next address to read from 
uint32_t nextFlashReadAddress = sizeof(dummy_data);

// recording flag
uint8_t isRecording = false;
uint8_t isFlashDataValid = false;
uint32_t prevMillis, currentMillis = 0;
uint32_t rawDumpAddress = 0;

// print out some details about the flash chip
void flashInfo() {
  Serial.print(F("Manufacturer: "));
  Serial.println(myFlash.getManID(), HEX);
  Serial.print(F("Capacity: 0x"));
  Serial.println(myFlash.getCapacity(), HEX);
}

/**
 * Read record 0 from flash memory to see if it contains our signature phrase
 * May indicate that there's actual data rather than data from another app
 * 
 */
bool getMyFlashSignature() {
  // read the first record
  myFlash.readByteArray(0, (uint8_t*)&dummy_data, sizeof(dummy_data));

  if(dummy_data.flightNumber == 0x524F434B) return true;

  return false;
  
}

/**
 * Erase flash chip
 */
void eraseFlash() {
  Serial.println(F("ERASING FLASH CHIP"));
  myFlash.eraseChip();

  // visual indicator of erasing
//  while(!myFlash.eraseChip()) {
//    // TODO: blink LED at 10 Hz
//  }

  // turn off LED solid here
  
  Serial.println(F("Writing unique signature"));
  dummy_data.flightNumber = 0x524f434b; // "ROCK"
  dummy_data.recordNumber = 0;
  dummy_data.timestamp = 0;
  dummy_data.ax = 0.0;
  dummy_data.ay = 0.0;
  dummy_data.alt = 1000;

  myFlash.writeByteArray(0, (uint8_t*)&dummy_data, sizeof(dummy_data) );
  nextFlashWriteAddress = sizeof(dummy_data);

  Serial.println(F("Done"));
  
}

// Scans the flash chip to locate the address of the next free unused memory location
// using a crude linear search from the start of memory upwards.
uint32_t getNextFlashStartAddress() {

  // start at first record past our signature header
  uint32_t testAddress = sizeof(dummy_data);

  myFlash.readByteArray(testAddress, (uint8_t *)&dummy_data, sizeof(dummy_data) );
  if ( dummy_data.flightNumber != 0xFFFFFFFF ) {
    // there's some data recorded so find the end of it
    while ( dummy_data.flightNumber != 0xFFFFFFFF ) {
      prevFlightNumber = dummy_data.flightNumber;
      testAddress = testAddress + sizeof(dummy_data);
      myFlash.readByteArray( testAddress, (uint8_t *)&dummy_data, sizeof(dummy_data) );

      if ( testAddress > flashMemTop ) break;
    }
  }
  return testAddress;
}

void dumpOneRecording() {
  Serial.println("\n Dumping one recording");
//  myFlash.readByte(nextFlashReadAddress, (uint8_t*)&dummy_data, sizeof(dummy_data));
  myFlash.readByte(nextFlashReadAddress, (uint8_t*)&dummy_data);
  nextFlashReadAddress = nextFlashReadAddress + sizeof(dummy_data);

   // quick sanity check - the flight number should NOT be 0xFFFFFFFF
  if (dummy_data.flightNumber != 0xFFFFFFFF) {
    // remember which flight this is
    uint32_t thisFlightNumber = dummy_data.flightNumber;

    // print out the headingson
    Serial.println(F("Flt Num,Rec Num,Timestamp,ax,ay,alt"));

    // erased flash memory reads as 0xFF so use that to check if the end of the data has been reached
    // also stop if the flight number has changed
    while ( (dummy_data.flightNumber != 0xFFFFFFFF ) && (dummy_data.flightNumber == thisFlightNumber ) )
    {
      Serial.print( dummy_data.flightNumber );
      Serial.print( "," );
      Serial.print( dummy_data.recordNumber );
      Serial.print( "," );
      Serial.print( dummy_data.timestamp );
      Serial.print( "," );
      Serial.print( dummy_data.ax );
      Serial.print( "," );
      Serial.print( dummy_data.ay );
      Serial.print( "," );
      Serial.println( dummy_data.alt );

      // read the next record from the flash
      myFlash.readByteArray(nextFlashReadAddress, (uint8_t *)&dummy_data, sizeof(dummy_data) );
      nextFlashReadAddress = nextFlashReadAddress + sizeof(dummy_data);
    }

    // reverse by 1 record as we may have read 1st record of the next flight
    nextFlashReadAddress = nextFlashReadAddress - sizeof(dummy_data);
  } else {
    Serial.println(F("No more flights"));
  }

}

void showMenu() {
  Serial.println(F("\nMENU OPTIONS:"));
  Serial.println(F("d : dump 1 recording (D to reset to 1st recording)"));
  Serial.println(F("e : erase flash (~20sec for 64mbit device)"));
  Serial.println(F("r : raw dump 1K blocks (R to restart at 0x0000)"));
  Serial.println(F("s : start recording"));
  Serial.println(F("p : stop recording"));
}

// Handle serial port commands
void checkForSerialCommand() {
  if (Serial.available() != 0) {
    char choice = Serial.read();

    switch ( choice ) {
      // ignore CR and LF characters
      case '\n':   // LF
      case '\r':   // CR
        break;

      // dump a single recording
      case 'D':
        // D = start with 1st recording
        nextFlashReadAddress = sizeof(dummy_data);
      case 'd':
        // d = just dump the next recording
        isRecording = false;
        dumpOneRecording();
        showMenu();
        break;

      // erase the flash - takes about 20 sec for my 25Q64 device
      case 'e':
      case 'E':
        isRecording = false;
        prevFlightNumber = 0;
        eraseFlash();
        showMenu();
        break;
//
//      // dump raw memory from the flash
//      case 'R':
//        // R = start at address 0x00000000
//        rawDumpAddress = 0;
//      case 'r':
//        // r = dump next 1k block
//        isRecording = false;
//        dumpRawMemory();
//        break;
//
//      // start recording
//      case 's':
//      case 'S':
//        Serial.println(F("START RECORDING"));
//        fakeTemperature = 12.0;
//        fakePressure = 1013.25;
//        fakeAcceleration = 1.0;
//        dummy_data.flightNumber = prevFlightNumber + 1;
//        dummy_data.recordNumber = 0;
//        isRecording = true;
//        break;
//
//      // stop recording
//      case 'p':
//      case 'P':
//        Serial.println(F("STOP RECORDING"));
//        prevFlightNumber = dummy_data.flightNumber;
//        Serial.print(F("Logged "));
//        Serial.print( dummy_data.recordNumber );
//        Serial.println(F(" records for that flight."));
//        Serial.print(F("Next free memory location is: 0x"));
//        Serial.println( nextFlashWriteAddress, HEX );
//        isRecording = false;
//        break;

      default:
        showMenu();
        break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nRocket data logger");

  if(myFlash.begin() == false) {
    Serial.println(F("Flash not detected"));
  }
  flashInfo();

  // check to see if flash holds our recordings 
  if(getMyFlashSignature() == false) {
    Serial.println(F("Flash does not appear to hold valid data. Erase"));

    // erase flash 
    eraseFlash();
    
    isFlashDataValid= false;
    
  } else {
    isFlashDataValid = true;

    // scan the flash for the next available free memory
    nextFlashWriteAddress = getNextFlashStartAddress();
    Serial.print(F("Next free mem loc: "));
    Serial.println(nextFlashWriteAddress, HEX);  
  }

  showMenu();

  randomSeed(analogRead(13));
  // generate fake data 
  p_dummy_data->ax = random(0, 56);
  p_dummy_data->ay = random(0, 10);
  p_dummy_data->alt = random(0, 1500);

  sprintf(data_buffer,
          "%.2f, %.2f, %d", 
          p_dummy_data->ax,
          p_dummy_data->ay,
          p_dummy_data->alt);

  Serial.print(F("Data to write: "));
  Serial.println(data_buffer);
  Serial.println();
}

void loop() {
  checkForSerialCommand();
}
