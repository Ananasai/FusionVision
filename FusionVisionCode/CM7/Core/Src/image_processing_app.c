/*
 * image_processing_app.c
 *
 *  Created on: Dec 17, 2023
 *      Author: simon
 */
#pragma GCC push_options
#pragma GCC optimize ("O3")

#include "image_processing_app.h"
#include "shared_param_api.h"
#include "debug_api.h"
#include <stdlib.h>

#define __DEBUG_FILE_NAME__ "PROC"

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 320

#define COMPUTE_WIDTH 480
#define COMPUTE_HEIGHT 320

#define EDGE_COLOUR 0xDD62

#define PIXEL(image, x, y) *(image + (y) * SCREEN_WIDTH + (x))
#define PIXEL_GRAY(image, x, y) (*(image + (y) * SCREEN_WIDTH + (x)))

static uint8_t gray_scale[480*320] = {0};
static uint16_t *image_buffer;
static uint32_t edge_threshold = 0;
static uint8_t loop_increase = 1;
static uint8_t line_start = 0;

bool IMG_PROCESSING_APP_Init(uint16_t *_image_buffer){
	if(_image_buffer == NULL){
		return false;
	}
	image_buffer = _image_buffer;
	return true;
}

/* https://homepages.inf.ed.ac.uk/rbf/HIPR2/sobel.htm
 * https://en.wikipedia.org/wiki/Prewitt_operator or
 * https://en.wikipedia.org/wiki/Sobel_operator
 * Sobel/Previt operator */
static inline int16_t Sobel_Conv_Gx(uint8_t *image_buffer, uint16_t x, uint16_t y){
	int16_t conv = PIXEL_GRAY(image_buffer, x-1, y-1) + 2*PIXEL_GRAY(image_buffer, x, y-1) + PIXEL_GRAY(image_buffer, x+1, y-1)
			+ -PIXEL_GRAY(image_buffer, x-1, y+1) + -2*PIXEL_GRAY(image_buffer, x, y+1) + -PIXEL_GRAY(image_buffer, x+1, y+1);
	return conv;
}

static inline int16_t Sobel_Conv_Gy(uint8_t *image_buffer, uint16_t x, uint16_t y){
	int16_t conv = PIXEL_GRAY(image_buffer, x-1, y-1) + -PIXEL_GRAY(image_buffer, x+1, y-1)
			+ 2*PIXEL_GRAY(image_buffer, x-1, y) + -2*PIXEL_GRAY(image_buffer, x+1, y)
			+ PIXEL_GRAY(image_buffer, x-1, y+1) + -PIXEL_GRAY(image_buffer, x+1, y+1);
	return conv;
}

/* https://homepages.inf.ed.ac.uk/rbf/HIPR2/roberts.htm
 * Roberts cross
 */
static inline int16_t Roberts_Conv_Gx(uint8_t *image_buffer, uint16_t x, uint16_t y){
	int16_t conv = PIXEL_GRAY(image_buffer, x, y) - PIXEL_GRAY(image_buffer, x-1, y-1);
	return conv;
}

static inline int16_t Roberts_Conv_Gy(uint8_t *image_buffer, uint16_t x, uint16_t y){
	int16_t conv = PIXEL_GRAY(image_buffer, x-1, y) - PIXEL_GRAY(image_buffer, x, y-1);
	return conv;
}

/* ARM DSP LIB https://community.st.com/t5/stm32-mcus/configuring-dsp-libraries-on-stm32cubeide/ta-p/49637 */
bool IMG_PROCESSING_APP_Compute(uint16_t *image_buffer){
	//TODO: dont read on every frame
	Shared_param_API_Read(eSharedParamEdgeThreshold, &edge_threshold);
	/* Optimisation level for interlacing */
	uint32_t optim_level = eScreenOptimFirst;
	Shared_param_API_Read(eSharedParamScreenOptim, &optim_level);
	switch(optim_level){
		case eScreenOptimNone: {
			line_start = 0;
			loop_increase = 1;
		}break;
		case eScreenOptimInterlacedAll:
		case eScreenOptimInterlacedProcessing:{
			loop_increase = 2;
			line_start = 1 ? line_start == 0 : 0;
		}break;
		default:{
			return false;
		}break;
	}
	/* Get edge algorithm */
	uint32_t edge_algorithm = eEdgeAlgorithmSobel;
	Shared_param_API_Read(eSharedParamEdgeAlgorithm, &edge_algorithm);
	/* Convert image buffer to grayscale and consequentially perform desired edge algorithm */
	for(uint16_t y = line_start; y < COMPUTE_HEIGHT; y += loop_increase){
		for(uint16_t x = 0; x < COMPUTE_WIDTH; x += 1){
			int16_t pixel = PIXEL(image_buffer, x, y);
			/* To grayscale https://stackoverflow.com/questions/58449462/rgb565-to-grayscale */
			int8_t red = ((pixel & 0xF800)>>11);
			int8_t green = ((pixel & 0x07E0)>>5);
			int8_t blue = (pixel & 0x001F);
			//uint8_t grayscale = (0.2126 * red) + (0.7152 * green / 2.0) + (0.0722 * blue);
			uint8_t grayscale = (red >> 2) + (green >> 2) + (blue >> 4); //TODO: play around with this
			//PIXEL(image_buffer, x, y) = (grayscale<<11)+(grayscale<<6)+grayscale;
			PIXEL(gray_scale, x, y) = grayscale;
			/* Perform edge detection */
			/* Only start at line two and leave empty border around for 3x3 grid algorithm*/
			if((y > 2) && (x > 1) && (x < 479)){
				uint32_t sum = 0;
				/* Apply desired edge algorithm */
				switch(edge_algorithm){
					case eEdgeAlgorithmRoberts: {
						sum = abs(Roberts_Conv_Gx(gray_scale, x, y-1)) + abs(Roberts_Conv_Gy(gray_scale, x, y-1));
					}break;
					case eEdgeAlgorithmSobel: {
						sum = abs(Sobel_Conv_Gx(gray_scale, x, y-1)) + abs(Sobel_Conv_Gy(gray_scale, x, y-1));
					}break;
					default:{
						error("Invalid edge algorithm\r\n");
					}return false;
				}
				if(sum > edge_threshold){
					//PIXEL(image_buffer, x, y-1) = EDGE_COLOUR;
					*(image_buffer + (y-1) * 480 + (x)) = EDGE_COLOUR;
				}
			}
		}
	}
	return true;
}

#pragma GCC pop_options
