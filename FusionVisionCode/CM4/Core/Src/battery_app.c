///*
// * battery_app.c
// *
// *  Created on: Nov 16, 2023
// *      Author: simon
// */
//
//#include "battery_app.h"
//#include "debug_api.h"
//
//#define __DEBUG_FILE_NAME__ "BAT_APP"
//
//static const osThreadAttr_t battery_thread_attribute = {
//		.name = "BAT APP"
//};
//
//static void Battery_APP_Thread(void *arguemnt);
//
//static osThreadId_t battery_thread_id = NULL;
//
//bool Battery_APP_Start(void){
//	battery_thread_id = osThreadNew(&Battery_APP_Thread, NULL, &battery_thread_attribute);
//	if(battery_thread_id == NULL){
//		error("Creating battery thread\r\n");
//		return false;
//	}
//	return true;
//}
//
//
//static void Battery_APP_Thread(void *arguemnt){
//	while(true){
//		osDelay(osWaitForever);
//	}
//}
