/**
 * This program processes a command through serial and changes the state of the  
 * state of the ESP32 from transmitting to receiving 
 */

enum states {
  RECEIVE_FILE = 0,
  RUN
};

/**
 * Check the curent state 
 */
void checkState() {

}

/**
 * Process the command being received from serial
 */
void parseSerialCommand(uint8_t cmd) {
  switch(cmd) {
    default:
      Serial.println("Invalid command");
  }
}


void setup() {
  Serial.begin(115200);
  
}

void loop() {
  

}
