/*!****************************************************************************
 * @file custom-time.h
 * @brief This file defeines functions needed for time conversions 
 * for data logging 
 *******************************************************************************/

#ifndef CUSTOM_TIME_H
#define CUSTOM_TIME_H

#include <Arduino.h>

char tstamp[50]; // to hold a timestamp
int minute=0, sec=0, msec=0;

char* convertTimestamp(unsigned long);

#endif