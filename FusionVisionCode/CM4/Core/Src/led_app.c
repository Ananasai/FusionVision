#include "led_app.h"
#include "debug_api.h"
#include <main.h>

#define __DEBUG_FILE_NAME__ "LED_APP"
#define BLINK_TIMER_PERIOD 100

void Blink_Timer_Callback(void);

static uint32_t last_time = 0;
static uint32_t new_time = 0;

bool Led_APP_Start(void){

	return true;
}

bool Led_APP_Run(void){
	new_time = HAL_GetTick();
	if(new_time - last_time > 100) {
		last_time = new_time;
		Blink_Timer_Callback();
	}
	return true;
}

void Blink_Timer_Callback(void){
	HAL_GPIO_TogglePin(SCREEN_LED_GPIO_Port, SCREEN_LED_Pin);
	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
}
