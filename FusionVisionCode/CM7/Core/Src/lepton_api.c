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

#define STATUS_REG 0x0002
#define POWER_REG  0x0000
#define COMMAND_ID_REG 0x0004
#define DATA_LEN_REG 0x0006

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

//TODO: figure out unions here
typedef union uLeptonStatusRegister_t {
	uint8_t buffer[2];
	__packed struct {
		unsigned int busy : 1;
		unsigned int boot_mode : 1;
		unsigned int boot_status : 1;
		unsigned int reserved : 5;
		unsigned int error_code : 8;
	}data;
}uLeptonStatusRegister_t;

typedef enum eLeptonCommandType_t {
	eLeptonCommandFirst = 0,
	eLeptonCommandGet = eLeptonCommandFirst,
	eLeptonCommandSet,
	eLeptonCommandRun,
	eLeptonCommandUndefined,
	eLeptonCommandLast
}eLeptonCommandType_t;

typedef enum eLeptonModuleId_t {
	eLeptonModuleFirst = 0,
	eLeptonModuleUndefined1= eLeptonModuleFirst,
	eLeptonModuleAGC,
	eLeptonModuleSYS,
	eLeptonModuleVID,
	eLeptonModuleUndefined2,
	eLeptonModuleUndefined3,
	eLeptonModuleUndefined4,
	eLeptonModuleUndefined5,
	eLeptonModuleOEM,
	eLeptonModuleUndefined6,
	eLeptonModuleUndefined7,
	eLeptonModuleUndefined8,
	eLeptonModuleReserved1,
	eLeptonModuleReserved2,
	eLeptonModuleRAD,
	eLeptonModuleReserved3,
	eLeptonModuleLast
}eLeptonModuleId_t;

/* Inspiration: https://github.com/groupgets/LeptonModule/blob/master/software/arduino_i2c/Lepton.ino#L210 */

static bool Lepton_API_SetRegister(uint16_t reg){ //Can be used for sending data too
	uint8_t tx_data[2] = {reg >> 8, reg & 0xFF};
	return HAL_I2C_Master_Transmit(&hi2c4, LEPTON_ADDRESS, tx_data, 2, 10) == HAL_OK;
}

static bool Lepton_API_ReadRegister(uint16_t *reg){
	uint8_t rx_data[2] = {0};
	if(HAL_I2C_Master_Receive(&hi2c4, LEPTON_ADDRESS, rx_data, 2, 10) != HAL_OK){
		return false;
	}
	*reg = (rx_data[0] << 8) | rx_data[1];
	return true;
}

static bool Lepton_API_SendCommand(eLeptonCommandType_t command,eLeptonModuleId_t module_id, uint8_t command_id){
	/* Wait for not busy */
	while(Lepton_API_CheckBusy() != true){

	}
	Lepton_API_SetRegister(COMMAND_ID_REG);
	uint16_t command_word = 0x0000;
	/* Set OEM bit */
	if((module_id == eLeptonModuleOEM) || (module_id == eLeptonModuleRAD)){
		command_word |= 0x4000;
	}
	/* Set module ID, command ID and command type*/
	command_word |= ((uint8_t)module_id << 8);
	command_word |= (command_id << 2);
	command_word |= (uint8_t)(command);
	return Lepton_API_SetRegister(command_word);
}

static bool Lepton_API_ReadData(){
	/* Wait for not busy */
	while(Lepton_API_CheckBusy() != true){

	}
	/* Check data length */
	Lepton_API_SetRegister(DATA_LEN_REG);
	uint16_t data_length = 0;
	Lepton_API_ReadRegister(&data_length);
	debug("Read data length: 0x%x\r\n", data_length);
	for(uint8_t i = 0; i < data_length; i++){
		uint16_t temp_data = 0x0000;
		Lepton_API_ReadRegister(&temp_data);
		debug("Read data: 0x%x\r\n", temp_data);
	}
	return true;
}

bool Lepton_API_SetGpio(void){
	//Lepton_API_SendCommand(eLeptonCommandGet, eLeptonModuleOEM, 0x54);
	//Lepton_API_ReadData();
	Lepton_API_SendCommand(eLeptonCommandRun, eLeptonModuleSYS, 0x04 >> 2);
	Lepton_API_ReadData();
	//Lepton_API_SendCommand(eLeptonCommandSet, eLeptonModuleOEM, 0x55);

	return true;
}

bool Lepton_API_CheckBusy(void){
	if(HAL_I2C_IsDeviceReady(&hi2c4, LEPTON_ADDRESS, 100, 1000) != HAL_OK){
		error("Lepton I2C unavailable\r\n");
		return false;
	}
	Lepton_API_SetRegister(STATUS_REG);
	uint16_t read_val = 0;
	Lepton_API_ReadRegister(&read_val);
	if((read_val & 0x01) == 0x01){
		/* Camera busy */
		return false;
	}
	else if((read_val & 0x02) == 0x00){
		/* Boot mode not sucessfull */
		return false;
	}
	else if((read_val & 0x04) == 0x00){
		/* Boot status unsucesfull */
		return false;
	}
	return true;
}
