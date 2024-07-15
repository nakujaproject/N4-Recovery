/**
 * Written by: Edwin Mwiti
 * Email: emwiti658@gmail.com
 *
 * This file is responsible for defining the message log levels for post flight analysis
 * Use sparingly and accurately depending on the message being logged
 *
 */

#ifndef LEVELS_H
#define LEVELS_H

enum LOG_LEVEL {
	DEBUG = 0,
	INFO,
	WARNING,
	CRITICAL,
	ERROR
};

#endif
