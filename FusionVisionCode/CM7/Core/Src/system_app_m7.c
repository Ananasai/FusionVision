/*
 * system_app_m7.c
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */
#include "system_app_m7.h"
#include "Camera_OV2640.h"
#include "LCD_ILI9486.h"
#include "ui_element_driver.h"

uint16_t image_buffer[480*320] = {0};

bool System_APP_M7_Start(void){
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(10);
	ili9486_Init();
	HAL_Delay(10);
	ov2640_Init(0x60);
	HAL_Delay(10);
#include "string.h"
	memset(image_buffer, 0x00, 320*480*2);
	//HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)image_buffer, 480*320/2);
	HAL_Delay(1000); //Bbz kam bet nx reikia ig
	//UI_DRIVER_DrawCharacter(100, 100, image_buffer, '1');
	//UI_DRIVER_DrawCharacter(80, 100, image_buffer, '1');
	UI_DRIVER_DrawString(100, 20, image_buffer, "Labas", 5);
	HAL_Delay(1000);
	return true;
}


bool System_APP_M7_Run(void){ //TODO: remove
	ili9486_DrawRGBImage(0, 0, 480, 320, image_buffer);
	return true;
}
