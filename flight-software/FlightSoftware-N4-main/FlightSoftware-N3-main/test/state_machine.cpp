/**
 * @file state_machine.cpp
 * @author Edwin Mwiti 
 * @brief 
 * @version 0.1
 * @date 2023-03-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <Arduino.h>
#include "state_machine.h"
#include "defs.h"

State_machine::State_machine(){

}

int State_machine::pre_flight(){
    return PRE_FLIGHT;
}

int State_machine::powered_flight(){
    return POWERED_FLIGHT;
}

int State_machine::coasting(){
    return COASTING;
}

int State_machine::apogee(){
    return APOGEE;
}

int State_machine::ballistic_descent(){
    return BALLISTIC_DESCENT;
}

int State_machine::parachute_deploy(){
    return PARACHUTE_DESCENT;
}

int State_machine::post_flight(){
    return POST_FLIGHT;
}

