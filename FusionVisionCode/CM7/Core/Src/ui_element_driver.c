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
			if(((font_16x26[FONT_HEIGHT + y] >> i) & 0x01) == 0x01){
				//memset(image_buffer + SCREEN_WIDTH*y+i, UI_COLOUR, 2);
				*(image_buffer + SCREEN_WIDTH*y+i) = UI_COLOUR;
			}
		}
	}
	return true;
}


