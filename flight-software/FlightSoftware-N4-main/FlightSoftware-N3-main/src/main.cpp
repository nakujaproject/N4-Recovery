#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h> 
#include <Adafruit_BMP085.h>
#include <TinyGPS++.h>
#include "FS.h"
#include "SPIFFS.h"
#include <SPIMemory.h>
#include "sensors.h"
#include "defs.h"
#include "state_machine.h"

/**
 * DEBUG 
 * debug functions
 * 1. display_data()
 * 
 */
int state_leds[1] = {LED_BUILTIN};
int state;
State_machine fsm;

/* create Wi-Fi Client */
WiFiClient wifi_client;

/* create MQTT publish-subscribe client */
PubSubClient mqtt_client(wifi_client);

/* create gyroscope object */
Adafruit_MPU6050 gyroscope;

/* create altimeter objects */
Adafruit_BMP085 altimeter;

/* GPS Setup*/
HardwareSerial hard(2);
TinyGPSPlus gps;

/* Onboard logging */
File file;
SPIFlash flash(SS, &SPI);

/* position integration variables */
long long current_time = 0;
long long previous_time = 0;

/* velocity integration variables */
double y_velocity = 0;
double y_displacement = 0;

float new_y_displacement = 0.0;
float old_y_displacement = 0.0;
double old_y_velocity = 0.0;
double new_y_velocity = 0.0;
double total_y_displacement = 0.0;

//
double fallBackLat = -1.0953775626377544;
double fallBackLong = 37.01223403257954;

// fallback eject timer
unsigned long TTA = 10000;
unsigned long timer_offset = 0;
bool offset_flag = true;

/* functions to initialize sensors */
void initialize_gyroscope(){
    /* attempt to initialize MPU6050 */
    if(!gyroscope.begin(0x68)){
        debugln("[-]Gyroscope allocation failed!");
        // loop forever until found
        while(true){
            if(gyroscope.begin(0x68)) break;
            delay(500);
            debug(".");
        }
    }

    debugln("[+]Gyroscope Initialized");
    gyroscope.setAccelerometerRange(MPU6050_RANGE_8_G);
    gyroscope.setGyroRange(MPU6050_RANGE_500_DEG);
    gyroscope.setFilterBandwidth(MPU6050_BAND_5_HZ);

    delay(SETUP_DELAY);
}

void initialize_altimeter(){
    if (!altimeter.begin()) {
        debugln("[-]Could not find a valid altimeter sensor");
        while (1) {
            // if(altimeter.begin()) break;
            debug(".");
        }
    }

    debugln("[+]Altimeter initialized");
}

/* data variables */
/* gyroscope data */

struct Acceleration_Data{
    double ax;
    double ay; 
    double az;
    double gx;
    double gy;
    double gz;
};

struct GPS_Data{
    double latitude;
    double longitude;; 
    uint time;
};

struct Altimeter_Data{
    int32_t pressure;
    double altitude;
    double velocity;
    double AGL; /* altitude above ground level */
};

struct Telemetry_Data{
    float ax;
    float ay; 
    float az;
    float gx;
    float gy; 
    float gz;
    int32_t pressure;
    float altitude;
    float velocity;
    float AGL; /* altitude above ground level */
    double latitude;
    double longitude;; 
    uint time;
};

/* create queue to store altimeter data
 * store pressure and altitude
 * */
QueueHandle_t gyroscope_data_queue;
QueueHandle_t altimeter_data_queue;
QueueHandle_t gps_data_queue;
QueueHandle_t telemetry_data_queue; /* This queue will hold all the sensor data for transmission to ground station*/
QueueHandle_t filtered_data_queue;
QueueHandle_t flight_states_queue;

void connectToWifi(){
    digitalWrite(LED_BUILTIN, HIGH);
    /* Connect to a Wi-Fi network */
    debugln("[..]Scanning for network...");

    WiFi.begin(SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        debugln("[..]Scanning for network...");
    }

    debugln("[+]Network found");debug("[+]My IP address: "); debugln();
    debugln(WiFi.localIP());
    digitalWrite(LED_BUILTIN, LOW);
}

void initializeMQTTParameters(){
    /* this functions creates an MQTT client for transmitting telemetry data */;
    mqtt_client.setServer(MQTT_SERVER, MQTT_PORT);
}

void readAltimeter(void* pvParameters){

    while(true){
        /* Read pressure.
         * This is the pressure from the sea level.
         * */
        struct Altimeter_Data altimeter_data;

        /* Read pressure
         * This is the pressure from the sea level
         * */
        altimeter_data.pressure = altimeter.readSealevelPressure();

        /* Read altitude
         * This is the altitude from the sea level
         * */
        altimeter_data.altitude = altimeter.readAltitude(SEA_LEVEL_PRESSURE);

        /* approximate velocity from acceleration by integration for apogee detection */
        current_time = millis();

        /* differentiate displacement to get velocity */
        new_y_displacement = altimeter_data.altitude - BASE_ALTITUDE;
        y_velocity = (new_y_displacement - old_y_displacement) / (current_time - previous_time);

        /* update integration variables */
        previous_time = current_time;
        old_y_displacement = new_y_displacement;

        /* ------------------------ END OF APOGEE DETECTION ALGORITHM ------------------------ */

        /* subtract current altitude to get the maximum height reached */
        float rocket_height = altimeter_data.altitude - BASE_ALTITUDE;

        /* update altimeter data */
        altimeter_data.velocity = y_velocity;
        altimeter_data.AGL = rocket_height;

        /* send data to altimeter queue */
        if(xQueueSend(altimeter_data_queue, &altimeter_data, portMAX_DELAY) != pdPASS){
            debugln("[-]Altimeter queue full");
        }

        // delay(TASK_DELAY);
    }
}

void readGyroscope(void* pvParameters){
    
    while(true){
        sensors_event_t a, g, temp;
        gyroscope.getEvent(&a, &g, &temp);
        
        struct Acceleration_Data gyro_data;
        /* 
        * Read accelerations on all axes
         * */
        gyro_data.ax = a.acceleration.x;
        gyro_data.ay = a.acceleration.y;
        gyro_data.az = a.acceleration.z;
        gyro_data.gx = g.gyro.x;
        gyro_data.gy = g.gyro.y;
        gyro_data.gz = g.gyro.z;

        // FILTER THIS READINGS

        /* send data to gyroscope queue */
        if(xQueueSend(gyroscope_data_queue, &gyro_data, portMAX_DELAY) != pdPASS){
            debugln("[-]Gyro queue full");
        }

        // delay(TASK_DELAY);
    }
}

void readGPS(void* pvParameters){
    /* This function reads GPS data and sends it to the ground station */
    struct GPS_Data gps_data;
    while(true){
        while (hard.available() > 0)
        {
            gps.encode(hard.read());
        }
        if (gps.location.isUpdated()){
            gps_data.latitude = gps.location.lat();
            gps_data.longitude = gps.location.lng();
            gps_data.time = gps.time.value();
            fallBackLat = gps_data.latitude;
            fallBackLong = gps_data.longitude;
            debugln("[!!] GPS Data Received [!!]");
            if(xQueueSend(gps_data_queue, &gps_data, portMAX_DELAY) != pdPASS){
                debugln("[-]GPS queue full");
            }
            // delay(TASK_DELAY);
        }else{
            gps_data.latitude = fallBackLat;
            gps_data.longitude = fallBackLong;
            gps_data.time = 20230601;
            if(xQueueSend(gps_data_queue, &gps_data, portMAX_DELAY) != pdPASS){
                debugln("[-]GPS queue full");
            }
            // delay(TASK_DELAY);
        }
    }
}

void displayData(void* pvParameters){
   while(true){
       struct Acceleration_Data gyroscope_buffer;
       struct Altimeter_Data altimeter_buffer;
       struct GPS_Data gps_buffer;
       if(xQueueReceive(gyroscope_data_queue, &gyroscope_buffer, portMAX_DELAY) == pdPASS){
           debugln("------------------------------");
            debug("x: "); debug(gyroscope_buffer.ax); debugln();
            debug("y: "); debug(gyroscope_buffer.ay); debugln();
            debug("z: "); debug(gyroscope_buffer.az); debugln();
            debug("roll: "); debug(gyroscope_buffer.gx); debugln();
            debug("pitch: "); debug(gyroscope_buffer.gy); debugln();
            debug("yaw: "); debug(gyroscope_buffer.gz); debugln();
       }else{
           /* no queue */
       }

        if(xQueueReceive(altimeter_data_queue, &altimeter_buffer, portMAX_DELAY) == pdPASS){
            debug("Pressure: "); debug(altimeter_buffer.pressure); debugln();
            debug("Altitude: "); debug(altimeter_buffer.altitude); debugln();
            debug("Velocity: "); debug(altimeter_buffer.velocity); debugln();
            debug("AGL: "); debug(altimeter_buffer.AGL); debugln();
            
        }else{
            /* no queue */
        }

        if(xQueueReceive(gps_data_queue, &gps_buffer, portMAX_DELAY) == pdPASS){
            debug("Lattitude: "); debug(gps_buffer.latitude); debugln();
            debug("Longitude: "); debug(gps_buffer.longitude); debugln();
            debug("Time: "); debug(gps_buffer.time); debugln();
            
        }else{
            /* no queue */
        }


    //    delay(10);
   }
}

void transmitTelemetry(void* pvParameters){
    /* This function sends data to the ground station */

     /*  create two pointers to the data structures to be transmitted */
    
    char telemetry_data[180];
    struct Acceleration_Data gyroscope_data_receive;
    struct Altimeter_Data altimeter_data_receive;
    struct GPS_Data gps_data_receive;
    int32_t flight_state_receive;
    int id = 0;

    while(true){
        file = SPIFFS.open("/log.csv", FILE_APPEND);
        if(!file) debugln("[-] Failed to open file for appending");
        else debugln("[+] File opened for appending");
        
        /* receive data into respective queues */
        if(xQueueReceive(gyroscope_data_queue, &gyroscope_data_receive, portMAX_DELAY) == pdPASS){
            debugln("[+]Gyro data ready for sending ");
        }else{
            debugln("[-]Failed to receive gyro data");
        }

        if(xQueueReceive(altimeter_data_queue, &altimeter_data_receive, portMAX_DELAY) == pdPASS){
            debugln("[+]Altimeter data ready for sending ");
        }else{
            debugln("[-]Failed to receive altimeter data");
        }

        if(xQueueReceive(gps_data_queue, &gps_data_receive, portMAX_DELAY) == pdPASS){
            debugln("[+]GPS data ready for sending ");
        }else{
            debugln("[-]Failed to receive GPS data");
        }

        if(xQueueReceive(flight_states_queue, &flight_state_receive, portMAX_DELAY) == pdPASS){
            debugln("[+]Flight state ready for sending ");
        }else{
            debugln("[-]Failed to receive Flight state");
        }

        sprintf(telemetry_data,
            "%i,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%i,%.16f,%.16f,%i,%i\n",
            id,//0
            gyroscope_data_receive.ax,//1
            gyroscope_data_receive.ay,//2
            gyroscope_data_receive.az,//3
            gyroscope_data_receive.gx,//4
            gyroscope_data_receive.gy,//5
            gyroscope_data_receive.gz,//6
            altimeter_data_receive.AGL,//7
            altimeter_data_receive.altitude,//8
            altimeter_data_receive.velocity,//9
            altimeter_data_receive.pressure,//10
            gps_data_receive.latitude,//11
            gps_data_receive.longitude,//12
            gps_data_receive.time,//13
            flight_state_receive//14
        );
        if(file.print(telemetry_data)){
            debugln("[+] Message appended");
        } else {
            debugln("[-] Append failed");
        }
        file.close();
        id+=1;

        if(mqtt_client.publish("n3/telemetry", telemetry_data)){
            debugln("[+]Data sent");
        } else{
            debugln("[-]Data not sent");
        }
    }
}

void reconnect(){

    while(!mqtt_client.connected()){
        debug("[..]Attempting MQTT connection...");
        String client_id = "[+]FC Client - ";
        client_id += String(random(0XFFFF), HEX);

        if(mqtt_client.connect(client_id.c_str())){
            debugln("[+]MQTT connected");
        }
    }
}

void testMQTT(void *pvParameters){
    while(true){
        debugln("Publishing data");
        if(mqtt_client.publish("n3/telemetry", "Hello from flight!")){
            debugln("Data sent");
        }else{
            debugln("Unable to send data");
        }
    }
}


void flight_state_check(void* pvParameters){
    /* Set flight state based on sensor values */
    int32_t flight_state = PRE_FLIGHT;
    struct Altimeter_Data altimeter_data_receive;
    while(true){
        if(xQueueReceive(altimeter_data_queue, &altimeter_data_receive, portMAX_DELAY) == pdPASS){
            debugln("[+]Altimeter data in state machine");

            /*------------- STATE MACHINE -------------------------------------*/
            flight_state = fsm.checkState(altimeter_data_receive.altitude, altimeter_data_receive.velocity);

            /*------------- DEPLOY PARACHUTE ALGORITHM -------------------------------------*/
            if(flight_state>=POWERED_FLIGHT){//start countdown to ejection
                if(offset_flag){
                    timer_offset = millis();
                    offset_flag = false;
                }
                if(TTA-(millis()-timer_offset)<0){
                    pinMode(EJECTION_PIN,HIGH);
                    delay(5000);
                }
                if(flight_state>=APOGEE && flight_state<PARACHUTE_DESCENT) {
                    pinMode(EJECTION_PIN,HIGH);
                    delay(5000);
                }
                else pinMode(EJECTION_PIN,LOW);
            }

        }else{
            debugln("[-]Failed to receive altimeter data in state machine");
        }
        if(xQueueSend(flight_states_queue, &flight_state, portMAX_DELAY) != pdPASS) debugln("[-]Failed to update state");
    }
}

void setup(){
    /* initialize serial */
    Serial.begin(115200);

    /* Setup GPS*/
    hard.begin(9600, SERIAL_8N1, RX, TX);

    //
    if (!SPIFFS.begin(true)) debugln("[-] An error occurred while mounting SPIFFS");
    else debugln("[+] SPIFFS mounted successfully");

    //setup flash memory
    // if (!flash.begin()) debugln("[-] An error occurred while mounting flash");
    // else{
    //     debug("[+] Flash mounted successfully ");
    //     debugln(((String)flash.getCapacity() + " bytes" ));

    //     flash.eraseChip();
    // }

    /* DEBUG: set up state simulation leds */
    for(auto pin: state_leds){
        pinMode(state_leds[pin], OUTPUT);
    }

    /* connect to WiFi*/
    connectToWifi();

    /* initialize sensors */
    initialize_gyroscope();
    initialize_altimeter();
    // todo: initialize flash memory

    // mqtt_client.setBufferSize(MQTT_BUFFER_SIZE);
    mqtt_client.setServer(MQTT_SERVER, MQTT_PORT);

    debugln("Creating queues...");

    /* create gyroscope data queue */
    gyroscope_data_queue = xQueueCreate(GYROSCOPE_QUEUE_LENGTH, sizeof(struct Acceleration_Data));

    /* create altimeter_data_queue */   
    altimeter_data_queue = xQueueCreate(ALTIMETER_QUEUE_LENGTH, sizeof(struct Altimeter_Data));

    /* create gps_data_queue */   
    gps_data_queue = xQueueCreate(GPS_QUEUE_LENGTH, sizeof(struct GPS_Data));

    /* create queue to hols all the sensor's data */
    telemetry_data_queue = xQueueCreate(ALL_TELEMETRY_DATA_QUEUE_LENGTH, sizeof(struct Telemetry_Data));

    /* this queue will hold the flight states */
    flight_states_queue = xQueueCreate(FLIGHT_STATES_QUEUE_LENGTH, sizeof(int32_t));


    /* check if the queues were created successfully */
    if(gyroscope_data_queue == NULL){
        debugln("[-]Gyroscope data queue creation failed!");
    } else{
        debugln("[+]Gyroscope data queue creation success");
    }
    
    if(altimeter_data_queue == NULL){
        debugln("[-]Altimeter data queue creation failed!");
    } else{
        debugln("[+]Altimeter data queue creation success");
    }

    if(gps_data_queue == NULL){
        debugln("[-]GPS data queue creation failed!");
    } else{
        debugln("[+]GPS data queue creation success");
    }

    if(filtered_data_queue == NULL){
        debugln("[-]Filtered data queue creation failed!");
    } else{
        debugln("[+]Filtered data queue creation success");
    }

    if(flight_states_queue == NULL){
        debugln("[-]Flight states queue creation failed!");
    } else{
        debugln("[+]Flight states queue creation success");
    }

    /* Create tasks
     * All tasks have a stack size of 1024 words - not bytes!
     * ESP32 is 32 bit, therefore 32bits x 1024 = 4096 bytes
     * So the stack size is 4096 bytes
     * */
    debugln("Creating tasks...");

    /* TASK 1: READ ALTIMETER DATA */
   if(xTaskCreate(
           readAltimeter,               /* function that executes this task*/
           "readAltimeter",             /* Function name - for debugging */
           STACK_SIZE,                  /* Stack depth in words */
           NULL,                        /* parameter to be passed to the task */
           2,        /* Task priority - in thGYROSCOPEis case 1 */
           NULL                         /* task handle that can be passed to other tasks to reference the task */
   ) != pdPASS){
    // if task creation is not successful
    debugln("[-]Read-Altimeter task creation failed!");

   }else{
    debugln("[+]Read-Altimeter task creation success");
   }

    /* TASK 2: READ GYROSCOPE DATA */
   if(xTaskCreate(
           readGyroscope,         
           "readGyroscope",
           STACK_SIZE*2,                  
           NULL,                       
           2,
           NULL    
   ) != pdPASS){
    debugln("[-]Read-Gyroscope task creation failed!");
   } else{
    debugln("[+]Read-Gyroscope task creation success!");
   }

   /* TASK 2: READ GPS DATA */
    if(xTaskCreate(
              readGPS,         
              "readGPS",
              STACK_SIZE,                  
              NULL,                       
              2,
              NULL
        ) != pdPASS){
        debugln("[-]Read-GPS task creation failed!");
    } else{
        debugln("[+]Read-GPS task creation success!");
    }

    /* TASK 3: DISPLAY DATA ON SERIAL MONITOR - FOR DEBUGGING */
    if (DEBUG)
    if(xTaskCreate(
            displayData,
            "displayData",
            STACK_SIZE,
            NULL,
            2,
            NULL
            ) != pdPASS){
        debugln("[-]Display data task creation failed!");
        }else{
        debugln("[+]Display data task creation success!");
    }

    /* TASK 4: TRANSMIT TELEMETRY DATA */
    if(xTaskCreate(
            transmitTelemetry,
            "transmit_telemetry",
            STACK_SIZE*2,
            NULL,
            2,
            NULL
    ) != pdPASS){
        debugln("[-]Transmit task failed to create");
    }else{
        debugln("[+]Transmit task created success");
    }

    // if(xTaskCreate(
    //         testMQTT,
    //         "testMQTT",
    //         STACK_SIZE,
    //         NULL,
    //         1,
    //         NULL
    // ) != pdPASS){
    //     debugln("[-]Test mqtt task failed to create");
    // }else{
    //     debugln("[+]Test mqtt task created success");
    // }

    if(xTaskCreate(
            flight_state_check,
            "testFSM",
            STACK_SIZE,
            NULL,
            2,
            NULL
    ) != pdPASS){
        debugln("[-]FSM task failed to create");
    }else{
        debugln("[+]FSM task created success");
    }

}

void loop(){
    if(WiFi.status() != WL_CONNECTED){
        WiFi.begin(SSID, PASSWORD);
        delay(500);
        debug(".");
    }

   if(!mqtt_client.connected()){
       /* try to reconnect if connection is lost */
       reconnect();
   }

   mqtt_client.loop();

}