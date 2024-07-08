#define LED_PIN 13  // Define the pin where the LED is connected

void setup() {
  // Initialize the LED pin as an output
  pinMode(LED_PIN, OUTPUT);
  // Ensure the LED is off initially
  digitalWrite(LED_PIN, LOW);
  
  // Begin serial communication at a baud rate of 9600
  Serial.begin(9600);

  Serial.println("Serial communication started. Send 'o' to turn on the LED and 'f' to turn it off.");

}

void loop() {
  // Check if data is available to read from the serial port
  if (Serial.available() > 0) {
    // Read the incoming byte
    char incomingByte = Serial.read();
    
    // Turn the LED on if the incoming byte is 'o'
    if (incomingByte == 'o') {
      delay(5000);
      Serial.println("LED turning on in 5 seconds");
      digitalWrite(LED_PIN, HIGH);
    }
    // Turn the LED off if the incoming byte is 'f'
    else if (incomingByte == 'f') {
      delay(2000);
      Serial.println("LED turning off in 2 seconds");
      digitalWrite(LED_PIN, LOW);
    }
  }
}