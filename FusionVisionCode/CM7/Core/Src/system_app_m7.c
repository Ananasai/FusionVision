/*
 * system_app_m7.c
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */
#include "system_app_m7.h"
#include "Camera_OV2640.h"
#include "LCD_ILI9486.h"
#include "ui_app.h"
#include "image_processing_app.h"
#include "debug_api.h"

#define __DEBUG_FILE_NAME__ "M7"

uint16_t image_buffer[480*320] = {0};

bool frame_event_flag = false;

bool System_APP_M7_Start(void){
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(10);
	ili9486_Init();
	HAL_Delay(10);
	ov2640_Init(0x60);
	HAL_Delay(10);
#include "string.h"
	memset(image_buffer, 0x00, 320*480*2); //DCMI_MODE_CONTINUOUS
	HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)image_buffer, 480*320/2);
	//HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)image_buffer, 480*320/2);
	//HAL_Delay(1000); //Bbz kam bet nx reikia ig
	//UI_APP_DrawAll(image_buffer);
	debug("Drawn\r\n");
	//UI_APP_Printout(image_buffer);
	HAL_Delay(100);
	return true;
}

bool System_APP_M7_Run(void){ //TODO: remove
	//HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)image_buffer, 480*320/2);
	//HAL_Delay(10);
	//HAL_Delay(10);
	//HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)image_buffer, 480*320/2);
	//HAL_DCMI_Resume(&hdcmi);
	/* Synchronization on new image frame received */
	if(frame_event_flag){
		frame_event_flag = false;
		IMG_PROCESSING_APP_Compute(image_buffer);
		HAL_Delay(10);
		UI_APP_DrawAll(image_buffer);
		HAL_Delay(10);
		ili9486_DrawRGBImage(0, 0, 480, 320, image_buffer);
	}
	return true;
}

void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi){
	frame_event_flag = true;
}
