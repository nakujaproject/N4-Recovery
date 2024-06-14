// Uses the SerialFlash library from Paul Stoffregen
// Get it from: https://github.com/PaulStoffregen/SerialFlash
//
// we used a WINBOND 128Q device (around 8MB)

#include <SerialFlash.h>

#define FILE_SIZE_512K 524288L
#define FILE_SIZE_1M   1048576L
#define FILE_SIZE_4M   4194304L
// TODO: add different file sizes

SerialFlashFile file;

// sampling interval in milliseconds - seems to work at 10 milliseconds
const uint32_t sampleInterval = 10;

const byte PIN_FLASH_CS = 5; // Change this to match the Chip Select pin on your board

// edit this structure to hold one set of parameters you want to log
struct oneRecordType {  
  uint32_t recordNumber;
  uint32_t timeStamp;
  float temperature;  
  float pressure;
  float acceleration;
} oneRecord;

// use this array to create a filename
char filename[16];

uint8_t dumpFileNumber = 1;

// recording flag
uint8_t isRecording = false;

uint32_t prevMillis, currentMillis = 0;

// used purely for fake data generation
float fakeTemperature = 12.0;
float fakePressure = 1013.25;
float fakeAcceleration = 1.0;

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n\nRocket SPI Flash Data Logger - File Sys Version."));

  if (!SerialFlash.begin( PIN_FLASH_CS )) {
    Serial.println(F("SPI Flash not detected. Check wiring. Maybe you need to pull up WP/IO2 and HOLD/IO3? Freezing..."));
    while (1);
  }
  
  flashInfo();

  // how do we know that there is a filesystem on the chip?
  // there doesn't seem to be anything obvious in the library!
  if ( !SerialFlash.exists( "dummy.txt" )) {
    Serial.println(F("Flash doesn't appear to hold a file system - may need erasing first."));
  }
  else {
    Serial.println(F("Files currently in flash:"));
    SerialFlash.opendir();
    while (1) {
      uint32_t filesize;
      if (SerialFlash.readdir(filename, sizeof(filename), filesize)) {
        Serial.print(filename);
        Serial.print(F("  "));
        Serial.print(filesize);
        Serial.print(F(" bytes"));
        Serial.println();
      }
      else {
        break; // no more files
      }
    }
  }

  showMenu();
}

void loop() {
  // grab the latest elapsed time
  currentMillis = millis();

  // see if there are any serial commands received
  checkForSerialCommand();

  // are we recording?
  if (isRecording == true) {
    // is it time for another sample?
    if ( currentMillis - prevMillis > sampleInterval ) {
      prevMillis = currentMillis;

      // prepare the record for writing to the flash chip
      // read in the data from your real sensors here!!!!
      oneRecord.recordNumber++;
      oneRecord.timeStamp = currentMillis;
      oneRecord.temperature = getFakeTemperature();
      oneRecord.pressure = getFakePressure();
      oneRecord.acceleration = getFakeAcceleration();

      // write the record to the flash chip
      file.write( (uint8_t *)&oneRecord, sizeof(oneRecord) );
    }
  }
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
        dumpFileNumber = 1;
      case 'd':
        // d = just dump the next recording
        isRecording = false;
        dumpOneRecording();
        dumpFileNumber++;
        showMenu();
        break;

      // erase the flash - takes about 20 sec for my 25Q64 device
      case 'e':
      case 'E':
        isRecording = false;
        eraseFlash();
        showMenu();
        break;

      // start recording
      case 's':
      case 'S':
        Serial.println(F("START RECORDING"));
        getNextFlashFilename();
        Serial.print( F("Writing to file: " ));
        Serial.println(filename);
        SerialFlash.create(filename, FILE_SIZE_512K);
        file = SerialFlash.open(filename);
        fakeTemperature = 12.0;
        fakePressure = 1013.25;
        fakeAcceleration = 1.0;
        oneRecord.recordNumber = 0;
        isRecording = true;
        break;

      // stop recording
      case 'p':
      case 'P':
        Serial.println(F("STOP RECORDING"));
        Serial.print(F("Logged "));
        Serial.print( oneRecord.recordNumber );
        Serial.println(F(" records for that flight."));
        file.close();
        isRecording = false;
        break;

      default:
        showMenu();
        break;
    }
  }
}
// Display the menu of supported commands
void showMenu() {
  Serial.println(F("\nMENU OPTIONS:"));
  Serial.println(F("d : dump 1 recording (D to reset to 1st recording)"));
  Serial.println(F("e : erase flash (~20sec for 64mbit device)"));
  Serial.println(F("s : start recording"));
  Serial.println(F("p : stop recording"));
}

// generate a CSV formatted output of one flights worth of recordings
void dumpOneRecording() {
  Serial.println(F("\nDUMP ONE RECORDING"));

//  sprintf( filename, "flt%02d.bin", dumpFileNumber );
//  file = SerialFlash.open( filename );

  sprintf( filename, "flight1.bin");
  file = SerialFlash.open( filename );

  if (file) {
    Serial.print( F("Contents of file: ") );
    Serial.println( filename );
    
    // print out the headings
    Serial.println(F("Rec Num,Timestamp,Temp,Press,Accel"));
    
    do {
      file.read( (uint8_t *)&oneRecord, sizeof(oneRecord) );

      // library doesn't know where end of actual written data is so we have
      // to look for it ourselves!
      if ( oneRecord.recordNumber != 0xFFFFFFFF ) {
        Serial.print( oneRecord.recordNumber );
        Serial.print( "," );
        Serial.print( oneRecord.timeStamp );
        Serial.print( "," );
        Serial.print( oneRecord.temperature );
        Serial.print( "," );
        Serial.print( oneRecord.pressure );
        Serial.print( "," );
        Serial.println( oneRecord.acceleration );
      }
    } while ( oneRecord.recordNumber != 0xFFFFFFFF );
    file.close();
  }
  else {
    Serial.println(F("No more files."));
  }
}

// erase the flash and write our unique signature in the 1st record
void eraseFlash() {
  Serial.println(F("ERASING FLASH CHIP"));
  SerialFlash.eraseAll();

  // create a dummy file so we can look for it
  // this seems the only way of knowing if there is an actual
  // filesystem present when we startup ...
  SerialFlash.create( "dummy.txt", 16 );
  file = SerialFlash.open("dummy.txt");
  file.write( "NakujaTest", 11 );
  file.close();
  Serial.println(F("Done"));
}

// print out some details about the flash chip
void flashInfo() {
  uint8_t id[5];
  
  SerialFlash.readID(id);
  Serial.print(F("Capacity: "));
  Serial.println(SerialFlash.capacity( id ));
}

// Scans the flash chip to determine the next filename to use.
uint8_t getNextFlashFilename() {
  uint8_t fn=0;

  do {
    // create the filename and see if it exists
    sprintf( filename, "flt%02d.bin", ++fn );
    Serial.println( filename );
  } while ( (SerialFlash.exists( filename )) && (fn<10) );
  
  Serial.println( fn );
  return fn;
}

float getFakeTemperature() {
  fakeTemperature = fakeTemperature + 0.01;
  return fakeTemperature;
}

float getFakePressure() {
  fakePressure = fakePressure - 0.01;
  return fakePressure;
}

float getFakeAcceleration() {
  fakeAcceleration = fakeAcceleration + 0.01;
  return fakeAcceleration;
}
