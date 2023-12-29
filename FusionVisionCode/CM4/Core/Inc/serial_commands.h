/*
 * serial_commands.h
 *
 *  Created on: Dec 28, 2023
 *      Author: simon
 */

#ifndef INC_SERIAL_COMMANDS_H_
#define INC_SERIAL_COMMANDS_H_

#include "string_common.h"
#include <stdint.h>

typedef struct sSerialCommand_t {
	sString_t string;
	void (*callback)(void); //TODO: add payload
}sSerialCommand_t;

typedef struct sSerialCommandTable_t {
	const sSerialCommand_t *commands;
	uint32_t length;
}sSerialCommandTable_t;

extern const sSerialCommandTable_t serial_commands;

#endif /* INC_SERIAL_COMMANDS_H_ */
