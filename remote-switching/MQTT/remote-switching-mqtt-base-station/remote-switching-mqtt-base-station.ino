/**
 * This program is uploaded on the base station ESP 
 * This ESP should be on the same network as the ESP on the flight computer
 */
 
#include <WiFi.h>
#include <PubSubClient.h>

// Wifi
const char* ssid = "Nakuja";  // your wifi name
const char* password = "987654321"; // your wifi password

// MQTT broker
const char* mqtt_broker = "broker.emqx.io"; // use the IP ADDRESS of the computer running the MQTT broker
const char* topic = "rocket/arm";
const char* mqtt_username = "emqx";
const char* mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// arming variables 
#define ARMING_BUTTON_PIN 13
uint8_t arming_status = 0;
unsigned long last_arming_time = 0; 
unsigned long current_arming_time = 0;
uint16_t arming_interval = 1000;

/**
 * Parse command received from MQTT
 */
void parseMQTTResponse(String response) {
  Serial.print("Calling parser with command: ");
  Serial.println(response);
  
  if(response == "\"[ACK] ARMED\"") {
    Serial.println("[acknowledge] Rocket armed successful");
    // do something for feedback
    // TODO: digital write to buzzer or LED
  } else if (response == "\"[ACK] FAILED ARMING \"") {
    Serial.println("[acknowledge] Rocket arming failed");
    digitalWrite(ARMING_PIN, LOW);
  } else if(response == "\"[ACK] DISARMED \"" ) {
    Serial.println("[acknowledge] Rocket disarmed successfully");
    // digitalWrite();
  }
  
}

void setup() {
  Serial.begin(115200);
  Serial.println("Arming circuit test");

  // connect to a WIFI network
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi network");
  
  // connecting to MQTT broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while(!client.connected()) {
    String client_id = "esp32_client-";
    client_id += String(WiFi.macAddress());

    if(client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected to public MQTT broker");    
    } else {
      Serial.print("Failed with state "); 
      Serial.println(client.state());
      delay(1000);
    }
  }

  // publish and subscribe
  // client.publish(topic, "[HELLO FROM ROCKET]");
  client.subscribe(topic);
  
  pinMode(ARMING_BUTTON, INPUT);
  
}

void loop() {
  client.loop();
}
