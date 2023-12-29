/*
 * fons.h
 *
 *  Created on: Dec 15, 2023
 *      Author: simon
 */

#ifndef INC_FONTS_H_
#define INC_FONTS_H_

#include <stdint.h>

#define SCREEN_HEIGHT 320
#define SCREEN_WIDTH 480

/* SPECIAL SYMBOLS ONLY ON 11x18 FONT */ //TODO: maybe use special ui elements instead of ascii?
#define BATTERY_ICON_FULL_LEFT '\x7F'
#define BATTERY_ICON_FULL_MIDDLE '\x80'
#define BATTERY_ICON_FULL_RIGHT '\x81'
#define BATTERY_ICON_EMPTY_LEFT '\x82'
#define BATTERY_ICON_EMPTY_MIDDLE '\x83'
#define BATTERY_ICON_EMPTY_RIGHT '\x84'

typedef enum eFont_t {
	eFontFirst = 0,
	eFont16x29 = eFontFirst,
	eFont7x10,
	eFont11x18,
	eFontLast
}eFont_t;

typedef enum eAlignmentVertical_t { //TODO: misleading naming?
	eVerticalAlignmentFirst = 0,
	eVerticalAlignmentNone = eVerticalAlignmentFirst,
	eVerticalAlignmentTop,
	eVerticalAlignmentMiddle,
	eVerticalAlignmentBottom,
	eVerticalAlignmentLast
}eAlignmentVertical_t;

typedef enum eAlignmentHorizontal_t {
	eHorizontalAlignmentFirst = 0,
	eHorizontalAlignmentNone = eHorizontalAlignmentFirst,
	eHorizontalAlignmentLeft,
	eHorizontalAlignmentCenter,
	eHorizontalAlignmentRight,
	eHorizonalAlignmentLast
}eAlignmentHorizontal_t;

typedef struct sTextParam_t {
	eFont_t font;
	eAlignmentHorizontal_t alignment;
}sTextParam_t;

typedef struct sFontDesc_t {
	uint8_t width;
	uint8_t height;
	const uint16_t *table;
}sFontDesc_t;

extern const uint16_t splash_screen[153600];
extern const sFontDesc_t font_lut[eFontLast];

#endif /* INC_FONTS_H_ */
