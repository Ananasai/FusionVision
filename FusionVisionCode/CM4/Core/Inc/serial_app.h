/*
 * serial_app.h
 *
 *  Created on: Dec 28, 2023
 *      Author: simon
 */

#ifndef INC_SERIAL_APP_H_
#define INC_SERIAL_APP_H_

#include <stdbool.h>
#include "main.h"

extern UART_HandleTypeDef huart3;

bool Serial_APP_Start(void);

#endif /* INC_SERIAL_APP_H_ */
