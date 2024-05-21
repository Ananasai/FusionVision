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
#include "shared_mem_api.h"
#include "common.h"

#define __DEBUG_FILE_NAME__ "PROC"

#define COMPUTE_WIDTH 480
#define COMPUTE_HEIGHT 320

#define EDGE_COLOUR 0xDD62

#define PIXEL(image, x, y) *(image + (y) * LCD_WIDTH + (x))
#define PIXEL_GRAY(image, width, x, y) (*(image + (y) * width + (x)))

static uint8_t grayscale_buffer[LCD_HEIGTH*LCD_WIDTH] = {0};

bool IMG_PROCESSING_APP_Init(){

	return true;
}

/* https://homepages.inf.ed.ac.uk/rbf/HIPR2/sobel.htm
 * https://en.wikipedia.org/wiki/Prewitt_operator or
 * https://en.wikipedia.org/wiki/Sobel_operator
 * Sobel/Previt operator */
static inline int16_t Sobel_Conv_Gx(uint8_t *image_buffer, uint16_t buffer_width, uint16_t x, uint16_t y){
	int16_t conv = PIXEL_GRAY(image_buffer, buffer_width,  x-1, y-1) + 2*PIXEL_GRAY(image_buffer, buffer_width, x, y-1) + PIXEL_GRAY(image_buffer, buffer_width, x+1, y-1)
			+ -PIXEL_GRAY(image_buffer, buffer_width,  x-1, y+1) + -2*PIXEL_GRAY(image_buffer, buffer_width,  x, y+1) + -PIXEL_GRAY(image_buffer, buffer_width, x+1, y+1);
	return conv;
}

static inline int16_t Sobel_Conv_Gy(uint8_t *image_buffer, uint16_t buffer_width, uint16_t x, uint16_t y){
	int16_t conv = PIXEL_GRAY(image_buffer,buffer_width,  x-1, y-1) + -PIXEL_GRAY(image_buffer, buffer_width,  x+1, y-1)
			+ 2*PIXEL_GRAY(image_buffer, buffer_width, x-1, y) + -2*PIXEL_GRAY(image_buffer, buffer_width, x+1, y)
			+ PIXEL_GRAY(image_buffer, buffer_width, x-1, y+1) + -PIXEL_GRAY(image_buffer, buffer_width, x+1, y+1);
	return conv;
}

/* https://homepages.inf.ed.ac.uk/rbf/HIPR2/roberts.htm
 * Roberts cross
 */
static inline int16_t Roberts_Conv_Gx(uint8_t *image_buffer, uint16_t buffer_width, uint16_t x, uint16_t y){
	int16_t conv = PIXEL_GRAY(image_buffer, buffer_width, x, y) - PIXEL_GRAY(image_buffer, buffer_width, x-1, y-1);
	return conv;
}

static inline int16_t Roberts_Conv_Gy(uint8_t *image_buffer, uint16_t buffer_width, uint16_t x, uint16_t y){
	int16_t conv = PIXEL_GRAY(image_buffer, buffer_width, x-1, y) - PIXEL_GRAY(image_buffer, buffer_width, x, y-1);
	return conv;
}

static inline void ConvertGrayscale(uint16_t *image_buffer, uint8_t *grayscale_buffer) {
	for(uint16_t y = 0; y < COMPUTE_HEIGHT; y += 1){
		for(uint16_t x = 0; x < COMPUTE_WIDTH; x += 1){
			int16_t pixel = PIXEL(image_buffer, x, y);
			/* To grayscale https://stackoverflow.com/questions/58449462/rgb565-to-grayscale */
			int8_t red = ((pixel & 0xF800)>>11);
			int8_t green = ((pixel & 0x07E0)>>5);
			int8_t blue = (pixel & 0x001F);
			//uint8_t grayscale = (0.2126 * red) + (0.7152 * green / 2.0) + (0.0722 * blue);
			uint8_t grayscale = (red >> 2) + (green >> 2) + (blue >> 4); //TODO: play around with this
			//PIXEL(image_buffer, x, y) = (grayscale<<11)+(grayscale<<6)+grayscale;
			PIXEL(grayscale_buffer, x, y) = grayscale;
		}
	}
}

static inline void DisplayContours(uint16_t *image_buffer, uint8_t *grayscale_buffer, uint16_t grayscale_width, uint16_t grayscale_heigth, eEdgeAlgorithm_t edge_algorithm, uint32_t threshold) {
	/* Convert image buffer to grayscale and consequentially perform desired edge algorithm */
	for(uint16_t y = 2; y < grayscale_heigth - 2; y += 1){ //TODO: make 2 dynamic
		for(uint16_t x = 2; x < grayscale_width - 2; x += 1){
			/* Perform edge detection */
			uint32_t sum = 0;
			/* Apply desired edge algorithm */
			switch(edge_algorithm){
				case eEdgeAlgorithmRoberts: {
					sum = abs(Roberts_Conv_Gx(grayscale_buffer, grayscale_width, x, y-1)) + abs(Roberts_Conv_Gy(grayscale_buffer, grayscale_width, x, y-1));
				}break;
				case eEdgeAlgorithmSobel: {
					sum = abs(Sobel_Conv_Gx(grayscale_buffer, grayscale_width, x, y-1)) + abs(Sobel_Conv_Gy(grayscale_buffer, grayscale_width, x, y-1));
				}break;
				default:{
					error("Invalid edge algorithm\r\n");
				} return;
			}
			if(sum > threshold){
				//TODO: fix workaround for lepton smaller view
				if(grayscale_width == TERMO_RAW_WIDTH) {
					PIXEL(image_buffer, x*3, y*2-1) = EDGE_COLOUR;
					PIXEL(image_buffer, x*3+1, y*2-1) = EDGE_COLOUR;
					PIXEL(image_buffer, x*3+2, y*2-1) = EDGE_COLOUR;
					PIXEL(image_buffer, x*3, y*2) = EDGE_COLOUR;
					PIXEL(image_buffer, x*3+1, y*2) = EDGE_COLOUR;
					PIXEL(image_buffer, x*3+2, y*2) = EDGE_COLOUR;
				} else {
					PIXEL(image_buffer, x, y-1) = EDGE_COLOUR;
				}
			}
		}
	}
}

static inline void DisplayTermo(uint16_t *image_buffer, uint32_t threshold) {
	uint32_t display_offset_y = (LCD_HEIGTH - TERMO_RAW_HEIGTH*2) / 2;
	uint32_t display_offset_x = 0;
	for(uint8_t y = 0; y < TERMO_RAW_HEIGTH; y++) {
		for(uint8_t x = 0; x < TERMO_RAW_WIDTH; x++) {
			uint32_t termo_pixel_coord = y * TERMO_RAW_WIDTH + x;
			uint32_t image_buffer_coord = (y*2 + display_offset_y) * LCD_WIDTH + (x*3 + display_offset_x);
			uint8_t termo_colour = *(uint8_t *)(SHARED_TERMO_BUF_START + termo_pixel_coord);
			/* Ignore values below threshold */
			if(termo_colour < threshold) {
				continue;
			}
			*(image_buffer + image_buffer_coord) = termo_colour;
			/* Extend by 3x2 grid */
			*(image_buffer + image_buffer_coord + 1) = termo_colour;
			*(image_buffer + image_buffer_coord + 2) = termo_colour;
			*(image_buffer + image_buffer_coord + LCD_WIDTH) = termo_colour;
			*(image_buffer + image_buffer_coord + LCD_WIDTH + 1) = termo_colour;
			*(image_buffer + image_buffer_coord + LCD_WIDTH + 2) = termo_colour;
		}
	}
}

/* ARM DSP LIB https://community.st.com/t5/stm32-mcus/configuring-dsp-libraries-on-stm32cubeide/ta-p/49637 */
bool IMG_PROCESSING_APP_Compute(uint16_t *image_buffer){
	uint32_t edge_threshold = 0;
	Shared_param_API_Read(eSharedParamEdgeThreshold, &edge_threshold);
	/* Optimisation level for interlacing */
	uint32_t optim_level = eScreenOptimFirst;
	Shared_param_API_Read(eSharedParamScreenOptim, &optim_level);
//	switch(optim_level){
//		case eScreenOptimNone: {
//			line_start = 0;
//			loop_increase = 1;
//		}break;
//		case eScreenOptimInterlacedAll:
//		case eScreenOptimInterlacedProcessing:{
//			loop_increase = 2;
//			line_start = 1 ? line_start == 0 : 0;
//		}break;
//		default:{
//			return false;
//		}break;
//	}
	/* Get edge algorithm */
	uint32_t edge_algorithm = eEdgeAlgorithmSobel;
	Shared_param_API_Read(eSharedParamEdgeAlgorithm, &edge_algorithm);
	ConvertGrayscale(image_buffer, grayscale_buffer);
	DisplayContours(image_buffer, grayscale_buffer, COMPUTE_WIDTH, COMPUTE_HEIGHT, edge_algorithm, edge_threshold);
	return true;
}

bool IMG_PROCESSING_APP_DrawTermo(uint16_t *image_buffer){
	uint32_t termo_state = eTermoStatePassthrough;
	//Shared_param_API_Read(eSharedParamTermoState, &termo_state);
	switch(termo_state) {
		case eTermoStatePassthrough: {
			uint32_t min_captured_temperature = 0;
			uint32_t max_captured_temperature = 0;
			Shared_param_API_Read(eSharedParamMinCapturedTemperature, &min_captured_temperature);
			Shared_param_API_Read(eSharedParamMaxCapturedTemperature, &max_captured_temperature);
			uint32_t threshold = max_captured_temperature - 10;
			DisplayTermo(image_buffer, threshold);
			debug("%u %u\r\n", min_captured_temperature, max_captured_temperature);
		} break;
		case eTermoStateThreshold: {
			uint32_t termo_threshold = 0;
			Shared_param_API_Read(eSharedParamTermoThreshold, &termo_threshold);
			DisplayTermo(image_buffer, termo_threshold);

		} break;
		case eTermoStateEdge: {
			uint32_t edge_algorithm = eEdgeAlgorithmSobel;
			uint32_t edge_threshold = 10;
			//Shared_param_API_Read(eSharedParamEdgeAlgorithm, &edge_algorithm);
			//Shared_param_API_Read(eSharedParamEdgeThreshold, &edge_threshold);
			DisplayContours(image_buffer, (uint8_t *)(SHARED_TERMO_BUF_START), TERMO_RAW_WIDTH, TERMO_RAW_HEIGTH, edge_algorithm, edge_threshold);
		} break;
		default: {
			return false;
		} break;
	}
	return true;
}

#pragma GCC pop_options
