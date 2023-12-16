/*
 * ui_app.c
 *
 *  Created on: Dec 16, 2023
 *      Author: simon
 */
#include "ui_app.h"
#include "ui_element_driver.h"
#include "debug_api.h"

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

typedef struct sUiElement_t {
	uint16_t x;
	uint16_t y;
}sUiElement_t;

static const sUiElement_t UI_Elements[] = {
		{.x = 100, .y = 20}
};

bool UI_APP_DrawAll(uint16_t *image_buffer){
	for(uint8_t i = 0; i < ARRAY_LENGTH(UI_Elements); i++){
		UI_DRIVER_DrawString(UI_Elements[i].x, UI_Elements[i].y, image_buffer, "Labas", 5);
	}
	return true;
}

bool UI_APP_Printout(uint16_t *image_buffer){
	for(uint16_t y = 0; y < 320; y++){
		for(uint16_t x = 0; x < 480; x++){
			DEBUG_API_LOG("%d,", NULL, NULL, *(image_buffer + x + y * 480));
			if(x % 120 == 0){
				DEBUG_API_LOG("\r\n", NULL, NULL);
			}
		}
		DEBUG_API_LOG("\r\n", NULL, NULL);
	}
	return true;
}

