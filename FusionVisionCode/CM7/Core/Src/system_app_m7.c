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

static uint16_t image_buffer[480*320+1000] = {0};

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
	Diagnostics_APP_Start();
	Diagnostics_APP_RecordStart(eDiagEventFrame);
	Diagnostics_APP_RecordStart(eDiagEventCamera);
	HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)image_buffer, 480*320/2);
	return true;
}

bool System_APP_M7_Run(void){
	/* Synchronization on new image frame received */
	if(frame_event_flag){
		frame_event_flag = false;
		Diagnostics_APP_RecordStart(eDiagEventDisplay);
		HAL_Delay(1);
		//IMG_PROCESSING_APP_Compute(image_buffer);
		UI_APP_DrawAll();
		//ili9486_SetDisplayWindow(0, 0, 480, 320);
		//LCD_IO_WriteCmd16(0x2C);
		//NOTE: DMA LENgth ONLY 16bit
		//HAL_StatusTypeDef status0 = HAL_DMA_Start(&hdma_memtomem_dma2_stream0, (uint32_t)image_buffer, LCD_ADDR_DATA, 480*106/2);
		//HAL_StatusTypeDef status3 = HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream0, HAL_DMA_FULL_TRANSFER, 100);
		ili9486_DrawRGBImage(0, 0, 480, 320, image_buffer);
		//ili9486_DrawRGBImageInterlaced(0, 0, 480, 300, image_buffer, 0);
		//SCB_CleanDCache_by_Addr((uint32_t*)LCD_ADDR_DATA, 480*320);
		//HAL_StatusTypeDef status = HAL_DMA2D_Start_IT(&hdma2d, (uint32_t)image_buffer, (uint32_t)LCD_ADDR_DATA, 100, 100);
		//HAL_StatusTypeDef status2 = HAL_DMA2D_PollForTransfer(&hdma2d, 1000);
		Diagnostics_APP_RecordEnd(eDiagEventDisplay);
		Diagnostics_APP_RecordEnd(eDiagEventFrame);
		//static uint8_t line_start = 0;
		//line_start = 1 ? line_start == 0 : 1;
		//
		Diagnostics_APP_RecordStart(eDiagEventFrame);
		Diagnostics_APP_RecordStart(eDiagEventCamera);
		HAL_DCMI_Resume(&hdcmi);
	}
	if(line_scanned_amount > 3){

	}
	if(frame_half_event_flag) {
		frame_half_event_flag = false;
		uint32_t screen_state = 0;
		Shared_param_API_Read(eSharedParamScreenState, &screen_state);
		if(screen_state == eScreenStateProcessed){
			Diagnostics_APP_RecordStart(eDiagEventProcessing);
			IMG_PROCESSING_APP_Compute(image_buffer);
			//HAL_Delay(1);
			Diagnostics_APP_RecordEnd(eDiagEventProcessing);
		}
		//UI_APP_DrawAll(image_buffer);
	}
	return true;
}

/* End of frame conversion IRQ */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi){
	HAL_DCMI_Suspend(hdcmi);
	Diagnostics_APP_RecordEnd(eDiagEventCamera);
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

void HAL_DMA2D_CLUTLoadingCpltCallback(DMA2D_HandleTypeDef *hdma2d){

}
