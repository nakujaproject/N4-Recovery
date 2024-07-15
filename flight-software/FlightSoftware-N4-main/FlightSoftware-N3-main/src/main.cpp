/**
 * 
 * 0x5765206D6179206D616B65206F757220706C616E73202C
 * 0x62757420476F642068617320746865206C61737420776F7264
 * 
*/

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h> // TODO: ADD A MQTT SWITCH - TO USE MQTT OR NOT
#include "sensors.h"
#include "defs.h"
#include "mpu.h"
#include <SFE_BMP180.h>
#include "SerialFlash.h"
#include "logger.h"
#include "data-types.h"
#include "custom-time.h"
#include <TinyGPSPlus.h>

uint8_t operation_mode = 0;     /*!< Tells whether software is in safe or flight mode - FLIGHT_MODE=1, SAFE_MODE=0 */

/* create Wi-Fi Client */
WiFiClient wifi_client;

/* create MQTT publish-subscribe client */
PubSubClient mqtt_client(wifi_client);

/* GPS object */
TinyGPSPlus gps;

/* Flight data logging */
uint8_t cs_pin = 5;                         /*!< External flash memory chip select pin */
uint8_t flash_led_pin = 4;                  /*!< LED pin connected to indicate flash memory formatting  */
char filename[] = "flight.bin";             /*!< data log filename - Filename must be less than 20 chars, including the file extension */
uint32_t FILE_SIZE_512K = 524288L;          /*!< 512KB */
uint32_t FILE_SIZE_1M  = 1048576L;          /*!< 1MB */
uint32_t FILE_SIZE_4M  = 4194304L;          /*!< 4MB */
SerialFlashFile file;                       /*!< object representing file object for flash memory */
unsigned long long previous_log_time = 0;   /*!< The last time we logged data to memory */
unsigned long long current_log_time = 0;    /*!< What is the processor time right now? */
uint16_t log_sample_interval = 10;          /*!< After how long should we sample and log data to flash memory? */

DataLogger data_logger(cs_pin, flash_led_pin, filename, file,  FILE_SIZE_4M);

/* position integration variables */
long long current_time = 0;
long long previous_time = 0;

/**
 * ///////////////////////// DATA VARIABLES /////////////////////////
*/

accel_type_t acc_data;
gyro_type_t gyro_data;
gps_type_t gps_data;
altimeter_type_t altimeter_data;
telemetry_type_t telemetry_packet;

/**
 * ///////////////////////// END OF DATA VARIABLES /////////////////////////
*/


///////////////////////// PERIPHERALS INIT /////////////////////////

/**
 * create an MPU6050 object
 * set gyro to max deg to 1000 deg/sec
 * set accel fs reading to 16g
*/
MPU6050 imu(0x68, 16, 1000);

// create BMP object 
SFE_BMP180 altimeter;
char status;
double T, P, p0, a;
#define ALTITUDE 1525.0 // altitude of iPIC building, JKUAT, Juja.

/*!****************************************************************************
 * @brief Initialize BMP180 barometric sensor
 * @return TODO: 1 if init OK, 0 otherwise
 * 
 *******************************************************************************/
void BMPInit() {
    if(altimeter.begin()) {
        Serial.println("BMP init success");
        // TODO: update system table
    } else {
        Serial.println("BMP init failed");
    }
}


/*!****************************************************************************
 * @brief Initialize the GPS connected on Serial2
 * @return 1 if init OK, 0 otherwise
 * 
 *******************************************************************************/
void GPSInit() {
    // create the GPS object 
    debugln("Initializing the GPS..."); // TODO: log to system logger
    Serial2.begin(GPS_BAUD_RATE);
    delay(100); // wait for GPS to init
}

/**
 * ///////////////////////// END OF PERIPHERALS INIT /////////////////////////
 */


// create data types to hold the sensor data 


/* create queue to store altimeter data
 * store pressure and altitude
 * */
QueueHandle_t telemetry_data_qHandle;
QueueHandle_t accel_data_qHandle;
QueueHandle_t altimeter_data_qHandle;
QueueHandle_t gps_data_qHandle;
// QueueHandle_t filtered_data_queue;
// QueueHandle_t flight_states_queue;


// void connectToWifi(){
//     digitalWrite(LED_BUILTIN, HIGH);
//     /* Connect to a Wi-Fi network */
//     debugln("[..]Scanning for network...");

//     WiFi.begin(SSID, PASSWORD);

//     while (WiFi.status() != WL_CONNECTED)
//     {
//         delay(500);
//         debugln("[..]Scanning for network...");
//     }

//     debugln("[+]Network found");debug("[+]My IP address: "); debugln();
//     debugln(WiFi.localIP());
//     digitalWrite(LED_BUILTIN, LOW);
// }

// void initializeMQTTParameters(){
//     /* this functions creates an MQTT client for transmitting telemetry data */;
//     mqtt_client.setServer(MQTT_SERVER, MQTT_PORT);
// }

//////////////////////////// ACCELERATION AND ROCKET ATTITUDE DETERMINATION /////////////////

/*!****************************************************************************
 * @brief Read aceleration data from the accelerometer
 * @param pvParameters - A value that is passed as the paramater to the created task.
 * If pvParameters is set to the address of a variable then the variable must still exist when the created task executes - 
 * so it is not valid to pass the address of a stack variable.
 * @return Updates accelerometer data struct on the telemetry queue
 * 
 *******************************************************************************/
void readAccelerationTask(void* pvParameter) {

    telemetry_type_t acc_data_lcl;

    while(1) {
        acc_data_lcl.operation_mode = operation_mode; // TODO: move these to check state function
        acc_data_lcl.record_number++;
        acc_data_lcl.state = 0;

        acc_data_lcl.acc_data.ax = imu.readXAcceleration();
        acc_data_lcl.acc_data.ay = imu.readYAcceleration();
        acc_data_lcl.acc_data.az = 0;

        // get pitch and roll
        acc_data_lcl.acc_data.pitch = imu.getPitch();
        acc_data_lcl.acc_data.roll = imu.getRoll();
        
        xQueueSend(telemetry_data_qHandle, &acc_data_lcl, portMAX_DELAY);

    }
}


///////////////////////// ALTITUDE AND VELOCITY DETERMINATION /////////////////////////

/*!****************************************************************************
 * @brief Read ar pressure data from the barometric sensor onboard
 * @param pvParameters - A value that is passed as the paramater to the created task.
 * If pvParameters is set to the address of a variable then the variable must still exist when the created task executes - 
 * so it is not valid to pass the address of a stack variable.
 * @return Sends altimeter data to altimeter queue
 *******************************************************************************/
void readAltimeterTask(void* pvParameters){
    telemetry_type_t alt_data_lcl;

    while(true){    
        // If you want to measure altitude, and not pressure, you will instead need
        // to provide a known baseline pressure. This is shown at the end of the sketch.

        // You must first get a temperature measurement to perform a pressure reading.
        
        // Start a temperature measurement:
        // If request is successful, the number of ms to wait is returned.
        // If request is unsuccessful, 0 is returned.
        status = altimeter.startTemperature();
        if(status !=0 ) {
            // wait for measurement to complete
            delay(status);

            // retrieve the completed temperature measurement 
            // temperature is stored in variable T

            status = altimeter.getTemperature(T);
            if(status != 0) {
                // print out the measurement 
                // Serial.print("temperature: ");
                // Serial.print(T, 2);
                // Serial.print(" \xB0 C, ");

                // start pressure measurement 
                // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
                // If request is successful, the number of ms to wait is returned.
                // If request is unsuccessful, 0 is returned.
                status = altimeter.startPressure(3);
                if(status != 0) {
                    // wait for the measurement to complete
                    delay(status);

                    // Retrieve the completed pressure measurement:
                    // Note that the measurement is stored in the variable P.
                    // Note also that the function requires the previous temperature measurement (T).
                    // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
                    // Function returns 1 if successful, 0 if failure.

                    status = altimeter.getPressure(P, T);
                    if(status != 0) {
                        // print out the measurement
                        // Serial.print("absolute pressure: ");
                        // Serial.print(P, 2);
                        // Serial.print(" mb, "); // in millibars

                        p0 = altimeter.sealevel(P,ALTITUDE);
                        // If you want to determine your altitude from the pressure reading,
                        // use the altitude function along with a baseline pressure (sea-level or other).
                        // Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
                        // Result: a = altitude in m.

                        a = altimeter.altitude(P, p0);
                        // Serial.print("computed altitude: ");
                        // Serial.print(a, 0);
                        // Serial.print(" meters, ");

                    } else {
                        Serial.println("error retrieving pressure measurement\n");
                    } 
                
                } else {
                    Serial.println("error starting pressure measurement\n");
                }

            } else {
                Serial.println("error retrieving temperature measurement\n");
            }

        } else {
            Serial.println("error starting temperature measurement\n");
        }

        // delay(2000);

        // TODO: compute the velocity from the altimeter data

        // assign data to queue
        alt_data_lcl.alt_data.pressure = P;
        alt_data_lcl.alt_data.altitude = a;
        alt_data_lcl.alt_data.velocity = 0;
        alt_data_lcl.alt_data.temperature = T;

        // send this pressure data to queue
        // do not wait for the queue if it is full because the data rate is so high, 
        // we might lose some data as we wait for the queue to get space
        xQueueSend(telemetry_data_qHandle, &alt_data_lcl, 0); // TODO: CHECK SUCCESS SENDING TO QUEUE

    }

}

/*!****************************************************************************
 * @brief Read the GPS location data and altitude and append to telemetry packet for transmission
 * @param pvParameters - A value that is passed as the paramater to the created task.
 * If pvParameters is set to the address of a variable then the variable must still exist when the created task executes - 
 * so it is not valid to pass the address of a stack variable.
 * 
 *******************************************************************************/
void readGPSTask(void* pvParameters){

    telemetry_type_t gps_data_lcl;

    while(true){

        // if(Serial2.available()) {
        //     char c = Serial2.read();

        //     if(gps.encode(c)) {
        //         // GPS lock hs been acquired 
        //         // set the new data lock to 1
        //         debugln(c); // dump gps data
        //     } else {
        //        // set new data lock to 0
        //     } 
        // }

        if (Serial2.available()) {
            char c = Serial2.read();
            if(gps.encode(c)){
                // get location, latitude and longitude 
                if(gps.location.isValid()) {
                    gps_data_lcl.gps_data.latitude = gps.location.lat();
                    gps_data_lcl.gps_data.longitude = gps.location.lng();
                } else {
                    debugln("Invalid GPS location");
                }

                if(gps.time.isValid()) {
                    gps_data_lcl.gps_data.time = gps.time.value(); // decode this time value post flight
                } else {
                    debugln("Invalid GPS time");
                }

                if(gps.altitude.isValid()) {
                    gps_data_lcl.gps_data.gps_altitude = gps.altitude.meters();
                } else {
                    debugln("Invalid altitude data"); // TODO: LOG to system logger
                }
            }
        }

        // send to telemetry queue
        if(xQueueSend(telemetry_data_qHandle, &gps_data_lcl, portMAX_DELAY) != pdPASS){
            debugln("[-]GPS queue full"); // TODO: LOG TO SYSTEM LOGGER
        } else {
            // debugln("Sent to GPS Queue"); // TODO: LOG TO SYSTEM LOGGER
        }
    }

}


/*!****************************************************************************
 * @brief debug flight/test data to terminal, this task is called if the DEBUG_TO_TERMINAL is set to 1 (see defs.h)
 * @param pvParameter - A value that is passed as the paramater to the created task.
 * If pvParameter is set to the address of a variable then the variable must still exist when the created task executes - 
 * so it is not valid to pass the address of a stack variable.
 * 
 *******************************************************************************/
void debugToTerminalTask(void* pvParameters){
    telemetry_type_t rcvd_data; // accelration received from acceleration_queue

    while(true){
        if(xQueueReceive(telemetry_data_qHandle, &rcvd_data, portMAX_DELAY) == pdPASS){
            // debug CSV to terminal 
            // debug(rcvd_data.acc_data.ax); debug(","); 
            // debug(rcvd_data.acc_data.ay); debug(","); 
            // debug(rcvd_data.acc_data.az); debug(","); 
            // debug(rcvd_data.acc_data.pitch); debug(","); 
            // debug(rcvd_data.acc_data.roll); debug(","); 
            // debug(rcvd_data.alt_data.pressure); debug(","); 
            // debug(rcvd_data.alt_data.velocity); debug(","); 
            // debug(rcvd_data.alt_data.altitude); debug(","); 
            // debug(rcvd_data.alt_data.temperature); debug(","); 

            debug(rcvd_data.gps_data.latitude); debug(","); 
            debug(rcvd_data.gps_data.longitude); debug(",");
            debug(rcvd_data.gps_data.gps_altitude); debug(",");
            debug(rcvd_data.gps_data.time); debug(","); 

            debugln();

        }else{
            /* no queue */
        }

        // if(xQueueReceive(altimeter_data_queue, &altimeter_buffer, portMAX_DELAY) == pdPASS){
        //     debug("Pressure: "); debug(altimeter_buffer.pressure); debugln();
        //     debug("Altitude: "); debug(altimeter_buffer.altitude); debugln();
        //     debug("Velocity: "); debug(altimeter_buffer.velocity); debugln();
        //     debug("AGL: "); debug(altimeter_buffer.AGL); debugln();
            
        // }else{
        //     /* no queue */
        // }

        // if(xQueueReceive(gps_data_queue, &gps_buffer, portMAX_DELAY) == pdPASS){
        //     debug("Lattitude: "); debug(gps_buffer.latitude); debugln();
        //     debug("Longitude: "); debug(gps_buffer.longitude); debugln();
        //     debug("Time: "); debug(gps_buffer.time); debugln();
            
        // }else{
        //     /* no queue */
        // }
    }
}


/*!****************************************************************************
 * @brief log the data to the external flash memory
 * @param pvParameter - A value that is passed as the paramater to the created task.
 * If pvParameter is set to the address of a variable then the variable must still exist when the created task executes - 
 * so it is not valid to pass the address of a stack variable.
 * 
 *******************************************************************************/
void logToMemory(void* pvParameter) {
    telemetry_type_t received_packet;

    while(1) {
        xQueueReceive(telemetry_data_qHandle, &received_packet, portMAX_DELAY);
        // received_packet.record_number++; 

        // is it time to record?
        current_log_time = millis();

        if(current_log_time - previous_log_time > log_sample_interval) {
            previous_log_time = current_log_time;
            data_logger.loggerWrite(received_packet);
        }
        
    }

}

// void transmitTelemetry(void* pvParameters){
//     /* This function sends data to the ground station */

//      /*  create two pointers to the data structures to be transmitted */
    
//     char telemetry_data[180];
//     struct Acceleration_Data gyroscope_data_receive;
//     struct Altimeter_Data altimeter_data_receive;
//     struct GPS_Data gps_data_receive;
//     int32_t flight_state_receive;
//     int id = 0;

//     while(true){
//         file = SPIFFS.open("/log.csv", FILE_APPEND);
//         if(!file) debugln("[-] Failed to open file for appending");
//         else debugln("[+] File opened for appending");
        
//         /* receive data into respective queues */
//         if(xQueueReceive(gyroscope_data_queue, &gyroscope_data_receive, portMAX_DELAY) == pdPASS){
//             debugln("[+]Gyro data ready for sending ");
//         }else{
//             debugln("[-]Failed to receive gyro data");
//         }

//         if(xQueueReceive(altimeter_data_queue, &altimeter_data_receive, portMAX_DELAY) == pdPASS){
//             debugln("[+]Altimeter data ready for sending ");
//         }else{
//             debugln("[-]Failed to receive altimeter data");
//         }

//         if(xQueueReceive(gps_data_queue, &gps_data_receive, portMAX_DELAY) == pdPASS){
//             debugln("[+]GPS data ready for sending ");
//         }else{
//             debugln("[-]Failed to receive GPS data");
//         }

//         if(xQueueReceive(flight_states_queue, &flight_state_receive, portMAX_DELAY) == pdPASS){
//             debugln("[+]Flight state ready for sending ");
//         }else{
//             debugln("[-]Failed to receive Flight state");
//         }

//         sprintf(telemetry_data,
//             "%i,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%i,%.16f,%.16f,%i,%i\n",
//             id,//0
//             gyroscope_data_receive.ax,//1
//             gyroscope_data_receive.ay,//2
//             gyroscope_data_receive.az,//3
//             gyroscope_data_receive.gx,//4
//             gyroscope_data_receive.gy,//5
//             gyroscope_data_receive.gz,//6
//             altimeter_data_receive.AGL,//7
//             altimeter_data_receive.altitude,//8
//             altimeter_data_receive.velocity,//9
//             altimeter_data_receive.pressure,//10
//             gps_data_receive.latitude,//11
//             gps_data_receive.longitude,//12
//             gps_data_receive.time,//13
//             flight_state_receive//14
//         );
//         if(file.print(telemetry_data)){
//             debugln("[+] Message appended");
//         } else {
//             debugln("[-] Append failed");
//         }
//         file.close();
//         id+=1;

//         if(mqtt_client.publish("n3/telemetry", telemetry_data)){
//             debugln("[+]Data sent");
//         } else{
//             debugln("[-]Data not sent");
//         }
//     }
// }

// void reconnect(){

//     while(!mqtt_client.connected()){
//         debug("[..]Attempting MQTT connection...");
//         String client_id = "[+]FC Client - ";
//         client_id += String(random(0XFFFF), HEX);

//         if(mqtt_client.connect(client_id.c_str())){
//             debugln("[+]MQTT connected");
//         }
//     }
// }

// void testMQTT(void *pvParameters){
//     while(true){
//         debugln("Publishing data");
//         if(mqtt_client.publish("n3/telemetry", "Hello from flight!")){
//             debugln("Data sent");
//         }else{
//             debugln("Unable to send data");
//         }
//     }
// }


/*!****************************************************************************
 * @brief Setup - perfom initialization of all hardware subsystems, create queues, create queue handles 
 * initialize system check table
 * 
 *******************************************************************************/
void setup(){
    /* initialize serial */
    Serial.begin(115200);

    uint8_t app_id = xPortGetCoreID();
    BaseType_t th; // task creation handle

    /* initialize the data logging system - logs to flash memory */
    data_logger.loggerInit();

    /* connect to WiFi*/
    // connectToWifi();

    ///////////////////////// PERIPHERALS INIT /////////////////////////
    imu.init();
    BMPInit();
    GPSInit();

    //==============================================================

    // mqtt_client.setBufferSize(MQTT_BUFFER_SIZE);
    // mqtt_client.setServer(MQTT_SERVER, MQTT_PORT);

    debugln("==============Creating queues=============="); // TODO: LOG TO SYSTEM LOGGER

    ///////////////////// create data queues ////////////////////
    // this queue holds the data from MPU 6050 - this data is filtered already
    accel_data_qHandle = xQueueCreate(GYROSCOPE_QUEUE_LENGTH, sizeof(accel_type_t)); 

    // this queue hold the data read from the BMP180
    altimeter_data_qHandle = xQueueCreate(ALTIMETER_QUEUE_LENGTH, sizeof(altimeter_type_t)); 

    /* create gps_data_queue */   
    gps_data_qHandle = xQueueCreate(GPS_QUEUE_LENGTH, sizeof(gps_type_t));

    // this queue holds the telemetry data packet
    telemetry_data_qHandle = xQueueCreate(TELEMETRY_DATA_QUEUE_LENGTH, sizeof(telemetry_packet));

    // /* this queue will hold the flight states */
    // flight_states_queue = xQueueCreate(FLIGHT_STATES_QUEUE_LENGTH, sizeof(int32_t));

    /* check if the queues were created successfully */
    if(accel_data_qHandle == NULL){
        debugln("[-]accel data queue creation failed!");
    } else{
        debugln("[+]Accelleration data queue creation success");
    }
    
    if(altimeter_data_qHandle == NULL){
        debugln("[-]Altimeter data queue creation failed!");
    } else{
        debugln("[+]Altimeter data queue creation success");
    }

    if(gps_data_qHandle == NULL){
        debugln("[-]GPS data queue creation failed!");
    } else{
        debugln("[+]GPS data queue creation success");
    }

    if(telemetry_data_qHandle == NULL) {
        debugln("Telemetry queue created");
    } else {
        debugln("Failed to create telemetry queue");
    }

    // if(filtered_data_queue == NULL){
    //     debugln("[-]Filtered data queue creation failed!");
    // } else{
    //     debugln("[+]Filtered data queue creation success");
    // }

    // if(flight_states_queue == NULL){
    //     debugln("[-]Flight states queue creation failed!");
    // } else{
    //     debugln("[+]Flight states queue creation success");
    // }

    //====================== TASK CREATION ==========================
    /* Create tasks
     * All tasks have a stack size of 1024 words - not bytes!
     * ESP32 is 32 bit, therefore 32bits x 1024 = 4096 bytes
     * So the stack size is 4096 bytes
     * */
    debugln("==============Creating tasks==============");

    /* TASK 1: READ ACCELERATION DATA */
   th = xTaskCreatePinnedToCore(
        readAccelerationTask,         
        "readGyroscope",
        STACK_SIZE*2,                  
        NULL,                       
        1,
        NULL,
        app_id
   );

   if(th == pdPASS) {
    Serial.println("Read acceleration task created");
   } else {
    Serial.println("Read acceleration task creation failed");
   }

    /* TASK 2: READ ALTIMETER DATA */
   th = xTaskCreatePinnedToCore(
           readAltimeterTask,           /* function that executes this task*/
           "readAltimeter",             /* Function name - for debugging */
           STACK_SIZE*2,                /* Stack depth in words */
           NULL,                        /* parameter to be passed to the task */
           2,                           /* Task priority - in this case 1 */
           NULL,                        /* task handle that can be passed to other tasks to reference the task */
           app_id
        );

    if(th == pdPASS) {
        debugln("Read altimeter task created successfully");
    } else {
        debugln("Failed to create read altimeter task");
    }

    /* TASK 3: READ GPS DATA */
    th = xTaskCreatePinnedToCore(
            readGPSTask,         
            "readGPS",
            STACK_SIZE*2,                  
            NULL,                       
            1,
            NULL,
            app_id
        );

    if(th == pdPASS) {
        debugln("GPS task created");
    } else {
        debugln("Failed to create GPS task");
    }

    #if DEBUG_TO_TERMINAL   // set SEBUG_TO_TERMINAL to 0 to prevent serial debug data to serial monitor
    /* TASK 4: DISPLAY DATA ON SERIAL MONITOR - FOR DEBUGGING */
    th = xTaskCreatePinnedToCore(
            debugToTerminalTask,
            "displayData",
            STACK_SIZE,
            NULL,
            1,
            NULL,
            app_id
        );
        
    if(th == pdPASS) {
        Serial.println("Task created");
    } else {
        Serial.println("Task not created");
    }

    #endif // DEBUG_TO_TERMINAL_TASK


    /* TASK 4: TRANSMIT TELEMETRY DATA */
    // if(xTaskCreate(
    //         transmitTelemetry,
    //         "transmit_telemetry",
    //         STACK_SIZE*2,
    //         NULL,
    //         2,
    //         NULL
    // ) != pdPASS){
    //     debugln("[-]Transmit task failed to create");
    // }else{
    //     debugln("[+]Transmit task created success");
    // }

    #if LOG_TO_MEMORY   // set LOG_TO_MEMORY to 1 to allow loggin to memory 
        /* TASK 4: LOG DATA TO MEMORY */
        if(xTaskCreate(
                logToMemory,
                "logToMemory",
                STACK_SIZE,
                NULL,
                1,
                NULL
        ) != pdPASS){
            debugln("[-]logToMemory task failed to create");
        }else{
            debugln("[+]logToMemory task created success");
        }
    #endif // LOG_TO_MEMORY

    // if(xTaskCreate(
    //         flight_state_check,
    //         "testFSM",
    //         STACK_SIZE,
    //         NULL,
    //         2,
    //         NULL
    // ) != pdPASS){
    //     debugln("[-]FSM task failed to create");
    // }else{
    //     debugln("[+]FSM task created success");
    // }

}


/*!****************************************************************************
 * @brief Main loop
 *******************************************************************************/
void loop(){
//     if(WiFi.status() != WL_CONNECTED){
//         WiFi.begin(SSID, PASSWORD);
//         delay(500);
//         debug(".");
//     }

//    if(!mqtt_client.connected()){
//        /* try to reconnect if connection is lost */
//        reconnect();
//    }

//    mqtt_client.loop();

}