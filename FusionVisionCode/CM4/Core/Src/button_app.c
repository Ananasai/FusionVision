/*
 * button_app.c
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#include "button_app.h"
#include "debug_api.h"
#include "shared_param_api.h"
#include "ui_interface.h"
#include <main.h>
#include <cmsis_os2.h>

#define __DEBUG_FILE_NAME__ "BTN"
#define BUTTON_EVENT_FLAG_BTN_1 0x01
#define BUTTON_EVENT_FLAG_BTN_2 0x02
#define BUTTON_EVENT_FLAG_BTN_3 0x04
#define BUTTON_EVENT_FLAG_ALL (BUTTON_EVENT_FLAG_BTN_1 | BUTTON_EVENT_FLAG_BTN_2 | BUTTON_EVENT_FLAG_BTN_3)
#define DOUBLE_CLICK_TIMEOUT_MS 400
#define LONG_CLICK_TIMEOUT_MS 500

typedef void (*Button_Callback)(eButtonType_t, eButtonPress_t);

typedef struct sButtonDesc_t {
	uint16_t pin;
	uint32_t flag; //TODO: remove flags? maybe a queue better?
	Button_Callback callback;
}sButtonDesc_t;

typedef struct sButtonState_t {
	uint32_t last_press_time;
	bool click_active;
	bool high;
}sButtonState_t;

static const sButtonDesc_t button_desc_lut[eButtonLast] = {
	[eButtonUp] = {.pin = BTN_1_Pin, .flag = BUTTON_EVENT_FLAG_BTN_1, .callback = &UI_Interface_ButtonPressed},
	[eButtonOk] = {.pin = BTN_2_Pin, .flag = BUTTON_EVENT_FLAG_BTN_2, .callback = &UI_Interface_ButtonPressed},
	[eButtonDown] = {.pin = BTN_3_Pin, .flag = BUTTON_EVENT_FLAG_BTN_3, .callback = &UI_Interface_ButtonPressed}
};
static sButtonState_t button_state_lut[eButtonLast] = {0};

static const osThreadAttr_t button_APP_thread_attribute = {
	.name = "button APP",
	.priority = osPriorityNormal
};

static osThreadId_t button_APP_thread_id = NULL;
static osEventFlagsId_t button_event_flags_id = NULL;
static osTimerId_t button_click_timer = NULL;

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
	debug("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\r\n"); //TODO: needed for middle button to work?? something with memory
	return true;
}

void Button_APP_Thread(void *argument){
	uint32_t flags = 0x00;
	while(true){
		flags = osEventFlagsWait(button_event_flags_id, BUTTON_EVENT_FLAG_ALL, osFlagsWaitAny, osWaitForever);
		for(eButtonType_t btn = eButtonFirst; btn < eButtonLast; btn++){
			  if((flags & button_desc_lut[btn].flag) != 0x00){
				  uint32_t curr_time = HAL_GetTick();
				  button_state_lut[btn].high = !button_state_lut[btn].high;
				  /* Falling btn edge */
				  if(button_state_lut[btn].high == false){
					  /* Is if long click */
					  if(curr_time - button_state_lut[btn].last_press_time >= LONG_CLICK_TIMEOUT_MS){
						  button_state_lut[btn].click_active = false;
						  debug("Long click\r\n");
						  (*button_desc_lut[btn].callback)(btn, eButtonPressLong);
					  }
				  }else{ /* Rising btn edge */
					  /* Check if double click */
					  if(curr_time - button_state_lut[btn].last_press_time <= DOUBLE_CLICK_TIMEOUT_MS){
						  button_state_lut[btn].click_active = false;
						  button_state_lut[btn].last_press_time = curr_time;
						  debug("Double click\r\n");
						  (*button_desc_lut[btn].callback)(btn, eButtonPressDouble);
					  }else{ /* Record time for single double/signal click tracking */
						  button_state_lut[btn].click_active = true;
						  button_state_lut[btn].last_press_time = curr_time;
					  }
				  }
			  }
		  }
	}
}

/* Timer for single click timeout  */
static void Button_Timer(void *argument){
	uint32_t curr_time = HAL_GetTick();
	for(eButtonType_t btn = eButtonFirst; btn < eButtonLast; btn++){
		/* If waiting for double click ant button low level */
		if(button_state_lut[btn].click_active && !button_state_lut[btn].high){
			if(curr_time - button_state_lut[btn].last_press_time >= DOUBLE_CLICK_TIMEOUT_MS+10){
				button_state_lut[btn].click_active = false;
				debug("Single click\r\n");
				(*button_desc_lut[btn].callback)(btn, eButtonPressSingle);
			}
		}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  NVIC_DisableIRQ(EXTI0_IRQn);
  NVIC_DisableIRQ(EXTI15_10_IRQn);
  for(eButtonType_t btn = eButtonFirst; btn < eButtonLast; btn++){
	  if(GPIO_Pin == button_desc_lut[btn].pin){
		  osEventFlagsSet(button_event_flags_id, button_desc_lut[btn].flag);
	  }
  }
  NVIC_EnableIRQ(EXTI0_IRQn);
  NVIC_EnableIRQ(EXTI15_10_IRQn);
}
