#include <string>
#include <iostream>
#include "logger.h"

// LOG MESSAGE STRUCTURE
// [TIMESTAMP]:[CLIENT_ID]:[LOG-LEVEL]:MESSAGE
// CLIENT_ID tells which ESP - flight computer(FC) or ground-station(GS) - this is an ID xtracted from the ESP's core 
// TIMESTAMP STRUCTURE: [HOUR/MIN/SEC/DAY/MONTH/YR] 

class LoggerConsole : public Logger {
		public:
			virtual void write(const uint32_t timestamp, const std::string& client, const std::string& log_level, const std::string& msg) {
				
			// package the log message structure
			sprintf(log_buffer, "[%s]:[%s]:[%s]:[%s]\n", timestamp, client, log_level, msg);

			std::cout<< timestamp << std::endl;
			std::cout << msg << std::endl;
			std::cout << client << std::endl;
			std::cout << log_level << std:endl;
			}
}
