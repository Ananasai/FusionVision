/*
 * lepton_app.h
 *
 *  Created on: Dec 30, 2023
 *      Author: simon
 */

#ifndef INC_LEPTON_APP_H_
#define INC_LEPTON_APP_H_

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

extern SPI_HandleTypeDef hspi4;
extern TIM_HandleTypeDef htim3;
extern I2C_HandleTypeDef hi2c4;

void Lepton_APP_VsyncIrq(void);
bool Lepton_APP_Start(uint16_t *new_image_buffer);
void Lepton_APP_Run(uint8_t *flag);

#endif /* INC_LEPTON_APP_H_ */
