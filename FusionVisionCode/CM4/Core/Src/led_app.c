/*
 * led_app.c
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#include "led_app.h"
#include <cmsis_os2.h>
#include <main.h>

#define blink_timer_period 1000

static const osTimerAttr_t blink_timer_attribute = {
		.name = "LED blink timer"
};

static void Blink_Timer_Callback(void *argument);

osTimerId_t led_timer_id = NULL;

bool led_APP_Start(void){
	/* Create blinking LED timer */
	led_timer_id = osTimerNew(Blink_Timer_Callback, osTimerPeriodic, NULL, &blink_timer_attribute);
	if(led_timer_id == NULL){
		return false;
	}
	osTimerStart(led_timer_id, blink_timer_period);
	return true;
}

static void Blink_Timer_Callback(void *argument){
	HAL_GPIO_TogglePin(SCREEN_LED_GPIO_Port, SCREEN_LED_Pin);
}
