#ifndef DEFS_H
#define DEFS_H

// to select the telemetry transfer method used 
#define MQTT 1 // set this to 1 if using MQTT for telemetry transfer
#define XBEE 1  // set to 1 if using XBEE for telemetry transfer

#define GPS_BAUD_RATE 9600
#define XBEE_BAUD_RATE  // TODO: set to XBEE baud rate

/* debug parameters for use during testing - set to 0 for production */
#define DEBUG 1
#define LOG_TO_MEMORY 0 // set to 1 during live testing
#define DEBUG_TO_TERMINAL 1 // set to 0 for production

#if DEBUG // DEBUG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define debugf(x, y) Serial.printf(x, y)

#else

#define debug(x)
#define debugln(x)
#define debugf(x, y)
#endif // DEBUG

/* end of debug parameters */

/* timing constant */
#define SETUP_DELAY 300
#define TASK_DELAY 10

/* flight constants */
#define EJECTION_HEIGHT 1000 // eject at 1000m AGL
#define SEA_LEVEL_PRESSURE 101325 // Assume the sea level pressure is 101325 Pascals - this can change with weather
#define BASE_ALTITUDE 1417 /* this value is the altitude at rocket launch site */

/* tasks constants */
#define STACK_SIZE 2048
#define ALTIMETER_QUEUE_LENGTH 10 // todo: change to 2 items
#define GYROSCOPE_QUEUE_LENGTH 10
#define GPS_QUEUE_LENGTH 24
#define TELEMETRY_DATA_QUEUE_LENGTH  10
#define FILTERED_DATA_QUEUE_LENGTH 10
#define FLIGHT_STATES_QUEUE_LENGTH 1

/* MQTT constants */
#define MQTT_SERVER "192.168.78.19"
#define MQTT_PORT 1882

/* WIFI credentials */
// const char* SSID = "Galaxy";
// const char* PASSWORD = "luwa2131";

/* ROCKET FLIGHT STATES */
#define PRE_FLIGHT          0
#define POWERED_FLIGHT      1
#define COASTING            2
#define APOGEE              3
#define BALLISTIC_DESCENT   4
#define PARACHUTE_DESCENT   5
#define POST_FLIGHT         6
#define UNDEFINED_STATE     7

#define EJECTION_PIN 12
/* LEDs for testing - remove on production */
#define PRE_FLIGHT_LED 4

// number of readings to take while callibrating the sensor
#define CALLIBRATION_READINGS   200

#define TX 17
#define RX 16

#endif // DEFS_H

