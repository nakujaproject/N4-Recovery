#include <string>
#include <iostream>
#include "SystemLogger.h"
#include "SystemLogLevels.h"

// DRIVER CODE FOR CONSOLE TESTING

// LOG MESSAGE STRUCTURE
// [TIMESTAMP]:[CLIENT_ID]:[LOG-LEVEL]:MESSAGE
// CLIENT_ID tells which ESP - flight computer(FC) or ground-station(GS) - this is an ID xtracted from the ESP's core 
// TIMESTAMP STRUCTURE: [HOUR/MIN/SEC/DAY/MONTH/YR] 

class LoggerConsole : public SystemLogger {
		public:
			void writeToConsole(const time_t timestamp, const char* client, uint8_t log_level, const char* msg) {
				char log_buffer[128];

				// get verbose log levels - returns int converted to string to tell the log level
				const char* log_level_str = getLogLevelString(log_level);

				// package the log message structure
				// int chars = sprintf(log_buffer, "[%s]:[%s]:[%s]:%s\n", timestamp, client, log_level_str, msg);
				sprintf(log_buffer, "[%s]:[%s]:[%s]",client, log_level_str, msg);

				// print to console
				std::cout << log_buffer;
				
		}
};

int main() {
	LoggerConsole syslogger;
	
	// fake message - timestamp, client_id, log_level, message
	time_t timestamp = 19400;
	const char* client = "123EDFE";
	uint8_t log_level = LOG_LEVEL::DEBUG;
	const char* msg = "Testing 1..2";

	std::cout<<"<Done logging>\n";
	
	// call the logger
	syslogger.writeToConsole(timestamp, client, log_level, msg);

	

	return 0;
}
