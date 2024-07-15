/**
 * @file state_machine.cpp
 * @author Edwin Mwiti 
 * @brief 
 * @version 0.1
 * @date 2023-03-28
 * 
 * 
 */
#include <Arduino.h>
#include "state_machine.h"
#include "defs.h"

// variable for detected apogee height
float MAX_ALTITUDE = 0;
float PREVIOUS_ALTITUDE = 0;
float ALTITUDE_BUFFER[5];
int ALTITUDE_INDEX = 0;
int PREVIOUS_STATE = 0;

//
bool pre_flight(float altitude){
    if(PREVIOUS_STATE>PRE_FLIGHT) return false;
    if(BASE_ALTITUDE-altitude<5) return true;
    return false;
}

// This checks that we have started ascent
// compares the current displacement to the set threshold of the ground state displacement
//if found to be above, we have achieved lift off
bool powered_flight(float altitude)
{
    if(PREVIOUS_STATE>POWERED_FLIGHT) return false;
    if (BASE_ALTITUDE-altitude>5)  return true;
    return false;
}

// This checks that we have reached apogee
bool apogee(float altitude){
    if(PREVIOUS_STATE>=POWERED_FLIGHT){
        if(ALTITUDE_BUFFER[4]-ALTITUDE_BUFFER[0]<5){
            return true;
        }
    }
    return false;
}

// This checks that we have reached the ground
// detects landing of the rocket
// TODO: ALTITUDE_OFFSET might be different from the original base altitude
bool post_flight(float altitude)
{
    if(PREVIOUS_STATE>=POWERED_FLIGHT){
        for(int i=0; i<5; i++){
            if(ALTITUDE_BUFFER[i]-altitude<5) continue;
            return false;
        }
        return true;
    }
    return false;
}

bool ballistic_descent(float velocity){
    if(velocity>0) return false;
    //TODO: calculate free fall velocity
    if(velocity<-20) return true;
    return false;
}
bool parachute_descent(float velocity){
    if(velocity>0) return false;
}

int checkState(float altitude, float velocity){
    if(ALTITUDE_INDEX==5) ALTITUDE_INDEX=0;
    ALTITUDE_BUFFER[ALTITUDE_INDEX] = altitude;
    ALTITUDE_INDEX++;
    if(pre_flight(altitude)){
        PREVIOUS_STATE = PRE_FLIGHT;
        return PRE_FLIGHT;
    }
    if(powered_flight(altitude)){
        PREVIOUS_STATE = POWERED_FLIGHT;
        return POWERED_FLIGHT;
    }
    if(apogee(altitude)){
        PREVIOUS_STATE = APOGEE;
        return APOGEE;
    }
    if(ballistic_descent(velocity)){
        PREVIOUS_STATE = BALLISTIC_DESCENT;
        return BALLISTIC_DESCENT;
    }
    if(parachute_descent(velocity)){
        PREVIOUS_STATE = PARACHUTE_DESCENT;
        return PARACHUTE_DESCENT;
    }
    if(post_flight(altitude)){
        PREVIOUS_STATE = POST_FLIGHT;
        return POST_FLIGHT;
    }
    return UNDEFINED_STATE;
}
