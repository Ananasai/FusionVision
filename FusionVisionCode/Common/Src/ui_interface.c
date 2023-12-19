#include "ui_interface.h"

static sUiPanel_t main_menu = {
    .labels = (sUiLabel_t[]) {
        {.content = "1", .length = 1},
        {.content = "2", .length = 1},
        {.content = "3", .length = 1},
    },
    .label_amount = 3
};

bool UI_Interface_GetCurrentPanel(uint32_t panel_id, sUiPanel_t *out){
	*out = main_menu; //TODO: make pointer to pointer
	return true;
}
