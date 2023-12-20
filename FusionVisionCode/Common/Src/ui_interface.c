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
#define BUTTON(_name, _callback) {.type = eUiElementTypeButton, .element.button = &(sUiButton_t){.content = _name, .length = strlen(_name), .callback = NULL}}
#else
#define BUTTON(_name, _callback) {.type = eUiElementTypeButton, .element.button = &(sUiButton_t){.content = _name, .length = strlen(_name), .callback = _callback}}
#endif

#define LABEL(_name) {.type = eUiElementTypeLabel, .element.label = &(sUiLabel_t){.content = _name}}

static char edge_text[20] = "DEFAULT";

static const sUiPanel_t main_menu = {
    .children = (sUiElementType_t[]) {
    	LABEL("Menu"),
    	BUTTON("Up", &Button_APP_EdgeThresholdUpPressed),
		LABEL(edge_text),
		BUTTON("Dwn", &Button_APP_EdgeThresholdDownPressed)
    },
    .children_amount = 4,
	.selectable = 2
};

bool UI_Interface_GetCurrentPanel(uint32_t panel_id, sUiPanel_t *out){
	*out = main_menu; //TODO: make pointer to pointer and fix this
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

bool UI_Interface_UpdateLabels(void){
	uint32_t edge_threshold = 0;
	Shared_param_API_Read(eSharedParamEdgeThreshold, &edge_threshold);
	snprintf(edge_text, 20, "Edge: %lu", edge_threshold);
	return true;
}
