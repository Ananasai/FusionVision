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

typedef enum eAlignment_t {
	eAlignmentFirst = 0,
	eAlignmentLeft = eAlignmentFirst,
	eAlignmentCenter,
	eAlignmentRight,
	eAlignemntLast
}eAlignment_t;

typedef struct sTextParam_t {
	eFont_t font;
	eAlignment_t alignment;
}sTextParam_t;

typedef struct sFontDesc_t {
	uint8_t width;
	uint8_t height;
	const uint16_t *table;
}sFontDesc_t;

extern const uint16_t splash_screen[153600];
extern const sFontDesc_t font_lut[eFontLast];

#endif /* INC_FONTS_H_ */
