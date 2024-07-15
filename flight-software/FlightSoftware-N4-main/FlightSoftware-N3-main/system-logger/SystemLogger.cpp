#include "SystemLogger.h"

/**
 * 
 * @brief convert the log level to string
 * 
 */
const char* SystemLogger::getLogLevelString(uint8_t log_level) {
    static const char* debug = "DEBUG";
    static const char* info = "INFO";
    static const char* warning = "WARNING";
    static const char* critical = "CRITICAL";
    static const char* error = "ERROR";
    static const char* unknown = "UNKNOWN";

    switch (log_level) {
    case 0:
        return debug;
        break;

    case 1:
        return info;
        break;

    case 2:
        return warning;
        break;

    case 3:
        return critical;
        break;
    
    case 4:
        return error;
        break;
    
    default:
        return unknown;
        break;
    }

}



