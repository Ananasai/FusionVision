#include "ui_interface.h"
#include <string.h>

#define BUTTON(_name, _callback) {.type = eUiElementTypeButton, .element.button = &(sUiButton_t){.content = _name, .length = strlen(_name), .callback = _callback}}
#define LABEL(_name) {.type = eUiElementTypeLabel, .element.label = &(sUiLabel_t){.content = _name, .length = strlen(_name)}}

static sUiPanel_t main_menu = {
    .children = (sUiElementType_t[]) {
    	LABEL("Menu"),
    	BUTTON("1", NULL),
		BUTTON("2", NULL)
    },
    .children_amount = 3,
	.selectable = 2
};

bool UI_Interface_GetCurrentPanel(uint32_t panel_id, sUiPanel_t *out){
	*out = main_menu; //TODO: make pointer to pointer and fix this
	return true;
}
