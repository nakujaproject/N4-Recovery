/**
 * ///////////////////////// DATA TYPES /////////////////////////
*/


#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <Arduino.h>

typedef struct Acceleration_Data{
    float ax;
    float ay; 
    float az;
    float pitch;
    float roll;
} accel_type_t;

typedef struct Gyroscope_Data {
    double gx;
    double gy;
    double gz;
} gyro_type_t;

typedef struct GPS_Data{
    double latitude;
    double longitude;; 
    uint time;
} gps_type_t;

typedef struct Altimeter_Data{
    double pressure;
    double altitude;
    double velocity;
    double temperature;
    double AGL; /* altitude above ground level */
} altimeter_type_t;

typedef struct Telemetry_Data {
    uint32_t record_number;
    uint8_t operation_mode;
    uint8_t state;
    accel_type_t acc_data;
    gyro_type_t gyro_data;
    gps_type_t gps_data;
    altimeter_type_t alt_data;
} telemetry_type_t;

#endif