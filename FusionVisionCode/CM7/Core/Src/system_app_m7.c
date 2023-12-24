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
#include "sync_api.h"
#include "shared_param_api.h"
#include "diagnostics_app.h"
#include <string.h>

#define __DEBUG_FILE_NAME__ "M7"

static uint16_t image_buffer[480*320] = {0};

static bool frame_event_flag = false;
static bool frame_half_event_flag = false;
static uint32_t line_scanned_amount = 0;

/* Executed before M4 is launched */
bool System_APP_M7_PreInit(void){
	if(Sync_API_ReleaseSemaphoreAll() == false){
		return false;
	}
	if(Shared_param_API_Init() == false){
		return false;
	}
	/* Clear image buffer */
	memset(image_buffer, 0x00, 320*480*2);
	return true;
}

bool System_APP_M7_Start(void){
	Debug_API_Start(huart3);
	UI_APP_Init(image_buffer);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(10);
	ili9486_Init();
	HAL_Delay(10);
	ov2640_Init(0x60);
	HAL_Delay(10);
	Diagnostics_APP_FrameStart();
	HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)image_buffer, 480*320/2);
	return true;
}

bool System_APP_M7_Run(void){ //TODO: remove
	/* Synchronization on new image frame received */
	if(frame_event_flag){
		frame_event_flag = false;
		//IMG_PROCESSING_APP_Compute(image_buffer);
		UI_APP_DrawAll();
		//ili9486_SetDisplayWindow(0, 0, 480, 320);
		//LCD_IO_WriteCmd8(0x2C);
		//HAL_DMA_Start(&hdma_memtomem_dma2_stream0, (uint32_t)image_buffer, LCD_ADDR_DATA, 479*319*2+22000);
		ili9486_DrawRGBImage(0, 0, 480, 320, image_buffer);
		//ili9486_DrawRGBImageInterlaced(0, 0, 480, 320, image_buffer, 0);
		Diagnostics_APP_FrameEnd();
		HAL_DCMI_Resume(&hdcmi);
	}
	if(line_scanned_amount > 3){

	}
	if(frame_half_event_flag) {
		frame_half_event_flag = false;
		uint32_t screen_state = 0;
		Shared_param_API_Read(eSharedParamScreenState, &screen_state);
		if(screen_state == eScreenStateProcessed){
			IMG_PROCESSING_APP_Compute(image_buffer);
		}
		//UI_APP_DrawAll(image_buffer);
	}
	return true;
}

/* End of frame conversion IRQ */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi){
	HAL_DCMI_Suspend(hdcmi);
	line_scanned_amount = 0;
	frame_event_flag = true;
}

//TODO: doesn't work
//void HAL_DCMI_LineEventCallback(DCMI_HandleTypeDef *hdcmi){
//	line_scanned_amount++;
//}

/* End of frame conversion IRQ */
void HAL_DCMI_HalfFrameEventCallback(void){
	frame_half_event_flag = true;
}
