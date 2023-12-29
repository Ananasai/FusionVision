/*
 * image_processing_app.h
 *
 *  Created on: Dec 17, 2023
 *      Author: simon
 */

#ifndef INC_IMAGE_PROCESSING_APP_H_
#define INC_IMAGE_PROCESSING_APP_H_

#include <stdbool.h>
#include <stdint.h>

bool IMG_PROCESSING_APP_Init(uint16_t *_image_buffer);
bool IMG_PROCESSING_APP_Compute(uint16_t *image_buffer);

#endif /* INC_IMAGE_PROCESSING_APP_H_ */
