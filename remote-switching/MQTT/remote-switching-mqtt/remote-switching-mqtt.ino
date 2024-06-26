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
#define ARMING_PIN 13
uint8_t arming_status = 0;
unsigned long last_arming_retry_time = 0; 
unsigned long current_arming_retry_time = 0;
uint16_t arming_interval = 1000;

/** 
 * implements remote switching for the avionics 
 */
void armAvionics(){
  digitalWrite(ARMING_PIN, HIGH);
}

/**
 * measures the voltage on the ARMING_PIN  
 */
uint8_t checkArmingStatus(){
  arming_status = digitalRead(ARMING_PIN);
  
  if(arming_status == 1) {
    // the rocket has been armed 
    return 1;
  } else {
    // avionics not armed
    return 0;
  }
}

/**
 * Parse command received from MQTT
 */
void parseMQTTCommand(String command) {
  Serial.print("Calling parser with command: ");
  Serial.println(command);
  
  if(command == "\"ARM\"") {
    Serial.println("Rocket armed");
    digitalWrite(ARMING_PIN, HIGH);

    // check whether we have armed successfully
    if (checkArmingStatus()) {
      Serial.println("[ACK]PIN HIGH. ARMED");
    } else {
      // TODO: since we are certain that the ARM command is sent, try arming again after arming_interval
      Serial.println("[ACK]PIN LOW. FAILED ARMING");      
    }
    
  } else if (command == "\"DISARM\"") {
    Serial.println("Rocket disarmed");
    digitalWrite(ARMING_PIN, LOW);
  }

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
  client.publish(topic, "[HELLO FROM ROCKET]");
  client.subscribe(topic);

  // set up the avionics arming pin
  pinMode(ARMING_PIN, OUTPUT); 

}

/**
 * 
 * Try reconnecting to MQTT if connection is lost
 * 
 */
void reconnect() {
  while(!client.connected()) {
    Serial.println("[reconnecting]");
    
    if (client.connect("flight-computer")) {
      Serial.println("[client reconnected]");
      // subscribe
      client.subscribe(topic);
    } else {
      Serial.println("[failed connecting]"); // TODO: log to system logger
    }
  }
}

void loop() {
  // if client loses connection, try reconnecting
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
}

/**
 * this code runs every time a message arrives on the topic
 */
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  String command;

  // receive the message from MQTT
  for (int i=0; i<length; i++) {
    command += (char) payload[i]; 
  }

  // parse the received command
  parseMQTTCommand(command);
  
  Serial.println("---------------------------------------");
}
