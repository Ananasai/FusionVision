/*
 * button_app.c
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#include "button_app.h"
#include "debug_api.h"
#include "shared_param_api.h"
#include <main.h>
#include <cmsis_os2.h>

#define __DEBUG_FILE_NAME__ "BTN"
#define BUTTON_EVENT_FLAG_BTN_1 0x01
#define BUTTON_EVENT_FLAG_BTN_2 0x02
#define BUTTON_EVENT_FLAG_BTN_3 0x04
#define BUTTON_EVENT_FLAG_ALL (BUTTON_EVENT_FLAG_BTN_1 | BUTTON_EVENT_FLAG_BTN_2 | BUTTON_EVENT_FLAG_BTN_3)
#define DOUBLE_CLICK_TIMEOUT_MS 200
#define LONG_CLICK_TIMEOUT_MS 500

typedef enum eButton_t {
	eButtonFirst = 0,
	eButtonUp = eButtonFirst,
	eButtonOk,
	eButtonDown,
	eButtonLast
}eButton_t;

typedef enum eButtonPress_t {
	eButtonPressFirst = 0,
	eButtonPressSingle = eButtonPressFirst,
	eButtonPressDouble,
	eButtonPressLong, //TODO: detect long click
	eButtonPressLast
}eButtonPress_t;

typedef void (*Button_Callback)(eButtonPress_t press);

typedef struct sButtonDesc_t {
	uint16_t pin;
	uint32_t flag; //TODO: remove flags? maybe a queue better?
	Button_Callback callback;
	uint32_t last_press_time;
	bool click_active;
	bool high;
}sButtonDesc_t;

void Button_UP_Callback(eButtonPress_t press);
void Button_OK_Callback(eButtonPress_t press);
void Button_DOWN_Callback(eButtonPress_t press);

//TODO: could make const
static sButtonDesc_t button_lut[eButtonLast] = {
	[eButtonUp] = {.pin = BTN_1_Pin, .flag = BUTTON_EVENT_FLAG_BTN_1, .callback = &Button_UP_Callback, .last_press_time = 0, .click_active = false, .high = false},
	[eButtonOk] = {.pin = BTN_2_Pin, .flag = BUTTON_EVENT_FLAG_BTN_2, .callback = &Button_OK_Callback, .last_press_time = 0, .click_active = false, .high = false},
	[eButtonDown] = {.pin = BTN_3_Pin, .flag = BUTTON_EVENT_FLAG_BTN_3, .callback = &Button_DOWN_Callback, .last_press_time = 0, .click_active = false, .high = false}
};

static const osThreadAttr_t button_APP_thread_attribute = {
	.name = "button APP",
	.priority = osPriorityNormal
};

static osThreadId_t button_APP_thread_id = NULL;
static osEventFlagsId_t button_event_flags_id = NULL;
static osTimerId_t button_click_timer = NULL;

static uint32_t current_active_panel_index = 0;
static uint32_t current_active_button_index = 0;

static void Button_APP_Thread(void *argument);
static void Button_Timer(void *argument);

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
	button_click_timer = osTimerNew(&Button_Timer, osTimerPeriodic, NULL, NULL);
	if(button_click_timer == NULL){
		error("Creating button timer\r\n");
		return false;
	}
	if(osTimerStart(button_click_timer, 10) != osOK){
		error("Starting button timer\r\n");
		return false;
	}
	return true;
}

void Button_APP_Thread(void *argument){
	uint32_t flags = 0x00;
	while(true){
		flags = osEventFlagsWait(button_event_flags_id, BUTTON_EVENT_FLAG_ALL, osFlagsWaitAny, osWaitForever);
		for(eButton_t btn = eButtonFirst; btn < eButtonLast; btn++){
			  if((flags & button_lut[btn].flag) != 0x00){
				  uint32_t curr_time = HAL_GetTick();
				  button_lut[btn].high = !button_lut[btn].high;
				  /* Falling btn edge */
				  if(button_lut[btn].high == false){
					  /* Is if long click */
					  if(curr_time - button_lut[btn].last_press_time >= LONG_CLICK_TIMEOUT_MS){
						  button_lut[btn].click_active = false;
						  debug("Long click\r\n");
						  (*button_lut[btn].callback)(eButtonPressLong);
					  }
				  }else{ /* Rising btn edge */
					  /* Check if double click */
					  if(curr_time - button_lut[btn].last_press_time <= DOUBLE_CLICK_TIMEOUT_MS){
						  button_lut[btn].click_active = false;
						  button_lut[btn].last_press_time = curr_time;
						  debug("Double click\r\n");
						  (*button_lut[btn].callback)(eButtonPressDouble);
					  }else{ /* Record time for single double/signel click tracking */
						  button_lut[btn].click_active = true;
						  button_lut[btn].last_press_time = curr_time;
					  }
				  }
			  }
		  }
	}
}

/* Timer for single click timeout  */
static void Button_Timer(void *argument){
	uint32_t curr_time = HAL_GetTick();
	for(eButton_t btn = eButtonFirst; btn < eButtonLast; btn++){
		/* If waiting for double click ant button low level */
		if(button_lut[btn].click_active && !button_lut[btn].high){
			if(curr_time - button_lut[btn].last_press_time >= DOUBLE_CLICK_TIMEOUT_MS+10){
				button_lut[btn].click_active = false;
				debug("Single click\r\n");
				(*button_lut[btn].callback)(eButtonPressSingle);
			}
		}
	}
}

void Button_UP_Callback(eButtonPress_t press){
	//debug("Pressed button UP\r\n");
	if(current_active_panel_index < 2){
		current_active_panel_index++;
		Shared_param_API_Write(eSharedParamActiveUiPanelIndex, &current_active_panel_index, 4);
	}
}
void Button_OK_Callback(eButtonPress_t press){
	//debug("Pressed button OK\r\n");
}
void Button_DOWN_Callback(eButtonPress_t press){
	//debug("Pressed button DOWN\r\n");
	if(current_active_panel_index > 0){
		current_active_panel_index--;
		Shared_param_API_Write(eSharedParamActiveUiPanelIndex, &current_active_panel_index, 4);
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  NVIC_DisableIRQ(EXTI0_IRQn);
  NVIC_DisableIRQ(EXTI15_10_IRQn);
  for(eButton_t btn = eButtonFirst; btn < eButtonLast; btn++){
	  if(GPIO_Pin == button_lut[btn].pin){
		  osEventFlagsSet(button_event_flags_id, button_lut[btn].flag);
	  }
  }
  NVIC_EnableIRQ(EXTI0_IRQn);
  NVIC_EnableIRQ(EXTI15_10_IRQn);
}
