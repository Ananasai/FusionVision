/*
 * fons.h
 *
 *  Created on: Dec 15, 2023
 *      Author: simon
 */

#ifndef INC_FONTS_H_
#define INC_FONTS_H_

#include <stdint.h>

typedef enum eFont_t {
	eFontFirst = 0,
	eFont16x29 = eFontFirst,
	eFont7x10,
	eFont11x18,
	eFontLast
}eFont_t;

typedef struct sFontDesc_t {
	uint8_t width;
	uint8_t height;
	const uint16_t *table;
}sFontDesc_t;

extern const uint16_t debug_image_320x240x16[76800];
extern const sFontDesc_t font_lut[eFontLast];

#endif /* INC_FONTS_H_ */
