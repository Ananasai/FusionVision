/*
 * ui_element_driver.c
 *
 *  Created on: Dec 15, 2023
 *      Author: simon
 */

#include "ui_element_driver.h"
#include <string.h>

#define UI_COLOUR 0xFFFF
#define UI_COLOUR_INVERTED 0x0000
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

#define PIXEL(x, y, buffer) (buffer + SCREEN_WIDTH*(y) + (x))

bool UI_DRIVER_DrawCharacter(uint16_t loc_x, uint16_t loc_y, uint16_t *image_buffer, char character, eFont_t font, bool invert){
	const uint16_t *curr_font_table = font_lut[font].table;
	uint8_t curr_font_height = font_lut[font].height;
	uint8_t curr_font_width = font_lut[font].width;
	for(uint8_t y = 0; y < curr_font_height; y++){
		for(uint8_t i = 0; i < curr_font_width; i++){
			uint32_t pixel_loc = i + loc_x + SCREEN_WIDTH*y + SCREEN_WIDTH * loc_y;
			if(pixel_loc >= (SCREEN_WIDTH * SCREEN_HEIGHT)){
				continue;
			}
			uint16_t new_colour = 0x0000; //TODO: remove cus is a magic variable to not crash
			if(((*(curr_font_table + (((character - 32 + 1) * curr_font_height) - 1 - y)) >> (i + 16 - curr_font_width)) & 0x01) == 0x01){
				*(image_buffer + pixel_loc) = invert ? UI_COLOUR_INVERTED : UI_COLOUR;
			}else if(invert){
				*(image_buffer + pixel_loc) = UI_COLOUR;
			}
			//*(image_buffer + SCREEN_WIDTH*y+i + SCREEN_WIDTH * loc_y + loc_x) = new_colour;
		}
	}
	return true;
}

bool UI_DRIVER_DrawString(uint16_t loc_x, uint16_t loc_y, uint16_t *image_buffer, sString_t string, sTextParam_t text_param, bool invert){
	uint8_t curr_font_width = font_lut[text_param.font].width;
	if((loc_x > SCREEN_WIDTH) || (loc_y > SCREEN_HEIGHT)){ //TODO: || (loc_x < length * curr_font_width)
		return false;
	}
	if(text_param.alignment == eAlignmentCenter) { //TODO: right and left alignment
		loc_x += (strlen(string.text) * curr_font_width) >> 1;
	}
	for(size_t i = 0; i < strlen(string.text); i++){ //TODO: string.length
		UI_DRIVER_DrawCharacter(loc_x - i * curr_font_width, loc_y, image_buffer, *(string.text + i), text_param.font, invert);
	}
	return true;
}

bool UI_DRIVER_DrawButton(uint16_t loc_x, uint16_t loc_y, uint16_t *image_buffer, sString_t string, sTextParam_t text_param, bool selected){
	if((loc_x > SCREEN_WIDTH) || (loc_y > SCREEN_HEIGHT)){
		return false;
	}
	UI_DRIVER_DrawString(loc_x, loc_y, image_buffer, string, text_param, selected);
	return true;
}


