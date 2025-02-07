#include "esp_wifi.h"
#include "esp_system.h"
#include <WiFi.h>    // For ESP32
#include <Arduino.h>
#define SP8T_PIN_1 17  // GPIO pin 13 for the first control pin
#define SP8T_PIN_2 5  // GPIO pin 12 for the second control pin
#define SP8T_PIN_3 18 // GPIO pin 14 for the third control pin
int antenna_switch(float altitude, int rssi); // Use 'float' for altitude, 'int' for rssi
void selectAntenna(int antenna);
struct Data
{
    uint64_t timeStamp;
    float altitude;
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
    float filtered_s;
    float filtered_v;
    float filtered_a;
    int state;
    float latitude;
    float longitude;
    float gpsAltitude;
    float temperature;
    int rssi;
    // int antenna_port;
};

void setupSP8T() {
  // Set the GPIO pins as output
  pinMode(SP8T_PIN_1, OUTPUT);
  pinMode(SP8T_PIN_2, OUTPUT);
  pinMode(SP8T_PIN_3, OUTPUT);

  // Start with all control lines low (default path)
  digitalWrite(SP8T_PIN_1, LOW);
  digitalWrite(SP8T_PIN_2, LOW);
  digitalWrite(SP8T_PIN_3, LOW);
}
// Define the promiscuous filter
wifi_promiscuous_filter_t filter = {
    .filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT // Capture only management frames
};

// Define the target MAC address (replace with the MAC address of your esp32 transmitting)
const uint8_t target_mac[6] = {0x10, 0x06, 0x1C, 0xA6, 0x0A, 0x5D};

// Callback function to handle incoming frames from tranmitting esp32
void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
    const wifi_promiscuous_pkt_t *packet = (wifi_promiscuous_pkt_t *)buf;
    const uint8_t *frame = packet->payload;
     int rssi =packet->rx_ctrl.rssi;
    // Check if it's a beacon frame (Frame Control field = 0x80 0x00)
    if (frame[0] == 0x80 && frame[1] == 0x00) { // Beacon frame
        // Extract source MAC address (frame bytes 10 to 15)
        const uint8_t *src_mac = frame + 10;
        
        // Compare the source MAC with the target MAC
        if (memcmp(src_mac, target_mac, 6) == 0) {
            // Serial.print("Beacon frame detected from target MAC: ");
            // for (int i = 0; i < 6; i++) {
            //     Serial.printf("%02X", src_mac[i]);
            //     if (i < 5) Serial.print(":");
            // }
            // Serial.println();

            // Start parsing the payload to find the Vendor-specific IE (0xDD)
            int offset = 36;  // Start after the MAC header + fixed parameters
            // while (offset <1000) {
                uint8_t element_id = frame[67];
                uint8_t length = frame[offset + 1];

                if (element_id == 0xDD) {  // Vendor-specific IE
                    // Check if the length matches the expected size
                        // Serial.printf("Offset:%d",offset);
                        //change the value based on where you data is located in the tranmitted payload
                        Data* data = (Data*)(frame+ 69);
                        data->rssi=rssi;
                        int antenna_Port=antenna_switch(data->altitude,data->rssi);
                        // Serial.printf("\n%ld",frame[69+i]);
                        // Process the custom data
                        // Serial.println("Custom Data Received:");
                        // Serial.printf("name: %.3f\n", data->name);
                        Serial.printf("Altitude: %.3f\n", data->altitude);
                        Serial.printf("Latitude: %.3f\n", data->latitude);
                        Serial.printf("temperature: %.3f\n", data->temperature);
                        Serial.printf("Timestamp: %.3f\n", data->timeStamp);
                        Serial.printf("state: %d\n", data->state);
                        Serial.printf("gx: %d\n", data->gx);
                        Serial.printf("gy: %d\n", data->gy);
                        Serial.printf("gz: %d\n", data->gz);
                        Serial.printf("state: %d\n", data->longitude);
                        Serial.printf("filtered_a: %d\n", data->filtered_a);
                        Serial.printf("filtered_v: %d\n", data->filtered_v);
                        Serial.printf("state: %d\n", data->filtered_s);
                        Serial.printf("ax: %d\n", data->ax);
                        Serial.printf("ay: %d\n", data->ay);
                        Serial.printf("az: %d\n", data->az);
                        Serial.printf("rssi: %d\n",data->rssi);
                        Serial.printf("antenna: %d\n",antenna_Port);
         
                    // break;  // Exit after processing the first Vendor-specific IE
                // }

                // Move to the next Information Element
                offset += 2 + length;
            // }
        }
    }
}
}
void setup() {
    Serial.begin(115200);
    delay(1000);
    setupSP8T();
  // Start Wi-Fi in Access Point mode to send data
  // WiFi.softAP(ssid, password);
    // Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // Configure WiFi to promiscuous mode
    esp_wifi_set_mode(WIFI_MODE_NULL);  // Disable STA/AP modes
    esp_wifi_start();  // Start WiFi

    esp_wifi_set_promiscuous_filter(&filter);  // Set the promiscuous filter
    esp_wifi_set_promiscuous_rx_cb(promiscuous_rx_cb);  // Set the callback

    Serial.println("Ready to capture beacon frames...");
}
//set your condition for antenna switching, using altitude and received signal strength in
int antenna_switch(float altitude,int rssi){
  if (altitude<1000 ){
    selectAntenna(4);
    Serial.printf("check on led");
    return 1;
  }
  else if (altitude>1000 && altitude<2000 ){
    selectAntenna(5);
    Serial.printf("check on led");
    return 2;
  }
  else if( altitude>2000 && altitude<3000 ){
    selectAntenna(6);
    Serial.printf("check on led");
    return 3;
  }
  else{
    selectAntenna(8);
    return 4;
  }
}
void selectAntenna(int antenna) {
  if (antenna < 0 || antenna > 7) {
    Serial.println("Invalid antenna index. Must be between 0 and 7.");
    return;
  }

  // Set the control pins based on the antenna selection
  digitalWrite(SP8T_PIN_1, (antenna & 0b001) ? HIGH : LOW);
  digitalWrite(SP8T_PIN_2, (antenna & 0b010) ? HIGH : LOW);
  digitalWrite(SP8T_PIN_3, (antenna & 0b100) ? HIGH : LOW);
}
void loop() {
    // Enable promiscuous mode to start scanning frames
    esp_wifi_set_promiscuous(true);

    // Capture frames for 10 seconds
    Serial.println("Starting frame capture...");
     delay(10000);  // Capture frames for 10 seconds

    // Disable promiscuous mode
    esp_wifi_set_promiscuous(false);
    Serial.println("Stopping frame capture. Waiting for the next scan...");

    // Wait for 20 seconds before the next scan (optional)
    // delay(20000);
}
