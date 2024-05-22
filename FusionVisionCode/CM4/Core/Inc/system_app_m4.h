/*
 * APP_M4_Program.h
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#ifndef INC_SYSTEM_APP_M4_H_
#define INC_SYSTEM_APP_M4_H_

#include <stdbool.h>
#include "main.h"

bool System_APP_M4_Start(void);
bool System_APP_M4_Run(void);

extern UART_HandleTypeDef huart3;

#endif /* INC_SYSTEM_APP_M4_H_ */
