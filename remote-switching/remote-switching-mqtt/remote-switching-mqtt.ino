/**
 * This program implements remote switching over MQTT
 * 
 * Upload this file to the ESP32 that is inside the rocket 
 * Change the topic and other credentials as desired
 * 
 */

#include <WiFi.h>
#include <PubSubClient.h>

// Wifi
const char* ssid = "";  // your wifi name
const char* password = ""; // your wifi password

// MQTT broker
const char* mqtt_broker = "broker.emqx.io";
const char* topic = "emqx/esp32";
const char* mqtt_username = "emqx";
const char* mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

#define ARMING_PIN 4

/** 
 * implements remote switching for the avionics 
 */
void armAvionics(){
  digitalWrite(ARMING_PIN, HIGH);
}

/**
 * measures the voltage on the ARMING_PIN  
 */
void checkArming(){
  
}

void setup() {
  Serial.begin(115200);
  Serial.println("Implementing remote switching");

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
  while(!client.connected) {
    String client_id = "esp32_client-";
    client_id_ += String(WiFi.macAddress());

    if(client.connect(client_id.c_str(), mqtt_username, mqtt_password) {
      Serial.println("Connected to public MQTT broker");    
    } else {
      Serial.print("Failed with state "); 
      Serial.println(client.state());
      delay(1000);
    }
  }

  // publish and subscribe
  client.publish(topic, "Hi, I'm ESP32");

  // set up the avionics arming pin
  pinMode(ARMING_PIN, OUTPUT); 
}

void loop() {
  client.loop();
}

/**
 * this code runs every time a message arrives on the topic
 */
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  for (int i=0; i<length; i++) {
    Serial.println((char) payload[i]);
  }

  Serial.println();
  Serial.println("---------------------------------------");
}
