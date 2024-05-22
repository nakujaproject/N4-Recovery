/**
This cprogram visualizes the orientation data received from IMU onboard the 
flight computer 
**/

import processing.serial.*;
import java.awt.event.KeyEvent;
import java.io.IOException;

Serial myPort;

String data = "";
float roll, pitch;

void setup() {
  size(960, 640, P3D);
  myPort = new Serial(this, "COM10", 115200);
  myPort.bufferUntil('\n');
}

void draw() {
  translate(width/2, height/2, 0);
  background(40);
  textSize(20);
  text("Roll:"+int(roll) + " Pitch:"+int(pitch), -100, 265);
  
  // rotate the object
  rotateX(radians(roll));
  rotateZ(radians(-pitch));
  
  // 3D object 
  textSize(22);
  fill(0, 76, 153);
  box(386, 40, 200); // draw box
  textSize(25);
  fill(255, 255, 255);
  text("Flight Comp", -183, 10, 101);
  
    
}

// read data from the serial port
void serialEvent(Serial myPort) {
  
  data =  myPort.readStringUntil('\n');
  println(data);
  
  
  if(data != null) {
    data = trim(data);
    
    // split the string at ,','
    String items[] = split(data, ',');
    if(items.length > 1) {
      
      // roll and pitch in degrees
      pitch = float(items[0]);
      roll = float(items[1]);
      
    }
  }
  
}
