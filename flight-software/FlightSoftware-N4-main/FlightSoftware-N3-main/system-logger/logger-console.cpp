#include <string>
#include <iostream>
#include "SystemLogger.h"
#include "SystemLogLevels.h"

// DRIVER CODE FOR COSOLE TESTING

// LOG MESSAGE STRUCTURE
// [TIMESTAMP]:[CLIENT_ID]:[LOG-LEVEL]:MESSAGE
// CLIENT_ID tells which ESP - flight computer(FC) or ground-station(GS) - this is an ID xtracted from the ESP's core 
// TIMESTAMP STRUCTURE: [HOUR/MIN/SEC/DAY/MONTH/YR] 

class LoggerConsole : public SystemLogger {
		public:
			virtual void write(const uint32_t timestamp, const char* client, uint8_t log_level, const char* msg) {
				char log_buffer[128];

				// get verbose log levels
				char* log_level_str = getLogLevelString(log_level);

				// package the log message structure
				sprintf(log_buffer, "[%s]:[%s]:[%s]:%s\n", timestamp, client, log_level_str, msg);

				// print to console
				std::cout << log_buffer;
				
		}
};

int main() {
	LoggerConsole syslogger;
	
	// fake message - timestamp, client_id, log_level, message
	uint32_t timestamp = 19400;
	const char* client = "123EDFE";
	uint8_t log_level = LOG_LEVEL::DEBUG;
	const char* msg = "Testing 1..2";
	
	// call the logger
	syslogger.write(timestamp, client, log_level, msg);

	return 0;
}
