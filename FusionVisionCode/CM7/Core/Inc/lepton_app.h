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

extern SPI_HandleTypeDef hspi4;
extern TIM_HandleTypeDef htim3;

void Lepton_APP_VsyncIrq(void);
bool Lepton_APP_Start(void);
void Lepton_APP_Run(void);

#endif /* INC_LEPTON_APP_H_ */
