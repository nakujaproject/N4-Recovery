// MPU6050 class
#ifndef MPU_H
#define MPU_H

#include "Arduino.h"
#include <Wire.h>
#include <math.h>
#include "defs.h"


// divisor factors based on full scale ranges
#define ACCEL_FACTOR_2G       16384      
#define ACCEL_FACTOR_4G       8192    
#define ACCEL_FACTOR_8G       4096
#define ACCEL_FACTOR_16G      2048
#define GYRO_FACTOR_250       131
#define GYRO_FACTOR_500       65.5
#define GYRO_FACTOR_1000      32.8
#define GYRO_FACTOR_2000      16.4

// MPU6050 addresses definitions 
#define MPU6050_ADDRESS         0x68
#define GYRO_CONFIG             0x1B
#define ACCEL_CONFIG            0x1C
#define PWR_MNGMT_1             0x6B
#define RESET                   0x00
#define SET_GYRO_FS_250         0x00
#define SET_GYRO_FS_500         0x01
#define SET_GYRO_FS_1000        0x02
#define SET_GYRO_FS_2000        0x18
#define SET_ACCEL_FS_2G         0x00
#define SET_ACCEL_FS_4G         0x01
#define SET_ACCEL_FS_8G         0x02
#define SET_ACCEL_FS_16G        0x18
#define ACCEL_XOUT_H            0x3B
#define ACCEL_XOUT_L            0x3C
#define ACCEL_YOUT_H            0x3D
#define ACCEL_YOUT_L            0x3E
#define ACCEL_ZOUT_H            0x3F
#define ACCEL_ZOUT_L            0x40
#define GYRO_XOUT_H             0x43
#define GYRO_XOUT_L             0x44
#define GYRO_YOUT_H             0x45
#define GYRO_YOUT_L             0x46
#define GYRO_ZOUT_H             0x47
#define GYRO_ZOUT_L             0x48
#define TEMP_OUT_H              0x41
#define TEMP_OUT_L              0x42
#define ONE_G                   9.80665
#define TO_DEG_FACTOR           57.32

class MPU6050 {
    private:
    uint8_t _address;
    uint32_t _accel_fs_range;
    uint32_t _gyro_fs_range;
    
    public:
    // sensor data
    int16_t acc_x, acc_y, acc_z; // raw acceleration values  
    float acc_x_real, acc_y_real, acc_z_real; // converted aceleration values 
    int16_t ang_vel_x, ang_vel_y, ang_vel_z;
    float ang_vel_x_real, ang_vel_y_real, ang_vel_z_real; // converted angular velocity values 
    int16_t temp;
    float temp_real;

    float pitch_angle, roll_angle;
    float acc_x_ms, acc_y_ms, acc_z_ms; // acceleration in m/s^2

    // error callibration variables
    float summed_err_val;
    float err_acc_x, err_acc_y, err_acc_z;
    float err_gyro_x, err_gyro_y, err_gyro_z;
    float err_temp;

    MPU6050(uint8_t address, uint32_t accel_fs_range, uint32_t gyro_fs_range);
    void init();
    float readXAcceleration();
    float readYAcceleration();
    float readZAcceleration();
    float readXAngularVelocity();
    float readYAngularVelocity();
    float readZAngularVelocity();
    float readTemperature();

    // callibrate sensor
    bool callibrateSensor();

    // filter data values 
    void filterImu();

    // // orientation calculation
    float getRoll();
    float getPitch();

};

#endif