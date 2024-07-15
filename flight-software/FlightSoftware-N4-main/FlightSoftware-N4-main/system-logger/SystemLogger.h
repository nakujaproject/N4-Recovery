#ifndef SYSTEMLOGGER_H
#define SYSTEMLOGGER_H

#include <cstdint>


class SystemLogger {
	public:
		void writeToConsole (const uint32_t timestamp, const char* client, uint8_t log_level, const char* msg);
		const char* getLogLevelString(uint8_t log_level);
};
	
#endif
