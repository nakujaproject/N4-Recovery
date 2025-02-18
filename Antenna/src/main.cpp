#include <Arduino.h>
#include "checkstate.h"
#include "readsensors.h"
#include "transmitwifi.h"
#include "defs.h"
#include "kalmanfilter.h"
#include "functions.h"
#include "SPIFFS.h"
#include<esp_wifi.h>
#include "esp_system.h"
#include "esp_mac.h"
#include "string.h"
#include "nvs_flash.h"
TimerHandle_t ejectionTimerHandle = NULL;

portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;

TaskHandle_t WiFiTelemetryTaskHandle;

TaskHandle_t GetDataTaskHandle;

TaskHandle_t SPIFFSWriteTaskHandle;

TaskHandle_t GPSTaskHandle;

// if 1 chute has been deployed
uint8_t isChuteDeployed = 0;

/* Onboard logging */
File file;

// determines whether we create an access point or connect to one
int access_point = 0;

float BASE_ALTITUDE = 0;

float temporalMaxAltitude = 0;

volatile int state = 0;

static uint16_t wifi_queue_length = 100;
static uint16_t spiff_queue_length = 500;
static uint16_t gps_queue_length = 100;

static QueueHandle_t wifi_telemetry_queue;
static QueueHandle_t spiff_queue;
static QueueHandle_t gps_queue;

void send_beacon(Data data) {
  Serial.printf("data receieved:%.3f",data.altitude);
  // long aaron = 78996566;
    // Buffer for the MAC address
    uint8_t mac_address[6];
    // Get the MAC address of the device
    esp_read_mac(mac_address, ESP_MAC_WIFI_SOFTAP); // Use the SoftAP MAC address

    // Construct a raw beacon frame
uint8_t raw_beacon_frame[] = {
    // MAC Header (24 bytes)
    0x80, 0x00,                 // Frame Control: Beacon frame
    0x00, 0x00,                 // Duration
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Destination (Broadcast)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Placeholder for Source MAC (to be set dynamically)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Placeholder for BSSID (to be set dynamically)
    0x00, 0x00,                 // Sequence number

    // Fixed Parameters (12 bytes)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Timestamp (8 bytes, zeroed for simplicity)
    0x64, 0x00,                                     // Beacon Interval (2 bytes, 1 TUs)
    0x01, 0x04,                                     // Capability Information (example: 0x0401)

    // Tagged Parameters (Payload)
    0x00, 0x04,                 // SSID Parameter Set (4 bytes following for SSID)
    'A', 'M', 'O', 'S',         // SSID ("AMOS")
    0x01, 0x08,                 // Supported rates (Example: 1, 2, 5.5, 11 Mbps)
    0x82, 0x84, 0x8b, 0x96,
    0x03, 0x01, 0x02,           // DS Parameter Set (Channel set to 2)

    // Vendor-Specific IE (Custom Message)
    0xDD,                       // Element ID (Vendor-Specific IE)
    0xFF,                       // Length (256 bytes)
    'C', 'u', 's', 't', 'o', 'm', ' ', 'M', 'e', 's', 's', 'a', 'g', 'e'
};



    // Custom message to include in the beacon frame
  //  const char custom_message[] = "Enock Sagit";

    // Calculate the size of the updated beacon frame
    size_t new_frame_size = sizeof(raw_beacon_frame) + 2 + sizeof(data);//increase the size of the new beacon frame
    uint8_t *custom_beacon_frame = (uint8_t *)malloc(new_frame_size);

    // Copy the original beacon frame into the new buffer
    memcpy(custom_beacon_frame, raw_beacon_frame, sizeof(raw_beacon_frame));
    Data* amos={0};
    // Append the custom message as an Information Element (IE)
    custom_beacon_frame[sizeof(raw_beacon_frame)] = 0xdd; // Vendor-specific IE ID
    Serial.printf("size of beacon frame:%d",sizeof(raw_beacon_frame));
    custom_beacon_frame[sizeof(raw_beacon_frame) + 1] =sizeof(data); // Length of the custom message
    memcpy(custom_beacon_frame + sizeof(raw_beacon_frame) + 2, &data, sizeof(data));
    // amos= (Data*)(custom_beacon_frame + sizeof(raw_beacon_frame) + 2);
    memcpy(custom_beacon_frame + 10, mac_address, 6); // SouSerial.printf("\nAfter: ");
    //  Serial.printf("\nAfter:%.3f,%.3f ",amos->altitude,amos->temperature);
    memcpy(custom_beacon_frame + 16, mac_address, 6); // BSSID

    // Transmit the raw beacon frame
    esp_wifi_80211_tx(WIFI_IF_AP, custom_beacon_frame, new_frame_size, true);

    // Free the allocated memory
    free(custom_beacon_frame);

    // Print debug information
    Serial.println("Beacon frame sent with the following MAC address:");
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", mac_address[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println();

    Serial.println("Custom message included in beacon frame:");
    Serial.printf("%.6f,%.3f",data.altitude,data.latitude);
} 
// callback for done ejection
void ejectionTimerCallback(TimerHandle_t ejectionTimerHandle)
{
  digitalWrite(EJECTION_PIN, LOW);
  isChuteDeployed = 1;
}

// Ejection fires the explosive charge using a relay or a mosfet
void ejection()
{
  if (isChuteDeployed == 0)
  {
    digitalWrite(EJECTION_PIN, HIGH);
    // TODO: is 3 seconds enough?
    ejectionTimerHandle = xTimerCreate("EjectionTimer", 3000 / portTICK_PERIOD_MS, pdFALSE, (void *)0, ejectionTimerCallback);
    xTimerStart(ejectionTimerHandle, portMAX_DELAY);
  }
}

struct Data readData()
{
  struct Data ld = {0};
  struct SensorReadings readings = {0};
  struct FilteredValues filtered_values = {0};

  readings = get_readings();//in readsensors.cpp

  // TODO: very important to know the orientation of the altimeter

  filtered_values = kalmanUpdate(readings.altitude, readings.ay - 9.8);

  // using mutex to modify state
  portENTER_CRITICAL(&mutex);
  state = checkState(filtered_values.displacement, temporalMaxAltitude, filtered_values.velocity, filtered_values.acceleration, state);
  portEXIT_CRITICAL(&mutex);
  if (temporalMaxAltitude < filtered_values.displacement)
  {
    temporalMaxAltitude = filtered_values.displacement;
  }

  ld = formart_data(readings, filtered_values);//in functions.cpp
  ld.state = state;
  ld.timeStamp = millis();
  return ld;
}

void GetDataTask(void *parameter)
{

  struct Data ld = {0};

  static int droppedWiFiPackets = 0;
  static int droppedSDPackets = 0;

  for (;;)
  {

    ld = readData();
    send_beacon(ld);
    // if (xQueueSend(wifi_telemetry_queue, (void *)&ld, 0) != pdTRUE)
    // {
    //   droppedWiFiPackets++;
    // }
    // if (xQueueSend(spiff_queue, (void *)&ld, 0) != pdTRUE)
    // {
    //   droppedSDPackets++;
    // }

    // debugf("Dropped WiFi Packets : %d\n", droppedWiFiPackets);
    // debugf("Dropped SD Packets : %d\n", droppedSDPackets);

    // yield to WiFi Telemetry task
    vTaskDelay(74 / portTICK_PERIOD_MS);
  }
}

// void WiFiTelemetryTask(void *parameter)
// {

//   struct Data sv = {0};
//   struct Data svRecords;
//   struct GPSReadings gpsReadings = {0};
//   float latitude = 0;
//   float longitude = 0;

//   for (;;)
//   {

//     // xQueueReceive(wifi_telemetry_queue, (void *)&sv, 10);
//     svRecords = sv;
//     svRecords.latitude = latitude;
//     svRecords.longitude = longitude;

//     if (xQueueReceive(gps_queue, (void *)&gpsReadings, 10) == pdTRUE)
//     {
//       latitude = gpsReadings.latitude;
//       longitude = gpsReadings.longitude;
//     }

//     // handleWiFi(svRecords);
// // send_beacon(svRecords);
//     // yield to Get Data task
//     vTaskDelay(35 / portTICK_PERIOD_MS);
//   }
// }

void readGPSTask(void *parameter)
{

  struct GPSReadings gpsReadings = {0};

  static int droppedGPSPackets = 0;

  for (;;)
  {
    gpsReadings = get_gps_readings();

    if (xQueueSend(gps_queue, (void *)&gpsReadings, 0) != pdTRUE)
    {
      droppedGPSPackets++;
    }

    debugf("Dropped GPS Packets : %d\n", droppedGPSPackets);

    // yield SD Write task
    // TODO: increase this up from 60 to 1000 in steps of 60 to improve queue performance at the expense of GPS
    // GPS will send 1 reading in 2s when set to 1000
    vTaskDelay(960 / portTICK_PERIOD_MS);
  }
}

void SPIFFSWriteTask(void *parameter)
{

  struct Data ld = {0};
  struct Data ldRecords[5];
  struct GPSReadings gps = {0};
  float latitude = 0;
  float longitude = 0;
  char telemetry_data[180];
  int id = 0;

  file = SPIFFS.open("/log.csv", FILE_APPEND);
  if (!file)
    debugln("[-] Failed to open file for appending");
  else
    debugln("[+] File opened for appending");

  for (;;)
  {

    for (int i = 0; i < 5; i++)
    {
      xQueueReceive(spiff_queue, (void *)&ld, 10);

      ldRecords[i] = ld;
      ldRecords[i].latitude = latitude;
      ldRecords[i].longitude = longitude;

      if (xQueueReceive(gps_queue, (void *)&gps, 10) == pdTRUE)
      {
        latitude = gps.latitude;
        longitude = gps.longitude;
      }
    }

    sprintf(telemetry_data,
            "%i,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%i,%.2f,%.2f,%.2f,%.16f,%.16f\n",
            id,            // 0
            ld.ax,         // 1
            ld.ay,         // 2
            ld.az,         // 3
            ld.gx,         // 4
            ld.gy,         // 5
            ld.gz,         // 6
            ld.state,      // 7
            ld.filtered_s, // 8
            ld.filtered_v, // 9
            ld.filtered_a, // 10
            gps.latitude,  // 11
            gps.longitude  // 12
    );
    // Serial.printf("Temperature:%.3f",ld.temperature);
    // send_beacon(ld);
    if (file.print(telemetry_data))
    {
      debugln("[+] Message appended");
    }
    else
    {
      debugln("[-] Append failed");
    }
    file.close();

    // yield to GPS Task
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup()
{

  Serial.begin(BAUD_RATE);
      WiFi.disconnect(true, true);
    // reset_wifi_configuration();
    // clear_wifi_credentials();
    // Initialize WiFi in AP mode
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_start();
  // set up ejection pin
  pinMode(EJECTION_PIN, OUTPUT);
  digitalWrite(EJECTION_PIN, LOW);

  // set up buzzer pin
  pinMode(buzzer_pin, OUTPUT);

  // if (access_point == 1)
  // {
  //   create_Accesspoint();
  // }
  // else
  // {

  //   setup_wifi();
  // }

  init_sensors();

  // setup flash memory
  if (!SPIFFS.begin(true))
    debugln("[-] An error occurred while mounting SPIFFS");
  else
    debugln("[+] SPIFFS mounted successfully");

  // get the base_altitude
  BASE_ALTITUDE = get_base_altitude();

  // wifi_telemetry_queue = xQueueCreate(wifi_queue_length, sizeof(Data));
  spiff_queue = xQueueCreate(spiff_queue_length, sizeof(Data));
  gps_queue = xQueueCreate(gps_queue_length, sizeof(GPSReadings));

  // initialize core tasks
  xTaskCreatePinnedToCore(GetDataTask, "GetDataTask", 3000, NULL, 1, &GetDataTaskHandle, 0);
  // xTaskCreatePinnedToCore(WiFiTelemetryTask, "WiFiTelemetryTask", 4000, NULL, 1, &WiFiTelemetryTaskHandle, 0);
  xTaskCreatePinnedToCore(readGPSTask, "ReadGPSTask", 3000, NULL, 1, &GPSTaskHandle, 1);
  xTaskCreatePinnedToCore(SPIFFSWriteTask, "SPIFFSWriteTask", 4000, NULL, 1, &SPIFFSWriteTaskHandle, 1);

  vTaskDelete(NULL);
}
void loop()
{
}




