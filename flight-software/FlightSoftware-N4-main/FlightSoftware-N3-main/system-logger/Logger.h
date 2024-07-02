#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include "levels.h"

class Logger {
	public:
		virtual ~Logger(void) {}
		virtual void write (const std::string&) = 0;
};

#endif
