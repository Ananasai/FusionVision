/*
 * lighting_app.c
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#include "lighting_app.h"
#include "debug_api.h"
#include <main.h>
#include <cmsis_os2.h>

#define __DEBUG_FILE_NAME__ "LGHT_APP"
#define LIGHTING_TIMER_PERIOD 1000
#define LIGHTING_EVENT_FLAGS_ADC_READY 0x01

extern ADC_HandleTypeDef hadc1;

const static osTimerAttr_t lighting_timer_attribute = {
		.name = "lighting timer"
};
const static osThreadAttr_t lughting_thread_attribute = {
		.name = "lighting app",
		.priority = osPriorityNormal
};

static osTimerId_t lighting_timer_id = NULL;
static osThreadId_t lighting_thread_id = NULL;
static osEventFlagsId_t lighting_event_flags_id = NULL;
static uint16_t curr_adc_value = 0x00;

static void Lighting_app_timer(void *argument);
static void Lighting_app_thread(void *argument);

bool Lighting_APP_Start(void){
	lighting_event_flags_id = osEventFlagsNew(NULL);
	if(lighting_event_flags_id == NULL){
		error("Creating lighting event flags\r\n");
		return false;
	}
	lighting_timer_id = osTimerNew(&Lighting_app_timer, osTimerPeriodic, NULL, &lighting_timer_attribute);
	if(lighting_thread_id == NULL){
		error("Creating lighting timer\r\n");
		return false;
	}
	if(osTimerStart(lighting_timer_id, LIGHTING_TIMER_PERIOD) != osOK){
		error("Starting lighting timer\r\n");
		return false;
	}
	lighting_thread_id = osThreadNew(&Lighting_app_thread, NULL, &lughting_thread_attribute);
	if(lighting_thread_id == NULL){
		error("Starting lighting thread\r\n");
		return false;
	}
	return true;
}


static void Lighting_app_timer(void *argument){
	/* Do ADC start here*/
	HAL_ADC_Start_IT (&hadc1);

}

static void Lighting_app_thread(void *argument){
	uint32_t current_flag = 0x00;
	/* Wait for ADC end conversion FLAG */
	while(true){
		current_flag = osEventFlagsWait(lighting_event_flags_id, LIGHTING_EVENT_FLAGS_ADC_READY, osFlagsWaitAny, osWaitForever);
		switch(current_flag){
			case LIGHTING_EVENT_FLAGS_ADC_READY: {
				/* ADC conversion here */
				break;
			}
			default: {
				error("Invalid flag\r\n");
				break;
			}
		}
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){ //TODO: NOW WORKS ONLY WITH ONE ADC, MAKE INTO SEPERATRE FILE
	curr_adc_value = HAL_ADC_GetValue(&hadc1);
	osEventFlagsSet(lighting_event_flags_id, LIGHTING_EVENT_FLAGS_ADC_READY);
}
