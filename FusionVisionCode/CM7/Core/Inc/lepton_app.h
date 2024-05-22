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
extern I2C_HandleTypeDef hi2c4;
extern TIM_HandleTypeDef htim4;

void Lepton_APP_VsyncIrq(void);
bool Lepton_APP_Start(void);
void Lepton_APP_Run(void);

#endif /* INC_LEPTON_APP_H_ */
