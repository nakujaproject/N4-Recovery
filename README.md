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

For logging and storage, we use two methods. One is logging to an external SPI flash memory, the WINBOND W25Q32JVSIQ2135, which is a 32Mbits(4 MB) storage chip. 
For redundancy, we add a microSD card into which data is dumped from the external SPI flash memory POST-FLIGHT.

The logginG flowchart is shown below:

## Encoding and space optimizations 

---

The aim of this optimization is to ensure we have as much memory as possible to store the flight data. To do this, we need to test how much in-memory space various data formats will take. This is explained below

### Encoding and storing integers and floats

To carry out this comparison, I created two files. One with purely integer values and the other with the same integer values but in HEX format.

```
// decimal-test-file.log
2245454
2245454
2245454
34
34
34
34
34
34
34

```

```
// hex-test-file.log
22434E
22434E
22434E
22
22
22
22
22
22
22

```

Saving and checking the properties of the two files in Windows platform shows the following: 

For the decimal file, the size is 53 bytes

![dec-test](./dec-test.png)

For the hexadecimal file, the size is 50 bytes

![hex-test](./hex-test.png)

The hexadecimal file is 3 bytes smaller as observed. 

### Encoding and storing strings

For strings, since we are not going to use any non-ASCII characters, we can store the string as is. This means that if we have a string like "flight data", it would take 12 bytes, since one char is 1 byte. In addition, the string metadata like length and nul terminator take some few extra bytes. 

If we could encode this string into hex, using UTF-8 encoding scheme, it would take the same 12 bytes. 
This test is carried out below to justify why we store the strings as is without encoding them. 

I wrote a program to convert a string to HEX format, and compared the following string:

```
flight data
```

In HEX format, it produces the following:

```
666C696768742064617461
```

Raw string size (Windows platform)

![raw-string](./raw-string.png)

Hex string size 

![hex-string](./dec-test.png)

The raw string occupies 11 bytes, while the same strinf in HEX format occupies a whooping 24 bytes! This is not feasible considering we will be storing several strings in a 4 MB limited flash memory over the flight time, which will certainly occupy lots of memory if we store HEX strings.



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
4. https://cdn.shopify.com/s/files/1/1014/5789/files/Standard-ASCII-Table_large.jpg?10669400161723642407
5. https://www.codeproject.com/Articles/99547/Hex-strings-to-raw-data-and-back
6. https://cdn.shopify.com/s/files/1/1014/5789/files/Standard-ASCII-Table_large.jpg?10669400161723642407
7. https://www.geeksforgeeks.org/convert-a-string-to-hexadecimal-ascii-values/








