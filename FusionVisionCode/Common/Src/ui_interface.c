#include "ui_interface.h"
#include "shared_param_api.h"
#include <string.h>
#include <stdio.h>

#ifdef CORE_CM7
#else
#include "button_app.h"
#endif

/* Todo: find a better way instead of this shit below */
#ifdef CORE_CM7
#define BUTTON(_name, _x, _y, _callback) {.type = eUiElementTypeButton, .x = _x, .y = _y, .element.button = &(sUiButton_t){.content = _name, .length = strlen(_name), .callback = NULL}}
#else
#define BUTTON(_name, _x, _y, _callback) {.type = eUiElementTypeButton, .x = _x, .y = _y, .element.button = &(sUiButton_t){.content = _name, .length = strlen(_name), .callback = _callback}}
#endif

#define LABEL(_name, _x, _y) {.type = eUiElementTypeLabel, .x = _x, .y = _y, .element.label = &(sUiLabel_t){.content = _name}}

static char edge_text[20] = "DEFAULT";

static const sUiPanel_t main_menu = {
	.x = 100,
	.y = 200,
    .children = (sUiElementType_t[]) {
    	LABEL("Menu", 0, 0),
    	BUTTON("Up", 0, 0, &Button_APP_EdgeThresholdUpPressed),
		LABEL(edge_text, 0, 0),
		BUTTON("Dwn", 0, 0, &Button_APP_EdgeThresholdDownPressed)
    },
    .children_amount = 4,
	.selectable = 2
};

static char time_text[20] = "66.66.66";

static const sUiPanel_t constant = {
	.x = 128,
	.y = 290,
	.children = (sUiElementType_t[]) {
		LABEL(time_text, 128, 290),
		LABEL("Group A tm", 120, 1)
	},
	.children_amount = 2,
	.selectable = 0
};

bool UI_Interface_GetCurrentPanel(uint32_t panel_id, sUiPanel_t *out){
	*out = main_menu; //TODO: make pointer to pointer and fix this
	return true;
}

bool UI_Interface_GetConstantPanel(sUiPanel_t *out){
	*out = constant;
	return true;
}

bool UI_Interface_ButtonPressed(uint32_t panel_id, uint32_t button_id){
#ifdef CORE_CM7
	return false;
#else
	uint32_t selectable = 0;
	for(uint32_t i = 0; i < main_menu.children_amount; i++){
		if(main_menu.children[i].type == eUiElementTypeButton){
			if(selectable == button_id){
				/* Found required button */
				sUiButton_t *button = main_menu.children[i].element.button;
				if(button->callback == NULL){
					return false;
				}
				(*button->callback)();
				return true;
			}
			else{
				selectable++;
			}
		}
	}
	return false;
#endif
}

bool UI_Interface_UpdateLabels(RTC_HandleTypeDef hrtc){
	uint32_t edge_threshold = 0;
	Shared_param_API_Read(eSharedParamEdgeThreshold, &edge_threshold);
	snprintf(edge_text, 20, "Edge: %lu", edge_threshold);
	RTC_TimeTypeDef time;
	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	/* Dummy read to */
	RTC_DateTypeDef date;
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	snprintf(time_text, 20, "%02d:%02d:%02d", time.Hours, time.Minutes, time.Seconds);
	return true;
}
