/*
 * serial_commands.c
 *
 *  Created on: Dec 28, 2023
 *      Author: simon
 */

#include "serial_commands.h"
#include "debug_api.h"
#include "sync_api.h"

#define __DEBUG_FILE_NAME__ "CMD"

#define CMD(_string, _callback) {.string = (sString_t){.text = _string, .length = sizeof(_string)-1}, .callback = _callback}

static void Callback_Test(void);
static void Callback_Printout(void);

static const sSerialCommand_t serial_commands_list[] = {
	CMD("test", &Callback_Test),
	CMD("printout", &Callback_Printout),
};

const sSerialCommandTable_t serial_commands = {
	.commands = serial_commands_list,
	.length = ARRAY_SIZE(serial_commands_list)
};

static void Callback_Test(void){
	debug("Test\r\n");
}
static void Callback_Printout(void){
	Sync_API_TakeSemaphore(eSemaphorePrintout);
	Sync_API_ReleaseSemaphore(eSemaphorePrintout);
}
