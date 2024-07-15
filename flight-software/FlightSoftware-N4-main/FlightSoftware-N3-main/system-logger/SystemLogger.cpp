#include "SystemLogger.h"

/**
 * 
 * @brief convert the log level to string
 * 
 */
char* SystemLogger::getLogLevelString(uint8_t log_level) {
    static char* debug = "DEBUG";
    static char* info = "INFO";
    static char* warning = "WARNING";
    static char* critical = "CRITICAL";
    static char* error = "ERROR";
    static char* unknown = "UNKNOWN";

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



