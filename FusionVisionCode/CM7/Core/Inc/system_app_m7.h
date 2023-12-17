/*
 * system_app_m7.h
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#ifndef INC_SYSTEM_APP_M7_H_
#define INC_SYSTEM_APP_M7_H_

#include <stdbool.h>
#include "main.h"
#include "stm32h7xx_hal_dcmi.h"

extern RTC_HandleTypeDef hrtc;
extern DCMI_HandleTypeDef hdcmi;

bool System_APP_M7_Start(void);
bool System_APP_M7_Run(void);

#endif /* INC_SYSTEM_APP_M7_H_ */
