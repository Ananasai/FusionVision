/*
 * lepton_api.c
 *
 *  Created on: Jan 6, 2024
 *      Author: simon
 */

#include "lepton_api.h"
#include "debug_api.h"

#define __DEBUG_FILE_NAME__ "LPTD"

#define LEPTON_ADDRESS 0x54
#define AGC (0x01)
#define SYS (0x02)
#define VID (0x03)
#define OEM (0x08)

#define GET (0x00)
#define SET (0x01)
#define RUN (0x02)

#define STATUS_REG 0x0002
#define POWER_REG  0x0000
#define COMMAND_ID_REG 0x0004
#define DATA_LEN_REG 0x0006

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

/* Inspiration: https://github.com/groupgets/LeptonModule/blob/master/software/arduino_i2c/Lepton.ino#L210 */

bool Lepton_API_CheckBusy(void){
	HAL_StatusTypeDef ready = HAL_OK;
	for(uint8_t i = 0; i < 5; i++){
		ready = HAL_I2C_IsDeviceReady(&hi2c4, 0x54, 100, 1000);
		if(ready != HAL_OK){
			error("Not ready\r\n");

		}
		else{
			debug("ready 0x54\r\n");
		}
	}
	for(uint8_t i = 0; i < 5; i++){
		ready = HAL_I2C_IsDeviceReady(&hi2c4, 0x2A, 100, 1000);
			if(ready != HAL_OK){
				error("Not ready\r\n");

			}
			else{
				debug("ready 0x2A\r\n");
			}
		}

	return false;
	uint8_t tx_data[2] = {STATUS_REG >> 8, STATUS_REG & 0xFF};
	HAL_StatusTypeDef state = HAL_OK;
	state = HAL_I2C_Master_Transmit(&hi2c4, LEPTON_ADDRESS, tx_data, 2, 10);
	if(state != HAL_OK){
		return false;
	}
	uint8_t rx_data[2] = {0};
	if(HAL_I2C_Master_Receive(&hi2c4, LEPTON_ADDRESS, rx_data, 2, 10) !=HAL_OK){
		return false;
	}
	return true;
}

void Lepton_API_SetReg(uint16_t reg) {
  uint8_t data[] = {reg >> 8 & 0xff, reg & 0xff};
  HAL_I2C_Master_Transmit(&hi2c4, LEPTON_ADDRESS, data, 2, 10);
}


uint16_t Lepton_API_ReadReg(uint8_t reg) {
  uint8_t rx_data[2] = {0};
  Lepton_API_SetReg(reg);
  HAL_I2C_Master_Receive(&hi2c4, LEPTON_ADDRESS, rx_data, 2, 10);
  return (rx_data[0] << 8) | rx_data[1];
}

uint32_t Lepton_API_ReadData(void) {
  uint8_t payload_length;

  while (Lepton_API_ReadReg(0x2) & 0x01) {
    debug("Busy\r\n");
  }

  payload_length = Lepton_API_ReadReg(0x6);
  debug("payload_length %d\r\n", payload_length);
  uint8_t rx_data[payload_length];
  HAL_I2C_Master_Receive(&hi2c4, LEPTON_ADDRESS, rx_data, payload_length, 10);
  return (rx_data[0] << 24) | (rx_data[1] << 16) | (rx_data[2] << 8) | (rx_data[3]);
}

void Lepton_API_SendCommand(uint8_t module_id, uint8_t command_id, uint8_t command){
	//byte error;
	// Command Register is a 16-bit register located at Register Address 0x0004
	uint8_t tx_data[4] = {0x00, 0x04, 0x00, 0x00};

	if (module_id == 0x08) { //OEM module ID
		tx_data[2] = 0x48;
	} else {
		tx_data[2] = module_id & 0x0f;
	}
	tx_data[3] =((command_id << 2 ) & 0xfc) | (command & 0x3);
	HAL_I2C_Master_Transmit(&hi2c4, LEPTON_ADDRESS, tx_data, 2, 10);
}
