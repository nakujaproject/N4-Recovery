
char tstamp[50]; // to hold a timestamp
int minute=0, sec=0, msec=0;

void getTimeStamp(unsigned long m) {
  // convert time to mins, secs, msecs
  
  minute = ((m/1000)/60) % 60;
  sec = (m/1000) % 60;
  msec = m%1000;

  sprintf(tstamp, "%d:%d:%ul", minute, sec, msec);

}

void setup() {
  Serial.begin(115200);

}

void loop() {

  unsigned long x = millis();
  getTimeStamp(x);

  Serial.println(tstamp);

}
