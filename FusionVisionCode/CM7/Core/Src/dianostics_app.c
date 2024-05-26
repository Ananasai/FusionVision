#include "diagnostics_app.h"
#include "debug_api.h"

#define __DEBUG_FILE_NAME__ "DIAG"
#define MAX_FRAMES 30
#define TICK_TIME 10 /* 1 tick - 10us */

static uint8_t curr_frame = 0;

/*
 * List of all tracked event print formats.
 */
static const char *diag_event_name_format_lut[eDiagEventLast] = {
	[eDiagEventFrame] = "Whole frame: %.3fms\r\n",
	[eDiagEventCamera] = "Camera(DCMI): %.3fms\r\n",
	[eDiagEventProcessing] = "Processing: %.3fms\r\n",
	[eDiagEventDisplay] = "Display(FSMC): %.3fms\r\n",
};

/*
 * List to store all event times start.
 */
static uint32_t diag_event_start[eDiagEventLast] = {0};
/*
 * List to store all event times end.
 */
static float diag_event_times[eDiagEventLast] = {0};

/*
 * Start function. Initialises all times to 0 and starts time tracking timer.
 */
void Diagnostics_APP_Start(void){
	HAL_TIM_Base_Stop(&htim2);
	for(eDiagEvent_t evt = eDiagEventFirst; evt < eDiagEventLast; evt++){
		diag_event_times[evt] = 0;
	}
	TIM2->CNT = 0;
	HAL_TIM_Base_Start(&htim2);
}

/*
 * Event started - record time.
 */
void Diagnostics_APP_RecordStart(eDiagEvent_t event) {
	if(event == eDiagEventFrame){
		TIM2->CNT = 0;
	}
	diag_event_start[event] = TIM2->CNT;
}

/*
 * Event ended - record time and printout if last one.
 */
void Diagnostics_APP_RecordEnd(eDiagEvent_t event) {
	diag_event_times[event] += TIM2->CNT - diag_event_start[event];
	/* If last event - increase frame counter and printout info */
	if(event == eDiagEventFrame){
		curr_frame++;
		/* Printout information every MAX_FRAMES amount of individual frames */
		if(curr_frame == MAX_FRAMES){
			debug("Diag times:\r\n");
			/* Printout all event averaged times */
			for(eDiagEvent_t evt = eDiagEventFirst; evt < eDiagEventLast; evt++){
				float averaged_time = diag_event_times[evt] / MAX_FRAMES * TICK_TIME / 1000.0f;
				debug((char *)diag_event_name_format_lut[evt], averaged_time);
				diag_event_times[evt] = 0;
			}
			/* Reset frame and time counter */
			curr_frame = 0;
			TIM2->CNT = 0;
		}
	}
}
