#include "checkstate.h"
#include "defs.h"
#include "functions.h"

//define default current state
uint8_t currentState = State::PRE_FLIGHT_GROUND;

//State machine transition conditions
uint8_t isInPoweredFlight(float altitude) {
  if (altitude > 50.0f) { 
    return State::POWERED_FLIGHT;
   } else {}
}

uint8_t isInCoasting(bool isDecelerating) {
  if (isDeceleratingContinuously() && millis() >= DECELERATION_CHECK_DURATION) {
    return State::COASTING;
   } else {}
}

uint8_t isInApogee(float velocity, float altitude) {
  if (velocity <= 0 && altitude >= APOGEE_ALTITUDE) {
    return State::DROGUE_DEPLOY;
   } else if (altitude < DROGUE_DEPLOY_MIN_ALTITUDE) {}
    else {}
}

uint8_t isInMainChuteDeploy(float altitude) {
  if (altitude <= MAIN_CHUTE_DEPLOY_ALTITUDE) {
    return State::MAIN_CHUTE_DEPLOY;
   } else {}
}

uint8_t isInPostFlight(float acceleration) {
  if (acceleration >= 9.81f) { // Assuming acceleration due to gravity
    return State::POST_FLIGHT_GROUND;
   } else {}
}

void loop() {

  // Read sensor data
  float currentAltitude = getAltitude();
  float velocity = getVelocity(); 
  bool isDecelerating = isDeceleratingContinuously();

  if(mode == FLIGHT_MODE) {
    
    // Call state transition functions
   currentState = isInPoweredFlight(currentAltitude);
   currentState = isInCoasting(isDecelerating);
   currentState = isInApogee(velocity,currentAltitude)
   currentState = isInMainChuteDeploy(currentAltitude)
   currentState = isInPostFlight(getAcceleration());

   } else {
    currentstate = PRE_FLIGHT_GROUND;
  }
  
  
 // State transitions
  switch (currentState) {
   case State::PRE_FLIGHT_GROUND:
    Serial.printIn( "Pre-Flight State")
    break;
   case State::POWERED_FLIGHT:
    Serial.printIn("Powered Flight state")
    break;
   case State::COASTING:
    Serial.printIn("Coasting State")
    break;
   case State::DROGUE_DEPLOY:
    Serial.printIn("Apogee reached")
   // Call drogue eject function
    eject_drogue();
    break;
   case State::DROGUE_DESCENT:
    Serial.printIn("Drogue Descent")
    break;
   case State::MAIN_CHUTE_DEPLOY:
    Serial.printIn("Main Chute Deploy")
    // Call main chute eject function
    eject_main();
    break;
   case State::POST_FLIGHT_GROUND:
    Serial.printIn("Post-Flight state")
    break;
   default:
    break;
  }

 // Delay between loop iterations
  delay(100); // will adjust as needed
}
