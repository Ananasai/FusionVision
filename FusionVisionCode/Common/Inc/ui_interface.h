/*
 * sync_api.h
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#ifndef INC_UI_INTERFACE_H_
#define INC_UI_INTERFACE_H_

#include "main.h"
#include "fonts.h"
#include "string_common.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum eButtonType_t {
	eButtonFirst = 0,
	eButtonUp = eButtonFirst,
	eButtonOk,
	eButtonDown,
	eButtonLast
}eButtonType_t;

typedef enum eButtonPress_t {
	eButtonPressFirst = 0,
	eButtonPressSingle = eButtonPressFirst,
	eButtonPressDouble,
	eButtonPressLong,
	eButtonPressLast
}eButtonPress_t;

typedef enum eUiElementType_t {
	eUiElementTypeFirst = 0,
	eUiElementTypeLabel = eUiElementTypeFirst,
	eUiElementTypeButton,
	eUiElementTypeLast
}eUiElementType_t;

typedef enum ePanel_t {
	ePanelFirst = 0,
	ePanelMainMenu = ePanelFirst,
	ePanelVideoSettings,
	ePanelEdgeSettings,
	ePanelThermalSettings,
	ePanelParamChangeDefault,
	ePanelLast
}ePanel_t;

typedef void (*UI_Button_Callback_t)(eButtonPress_t, const void *);
typedef void (*UI_PanelButton_Callback_t)(eButtonType_t, eButtonPress_t);

typedef struct sUiLabel_t {
	sString_t *string;
}sUiLabel_t;

typedef struct sUiButton_t {
	sString_t *string;
	UI_Button_Callback_t callback;
	const void *argument;
}sUiButton_t;

typedef struct sUiElementType_t {
	eUiElementType_t type;
	uint16_t x;
	uint16_t y;
	sTextParam_t *param;
	union {
		const sUiLabel_t *label;
		const sUiButton_t *button;
	} element;
}sUiElementType_t;

typedef struct sUiPanel_t {
	uint16_t x; /* x and y coordinated only used if no alignment selected */
	uint16_t y;
	eAlignmentVertical_t vert_align;
	eAlignmentHorizontal_t hor_align;
	uint16_t spacing_y;
	sUiElementType_t *children;
	size_t children_amount;
	size_t selectable;
	UI_PanelButton_Callback_t btn_callback;
}sUiPanel_t;

bool UI_Interface_GetCurrentPanel(const sUiPanel_t **out);
bool UI_Interface_GetConstantPanel(const sUiPanel_t **out);

void UI_Interface_ButtonPressed(eButtonType_t btn, eButtonPress_t press);

#ifdef CORE_CM7
bool UI_Interface_UpdateLabels(RTC_HandleTypeDef hrtc);
#endif

#endif /* INC_UI_INTERFACE_H_ */
