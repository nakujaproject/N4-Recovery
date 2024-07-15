#ifndef SYSTEMLOGGER_H
#define SYSTEMLOGGER_H

#include <cstdint>


class SystemLogger {
	public:
		SystemLogger();
		virtual ~SystemLogger(void);
		virtual void write (const uint32_t timestamp, const char* client, uint8_t log_level, const char* msg);
		char* getLogLevelString(uint8_t log_level);
};

#endif
