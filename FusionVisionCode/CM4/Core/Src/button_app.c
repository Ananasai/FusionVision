#include "button_app.h"
#include "debug_api.h"
#include "shared_param_api.h"
#include "ui_interface.h"
#include <main.h>
#include "common.h"

#define __DEBUG_FILE_NAME__ "BTN"
#define DEBOUNCE_TIMEOUT 100 /* ms */

typedef void (*Button_Callback)(eButtonType_t, eButtonPress_t);

typedef enum eButtonFlags {
	eButtonFlagsFirst,
	eButtonFlagsBtn1,
	eButtonFlagsBtn2,
	eButtonFlagsBtn3,
}eButtonFlags_t;

typedef struct sButtonDesc_t {
	uint16_t pin;
	eButtonFlags_t flag;
	Button_Callback callback;
}sButtonDesc_t;

typedef struct sButtonState_t {
	uint32_t last_press_time;
}sButtonState_t;

/*
 * Table of button flags, pins and click callbacks.
 */
static const sButtonDesc_t button_desc_lut[eButtonLast] = {
	[eButtonUp] = {.pin = BTN_1_Pin, .flag = eButtonFlagsBtn1, .callback = &UI_Interface_ButtonPressed},
	[eButtonOk] = {.pin = BTN_2_Pin, .flag = eButtonFlagsBtn2, .callback = &UI_Interface_ButtonPressed},
	[eButtonDown] = {.pin = BTN_3_Pin, .flag = eButtonFlagsBtn3, .callback = &UI_Interface_ButtonPressed}
};
static sButtonState_t button_state_lut[eButtonLast] = {0};

/* Flags for storing when buttons were pressed */
static uint32_t button_flags = 0x00;

/*
 * Start function, was used before.
 */
bool Button_APP_Start(void){
	return true;
}

/*
 * Executes in cycle, checks if any buttons were pressed.
 */
bool Button_APP_Run(void){
	for(eButtonType_t btn = eButtonFirst; btn < eButtonLast; btn++){
		if(READ_FLAG(button_flags, button_desc_lut[btn].flag)) {
			CLEAR_FLAG(button_flags, button_desc_lut[btn].flag);
			uint32_t curr_time = HAL_GetTick();
			/* Check if press is DEBOUNCE_TIMEOUT later than the last - do debounce */
			if(curr_time - button_state_lut[btn].last_press_time >= DEBOUNCE_TIMEOUT) {
				(*button_desc_lut[btn].callback)(btn, eButtonPressSingle);
				button_state_lut[btn].last_press_time = curr_time;
			}
		}
	}
	return true;
}

//TODO: port to separate driver
/*
 * GPIO button interrupt callback
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  NVIC_DisableIRQ(EXTI0_IRQn);
  NVIC_DisableIRQ(EXTI15_10_IRQn);
  /* Check which button pin was pressed */
  for(eButtonType_t btn = eButtonFirst; btn < eButtonLast; btn++){
	  if(GPIO_Pin == button_desc_lut[btn].pin){
		  SET_FLAG(button_flags, button_desc_lut[btn].flag);
	  }
  }
  NVIC_EnableIRQ(EXTI0_IRQn);
  NVIC_EnableIRQ(EXTI15_10_IRQn);
}
