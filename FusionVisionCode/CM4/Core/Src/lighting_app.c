/*
 * lighting_app.c
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#include "lighting_app.h"
#include "debug_api.h"
#include "job_api.h"
#include <main.h>
#include <cmsis_os2.h>

#define __DEBUG_FILE_NAME__ "LIGHT"

#define R2 25000
#define VIN 5

static void Lighting_app_thread(void *argument);
static void Lighting_job_recAdc(void *payload);

static const osThreadAttr_t lighting_thread_attribute = {
	.name = "lighting app",
	.priority = osPriorityNormal
};

static const sJobCallbackDesc_t job_callbacks[] = {
		{.callback = Lighting_job_recAdc, .job = eJobRecAdc},
};

static osThreadId_t lighting_thread_id = NULL;


bool Lighting_APP_Start(void){
	lighting_thread_id = osThreadNew(&Lighting_app_thread, NULL, &lighting_thread_attribute);
	if(lighting_thread_id == NULL){
		error("Starting lighting thread\r\n");
		return false;
	}
	if(Job_API_CreateQueue(eQueueLighting) == false){
		return false;
	}
	return true;
}
//TODO: add functionality and software PWM
static void Lighting_app_thread(void *argument){
	/* Wait for ADC end conversion FLAG */
	while(true){
		Job_API_WaitNew(eQueueLighting, job_callbacks, 1);
	}
}

/* Formula for photo resistor resistance R1 = (Vin * R2) / Vout - R2*/
static void Lighting_job_recAdc(void *payload){
	float *adc_val_v = (float *)payload;
	float R1 = (float)(VIN * R2) / (float)(*adc_va_v) - R2;
	debug("Photo resistor: %.2f\r\n", *R1);
}
