/*
 * sync_api.h
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#ifndef INC_UI_INTERFACE_H_
#define INC_UI_INTERFACE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum eUiElementType_t {
	eUiElementTypeFirst = 0,
	eUiElementTypeLabel = eUiElementTypeFirst,
	eUiElementTypeButton,
	eUiElementTypeLast
}eUiElementType_t;

typedef struct sUiLabel_t {
	char *content;
	size_t length;
}sUiLabel_t;

typedef void (*UI_Button_Callback_t)();

typedef struct sUiButton_t {
	char *content;
	size_t length;
	UI_Button_Callback_t callback;
}sUiButton_t;

typedef struct sUiElementType_t {
	eUiElementType_t type;
	union {
		sUiLabel_t *label;
		sUiButton_t *button;
	} element;
}sUiElementType_t;

typedef struct sUiPanel_t {
	sUiElementType_t *children;
	size_t children_amount;
	size_t selectable;
}sUiPanel_t;

bool UI_Interface_GetCurrentPanel(uint32_t panel_id, sUiPanel_t *out);
bool UI_Interface_ButtonPressed(uint32_t panel_id, uint32_t button_id);
bool UI_Interface_UpdateLabels(void);

#endif /* INC_UI_INTERFACE_H_ */
