#include "ui_element_driver.h"
#include <string.h>

#define MAX_CHAR_PER_LINE 10
#define UI_COLOUR 0xFFFF
#define UI_COLOUR_INVERTED 0x0000

#define PIXEL(x, y, buffer) (buffer + SCREEN_WIDTH*(y) + (x))

/*
 * Draw a single word character to screen at location with desired font.
 * if invert == false - will draw text as white, else - text as black and background white.
 */
bool UI_DRIVER_DrawCharacter(uint16_t loc_x, uint16_t loc_y, uint16_t *image_buffer, char character, eFont_t font, bool invert){
	const uint16_t *curr_font_table = font_lut[font].table;
	uint8_t curr_font_height = font_lut[font].height;
	uint8_t curr_font_width = font_lut[font].width;
	/* Draw character from font bytemap */
	for(uint8_t y = 0; y < curr_font_height; y++){
		for(uint8_t i = 0; i < curr_font_width; i++){
			uint32_t pixel_loc = i + loc_x + LCD_WIDTH*y + LCD_WIDTH * loc_y;
			/* Check if pixel coords are valid */
			if(pixel_loc >= (LCD_WIDTH * LCD_HEIGTH)){
				continue;
			}
			uint16_t new_colour = 0x0000; //TODO: remove cus is a magic variable to not crash ???
			if(((*(curr_font_table + (((character - 32 + 1) * curr_font_height) - 1 - y)) >> (i + 16 - curr_font_width)) & 0x01) == 0x01){
				*(image_buffer + pixel_loc) = invert ? UI_COLOUR_INVERTED : UI_COLOUR;
			}else if(invert){
				*(image_buffer + pixel_loc) = UI_COLOUR;
			}
		}
	}
	return true;
}

/*
 * Draw a string of characters to screen at location with desired font and alignment.
 */
bool UI_DRIVER_DrawString(uint16_t loc_x, uint16_t loc_y, uint16_t *image_buffer, sString_t string, sTextParam_t text_param, bool invert){
	uint8_t curr_font_width = font_lut[text_param.font].width;
	uint8_t curr_font_height = font_lut[text_param.font].height;
	/* Check if valid coordinates */
	if((loc_x > LCD_WIDTH) || (loc_y > LCD_HEIGTH)){ //TODO: || (loc_x < length * curr_font_width)
		return false;
	}
	/* Adjust text coordinates based on selected alignment */
	if(text_param.alignment == eHorizontalAlignmentCenter) { //TODO: right and left alignment
		loc_x += (strlen(string.text) * curr_font_width) >> 1;
	}
	/* Will split text lines every 10 characters */
	size_t lines = 1; //TODO: make several lines procedural
	if(strlen(string.text) > MAX_CHAR_PER_LINE){
		lines = 2;
	}
	/* Draws every character in every line */
	for(size_t line = 0; line < lines; line++){
		for(size_t i = 0; i < strlen(string.text); i++){ //TODO: use string.length
			size_t char_i = line * MAX_CHAR_PER_LINE + i;
			if(char_i == strlen(string.text)){
				return true;
			}
			UI_DRIVER_DrawCharacter(loc_x - i * curr_font_width, loc_y - line*curr_font_height, image_buffer, *(string.text + char_i), text_param.font, invert);
		}
	}
	return true;
}

/*
 * Draw a button to screen. If selected - colour in background instead of text.
 */
bool UI_DRIVER_DrawButton(uint16_t loc_x, uint16_t loc_y, uint16_t *image_buffer, sString_t string, sTextParam_t text_param, bool selected){
	if((loc_x >LCD_WIDTH) || (loc_y > LCD_HEIGTH)){
		return false;
	}
	UI_DRIVER_DrawString(loc_x, loc_y, image_buffer, string, text_param, selected);
	return true;
}
