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

### IMU

#### Calculating acceleration from accelerometer



#### Calculating velocity from accelerometer
The initial idea is to use integration. 
Since velocity is the first integral of acceleration. From the equation: 
``` v = u + at ```

So what we do to calculate the velocity is keep track of time, acceleration in the requires axis and then update the initial velocity. Consider the X axis: 

``` Vx = Ux + ACCx*Sample_time ```  
``` Ux = Vx  ```

(Let the sample tme be 1ms (0.001 s))

Known issue is velocity drift: where the velocity does not get to zero even when the sensor is stationary. small errors in the measurement of acceleration and angular velocity are integrated into progressively larger errors in velocity, which are compounded into still greater errors in position

Article: [IMU Velocity drift](https://en.wikipedia.org/wiki/Inertial_navigation_system#Drift_rate)

However, after extensive research online, it was concluded that getting velocity from accelerometer is very innacurate and unreliable. Check out this reddit thread:
[Acceleration & velocity with MPU6050](https://www.reddit.com/r/embedded/comments/138jnhu/acceleration_velocity_with_mpu6050/)

Check this arduinoForum article too (ArduinForum)
[https://forum.arduino.cc/t/integrating-acceleration-to-get-velocity/954731/8]

Following this, we decide to keep the accelerometer for measuring the acceleration and the rocket orientation.

### Data Filtering 

---

#### Complementary filter 



### References and Error fixes

1. (Wire LIbrary Device Lock) [Confusing overload of `Wire::begin` · Issue #6616 · espressif/arduino-esp32 · GitHub](https://github.com/espressif/arduino-esp32/issues/6616)
2. (Estimating velocity and altitude) [https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4179067/]
3. [rocket orientation and velocity] (https://www.reddit.com/r/rocketry/comments/10q7j8m/using_accelerometers_for_rocket_attitude/)









