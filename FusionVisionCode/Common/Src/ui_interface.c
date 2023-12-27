#include "ui_interface.h"
#include "shared_param_api.h"
#include "debug_api.h"
#include <string.h>
#include <stdio.h>

#ifdef CORE_CM7
#else
#include "button_app.h"
#endif

typedef struct sPanelNavDesc_t {
	const sUiPanel_t *panel;
	ePanel_t parent;
	uint32_t last_btn;
}sPanelNavDesc_t;

#define __DEBUG_FILE_NAME__ "UI"

#define BUTTON(_name, _x, _y, _font, _align, _callback) {.type = eUiElementTypeButton, .x = _x, .y = _y, .param = &(sTextParam_t){.font = _font, .alignment = _align}, .element.button = &(sUiButton_t){.string = &(sString_t){.text = _name, .length = sizeof(_name)}, .callback = _callback, .argument = NULL}}
#define NAV_BUTTON(_name, _x, _y, _font, _align, _target) {.type = eUiElementTypeButton, .x = _x, .y = _y, .param = &(sTextParam_t){.font = _font, .alignment = _align}, .element.button = &(sUiButton_t){.string = &(sString_t){.text = _name, .length = sizeof(_name)}, .callback = UI_NavButtonPressed, .argument = &(uint32_t){_target}}}
#define PARAM_CHANGE_BUTTON(_name, _x, _y, _font, _align, _target) {.type = eUiElementTypeButton, .x = _x, .y = _y, .param = &(sTextParam_t){.font = _font, .alignment = _align}, .element.button = &(sUiButton_t){.string = &(sString_t){.text = _name, .length = sizeof(_name)}, .callback = UI_ParamChangeButtonPressed, .argument = &(eSharedParamEnum_t){_target}}}
#define LABEL(_name, _x, _y, _font, _align) {.type = eUiElementTypeLabel, .x = _x, .y = _y, .param = &(sTextParam_t){.font = _font, .alignment = _align}, .element.label = &(sUiLabel_t){.string = &(sString_t){.text = _name, .length = sizeof(_name)}}}
#define PANEL(_panel, _parent) {.panel = _panel, .parent = _parent, .last_btn = 0}

static void UI_NavigationalButtonCallback(eButtonType_t btn, eButtonPress_t press);
static void UI_ParamChangeButtonCallback(eButtonType_t btn, eButtonPress_t press);
static void UI_NavButtonPressed(eButtonPress_t press, const void *argument);
static void UI_ParamChangeButtonPressed(eButtonPress_t press, const void *argument);

static char current_param_name_text[20] = "DEFAULT";
static char current_param_value_text[20] = "DEFAULT";
static char time_text[20] = "66.66.66";
static char battery_icon_text[3] = "000";

static const sUiPanel_t main_menu_panel = {
	.x = 50,
	.y = 175,
	.spacing_y = 40,
    .children = (sUiElementType_t[]) {
    	LABEL("Menu",  0, 0, eFont11x18, eAlignmentCenter),
    	PARAM_CHANGE_BUTTON("Edge", 0, 0, eFont11x18, eAlignmentCenter, eSharedParamEdgeThreshold),
		PARAM_CHANGE_BUTTON("State", 0, 0, eFont11x18, eAlignmentCenter, eSharedParamScreenState),
		PARAM_CHANGE_BUTTON("Optim", 0, 0, eFont11x18, eAlignmentCenter, eSharedParamScreenOptim),
    },
    .children_amount = 4,
	.selectable = 3,
	.btn_callback = &UI_NavigationalButtonCallback
};

/* Default menu for changing parameters */
static const sUiPanel_t param_change_default_panel = {
	.x = 50,
	.y = 175,
	.spacing_y = 40,
    .children = (sUiElementType_t[]) {
		LABEL(current_param_name_text, 0, 0, eFont11x18, eAlignmentCenter),
		LABEL(current_param_value_text, 0, 0, eFont11x18, eAlignmentCenter),
    },
    .children_amount = 2,
	.selectable = 0,
	.btn_callback = &UI_ParamChangeButtonCallback
};

/* Constant visible menu with RTC and TM */
static const sUiPanel_t constant_menu = {
	.x = 50,
	.y = 175,
	.spacing_y = 40,
	.children = (sUiElementType_t[]) {
		LABEL(time_text, 128, 290, eFont11x18, eAlignmentCenter),
		LABEL("Group A tm", 120, 1, eFont11x18, eAlignmentCenter),
		LABEL(battery_icon_text, 35, 290, eFont11x18, eAlignmentCenter),
	},
	.children_amount = 3,
	.selectable = 0
};

static sPanelNavDesc_t panel_lut[ePanelLast] = {
	[ePanelMainMenu] = PANEL(&main_menu_panel, ePanelMainMenu),
	[ePanelParamChangeDefault] = PANEL(&param_change_default_panel, ePanelMainMenu) //TODO: back button will not work
};

typedef struct sParamValTextDesc_t {
	uint32_t count;
	char *texts[10]; //TODO: could overflow if more than 10
}sParamValTextDesc_t;

static const sParamValTextDesc_t param_value_text[eSharedParamLast] = {
	[eSharedParamScreenState] = {
		.count = 2,
		.texts = {
			[eScreenStatePassthrough] = "Passthrough",
			[eScreenStateProcessed] = "Processed"}
	},
	[eSharedParamScreenOptim] = {
			.count = 3,
		.texts = {
			[eScreenOptimNone] = "None",
			[eScreenOptimInterlacedProcessing] = "Interlaced 1",
			[eScreenOptimInterlacedAll] = "Interlaced 2"}
	},
	[eSharedParamEdgeAlgorithm] = {
		.count = 2,
		.texts = {
			[eEdgeAlgorithmSobel] = "Sobel",
			[eEdgeAlgorithmRoberts] = "Roberts"
		}
	},
};

static uint32_t current_active_button_index = 0;
static uint32_t current_active_panel_index = 0;

static uint32_t current_param = eSharedParamFirst;
static uint32_t current_param_back_btn = 0;

bool UI_Interface_GetCurrentPanel(const sUiPanel_t **out){
	Shared_param_API_Read(eSharedParamActiveUiPanelIndex, &current_active_panel_index);
	*out = panel_lut[current_active_panel_index].panel;
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
	const sUiPanel_t *curr_panel = panel_lut[current_active_panel_index].panel;
	switch(btn){
		case eButtonUp: {
			if(current_active_button_index == 0){
				current_active_button_index = curr_panel->selectable - 1;
			}
			else{
				current_active_button_index--;
			}
			Shared_param_API_Write(eSharedParamActiveUiButtonIndex, &current_active_button_index);
			break;
		}
		case eButtonOk: {
			uint32_t selectable = 0;
			for(uint32_t i = 0; i < main_menu_panel.children_amount; i++){
				if(main_menu_panel.children[i].type == eUiElementTypeButton){
					if(selectable == current_active_button_index){
						/* Found required button */
						panel_lut[current_active_panel_index].last_btn = selectable;
						switch(main_menu_panel.children[i].type){
							case eUiElementTypeButton: {
								const sUiButton_t *button = main_menu_panel.children[i].element.button;
								if(button->callback == NULL){
									break;
								}
								(*button->callback)(press, button->argument);
							}break;
							default: {
								break;
							}
						}
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
			if(current_active_button_index == curr_panel->selectable - 1){
				current_active_button_index = 0;
			}
			else{
				current_active_button_index++;
			}
			Shared_param_API_Write(eSharedParamActiveUiButtonIndex, &current_active_button_index);
			break;
		}
		default: {
			return;
		}
	}
}

void UI_Interface_ButtonPressed(eButtonType_t btn, eButtonPress_t press){
	if(panel_lut[current_active_panel_index].panel->btn_callback == NULL){
		error("Invalid current panel\r\n");
		return;
	}
	(*panel_lut[current_active_panel_index].panel->btn_callback)(btn, press);
}

/* Defines nav button logic -> opening new panel */
static void UI_NavButtonPressed(eButtonPress_t press, const void *argument){
	ePanel_t *target = (ePanel_t *)argument;
	current_active_button_index = 0;
	current_active_panel_index = *target;
	Shared_param_API_Write(eSharedParamActiveUiPanelIndex, &current_active_panel_index);
	Shared_param_API_Write(eSharedParamActiveUiButtonIndex, &current_active_button_index);
}

/* Defines param button logic -> opening param change default panel */
static void UI_ParamChangeButtonPressed(eButtonPress_t press, const void *argument){
	current_param_back_btn = current_active_button_index;
	eSharedParamEnum_t *new_param = (eSharedParamEnum_t *)argument;
	current_active_panel_index = ePanelParamChangeDefault;
	current_param = *new_param;
	Shared_param_API_Write(eSharedParamActiveUiPanelIndex, &current_active_panel_index);
	/* Set current active button to param value to informt M7 core */
	Shared_param_API_Write(eSharedParamActiveUiButtonIndex, &current_param);
}

/* Defines what up/down/ok buttons do in parameter change panels */
static void UI_ParamChangeButtonCallback(eButtonType_t btn, eButtonPress_t press){
	if((btn >= eButtonLast) || (press >= eButtonPressLast)){
		return;
	}
	uint32_t current_param_val = 0;
	sSharedParam_t current_param_desc;
	Shared_param_API_Read(current_param, &current_param_val);
	Shared_param_API_GetDesc(current_param, &current_param_desc);
	switch(btn){
		case eButtonUp: {
			if(current_param_val == current_param_desc.max - 1){
				current_param_val = current_param_desc.min;
			}else{
				current_param_val++;
			}
			Shared_param_API_Write(current_param, &current_param_val); //TODO: ONLY uint32_t supported
			break;
		}
		case eButtonOk: {
			sPanelNavDesc_t last_panel = panel_lut[current_active_panel_index];
			current_active_button_index = panel_lut[last_panel.parent].last_btn;
			current_active_panel_index = last_panel.parent;
			Shared_param_API_Write(eSharedParamActiveUiPanelIndex, &current_active_panel_index);
			Shared_param_API_Write(eSharedParamActiveUiButtonIndex, &current_active_button_index);
			break;
		}
		case eButtonDown: {
			if(current_param_val > current_param_desc.min){
				current_param_val--;
			}
			else{
				current_param_val = current_param_desc.max - 1;
			}
			Shared_param_API_Write(current_param, &current_param_val); //TODO: here too
			break;
		}
		default: {
			return;
		}
	}
}

#ifdef CORE_CM7
bool UI_Interface_UpdateLabels(RTC_HandleTypeDef hrtc){ //TODO: could simplify into loop or lut?
	Shared_param_API_Read(eSharedParamActiveUiPanelIndex, &current_active_panel_index);
	/* Current selected value */
	if(current_active_panel_index == ePanelParamChangeDefault){
		/* Fill value of param*/
		Shared_param_API_Read(eSharedParamActiveUiButtonIndex, &current_param);
		uint32_t current_param_val = 0; //TODO: dont know which parameter is read
		Shared_param_API_Read(current_param, &current_param_val);
		if(param_value_text[current_param].count == 0){
			snprintf(current_param_value_text, 20, "%lu", current_param_val);
		}else{
			snprintf(current_param_value_text, 20, "%s", param_value_text[current_param].texts[current_param_val]);
		}
		/* Fill name of param */
		sSharedParam_t current_shared_param_desc;
		Shared_param_API_GetDesc(current_param, &current_shared_param_desc);
		snprintf(current_param_name_text, 20, "%s", current_shared_param_desc.name);
	}
	/* RTC */
	RTC_TimeTypeDef time;
	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	RTC_DateTypeDef date; /* Need read date for value to update */
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	snprintf(time_text, 20, "%02d:%02d:%02d", time.Hours, time.Minutes, time.Seconds);
	/* Battery */
	uint32_t battery_level = 67;
	if(battery_level > 70){ //TODO: improve
		battery_icon_text[0] = BATTERY_ICON_FULL_LEFT;
		battery_icon_text[1] = BATTERY_ICON_FULL_MIDDLE;
		battery_icon_text[2] = BATTERY_ICON_FULL_RIGHT;
	} else if(battery_level > 60){
		battery_icon_text[0] = BATTERY_ICON_FULL_LEFT;
		battery_icon_text[1] = BATTERY_ICON_FULL_MIDDLE;
		battery_icon_text[2] = BATTERY_ICON_EMPTY_RIGHT;
	} else if (battery_level > 30){
		battery_icon_text[0] = BATTERY_ICON_FULL_LEFT;
		battery_icon_text[1] = BATTERY_ICON_EMPTY_MIDDLE;
		battery_icon_text[2] = BATTERY_ICON_EMPTY_RIGHT;
	} else{
		battery_icon_text[0] = BATTERY_ICON_EMPTY_LEFT;
		battery_icon_text[1] = BATTERY_ICON_EMPTY_MIDDLE;
		battery_icon_text[2] = BATTERY_ICON_EMPTY_RIGHT;
	}
	return true;
}
#endif

