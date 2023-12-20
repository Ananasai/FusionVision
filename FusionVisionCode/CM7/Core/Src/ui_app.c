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

bool UI_APP_DrawAll(uint16_t *image_buffer){
	UI_Interface_UpdateLabels(hrtc);
	sUiPanel_t curr_panel;
	UI_Interface_GetConstantPanel(&curr_panel);
	for(size_t i = 0; i < curr_panel.children_amount; i++){
		UI_DRIVER_DrawString(curr_panel.children[i].x, curr_panel.children[i].y, image_buffer, curr_panel.children[i].element.label->content, strlen(curr_panel.children[i].element.label->content), eFont11x18);
	}
	//TODO: make if active
	/* Draw menu if active */
	if(UI_Interface_GetCurrentPanel(0, &curr_panel) == false){
		return false;
	}
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

