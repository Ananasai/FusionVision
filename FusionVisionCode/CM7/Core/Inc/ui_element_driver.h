/*
 * ui_element_driver.h
 *
 *  Created on: Dec 15, 2023
 *      Author: simon
 */

#ifndef INC_UI_ELEMENT_DRIVER_H_
#define INC_UI_ELEMENT_DRIVER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "fonts.h"

bool UI_DRIVER_DrawCharacter(uint16_t loc_x, uint16_t loc_y, uint16_t *image_buffer, char character, eFont_t font, bool invert);
bool UI_DRIVER_DrawString(uint16_t loc_x, uint16_t loc_y, uint16_t *image_buffer, const char *text, size_t length, eFont_t font);
bool UI_DRIVER_DrawButton(uint16_t loc_x, uint16_t loc_y, uint16_t *image_buffer, const char *text, size_t length, eFont_t font, bool selected);


#endif /* INC_UI_ELEMENT_DRIVER_H_ */
