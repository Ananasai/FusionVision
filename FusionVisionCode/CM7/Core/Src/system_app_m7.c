/*
 * system_app_m7.c
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */
#include "system_app_m7.h"
#include "Camera_OV2640.h"
#include "LCD_ILI9486.h"
#include <cmsis_os2.h>
#include "ui_element_driver.h"

uint16_t image_buffer[480*320] = {0};

bool System_APP_M7_Start(void){
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
	osDelay(10);
	ili9486_Init();
	osDelay(10);
	ov2640_Init(0x60);
	osDelay(10);
#include "string.h"
	//memset(image_buffer, 0x00, 320*480*2);
	//HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)image_buffer, 480*320/2);
	//UI_DRIVER_DrawCharacter(0, 0, image_buffer, 1);
	#define UI_COLOUR 0xFFFF
	#define SCREEN_WIDTH 480
	#define SCREEN_HEIGHT 320

	#define FONT_WIDTH 16
	#define FONT_HEIGHT 26
#include "fonts.h"
	for(uint8_t y = 0; y < 26; y++){
		for(uint8_t i = 0; i < 16; i++){
			if(((font_16x26[26 + y] >> i) & 0x01) == 0x01){
				image_buffer[480*y+i] = UI_COLOUR;
			}
		}
	}
	osDelay(1000);
	return true;
}


bool System_APP_M7_Run(void){ //TODO: remove
	ili9486_DrawRGBImage(0, 0, 480, 320, image_buffer);
	return true;
}
