/**
 * Use this program to read the recorded data from the onboard flash 
 * memory
 */

#include <SerialFlash.h>

const byte PIN_FLASH_CS = 5; // Change this to match the Chip Select pin on your board
SerialFlashFile file;
#include "data-types.h"

const byte PIN_FLASH_CS = 5; // Change this to match the Chip Select pin on your board
SerialFlashFile file;
uint8_t dumpFileNumber = 1;

// telemetry packet
telemetry_type_t oneRecord;

void setup() {
  Serial.begin(115200);
  if(!SerialFlash.begin(PIN_FLASH_CS)) {
    Serial.println(F("Flash not found! Check wiring."));
  }

  Serial.println(F("Flash memory found!"));

  showMenu();
  
}

void loop() {
  checkForSerialCommand();
  
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
        
      case 'd':
        dumpOneRecording();
        dumpFileNumber++;
        showMenu();
        break;

      default:
        showMenu();
        break;
    }
  }
}
// Display the menu of supported commands
void showMenu() {
  Serial.println(F("\n===================== Nakuja Project post-flight data recovery ==================="));
  Serial.println(F("\nMENU OPTIONS:"));
  Serial.println(F("d : Dump data"));
}

// generate a CSV formatted output of one flight's worth of recordings
void dumpOneRecording() {
  Serial.println(F("\n================ Flight data ================"));
  file = SerialFlash.open( "flight1.bin" );
//  file = SerialFlash.open( "flight.bin" );

  Serial.print(F("Files Found: ")); Serial.println(file);

  if (file) {
    Serial.print( F("Contents of file: ") );
    Serial.println( F("flight1.bin" ));
//    Serial.println( F("flight.bin" ));
    
    // print out the headings
    Serial.println(F("record_number,operation_mode,state,ax,ay,az,pitch,roll,gx,gy,gz,alt,velocity,pressure,temp"));
    
    do {
      file.read( (uint8_t *)&oneRecord, sizeof(oneRecord) );

      // library doesn't know where end of actual written data is so we have
      // to look for it ourselves!
      if ( oneRecord.record_number != 0xFFFFFFFF ) {
        Serial.print( oneRecord.record_number );
        Serial.print( "," );
        Serial.print( oneRecord.operation_mode );
        Serial.print( "," );
        Serial.print( oneRecord.state );
        Serial.print( "," );
        Serial.print( oneRecord.acc_data.ax );
        Serial.print( "," );
        Serial.print( oneRecord.acc_data.ay );
        Serial.print( "," );
        Serial.print( oneRecord.acc_data. az );
        Serial.print( "," );
        Serial.print( oneRecord.acc_data.pitch );
        Serial.print( "," );
        Serial.print( oneRecord.acc_data.roll );
        Serial.print( "," );
        Serial.print( oneRecord.gyro_data.gx );
        Serial.print( "," );
        Serial.print( oneRecord.gyro_data.gy );
        Serial.print( "," );
        Serial.print( oneRecord.gyro_data.gz );
        Serial.print( "," );
        Serial.print( oneRecord.alt_data.altitude );
        Serial.print( "," );
        Serial.print( oneRecord.alt_data.velocity );
        Serial.print( "," );
        Serial.print( oneRecord.alt_data.pressure );
        Serial.print( "," );
        Serial.println( oneRecord.alt_data.temperature );
      }
    } while ( oneRecord.record_number != 0xFFFFFFFF );
    file.close();
  }
  else {
    Serial.println(F("No more files."));
  }
}
