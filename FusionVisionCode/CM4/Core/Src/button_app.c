/*
 * button_app.c
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#include "button_app.h"
#include "debug_api.h"
#include <main.h>
#include <cmsis_os2.h>

#define __DEBUG_FILE_NAME__ "BTN_APP"
#define BUTTON_EVENT_FLAG_BTN_1 0x01
#define BUTTON_EVENT_FLAG_BTN_2 0x02
#define BUTTON_EVENT_FLAG_BTN_3 0x04
#define BUTTON_EVENT_FLAG_ALL (BUTTON_EVENT_FLAG_BTN_1 | BUTTON_EVENT_FLAG_BTN_2 | BUTTON_EVENT_FLAG_BTN_3)

static const osThreadAttr_t button_APP_thread_attribute = {
		.name = "button APP",
		.priority = osPriorityNormal
};

//TODO: Make buttons into one look up table

static osThreadId_t button_APP_thread_id = NULL;
static osEventFlagsId_t button_event_flags_id = NULL;

void Button_APP_Thread(void *argument);

bool Button_APP_Start(void){
	button_event_flags_id = osEventFlagsNew(NULL);
	if(button_event_flags_id == NULL){
		error("Creating button event flags\r\n");
		return false;
	}
	button_APP_thread_id = osThreadNew(&Button_APP_Thread, NULL, &button_APP_thread_attribute);
	if(button_APP_thread_id == NULL){
		error("Creating button thread\r\n");
		return false;
	}
	return true;
}

void Button_APP_Thread(void *argument){
	uint32_t flags = 0x00;
	while(true){
		flags = osEventFlagsWait(button_event_flags_id, BUTTON_EVENT_FLAG_ALL, osFlagsWaitAny, osWaitForever);
		switch(flags){
			case BUTTON_EVENT_FLAG_BTN_1: {
				debug("Pressed button 1\r\n");
				break;
			}
			case BUTTON_EVENT_FLAG_BTN_2: {
				debug("Pressed button 2\r\n");
				break;
			}
			case BUTTON_EVENT_FLAG_BTN_3: {
				debug("Pressed button 3\r\n");
				break;
			}
			default: {
				error("Invalid button pressed\r\n");
				break;
			}
		}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  NVIC_DisableIRQ(EXTI0_IRQn);
  NVIC_DisableIRQ(EXTI15_10_IRQn);
  if (GPIO_Pin == BTN_1_Pin) {
	  osEventFlagsSet(button_event_flags_id, BUTTON_EVENT_FLAG_BTN_1);
  }
  else if(GPIO_Pin == BTN_2_Pin){
	  osEventFlagsSet(button_event_flags_id, BUTTON_EVENT_FLAG_BTN_2);
  }
  else if (GPIO_Pin == BTN_3_Pin){
	  osEventFlagsSet(button_event_flags_id, BUTTON_EVENT_FLAG_BTN_3);
  }
  NVIC_EnableIRQ(EXTI0_IRQn);
  NVIC_EnableIRQ(EXTI15_10_IRQn);
}
