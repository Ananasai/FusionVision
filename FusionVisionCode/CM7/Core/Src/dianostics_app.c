/*
 * dianostics_app.c
 *
 *  Created on: Dec 22, 2023
 *      Author: simon
 */

#include "diagnostics_app.h"
#include "debug_api.h"

#define __DEBUG_FILE_NAME__ "DIAG"
#define MAX_FRAMES 30
#define TICK_TIME 10 /* 1 tick - 10us */

static uint8_t curr_frame = 0;

static const char *diag_event_name_format_lut[eDiagEventLast] = {
	[eDiagEventFrame] = "Whole frame: %.3fms\r\n",
	[eDiagEventCamera] = "Camera(DCMI): %.3fms\r\n",
	[eDiagEventProcessing] = "Processing: %.3fms\r\n",
	[eDiagEventDisplay] = "Display(FSMC): %.3fms\r\n",
};

static uint32_t diag_event_start[eDiagEventLast] = {0};
static float diag_event_times[eDiagEventLast] = {0};

void Diagnostics_APP_Start(void){
	HAL_TIM_Base_Stop(&htim1);
	for(eDiagEvent_t evt = eDiagEventFirst; evt < eDiagEventLast; evt++){
		diag_event_times[evt] = 0;
	}
	TIM1->CNT = 0;
	HAL_TIM_Base_Start(&htim1);
}

void Diagnostics_APP_RecordStart(eDiagEvent_t event) {
	if(event == eDiagEventFrame){
		TIM1->CNT = 0;
	}
	diag_event_start[event] = TIM1->CNT;
}

void Diagnostics_APP_RecordEnd(eDiagEvent_t event) {
	diag_event_times[event] = TIM1->CNT - diag_event_start[event];
	/* If last event - increase frame counter and printout info */
	if(event == eDiagEventFrame){
		curr_frame++;
		if(curr_frame == MAX_FRAMES){
			debug("Diag times:\r\n"); //TODO: all times incorrect, need to make separate event for it
			for(eDiagEvent_t evt = eDiagEventFirst; evt < eDiagEventLast; evt++){
				float new_time = diag_event_times[evt] / MAX_FRAMES * TICK_TIME / 1000.0f;
				debug((char *)diag_event_name_format_lut[evt], new_time);
			}
			curr_frame = 0;
			TIM1->CNT = 0;
		}
	}
}
//TODO: might not work as differencyt interrupt is trigered
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  // Check which version of the timer triggered this callback and toggle LED
  if (htim == &htim1)
  {
	  error("TIMER OVERFLOW\r\n");
  }
}

