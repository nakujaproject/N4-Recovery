#include <TinyGPSPlus.h>

unsigned long previous_time, current_time = 0;
uint16_t interval = 50; // GPS sample time

TinyGPSPlus gps;

typedef struct {
  double latitude;
  double longitude;
  uint32_t t; 
} gps_type_t;

gps_type_t gps_data;

void readGPS() {
  gps_data.latitude = gps.location.lat();
  gps_data.longitude = gps.location.longitude();
  gps_data.t = gps.time.value();
}

void setup() {
  Serial.begin(115200);
  
}

void loop() {
  current_time = millis();
  if(current_time - previous_time > interval) {
    previous_time = current_time;
    readGPS();

    // print the results 
    Serial.print(F("LAT: ")); Serial.print(gps_data.latitude); 
    Serial.print(F("LONG: ")); Serial.print(gps_data.longitude); 
    Serial.print(F("TIME: ")); Serial.print(gps_data.t);
  }  

}
