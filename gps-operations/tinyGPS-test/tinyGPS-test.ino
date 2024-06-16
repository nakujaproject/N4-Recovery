#include <TinyGPSPlus.h>

void GPSInit();
void dumpGPSData();

#define GPS_WAIT_TIME 2000 // wait for 2 mins to acquire GPS lock
uint8_t lock_acquired = 0;
unsigned long previous_time=0, current_time = 0;
uint16_t interval = 250; // GPS sample time

TinyGPSPlus gps;
uint8_t newdata = 0;

typedef struct {
  double latitude;
  double longitude;
  uint32_t t; 
} gps_type_t;

gps_type_t gps_data;

void GPSInit() {
  Serial.println("[+] Initializing GPS");
  Serial2.begin(9600); // TODO: MOVE TO DEFS
  delay(100); // wait for serial to init
}

void dumpGPSData() {
  double lt, lon;
  float flat, flon;
  unsigned long ag, dt, tm, chars;
  int yr;
  byte mon, dy, hr, minut, sec, centisec;
  unsigned short sentences, failed;
  unsigned int alt;

  // get location( lat and long)
  if(gps.location.isValid()) {
    Serial.println("Location:");
    lt = gps.location.lat();
    lon = gps.location.lng();
    Serial.print("Lat/long: ");
    Serial.print(lt);
    Serial.print(", ");
    Serial.println(lon);
    
  } else {
    Serial.println("INVALID LOCATION"); // TODO: LOG TO SYSTEM LOGGER
  }

  // get time 
  if(gps.time.isValid()) {
    Serial.print("Time: ");
    hr = gps.time.hour();
    minut = gps.time.minute();
    sec = gps.time.second();
    centisec = gps.time.centisecond();

    Serial.print(hr);Serial.print(":");Serial.print(minut);Serial.print(":");
    Serial.print(sec); Serial.print(":"); Serial.println(centisec);
    
  } else {
    Serial.println("INVALID TIME");
  }

  // get date 
  if(gps.date.isValid()) {
    Serial.print("Date: ");
    dy = gps.date.day();
    mon = gps.date.month();
    yr = gps.date.year();

    Serial.print(dy); Serial.print("/");Serial.print(mon);Serial.print("/"); Serial.println(yr); 
       
  } else {
    Serial.println("INVALID DATE"); // TODO: LOG TO SYSTEM LOGGER
  }

  // get motion data
  if(gps.altitude.isValid()) {
    Serial.print("Altitude(m): ");
    alt = gps.altitude.meters();

    Serial.println(alt);
  }  else {
    Serial.println("Invalid altitude data");
  }
  
}

void setup() {
  Serial.begin(115200);
  GPSInit();
  
}

void loop() {
  current_time = millis();
  if(current_time - previous_time > interval) {
    previous_time = current_time;

    if(Serial2.available()) {
      
      char c = Serial2.read();
//      Serial.print(c);

      if(gps.encode(c)) {
        lock_acquired = 1; // GPS lock acquired
        newdata = 1; // new data acquired
      }
          
    }  else { 
      // TODO: implement timeout for acquiring GPS signaal
      Serial.println("Waiting for GPS");
      
    }

    // check for GPS lock 
    Serial.print("LOCK: "); Serial.println(lock_acquired);
    Serial.print("NEWDATA: "); Serial.println(newdata);
    
    if(newdata) { // TODO: check for GPS_LOCK
      Serial.println("Acquired data");
      Serial.println("---------------");
      dumpGPSData();
      Serial.println("---------------");
      Serial.println();
    }

  }
}
