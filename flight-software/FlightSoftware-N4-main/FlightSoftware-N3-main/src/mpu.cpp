#include "mpu.h"

// constructor
MPU6050::MPU6050(uint8_t address, uint32_t accel_fs_range, uint32_t gyro_fs_range) {
    this->_address = address;
    this->_accel_fs_range = accel_fs_range;
    this->_gyro_fs_range = gyro_fs_range;

}

// initialize the MPU6050 
void MPU6050::init() {
    // initialize the MPU6050 
    Wire.begin();
    Wire.beginTransmission(this->_address);
    Wire.write(PWR_MNGMT_1); // power on the device 
    Wire.write(RESET);
    Wire.endTransmission(true);
    delay(50);

    // // configure the gyroscope
    // Wire.beginTransmission(this->_address);
    // Wire.begin(GYRO_CONFIG);
    // if(this->_gyro_fs_range == 250) {
    //     Wire.write(SET_GYRO_FS_250);
    // } else if(this->_gyro_fs_range == 500) {
    //     Wire.write(SET_GYRO_FS_500);
    // } else if (this->_gyro_fs_range == 1000) {
    //     Wire.write(SET_GYRO_FS_1000);
    // } else if (this->_gyro_fs_range == 2000) {
    //     Wire.write(SET_GYRO_FS_2000);
    // }
    // Wire.endTransmission(true);

    // // configure the accelerometer
    // Wire.beginTransmission(this->_address);
    // Wire.write(ACCEL_CONFIG);

    // if(this->_accel_fs_range == 2) {
    //     Wire.write(SET_ACCEL_FS_2G);
    // } else if (this->_accel_fs_range == 4) {
    //      Wire.write(SET_ACCEL_FS_4G);
    // }  else if (this->_accel_fs_range== 8) {
    //      Wire.write(SET_ACCEL_FS_8G);
    // }  else if (this->_accel_fs_range == 16) {
    //      Wire.write(SET_ACCEL_FS_16G);
    // }
    // Wire.endTransmission(true);
}

float MPU6050::readXAcceleration() {
    Wire.beginTransmission(this->_address);
    Wire.write(ACCEL_XOUT_H);
    Wire.endTransmission(true);

    Wire.requestFrom(this->_address, 2, false);
    this->acc_x = Wire.read()<<8 | Wire.read();

    // divide by the respective factors
    if(this->_accel_fs_range == 2) {
        this->acc_x = acc_x / ACCEL_FACTOR_2G;
    } else if(this->_accel_fs_range == 4) {
        this->acc_x = acc_x / ACCEL_FACTOR_4G; 
    } else if(this->_accel_fs_range == 8) {
        this->acc_x = acc_x / ACCEL_FACTOR_8G;
    } else if(this->_accel_fs_range == 16) {
        this->acc_x = acc_x / ACCEL_FACTOR_16G;
    }

    return this->acc_x;
    
}