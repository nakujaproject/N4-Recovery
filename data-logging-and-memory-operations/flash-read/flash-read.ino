/**
 * Use this program to read the recorded data from the onboard flash 
 * memory
 */

#include <SerialFlash.h>

const byte PIN_FLASH_CS = 5; // Change this to match the Chip Select pin on your board
SerialFlashFile file;

void setup() {
  Serial.begin(115200);
  if(!SerialFlash.begin(PIN_FLASH_CS)) {
    Serial.println(f("Flash not found! Check wiring."));
  }

  Serial.println(F("Flash memory found!"));
  
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
        // d = just dump the next recording
        isRecording = false;
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
