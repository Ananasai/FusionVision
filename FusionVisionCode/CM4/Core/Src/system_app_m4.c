/*
 * APP_M4_Program.c
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */
#include "system_app_m4.h"
#include "led_app.h"
#include "button_app.h"
//#include "lighting_app.h"
//#include "battery_app.h"
//#include "adc_app.h"
#include "debug_api.h"
//#include "job_api.h"
#include "shared_param_api.h"
//#include "power_track_app.h"
//#include "serial_app.h"
//#include "lepton_app.h"
#include <stdbool.h>
#include "string_common.h"

/* Both core debugging: https://www.st.com/resource/en/application_note/dm00629855-getting-started-with-projects-based-on-dualcore-stm32h7-microcontrollers-in-stm32cubeide-stmicroelectronics.pdf */

#define __DEBUG_FILE_NAME__ "SYS"
#define APP_DESC(app_enum, name, init, run) [app_enum] = {(sString_t){.text = name, .length = sizeof(name)}, init, run}

typedef struct sAppDesc_t {
	sString_t name;
	bool (*init_func)(void);
	bool (*run_func)(void);
}sAppDesc_t;

/*
 * List of all apps to run.
 */
typedef enum eAppEnum_t {
	eAppFirst = 0,
	eAppLed = eAppFirst,
	eAppButton,
	eAppLighting,
	eAppBattery,
	eAppADC,
	eAppPowerTrack,
	eAppSerial,
	//eAppLepton,
	eAppLast /* MUST BE LEFT LAST */
}eAppEnum_t;

/*
 * Table of apps, their names and init/run function pointers.
 */
static const sAppDesc_t const_app_lut[] = {
	APP_DESC(eAppLed, "LED", &Led_APP_Start, &Led_APP_Run),
	APP_DESC(eAppButton, "BUTTON", &Button_APP_Start, &Button_APP_Run),
//	APP_DESC(eAppLighting, "LIGHTING", &Lighting_APP_Start, NULL),
//	APP_DESC(eAppBattery, "BATTERY", &Battery_APP_Start, NULL),
//	APP_DESC(eAppADC, "ADC", &Adc_APP_Start, NULL),
//	APP_DESC(eAppPowerTrack, "PWR TRACK", &Power_track_APP_Start, NULL),
//	APP_DESC(eAppSerial, "SERIAL", &Serial_APP_Start, NULL),
	//APP_DESC(eAppLepton, "LEPTON", &Lepton_APP_Start)
};

/*
 * Start function - initialises all apps by calling the run functions.
 */
bool System_APP_M4_Start(void){
	/* Start DEBUG uart */
	Debug_API_Start(huart3);
	debug("Init starting\r\n");
	Shared_param_API_Init();
	/* Init all apps by looping thru app lut */
	for(uint8_t i = 0; i < ARRAY_SIZE(const_app_lut); i++){
		/* Execute initialise function of every app*/
		if(const_app_lut[i].init_func == NULL){
			error("Invalid init func of app %s\r\n", const_app_lut[i].name.text);
			return false;
		}
		(*const_app_lut[i].init_func)();
	}
	debug("Init done\r\n");
	return true;
}

/*
 * Run function that executes every cycle.
 */
bool System_APP_M4_Run(void){
	/* Loop thru app lut */
	for(uint8_t i = 0; i < ARRAY_SIZE(const_app_lut); i++){
		/* Execute initialise function of every app */
		if(const_app_lut[i].run_func == NULL){
			continue;
		}
		(*const_app_lut[i].run_func)();
	}
	return true;
}
