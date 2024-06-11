/**
 * Implement onboard logger class functions
*/

#include "logger.h"


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
Logger::Logger(uint8_t cs_pin, uint8_t led_pin, char* filename, uint32_t filesize) {
    this->_cs_pin = cs_pin;
    this->_led_pin = led_pin;
    this->_file_size = filesize;

    strcpy(this->_filename, filename);
    
}

/**
 * @brief format the flash memory
 * @param none
*/
void Logger::loggerFormat() {
    uint8_t id[5];
    SerialFlash.readID(id);
    SerialFlash.eraseAll();

    // while the flash is formatting, blink the LED at a frequency of 10Hz
    while(!SerialFlash.ready()) {
        digitalWrite(this->_led_pin, HIGH);
        delay(_flash_delay);
        digitalWrite(this->_led_pin, LOW);
        delay(_flash_delay);
    }

    // remain solid lit once formatting is done 
    digitalWrite(this->_led_pin, HIGH);

}

/**
 * @brief Initialize the flash memory 
 * @return true on success and false on fail
 * 
*/
bool Logger::loggerInit() {


    // if (!SerialFlash.begin(this->_cs_pin)) {
    //     return false;
    // } else {
    //     // format the flash memory
    //     this->loggerFormat();

    //     // init flash LED
    //     pinMode(this->_led_pin, OUTPUT);

    //     // create logging file with the provided filename
    //     if (!SerialFlash.createErasable(this->_filename, this->_file_size)) {
    //         return false;
    //     }

    //     return true;
    // }
}

/**
 * @brief test the flash memory write and read function by reading and 
 * writing a variable to it
 * @param none
 * @return true if R/W OK, false otherwise
 * 
*/
bool Logger::loggerTest() {
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
void Logger::loggerWrite(){
    // at this point, the flash memory is ready for writing and reading 
    // check that the passed struct is not null
    // if(data == NULL) {
    //     // do sth here 
    //     // maybe log error
    // } else {
    //     // data valid, ready to proceed

    // }

    // TODO: check splitting the passed struct into single values 

    // TODO: maybe return the size of memory written 

    


}

/**
 * @brief Read data from the start of the file to the end of the file 
 * 
 * @param _file_pointer pointer to where we want to start reading the file. By default, this value os 0
 * @param buffer char array to read the data into
*/
void Logger::loggerRead(uint8_t file_pointer, char buffer) {
    // confirm the file exists
    // seek the file to the start 


}