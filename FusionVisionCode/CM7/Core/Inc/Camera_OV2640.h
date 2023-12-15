/*
 * Camera_OV2640.h
 *
 *  Created on: Dec 15, 2023
 *      Author: auris
 */

#ifndef INC_CAMERA_OV2640_H_
#define INC_CAMERA_OV2640_H_

#include "stm32h7xx_hal.h"

void     ov2640_Init(uint16_t DeviceAddr);
void     CAMERA_IO_Write(uint8_t addr, uint8_t reg, uint8_t value);

#endif /* INC_CAMERA_OV2640_H_ */
