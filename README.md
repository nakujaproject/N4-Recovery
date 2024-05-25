# N4-Recovery
RND for N4 recovery team

## N4 Flight Software Documentation

### N4 Flight software requirements 

---

(1)

(2)

(3)

### Tasks and task creation

---

### 

### Data queues and task communication

---



### Telemetry and transmission to ground

----



### Data Logging and storage 

---



### State machine logic and operation

---

#### States

#### State transition conditions 

#### State functions handling 



### IMU Calibration

---

#### 1.1 Gyroscope, accelerometer and altimeter calibration

Calibration is important to get the most accurate values from the gyro. To do this, ensure the gyroscope is resting on a surface that is still and not moving. If it is moving you will introduce errors in your calibration process. 

The expected values of angular velocity when the gyroscope and accelerometer is resting is 0. However you notice that there is still some reading. This reading is the error we want to offset.  We have to calculate 3 offset values that bring the sensor reading closer to 0. 

To callibrate:

1. We take a running sum, for a given amount of time,  of gyro readings in all the 3 axes. 

2. We know that the readings should be 0 for all readings of angular acceleration. However the sensor gives the true value plus some offset. Thus:

   ```  true_sensor_value = measured_sensor_value - offset ```

3. Take the average of the *running sum* and you get the *average of the offset*, which we now need to subtract from the measured value and that's it. 

   

This calibration function is called in the setup function during peripherals init. 

``` imu.callibrate() ```



### Filters 

---

#### Complementary filter 



### References and Error fixes

1. (Wire LIbrary Device Lock) [Confusing overload of `Wire::begin` · Issue #6616 · espressif/arduino-esp32 · GitHub](https://github.com/espressif/arduino-esp32/issues/6616)









