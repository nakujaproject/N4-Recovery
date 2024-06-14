/**
 * Implement onboard logger class functions
*/

#include "logger.h"
#include "data-types.h"


telemetry_type_t t;
char pckt_buff[50];

/**
 * 
 * @brief class constructor 
 * pass the chip select pin as a parameter for that class instance
 * 
 * @param cs_pin chip select pin
 * @param flas_led LED to show formatting status
 * @param filename the filename of the file being created
 * @param file_size the size of the file being created
*/
DataLogger::DataLogger(uint8_t cs_pin, uint8_t led_pin, char* filename, SerialFlashFile file, uint32_t filesize) {
    this->_cs_pin = cs_pin;
    this->_led_pin = led_pin;
    this->_file_size = filesize;

    strcpy(this->_filename, filename);
    this->_file = file;

}

/**
 * @brief format the flash memory
 * @param none
*/
void DataLogger::loggerFormat() {
    this->loggerEquals();
    Serial.println(F("Formatting flash memory..."));
 
    SerialFlash.eraseAll();

    // create a dummy file so that next time we start up, we know there
    // is an actual file system
    if(SerialFlash.create("dummy.txt", 100)) {
        Serial.println(F("Created dummy file "));
        SerialFlashFile file;
        file = SerialFlash.open("dummy.txt");
        file.write("Recovery team", 14);
        file.close();
    } else {
        Serial.println(F("Failed to create dummy file "));
    }   
    
    Serial.println(F("Done"));

    // while the flash is formatting, blink the LED at a frequency of 10Hz
    while(!SerialFlash.ready()) {
        digitalWrite(this->_led_pin, HIGH);
        delay(_flash_delay);
        digitalWrite(this->_led_pin, LOW);
        delay(_flash_delay);
    }

    // remain solid lit once formatting is done 
    digitalWrite(this->_led_pin, HIGH);

    this->loggerEquals();

}

/**
 * @brief Initialize the flash memory 
 * @return true on success and false on fail
 * 
*/
bool DataLogger::loggerInit() {
    char filename[20];

    if (!SerialFlash.begin(this->_cs_pin)) {
        return false;
    } else {
        this->loggerEquals(); // prettify
        this->loggerInfo();

        // init flash LED
        pinMode(this->_led_pin, OUTPUT);
        digitalWrite(this->_led_pin, HIGH);        

        // return a list of files currently in the memory
        if(!SerialFlash.exists("dummy.txt")) {
            Serial.println(F("Flash doesn't appear to hold a file system - may need erasing first.")); // TODO: Log to system logger

            // format the memory
            this->loggerFormat();

        } else {
            Serial.println(F("File system found"));
            Serial.println(F("Files currently in flash:"));
            SerialFlash.opendir();

            while (1) {
                uint32_t filesize;
                if (SerialFlash.readdir(filename, sizeof(filename), filesize)) {
                    Serial.print(filename);
                    Serial.print(F("  "));
                    Serial.print(filesize);
                    Serial.print(F(" bytes"));
                    Serial.println();
                }
                else {
                    break; // no more files
                }
            }

            uint8_t file_create_status = SerialFlash.create(this->_filename, this->_file_size);

            // create logging file with the provided filename
            if (!file_create_status) {
                Serial.println(F("Failed to create file"));
                return false;

            } else {
                // open the created file 
                Serial.println(F("Created flight bin file"));
                this->_file = SerialFlash.open(this->_filename);
            }

        }
        
        this->loggerEquals(); 

        return true;
    }
}

/**
 * @brief test the flash memory write and read function by reading and 
 * writing a variable to it
 * @param none
 * @return true if R/W OK, false otherwise
 * 
*/
bool DataLogger::loggerTest() {
    // create a string variable 
    char tst_var[15] = "FlashTesting";

    // write 

    // read 

}

/**
 * @brief write the provided data to the file created
 * @param data this is a struct pointer to the struct that contains the data that needs to 
 * be written to the memory
 * 
 * in this function, we write the data structs to the file as comma separated values
 * 
*/
void DataLogger::loggerWrite(telemetry_type_t packet){

    // Serial.print("FROM LOGGER: ");
    // Serial.println(t->alt_data.altitude);

    // write the record to the flash chip
    
    sprintf(pckt_buff, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
            packet.acc_data.ax,
            packet.acc_data.ay,
            packet.acc_data.az,
            packet.acc_data.pitch,
            packet.acc_data.roll,
            packet.alt_data.pressure);

    this->_file.write((uint8_t*)&packet, sizeof(packet));

    Serial.print( packet.record_number );
    Serial.print( "," );
    Serial.print( packet.operation_mode );
    Serial.print( "," );
    Serial.print( packet.state );
    Serial.print( "," );
    Serial.print( packet.acc_data.ax );
    Serial.print( "," );
    Serial.print( packet.acc_data.ay );
    Serial.print( "," );
    Serial.print( packet.acc_data. az );
    Serial.print( "," );
    Serial.print( packet.acc_data.pitch );
    Serial.print( "," );
    Serial.print( packet.acc_data.roll );
    Serial.print( "," );
    Serial.print( packet.gyro_data.gx );
    Serial.print( "," );
    Serial.print( packet.gyro_data.gy );
    Serial.print( "," );
    Serial.print( packet.gyro_data.gz );
    Serial.print( "," );
    Serial.print( packet.alt_data.altitude );
    Serial.print( "," );
    Serial.print( packet.alt_data.velocity );
    Serial.print( "," );
    Serial.print( packet.alt_data.pressure );
    Serial.print( "," );
    Serial.println( packet.alt_data.temperature );

    // Serial.println(F("logged"));
    
    // at this point, the flash memory is ready for writing and reading 
    // check that the passed struct is not null
    // if(data == NULL) {
    //     // do sth here 
    //     // maybe log error
    // } else {
    //     // data valid, ready to proceed
    // }

    // TODO: maybe return the size of memory written 

}

/**
 * @brief Read data from the start of the file to the end of the file 
 * 
 * @param _file_pointer pointer to where we want to start reading the file. By default, this value os 0
 * @param buffer char array to read the data into
*/
void DataLogger::loggerRead(uint8_t file_pointer, char buffer) {
    // confirm the file exists
    // seek the file to the start 


}

/**
 * @brief print the data about the flash memory
 *  
*/
void DataLogger::loggerInfo() {
    uint8_t id[5];
    Serial.println(F("Data logging system check"));
    SerialFlash.readID(id);
    Serial.println(F("Data logging system OK!"));
    Serial.print(F("Capacity: "));
    Serial.println(SerialFlash.capacity( id ));

}

/**
 * @brief helper function to print spaces for data formatting
*/
void DataLogger::loggerSpaces() {
    for(int i = 0; i < 25; i++) {
        Serial.println(F(" "));
    }
}

/**
 * @brief helper function to print = for data formatting
*/
void DataLogger::loggerEquals() {
    Serial.println();
    for(int i = 0; i < 25; i++) {
        Serial.print("=");
    }
    Serial.println();
}