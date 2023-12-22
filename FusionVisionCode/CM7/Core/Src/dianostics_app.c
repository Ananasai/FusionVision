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
static float avg_frame_time = 0;

void Diagnostics_APP_FrameStart(void){
	TIM1->CNT = 0;
	HAL_TIM_Base_Start(&htim1);
}

void Diagnostics_APP_FrameEnd(void){
	avg_frame_time += TIM1->CNT;
	TIM1->CNT = 0;
	curr_frame++;
	if(curr_frame == MAX_FRAMES){
		avg_frame_time = avg_frame_time / MAX_FRAMES * TICK_TIME / 1000.0f;
		debug("Avg frame: %.3fms\r\n", avg_frame_time);
		curr_frame = 0;
		avg_frame_time = 0;
	}
}

