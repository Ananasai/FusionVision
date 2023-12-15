/*
 * ui_element_driver.c
 *
 *  Created on: Dec 15, 2023
 *      Author: simon
 */

#include "ui_element_driver.h"
#include "fonts.h"

#define UI_COLOUR 0xFFFF
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

#define FONT_WIDTH 16
#define FONT_HEIGHT 26

bool UI_DRIVER_DrawCharacter(uint16_t loc_x, uint16_t loc_y, uint16_t *image_buffer, char character){
	for(uint8_t y = 0; y < FONT_HEIGHT; y++){
		for(uint8_t i = 0; i < FONT_WIDTH; i++){
			if(((font_16x26[((character - 32 + 1) * FONT_HEIGHT) - 1 - y] >> i) & 0x01) == 0x01){
				*(image_buffer + SCREEN_WIDTH*y+i + SCREEN_WIDTH * loc_y + loc_x) = UI_COLOUR;
			}
		}
	}
	return true;
}

bool UI_DRIVER_DrawString(uint16_t loc_x, uint16_t loc_y, uint16_t *image_buffer, const char *text, size_t length){
	if((loc_x > SCREEN_WIDTH) || (loc_y > SCREEN_HEIGHT) || (loc_x < length * FONT_WIDTH)){
		return false;
	}
	for(size_t i = 0; i < length; i++){
		UI_DRIVER_DrawCharacter(loc_x - i * FONT_WIDTH, loc_y, image_buffer, *(text + i));
	}
	return true;
}


