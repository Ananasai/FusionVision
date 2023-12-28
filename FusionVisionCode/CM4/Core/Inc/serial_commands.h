/*
 * serial_commands.h
 *
 *  Created on: Dec 28, 2023
 *      Author: simon
 */

#ifndef INC_SERIAL_COMMANDS_H_
#define INC_SERIAL_COMMANDS_H_

#include "string_common.h"

typedef struct sSerialCommand_t {
	sString_t string;
	void (*callback)(void); //TODO: add payload
}sSerialCommand_t;

extern const sSerialCommand_t serial_commands[];

#endif /* INC_SERIAL_COMMANDS_H_ */
