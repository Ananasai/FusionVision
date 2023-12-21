/*
 * image_processing_app.c
 *
 *  Created on: Dec 17, 2023
 *      Author: simon
 */

#include "image_processing_app.h"
#include "shared_param_api.h"
#include <arm_math.h>
#include <stdlib.h>

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

#define COMPUTE_WIDTH 400
#define COMPUTE_HEIGHT 300

#define EDGE_COLOUR 0xFAFA

#define PIXEL(image, x, y) *(image + (y) * SCREEN_WIDTH + (x))
#define PIXEL_GRAY(image, x, y) (*(image + (y) * SCREEN_WIDTH + (x)))

static uint8_t gray_scale[480*320] = {0};

static int16_t Conv_Gx(uint8_t *image_buffer, uint16_t x, uint16_t y){
	int16_t conv = PIXEL_GRAY(image_buffer, x-1, y-1) + 2*PIXEL_GRAY(image_buffer, x, y-1) + PIXEL_GRAY(image_buffer, x+1, y-1)
			+ -PIXEL_GRAY(image_buffer, x-1, y+1) + -2*PIXEL_GRAY(image_buffer, x, y+1) + -PIXEL_GRAY(image_buffer, x+1, y+1);
	return conv;
}

static int16_t Conv_Gy(uint8_t *image_buffer, uint16_t x, uint16_t y){
	int16_t conv = PIXEL_GRAY(image_buffer, x-1, y-1) + -PIXEL_GRAY(image_buffer, x+1, y-1)
			+ 2*PIXEL_GRAY(image_buffer, x-1, y) + -2*PIXEL_GRAY(image_buffer, x+1, y)
			+ PIXEL_GRAY(image_buffer, x-1, y+1) + -PIXEL_GRAY(image_buffer, x+1, y+1);

	return conv;
}
/* ARM DSP LIB https://community.st.com/t5/stm32-mcus/configuring-dsp-libraries-on-stm32cubeide/ta-p/49637 */
bool IMG_PROCESSING_APP_Compute(uint16_t *image_buffer){
	uint32_t edge_threshold = 0;
	//TODO: dont read on every frame
	Shared_param_API_Read(eSharedParamEdgeThreshold, &edge_threshold);
	/* To grayscale https://stackoverflow.com/questions/58449462/rgb565-to-grayscale */
	for(uint16_t y = 0; y < COMPUTE_HEIGHT; y++){
		for(uint16_t x = 0; x < COMPUTE_WIDTH; x++){
			int16_t pixel = PIXEL(image_buffer, x, y);
			int8_t red = ((pixel & 0xF800)>>11);
			int8_t green = ((pixel & 0x07E0)>>5);
			int8_t blue = (pixel & 0x001F);
			//uint8_t grayscale = (0.2126 * red) + (0.7152 * green / 2.0) + (0.0722 * blue);
			uint8_t grayscale = (red >> 2) + (green >> 2) + (blue >> 4); //TODO: play around with this
			//PIXEL(image_buffer, x, y) = (grayscale<<11)+(grayscale<<6)+grayscale;
			PIXEL(gray_scale, x, y) = grayscale;
		}
	}
	/* https://homepages.inf.ed.ac.uk/rbf/HIPR2/sobel.htm
	 * https://en.wikipedia.org/wiki/Prewitt_operator or
	 * https://en.wikipedia.org/wiki/Sobel_operator
	 * for edge detection */
	for(uint16_t y = 1; y < COMPUTE_HEIGHT-1; y++){
		for(uint16_t x = 1; x < COMPUTE_WIDTH-1; x++){
			int32_t sum = abs(Conv_Gx(gray_scale, x, y)) + abs(Conv_Gy(gray_scale, x, y));
			if(sum > edge_threshold){
				//PIXEL(image_buffer, x, y) = 0xFFFF;
				*(image_buffer + (y) * 480 + (x)) = EDGE_COLOUR;
			}

		}
	}

	return true;
}
