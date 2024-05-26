#include "ui_app.h"
#include "ui_element_driver.h"
#include "debug_api.h"
#include "ui_interface.h"
#include "shared_param_api.h"
#include "sync_api.h"
#include <stdio.h>
#include <string.h>

#define UI_UPDATE_TIMEOUT 4000
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

static void UI_Updated(void);

static uint16_t *image_buffer;
static uint32_t curr_active_ui_button = 0;
static uint32_t last_ui_update_time = 0;
static bool last_ui_update_flag = true;

/*
 * Start function.
 */
bool UI_APP_Init(uint16_t *new_image_buffer){
	if(new_image_buffer == NULL){
		return false;
	}
	image_buffer = new_image_buffer;
	return Sync_API_ActivateSemaphoreIrq(eSemaphoreUiUpdate, &UI_Updated);
}

/*
 * Draw Ui to image_buffer
 */
bool UI_APP_DrawAll(void){
	const sUiPanel_t *curr_panel;
	uint32_t curr_time = HAL_GetTick();
	UI_Interface_UpdateLabels(hrtc);
	/* Draw the constant visible panel - time and battery indicator. */
	UI_Interface_GetConstantPanel(&curr_panel);
	for(size_t i = 0; i < curr_panel->children_amount; i++){
		UI_DRIVER_DrawString(curr_panel->children[i].x, curr_panel->children[i].y, image_buffer, *curr_panel->children[i].element.label->string, *curr_panel->children[i].param, false);
	}
	/* Draw MENU UI */
	/* Only draw menu UI if last action was no later than UI_UPDATE_TIMEOUT */
	if(curr_time - last_ui_update_time < UI_UPDATE_TIMEOUT){
		/* Draw menu if active */
		if(UI_Interface_GetCurrentPanel(&curr_panel) == false){
			return false;
		}
		/* Get new active UI button if it was changed */
		if(last_ui_update_flag){
			Shared_param_API_Read(eSharedParamActiveUiButtonIndex, &curr_active_ui_button);
			last_ui_update_flag = false;
		}
		/* Decide on panel global position and alignment */
		uint16_t panel_x;
		uint16_t panel_y;
		if(curr_panel->vert_align == eVerticalAlignmentNone){
			panel_y = curr_panel->y;
		} else{
			//TODO: add protections
			panel_y = LCD_HEIGTH - (LCD_HEIGTH - curr_panel->children_amount * curr_panel->spacing_y) / 2 - curr_panel->spacing_y;
		}
		/* Calculate horizontal alignment of menu panel */
		if(curr_panel->hor_align == eHorizontalAlignmentNone){
			panel_x = curr_panel->x;
		} else{
			panel_x = curr_panel->x; //TODO: change
		}
		/* Draw all current active panel intems (buttons, labels ...) */
		uint32_t selectable_i = 0;
		for(size_t i = 0; i < curr_panel->children_amount; i++){
			switch(curr_panel->children[i].type){
				case(eUiElementTypeLabel): {
					sUiLabel_t label = (sUiLabel_t)(*curr_panel->children[i].element.label); //TODO: DONT COPy
					UI_DRIVER_DrawString(panel_x, panel_y, image_buffer, *label.string, *curr_panel->children[i].param, false);
				} break;
				case (eUiElementTypeButton): {
					sUiButton_t button = (sUiButton_t)(*curr_panel->children[i].element.button);
					UI_DRIVER_DrawButton(panel_x, panel_y, image_buffer, *button.string, *curr_panel->children[i].param, selectable_i == curr_active_ui_button);
					selectable_i++;
				}break;
				default: {
					/* Should not happen */
				}break;
			}
			panel_y -= curr_panel->spacing_y;
		}
	}
	return true;
}

/*
 * Selected button/parameter was changed - update all UI.
 */
static void UI_Updated(void){
	last_ui_update_time = HAL_GetTick();
	last_ui_update_flag = true;
}

