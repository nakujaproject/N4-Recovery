#include <Arduino.h>
#include <SFE_BMP180.h>
#include <Wire.h>

// create BMP object 
SFE_BMP180 altimeter;

#define ALTITUDE 1525.0 // altitude of iPIC building, JKUAT, Juja.


/**
 * Initialize BMP
*/
void BMPInit() {
    if(altimeter.begin()) {
        Serial.println("BMP init success");
        // TODO: update system table
    } else {
        Serial.println("BMP init failed");
    }

}

void setup() {
    Serial.begin(115200);
    delay(100);

    BMPInit();
    
}

void loop() {
    char status;
    double T, P, p0, a;

    // If you want sea-level-compensated pressure, as used in weather reports,
    // you will need to know the altitude at which your measurements are taken.
    // We're using a constant called ALTITUDE in this sketch:
    Serial.println();
    Serial.print("provided altitude: ");
    Serial.print(ALTITUDE, 0);
    Serial.print(" meters, ");

    // If you want to measure altitude, and not pressure, you will instead need
    // to provide a known baseline pressure. This is shown at the end of the sketch.

    // You must first get a temperature measurement to perform a pressure reading.
    
    // Start a temperature measurement:
    // If request is successful, the number of ms to wait is returned.
    // If request is unsuccessful, 0 is returned.

    status = altimeter.startTemperature();
    if(status !=0 ) {
        // wait for measurement to complete
        delay(status);

        // retrieve the completed temperature measurement 
        // temperature is stored in variable T

        status = altimeter.getTemperature(T);
        if(status != 0) {
            // print out the measurement 
            Serial.print("temperature: ");
            Serial.print(T, 2);
            Serial.print(" \xB0 C, ");

            // start pressure measurement 
            // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
            // If request is successful, the number of ms to wait is returned.
            // If request is unsuccessful, 0 is returned.
            status = altimeter.startPressure(3);
            if(status != 0) {
                // wait for the measurement to complete
                delay(status);

                // Retrieve the completed pressure measurement:
                // Note that the measurement is stored in the variable P.
                // Note also that the function requires the previous temperature measurement (T).
                // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
                // Function returns 1 if successful, 0 if failure.

                status = altimeter.getPressure(P, T);
                if(status != 0) {
                    // print out the measurement
                    Serial.print("absolute pressure: ");
                    Serial.print(P, 2);
                    Serial.print(" mb, "); // in millibars

                    p0 = altimeter.sealevel(P,ALTITUDE);
                    // If you want to determine your altitude from the pressure reading,
                    // use the altitude function along with a baseline pressure (sea-level or other).
                    // Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
                    // Result: a = altitude in m.

                    a = altimeter.altitude(P, p0);
                    Serial.print("computed altitude: ");
                    Serial.print(a, 0);
                    Serial.print(" meters, ");

                } else {
                    Serial.println("error retrieving pressure measurement\n");
                } 
            
            } else {
                Serial.println("error starting pressure measurement\n");
            }

        } else {
            Serial.println("error retrieving temperature measurement\n");
        }

    } else {
        Serial.println("error starting temperature measurement\n");
    }

    delay(2000);

}