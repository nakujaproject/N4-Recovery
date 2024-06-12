/**
 * 
 * 0x5765206D6179206D616B65206F757220706C616E73202C
 * 0x62757420476F642068617320746865206C61737420776F7264
 * 
*/

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <TinyGPS++.h>
#include "sensors.h"
#include "defs.h"
#include "state_machine.h"
#include "mpu.h"
#include <SFE_BMP180.h>
#include "SerialFlash.h"
#include "logger.h"
#include "data-types.h"

/**
 * DEBUG 
 * debug functions
 * 1. display_data()
 * 
 */

/* create Wi-Fi Client */
WiFiClient wifi_client;

/* create MQTT publish-subscribe client */
PubSubClient mqtt_client(wifi_client);

/* GPS Setup*/
HardwareSerial hard(2);
TinyGPSPlus gps;

/* Flight data logging */
uint8_t cs_pin = 5;
uint8_t flash_led_pin = 4;
char filename[] = "flight1.bin";    // Filename must be less than 20 chars, including the file extension
uint32_t FILE_SIZE_512K = 524288L;  // 512KB
uint32_t FILE_SIZE_1M  = 1048576L;  // 1MB
uint32_t FILE_SIZE_4M  = 4194304L;  // 4MB
SerialFlashFile file;
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

// initialize the BMP180 altimeter
void BMPInit() {
    if(altimeter.begin()) {
        Serial.println("BMP init success");
        // TODO: update system table
    } else {
        Serial.println("BMP init failed");
    }
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
// QueueHandle_t gps_data_queue;
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
void readAccelerationTask(void* pvParameter) {

    telemetry_type_t acc_data_lcl;

    while(1) {
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

void readAltimeter(void* pvParameters){
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
        xQueueSend(telemetry_data_qHandle, &alt_data_lcl, 0); 

    }

}


// void readGPS(void* pvParameters){
//     /* This function reads GPS data and sends it to the ground station */
//     struct GPS_Data gps_data;
//     while(true){
//         while (hard.available() > 0)
//         {
//             gps.encode(hard.read());
//         }
//         if (gps.location.isUpdated()){
//             gps_data.latitude = gps.location.lat();
//             gps_data.longitude = gps.location.lng();
//             gps_data.time = gps.time.value();
//             fallBackLat = gps_data.latitude;
//             fallBackLong = gps_data.longitude;
//             debugln("[!!] GPS Data Received [!!]");
//             if(xQueueSend(gps_data_queue, &gps_data, portMAX_DELAY) != pdPASS){
//                 debugln("[-]GPS queue full");
//             }
//             // delay(TASK_DELAY);
//         }else{
//             gps_data.latitude = fallBackLat;
//             gps_data.longitude = fallBackLong;
//             gps_data.time = 20230601;
//             if(xQueueSend(gps_data_queue, &gps_data, portMAX_DELAY) != pdPASS){
//                 debugln("[-]GPS queue full");
//             }
//             // delay(TASK_DELAY);
//         }
//     }
// }

void debugToTerminal(void* pvParameters){
    telemetry_type_t rcvd_data; // accelration received from acceleration_queue

    while(true){
        if(xQueueReceive(telemetry_data_qHandle, &rcvd_data, portMAX_DELAY) == pdPASS){
            // debug CSV to terminal 
            debug(rcvd_data.acc_data.ax); debug(","); 
            debug(rcvd_data.acc_data.ay); debug(","); 
            debug(rcvd_data.acc_data.az); debug(","); 
            debug(rcvd_data.acc_data.pitch); debug(","); 
            debug(rcvd_data.acc_data.roll); debug(","); 
            debug(rcvd_data.alt_data.pressure); debug(","); 
            debug(rcvd_data.alt_data.velocity); debug(","); 
            debug(rcvd_data.alt_data.altitude); debug(","); 
            debug(rcvd_data.alt_data.temperature); debug(","); 

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

/**
 * Task to log the data to flash memory
 * 
*/
void logToMemory(void* pvParameter) {
    telemetry_type_t received_packet;
    telemetry_type_t* p_received_packet = &received_packet;

    while(1) {
        xQueueReceive(telemetry_data_qHandle, &received_packet, portMAX_DELAY);
        data_logger.loggerWrite(p_received_packet);
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


// void flight_state_check(void* pvParameters){
//     /* Set flight state based on sensor values */
//     int32_t flight_state = PRE_FLIGHT;
//     struct Altimeter_Data altimeter_data_receive;
//     while(true){
//         if(xQueueReceive(altimeter_data_queue, &altimeter_data_receive, portMAX_DELAY) == pdPASS){
//             debugln("[+]Altimeter data in state machine");

//             /*------------- STATE MACHINE -------------------------------------*/
//             flight_state = fsm.checkState(altimeter_data_receive.altitude, altimeter_data_receive.velocity);

//             /*------------- DEPLOY PARACHUTE ALGORITHM -------------------------------------*/
//             if(flight_state>=POWERED_FLIGHT){//start countdown to ejection
//                 if(offset_flag){
//                     timer_offset = millis();
//                     offset_flag = false;
//                 }
//                 if(TTA-(millis()-timer_offset)<0){
//                     pinMode(EJECTION_PIN,HIGH);
//                     delay(5000);
//                 }
//                 if(flight_state>=APOGEE && flight_state<PARACHUTE_DESCENT) {
//                     pinMode(EJECTION_PIN,HIGH);
//                     delay(5000);
//                 }
//                 else pinMode(EJECTION_PIN,LOW);
//             }

//         }else{
//             debugln("[-]Failed to receive altimeter data in state machine");
//         }
//         if(xQueueSend(flight_states_queue, &flight_state, portMAX_DELAY) != pdPASS) debugln("[-]Failed to update state");
//     }
// }

void setup(){
    /* initialize serial */
    Serial.begin(115200);

    uint8_t app_id = xPortGetCoreID();
    BaseType_t th;

    /* Setup GPS*/
    // hard.begin(9600, SERIAL_8N1, RX, TX);

    /* initialize the data logging system - logs to flash memory */
    data_logger.loggerInit();

    /* connect to WiFi*/
    // connectToWifi();

    ///////////////////////// PERIPHERALS INIT /////////////////////////
    imu.init();
    BMPInit();

    //==============================================================
    ;
    // initialize_altimeter();
    // todo: initialize flash memory

    // mqtt_client.setBufferSize(MQTT_BUFFER_SIZE);
    // mqtt_client.setServer(MQTT_SERVER, MQTT_PORT);

    // debugln("Creating queues...");

    ///////////////////// create data queues ////////////////////
    // this queue holds the data from MPU 6050 - this data is filtered already
    accel_data_qHandle = xQueueCreate(GYROSCOPE_QUEUE_LENGTH, sizeof(accel_type_t)); 

    // this queue hold the data read from the BMP180
    altimeter_data_qHandle = xQueueCreate(ALTIMETER_QUEUE_LENGTH, sizeof(altimeter_type_t)); 

    // this queue holds the telemetry data packet
    telemetry_data_qHandle = xQueueCreate(TELEMETRY_DATA_QUEUE_LENGTH, sizeof(telemetry_packet));

    // /* create altimeter_data_queue */   
    // altimeter_data_queue = xQueueCreate(ALTIMETER_QUEUE_LENGTH, sizeof(struct Altimeter_Data));

    // /* create gps_data_queue */   
    // gps_data_queue = xQueueCreate(GPS_QUEUE_LENGTH, sizeof(struct GPS_Data));

    // /* create queue to hols all the sensor's data */
    // telemetry_data_queue = xQueueCreate(ALL_TELEMETRY_DATA_QUEUE_LENGTH, sizeof(struct Telemetry_Data));

    // /* this queue will hold the flight states */
    // flight_states_queue = xQueueCreate(FLIGHT_STATES_QUEUE_LENGTH, sizeof(int32_t));


    /* check if the queues were created successfully */
    // if(accel_data_qHandle == NULL){
    //     debugln("[-]accel data queue creation failed!");
    // } else{
    //     debugln("[+]accel data queue creation success");
    // }
    
    // if(altimeter_data_queue == NULL){
    //     debugln("[-]Altimeter data queue creation failed!");
    // } else{
    //     debugln("[+]Altimeter data queue creation success");
    // }

    // if(gps_data_queue == NULL){
    //     debugln("[-]GPS data queue creation failed!");
    // } else{
    //     debugln("[+]GPS data queue creation success");
    // }

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
    // debugln("Creating tasks...");

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
    Serial.println("Task created");
   } else {
    Serial.println("Task creation failed");
   }

   
    /* TASK 1: READ ALTIMETER DATA */
   if(xTaskCreate(
           readAltimeter,               /* function that executes this task*/
           "readAltimeter",             /* Function name - for debugging */
           STACK_SIZE,                  /* Stack depth in words */
           NULL,                        /* parameter to be passed to the task */
           2,                           /* Task priority - in this case 1 */
           NULL                         /* task handle that can be passed to other tasks to reference the task */
   ) != pdPASS){
    // if task creation is not successful
    debugln("[-]Read-Altimeter task creation failed!");

   }else{
    debugln("[+]Read-Altimeter task creation success");
   }

   /* TASK 2: READ GPS DATA */
    // if(xTaskCreate(
    //           readGPS,         
    //           "readGPS",
    //           STACK_SIZE,                  
    //           NULL,                       
    //           2,
    //           NULL
    //     ) != pdPASS){
    //     debugln("[-]Read-GPS task creation failed!");
    // } else{
    //     debugln("[+]Read-GPS task creation success!");
    // }

    /* TASK 3: DISPLAY DATA ON SERIAL MONITOR - FOR DEBUGGING */
    th = xTaskCreatePinnedToCore(
            debugToTerminal,
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