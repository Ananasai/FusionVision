/*
 * ui_app.c
 *
 *  Created on: Dec 16, 2023
 *      Author: simon
 */
#include "ui_app.h"
#include "ui_element_driver.h"
#include "debug_api.h"
#include "ui_interface.h"
#include "shared_param_api.h"
#include <stdio.h>
#include <string.h>

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

typedef enum eUiElementSourceType_t {
	eUiElementFirst = 0,
	eUiElementText = eUiElementFirst,
	eUiElementTime,
	eUiElementLast
}eUiElementSourceType_t;

typedef struct sUiElement_t {
	uint16_t x;
	uint16_t y;
	eUiElementSourceType_t source_type;
	uint32_t *source;
}sUiElement_t;

static const char tekstas[] = "Lab";

// Define the signature for the functions
typedef const char* (*GetterFunc)(void);

// Define some example functions
static const char* Text_getter(void) {
    return tekstas;
}

char time_text_buffer[20] = {0};
static const char* Time_getter(void) {
	RTC_TimeTypeDef time;
	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	/* Dummy read to */
	RTC_DateTypeDef date;
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	snprintf(time_text_buffer, 20, "%02d:%02d:%02d", time.Hours, time.Minutes, time.Seconds);
    return time_text_buffer;
}

// Create an array of function pointers
static const GetterFunc function_lut[eUiElementLast] = {
    [eUiElementText] = Text_getter,
    [eUiElementTime] = Time_getter,
};

static const sUiElement_t UI_Elements[] = {
	{.x = 100, .y = 20, .source_type = eUiElementText, .source = (uint32_t *)&tekstas},
	{.x = 128, .y = 290, .source_type = eUiElementTime, .source = (uint32_t *)&tekstas}
};

bool UI_APP_DrawAll(uint16_t *image_buffer){
	for(uint8_t i = 0; i < ARRAY_LENGTH(UI_Elements); i++){
		HAL_Delay(1); //TODO: MAGIC
		//UI_DRIVER_DrawString(UI_Elements[i].x, UI_Elements[i].y, image_buffer, (char *)UI_Elements[i].source, strlen((char *)UI_Elements[i].source));
		const char *text = function_lut[UI_Elements[i].source_type]();
		UI_DRIVER_DrawString(UI_Elements[i].x, UI_Elements[i].y, image_buffer, text, strlen(text), eFont11x18);
	}
	/* Draw menu if active */
	//TODO: make if active
	sUiPanel_t curr_panel;
	if(UI_Interface_GetCurrentPanel(0, &curr_panel) == false){
		return false;
	}
	UI_Interface_UpdateLabels();
	uint16_t panel_x = 100;
	uint16_t panel_y = 200;
	volatile uint32_t selected = 0;
	Shared_param_API_Read(eSharedParamActiveUiButtonIndex, &selected);
	uint32_t selectable_i = 0;
	for(size_t i = 0; i < curr_panel.children_amount; i++){
		HAL_Delay(1); //TODO: MAGIC
		switch(curr_panel.children[i].type){
			case(eUiElementTypeLabel): {
				sUiLabel_t label = (sUiLabel_t)(*curr_panel.children[i].element.label); //TODO: DONT COPy
				UI_DRIVER_DrawString(panel_x, panel_y, image_buffer, label.content, strlen(label.content), eFont11x18);
			} break;
			case (eUiElementTypeButton): {
				sUiButton_t button = (sUiButton_t)(*curr_panel.children[i].element.button);
				UI_DRIVER_DrawButton(panel_x, panel_y, image_buffer, button.content, button.length, eFont11x18, selectable_i == selected);
				selectable_i++;
			}break;
			default: {
				/* Should not happen */
			}break;
		}
		panel_y -= 50;
	}
	return true;
}

bool UI_APP_Printout(uint16_t *image_buffer){
	for(uint16_t y = 0; y < 320; y++){
		for(uint16_t x = 0; x < 480; x++){
			DEBUG_API_LOG("%d,", NULL, NULL, *(image_buffer + x + y * 480));
			if(x % 120 == 0){
				DEBUG_API_LOG("\r\n", NULL, NULL);
			}
		}
		DEBUG_API_LOG("\r\n", NULL, NULL);
	}
	return true;
}

