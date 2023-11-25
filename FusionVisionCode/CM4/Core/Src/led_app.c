/*
 * led_app.c
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#include "led_app.h"
#include "debug_api.h"
#include <cmsis_os2.h>
#include <main.h>

#define __DEBUG_FILE_NAME__ "LED_APP"
#define BLINK_TIMER_PERIOD 100

static const osTimerAttr_t blink_timer_attribute = {
	.name = "LED blink timer"
};

void Blink_Timer_Callback(void *argument);

osTimerId_t led_timer_id = NULL;

bool led_APP_Start(void){
	/* Create blinking LED timer */
	led_timer_id = osTimerNew(&Blink_Timer_Callback, osTimerPeriodic, NULL, &blink_timer_attribute);
	if(led_timer_id == NULL){
		error("Creating led timer\r\n");
		return false;
	}
	if(osTimerStart(led_timer_id, BLINK_TIMER_PERIOD) != osOK){
		error("Starting led timer\r\n");
		return false;
	}
	return true;
}

void Blink_Timer_Callback(void *argument){
	HAL_GPIO_TogglePin(SCREEN_LED_GPIO_Port, SCREEN_LED_Pin);
	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}
