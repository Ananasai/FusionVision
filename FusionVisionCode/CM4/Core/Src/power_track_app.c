///*
// * power_track_app.c
// *
// *  Created on: Dec 26, 2023
// *      Author: simon
// */
//
//#include "power_track_app.h"
//#include "debug_api.h"
//#include "job_api.h"
//#include <main.h>
//
//#define __DEBUG_FILE_NAME__ "PWR"
//
//#define G 56.0f
//#define R3 100.0f
//#define R4 5600.0f
//#define RSHUNT 0.1f
//
//#define INFO_INTERVAL 10000
//
//const static osThreadAttr_t power_thread_attribute = {
//	.name = "power track app",
//	.priority = osPriorityNormal
//};
//
//static osThreadId_t power_thread_id = NULL;
//static float avg_current_ma = 0;
//static uint32_t adc_index = 0;
//
//static void Power_track_app_thread(void *argument);
//static void Power_track_job_recAdc(void *payload);
//
//static const sJobCallbackDesc_t job_callbacks[] = {
//		{.callback = Power_track_job_recAdc, .job = eJobRecAdc},
//};
//
//bool Power_track_APP_Start(void){
//	power_thread_id = osThreadNew(&Power_track_app_thread, NULL, &power_thread_attribute);
//	if(power_thread_id == NULL){
//		error("Starting power track thread\r\n");
//		return false;
//	}
//	if(Job_API_CreateQueue(eQueuePowerTrack) == false){
//		return false;
//	}
//	return true;
//}
//
//static void Power_track_app_thread(void *argument){
//	while(true){
//		Job_API_WaitNew(eQueuePowerTrack, job_callbacks, 1);
//	}
//}
//
//static void Power_track_job_recAdc(void *payload){
//	float *adc_val_v = (float *)payload;
//	/* Adjusting for OP-AMP input voltage offset */
//	float adjusted_adc_val = (G+1)*40.0f/1000000.0f+(*adc_val_v)/2.0f; //TODO: why 2?
//	float current = adjusted_adc_val*R3/R4/RSHUNT*1000.0f;
//	avg_current_ma += current;
//	adc_index++;
//	if(adc_index == INFO_INTERVAL){
//		avg_current_ma /= INFO_INTERVAL;
//		debug("Avg current: %.2fma\r\n", avg_current_ma);
//		avg_current_ma = 0;
//		adc_index = 0;
//	}
//}
