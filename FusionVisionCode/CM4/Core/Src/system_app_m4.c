/*
 * APP_M4_Program.c
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */
#include "system_app_m4.h"
#include "led_app.h"
#include "button_app.h"
#include "lighting_app.h"
#include "battery_app.h"
#include "adc_app.h"

/* Rtos documentation: https://www.keil.com/pack/doc/CMSIS/RTOS2/html/rtos_api2.html */
bool System_APP_M4_Start(void){
	if(led_APP_Start() == false){
		return false;
	}
	if(Button_APP_Start() == false){
		return false;
	}
	if(Lighting_APP_Start() == false){
		return false;
	}
	if(Battery_APP_Start() == false){
		return false;
	}
	if(Adc_APP_Start() == false){
		return false;
	}
	return true;
}
