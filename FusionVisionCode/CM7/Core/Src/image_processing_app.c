#include "image_processing_app.h"
#include "shared_param_api.h"
#include "debug_api.h"
#include <stdlib.h>
#include "shared_mem_api.h"
#include "common.h"

#define __DEBUG_FILE_NAME__ "PROC"

/*
 * Width and height for which to draw edges.
 */
#define COMPUTE_WIDTH 480
#define COMPUTE_HEIGHT 320

#define EDGE_COLOUR 0xDD62

/*
 * Convert 2D coordinates to 1D array coordinates. //TODO: port to common.h
 */
#define PIXEL(image, x, y) *(image + (y) * LCD_WIDTH + (x))
#define PIXEL_GRAY(image, width, x, y) (*(image + (y) * width + (x)))

static uint8_t grayscale_buffer[LCD_HEIGTH*LCD_WIDTH] = {0};

/*
 * Coordinates for termo vision display offset as OV2640 and Lepton has different resolutions.
 */
#define TERMO_DISPLAY_OFFSET_Y ((LCD_HEIGTH - TERMO_RAW_HEIGTH*2) / 2)
#define TERMO_DISPLAY_OFFSET_X (0)

bool IMG_PROCESSING_APP_Init(){

	return true;
}

/*
 * Do one Sobel matrix operation on specific image_buffer coordinates.
 */
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

/*
 * Do one Roberts Cross matrix operation on specific image_buffer coordinates.
 */
static inline int16_t Roberts_Conv_Gx(uint8_t *image_buffer, uint16_t buffer_width, uint16_t x, uint16_t y){
	int16_t conv = PIXEL_GRAY(image_buffer, buffer_width, x, y) - PIXEL_GRAY(image_buffer, buffer_width, x-1, y-1);
	return conv;
}

static inline int16_t Roberts_Conv_Gy(uint8_t *image_buffer, uint16_t buffer_width, uint16_t x, uint16_t y){
	int16_t conv = PIXEL_GRAY(image_buffer, buffer_width, x-1, y) - PIXEL_GRAY(image_buffer, buffer_width, x, y-1);
	return conv;
}

/*
 * Convert full RGB565 image buffer to grayscale buffer.
 */
static inline void ConvertGrayscale(uint16_t *image_buffer, uint8_t *grayscale_buffer) {
	for(uint16_t y = 0; y < COMPUTE_HEIGHT; y += 1){
		for(uint16_t x = 0; x < COMPUTE_WIDTH; x += 1){
			int16_t pixel = PIXEL(image_buffer, x, y);
			/* To grayscale https://stackoverflow.com/questions/58449462/rgb565-to-grayscale */
			int8_t red = ((pixel & 0xF800)>>11); //TODO: could be faster using LUT
			int8_t green = ((pixel & 0x07E0)>>5);
			int8_t blue = (pixel & 0x001F);
			/* Correct formula should be
			 * uint8_t grayscale = (0.2126 * red) + (0.7152 * green / 2.0) + (0.0722 * blue);
			 * But using shift operations for rough approximation to increase speed.
			 * */
			uint8_t grayscale = (red >> 2) + (green >> 2) + (blue >> 4); //TODO: play around with this
			PIXEL(grayscale_buffer, x, y) = grayscale;
		}
	}
}

/*
 * Display edges from desired grayscale buffer on desired image_buffer in RGB565.
 */
static inline void DisplayContours(uint16_t *image_buffer, uint8_t *grayscale_buffer, uint16_t grayscale_width, uint16_t grayscale_heigth, eEdgeAlgorithm_t edge_algorithm, uint32_t threshold) {
	/* Convert image buffer to grayscale and consequentially perform desired edge algorithm */
	for(uint16_t y = 3; y < grayscale_heigth - 3; y += 1){ //TODO: make 3 dynamic - used for matrix operations to not exceed image_buffer limits
		for(uint16_t x = 3; x < grayscale_width - 3; x += 1){
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
			/* Draw edge if above threshold */
			if(sum > threshold){
				//TODO: fix workaround for lepton smaller view
				if(grayscale_width == TERMO_RAW_WIDTH) {
					/* If drawing from termo image - extend in by 3x2 */
					PIXEL(image_buffer, (x*3+TERMO_DISPLAY_OFFSET_X), (y*2+TERMO_DISPLAY_OFFSET_Y)-1) = EDGE_COLOUR;
					PIXEL(image_buffer, (x*3+TERMO_DISPLAY_OFFSET_X)+1, (y*2+TERMO_DISPLAY_OFFSET_Y)-1) = EDGE_COLOUR;
					PIXEL(image_buffer, (x*3+TERMO_DISPLAY_OFFSET_X)+2, (y*2+TERMO_DISPLAY_OFFSET_Y)-1) = EDGE_COLOUR;
					PIXEL(image_buffer, (x*3+TERMO_DISPLAY_OFFSET_X), (y*2+TERMO_DISPLAY_OFFSET_Y)) = EDGE_COLOUR;
					PIXEL(image_buffer, (x*3+TERMO_DISPLAY_OFFSET_X)+1, (y*2+TERMO_DISPLAY_OFFSET_Y)) = EDGE_COLOUR;
					PIXEL(image_buffer, (x*3+TERMO_DISPLAY_OFFSET_X)+2, (y*2+TERMO_DISPLAY_OFFSET_Y)) = EDGE_COLOUR;
				} else {
					/* Drawing from regular camera */
					PIXEL(image_buffer, x, y-1) = EDGE_COLOUR;
				}
			}
		}
	}
}
/*
 * Display raw termo image with desired threshold.
 */
static inline void DisplayTermo(uint16_t *image_buffer, uint32_t threshold) {
	for(uint8_t y = 0; y < TERMO_RAW_HEIGTH; y++) {
		for(uint8_t x = 0; x < TERMO_RAW_WIDTH; x++) {
			uint32_t termo_pixel_coord = y * TERMO_RAW_WIDTH + x;
			uint16_t termo_colour = *(uint8_t *)(SHARED_TERMO_BUF_START + termo_pixel_coord);
			/* Ignore values below threshold */
			if(termo_colour < threshold) {
				continue;
			}
			/* Extend by 3x2 grid */
			PIXEL(image_buffer, (x*3 + TERMO_DISPLAY_OFFSET_X), (y*2+TERMO_DISPLAY_OFFSET_Y)-1) = termo_colour;
			PIXEL(image_buffer, (x*3 + TERMO_DISPLAY_OFFSET_X)+1, (y*2+TERMO_DISPLAY_OFFSET_Y)-1) = termo_colour;
			PIXEL(image_buffer, (x*3 + TERMO_DISPLAY_OFFSET_X)+2, (y*2+TERMO_DISPLAY_OFFSET_Y)-1) = termo_colour;
			PIXEL(image_buffer, (x*3 + TERMO_DISPLAY_OFFSET_X), (y*2+TERMO_DISPLAY_OFFSET_Y)) = termo_colour;
			PIXEL(image_buffer, (x*3 + TERMO_DISPLAY_OFFSET_X)+1, (y*2+TERMO_DISPLAY_OFFSET_Y)) = termo_colour;
			PIXEL(image_buffer, (x*3 + TERMO_DISPLAY_OFFSET_X)+2, (y*2+TERMO_DISPLAY_OFFSET_Y)) = termo_colour;
		}
	}
}

/*
 * Find and display edges/contours on image_buffer.
 */
bool IMG_PROCESSING_APP_Compute(uint16_t *image_buffer){
	/* Get edge threshold */
	uint32_t edge_threshold = 0;
	Shared_param_API_Read(eSharedParamEdgeThreshold, &edge_threshold);
	/* Optimisation level for interlacing */
	uint32_t optim_level = eScreenOptimFirst;
	Shared_param_API_Read(eSharedParamScreenOptim, &optim_level);
	/* Get edge algorithm */
	uint32_t edge_algorithm = eEdgeAlgorithmSobel;
	Shared_param_API_Read(eSharedParamEdgeAlgorithm, &edge_algorithm);
	/* Convert image to grayscale and draw out */
	ConvertGrayscale(image_buffer, grayscale_buffer);
	DisplayContours(image_buffer, grayscale_buffer, COMPUTE_WIDTH, COMPUTE_HEIGHT, edge_algorithm, edge_threshold);
	return true;
}

/*
 * Draw termo view into image_buffer. Could be edge detection ,raw or threshold display.
 */
bool IMG_PROCESSING_APP_DrawTermo(uint16_t *image_buffer){
	/* Read termo display state */
	uint32_t termo_state = eTermoStateAutoThreshold;
	Shared_param_API_Read(eSharedParamTermoState, &termo_state);
	switch(termo_state) {
		case eTermoStateAutoThreshold: {
			/* Draw only pixels above automatically generated threshold */
			uint32_t min_captured_temperature = 0;
			uint32_t max_captured_temperature = 0;
			uint32_t avg_captured_temperature = 0;
			Shared_param_API_Read(eSharedParamMinCapturedTemperature, &min_captured_temperature);
			Shared_param_API_Read(eSharedParamMaxCapturedTemperature, &max_captured_temperature);
			Shared_param_API_Read(eSharedParamAvgCapturedTemperature, &avg_captured_temperature);
			/* Calculate automatic threshold */
			uint32_t threshold = avg_captured_temperature + (max_captured_temperature - avg_captured_temperature)/2;
			DisplayTermo(image_buffer, threshold);
		} break;
		case eTermoStateThreshold: {
			/* Draw only pixels above set threshold */
			uint32_t termo_threshold = 0;
			Shared_param_API_Read(eSharedParamTermoThreshold, &termo_threshold);
			DisplayTermo(image_buffer, termo_threshold);

		} break;
		case eTermoStateEdge: {
			/* Draw only edge.contour pixels */
			uint32_t edge_algorithm = eEdgeAlgorithmSobel;
			uint32_t edge_threshold = 10;
			//TODO: algorithm and threshold are hardcoded
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
