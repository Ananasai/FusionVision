#include "ui_interface.h"
#include "shared_param_api.h"
#include "debug_api.h"
#include <string.h>
#include <stdio.h>

#ifdef CORE_CM7
#else
#include "button_app.h"
#endif

#define __DEBUG_FILE_NAME__ "UI"
/* Todo: find a better way instead of this shit below */
#define BUTTON(_name, _x, _y, _callback) {.type = eUiElementTypeButton, .x = _x, .y = _y, .element.button = &(sUiButton_t){.content = _name, .length = strlen(_name), .callback = _callback}}

#define LABEL(_name, _x, _y) {.type = eUiElementTypeLabel, .x = _x, .y = _y, .element.label = &(sUiLabel_t){.content = _name}}

static void UI_NavigationalButtonCallback(eButtonType_t btn, eButtonPress_t press);
static void UI_EdgeButtonCallback(eButtonType_t btn, eButtonPress_t press);
static void UI_ScreenStateButtonCallback(eButtonType_t btn, eButtonPress_t press);
static void UI_Interface_EdgeButtonPressed(eButtonPress_t press);
static void UI_Interface_ScreenStateButtonPressed(eButtonPress_t press);

static char edge_text[20] = "DEFAULT";
static char screen_state_text[20] = "DEFAULT";

static const sUiPanel_t main_menu = {
	.x = 100,
	.y = 200,
    .children = (sUiElementType_t[]) {
    	LABEL("Menu", 0, 0),
    	BUTTON("Edge", 0, 0, &UI_Interface_EdgeButtonPressed),
		BUTTON("Screen", 0, 0, &UI_Interface_ScreenStateButtonPressed),
		//BUTTON("NONE", 0, 0, &UI_Interface_EdgeButtonPressed),
    },
    .children_amount = 3,
	.selectable = 2,
	.btn_callback = &UI_NavigationalButtonCallback
};

static const sUiPanel_t edge_threshold_menu = {
	.x = 100,
	.y = 200,
    .children = (sUiElementType_t[]) {
		LABEL(edge_text, 0, 0),
    },
    .children_amount = 1,
	.selectable = 1,
	.btn_callback = &UI_EdgeButtonCallback
};

static const sUiPanel_t screen_state_menu = {
	.x = 100,
	.y = 200,
    .children = (sUiElementType_t[]) {
		LABEL(screen_state_text, 0, 0),
    },
    .children_amount = 1,
	.selectable = 1,
	.btn_callback = &UI_ScreenStateButtonCallback
};

static char time_text[20] = "66.66.66";

static const sUiPanel_t constant_menu = {
	.x = 128,
	.y = 290,
	.children = (sUiElementType_t[]) {
		LABEL(time_text, 128, 290),
		LABEL("Group A tm", 120, 1)
	},
	.children_amount = 2,
	.selectable = 0
};

static const sUiPanel_t *panel_lut[ePanelLast] = {
	[ePanelMainMenu] = &main_menu,
	[ePanelEdge] = &edge_threshold_menu,
	[ePanelScreenState] = &screen_state_menu
};

static const char *screen_state_text_lut[eScreenStateLast] = {
	[eScreenStatePassthrough] = "Passthrough",
	[eScreenStateProcessed] = "Processed"
};

static uint32_t current_active_button_index = 0;
static uint32_t current_active_panel_index = 0;
static uint32_t edge_threshold = 0;
static uint32_t screen_state = eScreenStateFirst;
static const sUiPanel_t *current_panel = &main_menu;

bool UI_Interface_GetCurrentPanel(const sUiPanel_t **out){
	Shared_param_API_Read(eSharedParamActiveUiPanelIndex, &current_active_panel_index);
	current_panel = panel_lut[current_active_panel_index];
	*out = current_panel;
	return true;
}

bool UI_Interface_GetConstantPanel(const sUiPanel_t **out){
	*out = &constant_menu;
	return true;
}

static void UI_NavigationalButtonCallback(eButtonType_t btn, eButtonPress_t press){
	if((btn >= eButtonLast) || (press >= eButtonPressLast)){
		return;
	}
	switch(btn){
		case eButtonUp: {
			if(current_active_button_index > 0){
				current_active_button_index--;
			}
			Shared_param_API_Write(eSharedParamActiveUiButtonIndex, &current_active_button_index, 4);
			break;
		}
		case eButtonOk: {
			uint32_t selectable = 0;
			for(uint32_t i = 0; i < main_menu.children_amount; i++){
				if(main_menu.children[i].type == eUiElementTypeButton){
					if(selectable == current_active_button_index){
						/* Found required button */
						sUiButton_t *button = main_menu.children[i].element.button;
						if(button->callback == NULL){
							break;
						}
						(*button->callback)(press);
						break;
					}
					else{
						selectable++;
					}
				}
			}
			break;
		}
		case eButtonDown: {
			if(current_active_button_index < current_panel->selectable - 1){
				current_active_button_index++;
			}
			Shared_param_API_Write(eSharedParamActiveUiButtonIndex, &current_active_button_index, 4);
			break;
		}
		default: {
			return;
		}
	}
}

void UI_Interface_ButtonPressed(eButtonType_t btn, eButtonPress_t press){
	(*current_panel->btn_callback)(btn, press);
}
/* TODO: standardize this */
static void UI_Interface_EdgeButtonPressed(eButtonPress_t press){
	current_panel = &edge_threshold_menu;
	current_active_button_index = 0;
	current_active_panel_index = ePanelEdge;
	Shared_param_API_Write(eSharedParamActiveUiPanelIndex, &current_active_panel_index, 4);
	Shared_param_API_Write(eSharedParamActiveUiButtonIndex, &current_active_panel_index, 4);
}

/* TODO: standardize this */
static void UI_Interface_ScreenStateButtonPressed(eButtonPress_t press){
	current_panel = &screen_state_menu;
	current_active_button_index = 0;
	current_active_panel_index = ePanelScreenState;
	Shared_param_API_Write(eSharedParamActiveUiPanelIndex, &current_active_panel_index, 4);
	Shared_param_API_Write(eSharedParamActiveUiButtonIndex, &current_active_panel_index, 4);
}

static void UI_EdgeButtonCallback(eButtonType_t btn, eButtonPress_t press){
	if((btn >= eButtonLast) || (press >= eButtonPressLast)){
		return;
	}
	switch(btn){
		case eButtonUp: {
			edge_threshold++;
			Shared_param_API_Write(eSharedParamEdgeThreshold, &edge_threshold, 4);
			break;
		}
		case eButtonOk: {
			current_panel = &main_menu;
			current_active_button_index = 0;
			current_active_panel_index = 0;
			Shared_param_API_Write(eSharedParamActiveUiPanelIndex, &current_active_panel_index, 4);
			Shared_param_API_Write(eSharedParamActiveUiButtonIndex, &current_active_panel_index, 4);
			break;
		}
		case eButtonDown: {
			if(edge_threshold > 0){
				edge_threshold--;
			}
			Shared_param_API_Write(eSharedParamEdgeThreshold, &edge_threshold, 4);
			break;
		}
		default: {
			return;
		}
	}
}

static void UI_ScreenStateButtonCallback(eButtonType_t btn, eButtonPress_t press){
	if((btn >= eButtonLast) || (press >= eButtonPressLast)){
		return;
	}
	switch(btn){
		case eButtonUp: {
			if(screen_state == eScreenStateLast - 1){
				screen_state = eScreenStateFirst;
			}else{
				screen_state++;
			}
			Shared_param_API_Write(eSharedParamScreenState, &screen_state, 4);
			break;
		}
		case eButtonOk: {
			current_panel = &main_menu;
			current_active_button_index = 0;
			current_active_panel_index = 0;
			Shared_param_API_Write(eSharedParamActiveUiPanelIndex, &current_active_panel_index, 4);
			Shared_param_API_Write(eSharedParamActiveUiButtonIndex, &current_active_panel_index, 4);
			break;
		}
		case eButtonDown: {
			if(screen_state > eScreenStateFirst){
				screen_state--;
			}
			Shared_param_API_Write(eSharedParamScreenState, &screen_state, 4);
			break;
		}
		default: {
			return;
		}
	}
}

#ifdef CORE_CM7
bool UI_Interface_UpdateLabels(RTC_HandleTypeDef hrtc){
	/* Edge threshold */
	uint32_t edge_threshold = 0;
	Shared_param_API_Read(eSharedParamEdgeThreshold, &edge_threshold);
	snprintf(edge_text, 20, "Edge: %lu", edge_threshold);
	/* RTC */
	RTC_TimeTypeDef time;
	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	RTC_DateTypeDef date; /* Need read date for value to update */
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	snprintf(time_text, 20, "%02d:%02d:%02d", time.Hours, time.Minutes, time.Seconds);
	/* Screen state */
	uint32_t screen_state = 0;
	Shared_param_API_Read(eSharedParamScreenState, &screen_state);
	if(screen_state >= eScreenStateLast){
		error("Screen: %ld", screen_state);
		return false;
	}
	snprintf(screen_state_text, 20, "%s", screen_state_text_lut[screen_state]);
	return true;
}
#endif


