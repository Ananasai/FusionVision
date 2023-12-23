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
	uint32_t parent_btn;
}sPanelNavDesc_t;

typedef struct sParamChange_t {
	uint32_t *local;
	eSharedParamEnum_t param;
	uint32_t max;
	uint32_t min;
}sParamChange_t;

#define __DEBUG_FILE_NAME__ "UI"

#define BUTTON(_name, _length, _x, _y, _font, _align, _callback) {.type = eUiElementTypeButton, .x = _x, .y = _y, .param = &(sTextParam_t){.font = _font, .alignment = _align}, .element.button = &(sUiButton_t){.string = &(sString_t){.text = _name, .length = _length}, .callback = _callback}}
#define LABEL(_name, _length, _x, _y, _font, _align) {.type = eUiElementTypeLabel, .x = _x, .y = _y, .param = &(sTextParam_t){.font = _font, .alignment = _align}, .element.label = &(sUiLabel_t){.string = &(sString_t){.text = _name, .length = _length}}}
#define NAV_BUTTON(_name, _length, _x, _y, _font, _align, _target) {.type = eUiElementTypeNavButton, .x = _x, .y = _y, .param = &(sTextParam_t){.font = _font, .alignment = _align}, .element.nav_button = &(sUiNavButton_t){.string = &(sString_t){.text = _name, .length = _length}, .target = _target}}

#define PANEL(_panel, _parent, _parent_btn) {.panel = _panel, .parent = _parent, .parent_btn = _parent_btn}

static void UI_NavigationalButtonCallback(eButtonType_t btn, eButtonPress_t press);
static void UI_ParamChangeButtonCallback(eButtonType_t btn, eButtonPress_t press);
static void UI_NavButtonPressed(eButtonPress_t press, ePanel_t target);

static char edge_text[20] = "DEFAULT";
static char screen_state_text[20] = "DEFAULT";
static char screen_optim_text[20] = "DEFAULT";
static char time_text[20] = "66.66.66";

static const sUiPanel_t main_menu = {
	.x = 50,
	.y = 175,
	.spacing_y = 40,
    .children = (sUiElementType_t[]) {
    	LABEL("Menu", 4,  0, 0, eFont11x18, eAlignmentCenter),
    	NAV_BUTTON("Edge", 4, 0, 0, eFont11x18, eAlignmentCenter, ePanelEdge),
		NAV_BUTTON("Screen", 6, 0, 0, eFont11x18, eAlignmentCenter, ePanelScreenState),
		//BUTTON("Optim", 12, 0, 0, eFont11x18, eAlignmentCenter, &UI_Interface_ScreenStateButtonPressed),
    },
    .children_amount = 3,
	.selectable = 2,
	.btn_callback = &UI_NavigationalButtonCallback
};

static const sUiPanel_t edge_threshold_menu = {
	.x = 50,
	.y = 175,
	.spacing_y = 40,
    .children = (sUiElementType_t[]) {
		LABEL("Threshold:", 9, 0, 0, eFont11x18, eAlignmentCenter),
		LABEL(edge_text, 20, 0, 0, eFont11x18, eAlignmentCenter),
    },
    .children_amount = 2,
	.selectable = 0,
	.btn_callback = &UI_ParamChangeButtonCallback
};

static const sUiPanel_t screen_state_menu = {
	.x = 50,
	.y = 175,
	.spacing_y = 40,
    .children = (sUiElementType_t[]) {
		LABEL("Screen:", 7, 0, 0, eFont11x18, eAlignmentCenter),
		LABEL(screen_state_text, 20, 0, 0, eFont11x18, eAlignmentCenter),
    },
    .children_amount = 2,
	.selectable = 0,
	.btn_callback = &UI_ParamChangeButtonCallback
};

static const sUiPanel_t screen_optim_menu = {
	.x = 50,
	.y = 175,
	.spacing_y = 40,
    .children = (sUiElementType_t[]) {
		LABEL("Screen:", 7, 0, 0, eFont11x18, eAlignmentCenter),
		LABEL(screen_state_text, 20, 0, 0, eFont11x18, eAlignmentCenter),
    },
    .children_amount = 2,
	.selectable = 0,
	.btn_callback = &UI_ParamChangeButtonCallback
};

static const sUiPanel_t constant_menu = {
	.x = 50,
	.y = 175,
	.spacing_y = 40,
	.children = (sUiElementType_t[]) {
		LABEL(time_text, 8, 128, 290, eFont11x18, eAlignmentCenter),
		LABEL("Group A tm", 10, 120, 1, eFont11x18, eAlignmentCenter)
	},
	.children_amount = 2,
	.selectable = 0
};

static const sPanelNavDesc_t panel_lut[ePanelLast] = {
	[ePanelMainMenu] = PANEL(&main_menu, ePanelMainMenu, 0),
	[ePanelEdge] = PANEL(&edge_threshold_menu, ePanelMainMenu, 0),
	[ePanelScreenState] = PANEL(&screen_state_menu, ePanelMainMenu, 1),
	[ePanelScreenOptim] = PANEL(&screen_optim_menu, ePanelMainMenu, 2)
};

static const char *screen_state_text_lut[eScreenStateLast] = {
	[eScreenStatePassthrough] = "Passthrough",
	[eScreenStateProcessed] = "Processed"
};

static const char *screen_optim_text_lut[eScreenOptimLast] = {
	[eScreenOptimNone] = "None",
};

static uint32_t current_active_button_index = 0;
static uint32_t current_active_panel_index = 0;
static uint32_t edge_threshold = 5;
static uint32_t screen_state = eScreenStateFirst;
static uint32_t screen_optim = eScreenStateFirst;

static const sParamChange_t param_change_lut[ePanelLast] = { //TODO: several luts could be merged
	[ePanelEdge] = {.local = &edge_threshold, .param = eSharedParamEdgeThreshold, .max = 999, .min = 0},
	[ePanelScreenState] = {.local = &screen_state, .param = eSharedParamScreenState, .max = eScreenStateLast, .min = eScreenStateFirst},
	[ePanelScreenOptim] = {.local = &screen_optim, .param = eSharedParamScreenOptim, .max = eScreenOptimLast, .min = eScreenOptimFirst}
};

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
			Shared_param_API_Write(eSharedParamActiveUiButtonIndex, &current_active_button_index, 4);
			break;
		}
		case eButtonOk: {
			uint32_t selectable = 0;
			for(uint32_t i = 0; i < main_menu.children_amount; i++){
				if((main_menu.children[i].type == eUiElementTypeButton) ||
						(main_menu.children[i].type == eUiElementTypeNavButton)){
					if(selectable == current_active_button_index){
						/* Found required button */
						switch(main_menu.children[i].type){
							case eUiElementTypeButton: {
								const sUiButton_t *button = main_menu.children[i].element.button;
								if(button->callback == NULL){
									break;
								}
								(*button->callback)(press);
							}break;
							case eUiElementTypeNavButton: {
								const sUiNavButton_t *button = main_menu.children[i].element.nav_button;
								UI_NavButtonPressed(press, button->target);
								break;
							}
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
			Shared_param_API_Write(eSharedParamActiveUiButtonIndex, &current_active_button_index, 4);
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

/* Defines nav button logic -> opening new panel*/
static void UI_NavButtonPressed(eButtonPress_t press, ePanel_t target){
	current_active_button_index = 0;
	current_active_panel_index = target;
	Shared_param_API_Write(eSharedParamActiveUiPanelIndex, &current_active_panel_index, 4);
	Shared_param_API_Write(eSharedParamActiveUiButtonIndex, &current_active_panel_index, 4);
}

/* Defines what up/down/ok buttons do in parameter change panels */
static void UI_ParamChangeButtonCallback(eButtonType_t btn, eButtonPress_t press){
	if((btn >= eButtonLast) || (press >= eButtonPressLast)){
		return;
	}
	sParamChange_t param = param_change_lut[current_active_panel_index];
	if(param.local == NULL){
		return;
	}
	switch(btn){
		case eButtonUp: {
			if(*param.local == param.max - 1){
				*param.local = param.min;
			}else{
				(*param.local)++;
			}
			Shared_param_API_Write(param.param, param.local, 4); //TODO: ONLY uint32_t supported
			break;
		}
		case eButtonOk: {
			sPanelNavDesc_t last_panel = panel_lut[current_active_panel_index];
			current_active_button_index = last_panel.parent_btn;
			current_active_panel_index = last_panel.parent;
			Shared_param_API_Write(eSharedParamActiveUiPanelIndex, &current_active_panel_index, 4);
			Shared_param_API_Write(eSharedParamActiveUiButtonIndex, &current_active_button_index, 4);
			break;
		}
		case eButtonDown: {
			if(*param.local > param.min){
				(*param.local)--;
			}
			else{
				*param.local = param.max - 1;
			}
			Shared_param_API_Write(param.param, param.local, 4); //TODO: here too
			break;
		}
		default: {
			return;
		}
	}
}

#ifdef CORE_CM7
bool UI_Interface_UpdateLabels(RTC_HandleTypeDef hrtc){ //TODO: could simplify into loop or lut?
	/* Edge threshold */
	uint32_t edge_threshold = 0;
	Shared_param_API_Read(eSharedParamEdgeThreshold, &edge_threshold);
	snprintf(edge_text, 20, "%lu", edge_threshold);
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
		return false;
	}
	snprintf(screen_state_text, 20, "%s", screen_state_text_lut[screen_state]);
	/* Screen optim */
	uint32_t screen_optim = 0;
	Shared_param_API_Read(eSharedParamScreenOptim, &screen_optim);
	if(screen_optim >= eScreenOptimLast){
		return false;
	}
	snprintf(screen_optim_text, 20, "%s", screen_optim_text_lut[screen_optim]);
	return true;
}
#endif


