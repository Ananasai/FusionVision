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
#include "fonts.h"
#include <string.h>

#define __DEBUG_FILE_NAME__ "M7"

static void Printout_IRQ(void);
static bool Printout(void);

static uint16_t image_buffer[480*320] = {0};

static bool frame_event_flag = false;
static bool frame_line_event_flag = false;
static bool printout_flag = false;

static uint32_t line_scanned_amount = 0;
static bool first_vsync = true;

static uint32_t edge_algorithm = eEdgeAlgorithmSobel;

static const uint32_t algorithm_line_num[eEdgeAlgorithmLast] = {
		[eEdgeAlgorithmSobel] = 50,
		[eEdgeAlgorithmRoberts] = 160
};

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
	IMG_PROCESSING_APP_Init(image_buffer);
	Sync_API_ActivateSemaphoreIrq(eSemaphorePrintout, Printout_IRQ);
	/* Init screen */
	ili9486_Init();
	/* Display splash screen */
	ili9486_DrawRGBImage(0, 0, 480, 320, (uint16_t *)splash_screen);
	//HAL_Delay(10);
	/* Init camera */
	ov2640_Init(0x60);
	/* Start time tracking */
	Diagnostics_APP_Start();
	Diagnostics_APP_RecordStart(eDiagEventFrame);
	Diagnostics_APP_RecordStart(eDiagEventCamera);
	/* Start camera conversion */
	first_vsync = true;
	HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)image_buffer, 480*320/2);
	return true;
}

bool System_APP_M7_Run(void){
	/* Event on full frame received from DCMI  */
	if(frame_event_flag){
		frame_event_flag = false;
		Diagnostics_APP_RecordStart(eDiagEventDisplay);
		UI_APP_DrawAll();
		//ili9486_SetDisplayWindow(0, 0, 480, 320);
		//LCD_IO_WriteCmd16(0x2C);
		//NOTE: DMA LENgth ONLY 16bit
		//HAL_StatusTypeDef status0 = HAL_DMA_Start(&hdma_memtomem_dma2_stream0, (uint32_t)image_buffer, LCD_ADDR_DATA, 480*106/2);
		//HAL_StatusTypeDef status3 = HAL_DMA_PollForTransfer(&hdma_memtomem_dma2_stream0, HAL_DMA_FULL_TRANSFER, 100);
		ili9486_DrawRGBImage(0, 0, 480, 320, image_buffer);
		//ili9486_DrawRGBImageInterlaced(0, 0, 480, 320, image_buffer, 0);
		//SCB_CleanDCache_by_Addr((uint32_t*)LCD_ADDR_DATA, 480*320);
		//HAL_StatusTypeDef status = HAL_DMA2D_Start_IT(&hdma2d, (uint32_t)image_buffer, (uint32_t)LCD_ADDR_DATA, 100, 100);
		//HAL_StatusTypeDef status2 = HAL_DMA2D_PollForTransfer(&hdma2d, 1000);
		Diagnostics_APP_RecordEnd(eDiagEventDisplay);
		Diagnostics_APP_RecordEnd(eDiagEventFrame);
		/* Printout whole frame if callback received */
		if(printout_flag){
			Printout();
			printout_flag = false;
		}
		Shared_param_API_Read(eSharedParamEdgeAlgorithm, &edge_algorithm);
		Diagnostics_APP_RecordStart(eDiagEventFrame);
		Diagnostics_APP_RecordStart(eDiagEventCamera);
		/* Start recording next frame */
		line_scanned_amount = 0;
		first_vsync = true;

		HAL_DCMI_Resume(&hdcmi);
	}
	/* Event after some lines have been received from DCMI */
	if(frame_line_event_flag) {
		uint32_t screen_state = 0;
		Shared_param_API_Read(eSharedParamScreenState, &screen_state);
		if(screen_state == eScreenStateProcessed){
			Diagnostics_APP_RecordStart(eDiagEventProcessing);
			IMG_PROCESSING_APP_Compute(image_buffer);
			Diagnostics_APP_RecordEnd(eDiagEventProcessing);
		}
		frame_line_event_flag = false;
	}
	return true;
}

static void Printout_IRQ(void){
	printout_flag = true;
}

static bool Printout(void){
	for(uint16_t y = 0; y < 320; y++){
		for(uint16_t x = 0; x < 480; x++){
			DEBUG_API_LOG("%d,", NULL, NULL, *(image_buffer + x + y * 480));
			if(x % 120 == 0){
				DEBUG_API_LOG("\r\n", NULL, NULL);
			}
		}
		DEBUG_API_LOG("\r\n", NULL, NULL);
	}
	return true;
}

/* End of full frame conversion IRQ */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi){
	HAL_DCMI_Suspend(hdcmi);
	Diagnostics_APP_RecordEnd(eDiagEventCamera);
	frame_event_flag = true;

}
/* Used to reset line amount after first vsync - because lines keep getting caluclated
 * even when dcmi is suspended and after wakeup it is waiting for new frame start - vsync
 * to capture data */
void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi){
	if(first_vsync){
		line_scanned_amount = 0;
		first_vsync = false;
	}
}
/* Line end event, used for faster response than half transfer */
//TODO: big improvement if line repsonse set at 3, find out why
void HAL_DCMI_LineEventCallback(DCMI_HandleTypeDef *hdcmi){
	line_scanned_amount++;
	if(first_vsync == false){
		if(line_scanned_amount == algorithm_line_num[edge_algorithm]){
			frame_line_event_flag = true;
		}
	}
}

/* End of frame conversion IRQ */
//void HAL_DCMI_HalfFrameEventCallback(void){
	//frame_half_event_flag = true;
//}
