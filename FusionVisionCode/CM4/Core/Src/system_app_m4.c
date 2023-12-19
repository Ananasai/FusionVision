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
#include "debug_api.h"
#include "job_api.h"
#include "shared_param_api.h"
#include <stdbool.h>
#include "string_common.h"

#define __DEBUG_FILE_NAME__ "SYS"
#define APP_DESC(app_enum, name, init) [app_enum] = {(sString_t){.string = name, .length = sizeof(name)}, init}

typedef struct sAppDesc_t {
	sString_t name;
	bool (*init_func)(void);
}sAppDesc_t;

/* ADD new apps here */
typedef enum eAppEnum_t {
	eAppFirst = 0,
	eAppLed = eAppFirst,
	eAppButton,
	eAppLighting,
	eAppBattery,
	eAppADC,
	eAppLast /* MUST BE LEFT LAST */
}eAppEnum_t;

static const sAppDesc_t const_app_lut[eAppLast] = {
	APP_DESC(eAppLed, "LED", &led_APP_Start),
	APP_DESC(eAppButton, "BUTTON", &Button_APP_Start),
	APP_DESC(eAppLighting, "LIGHTING", &Lighting_APP_Start),
	APP_DESC(eAppBattery, "BATTERY", &Battery_APP_Start),
	APP_DESC(eAppADC, "ADC", &Adc_APP_Start),
};

/* RTOS CMSIS V2 documentation: https://www.keil.com/pack/doc/CMSIS/RTOS2/html/rtos_api2.html */
bool System_APP_M4_Start(void){
	//Debug_API_Start();
	debug("Init starting\r\n");
	Shared_param_API_Init();
	Job_API_InitAll();
	for(eAppEnum_t app = eAppFirst; app < eAppLast; app++){
		/* Execute initialise function of every app*/
		if(const_app_lut[app].init_func == NULL){
			error("Invalid init func of app %s\r\n", const_app_lut[app].name.string);
			return false;
		}
		(*const_app_lut[app].init_func)();
	}
	debug("Init done\r\n");
	return true;
}
