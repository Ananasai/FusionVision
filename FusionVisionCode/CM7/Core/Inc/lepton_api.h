/*
 * lepton_api.h
 *
 *  Created on: Jan 6, 2024
 *      Author: simon
 */

#ifndef INC_LEPTON_API_H_
#define INC_LEPTON_API_H_

#include "main.h"
#include <stdbool.h>

extern I2C_HandleTypeDef hi2c4;

bool Lepton_API_CheckBusy(void);
bool Lepton_API_SetGpio(void);


#endif /* INC_LEPTON_API_H_ */
