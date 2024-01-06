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

void Lepton_API_SetReg(uint16_t reg);
uint16_t Lepton_API_ReadReg(uint8_t reg);
uint32_t Lepton_API_ReadData(void);
void Lepton_API_SendCommand(uint8_t module_id, uint8_t command_id, uint8_t command);

#endif /* INC_LEPTON_API_H_ */
