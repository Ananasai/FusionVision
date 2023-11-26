/*
 * system_app_m7.c
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */
#include "system_app_m7.h"
#include "ili9486.h"

bool System_APP_M7_Start(void){
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
	ili9486_Init();
	ili9486_FillRect(0, 0, 480, 320, 0x5790);
	return true;
}
