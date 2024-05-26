#include "lepton_api.h"
#include "debug_api.h"
#include <stdlib.h>

#define __DEBUG_FILE_NAME__ "LPTD"

#define LEPTON_ADDRESS 0x54

#define STATUS_REG 0x0002
#define POWER_REG  0x0000
#define COMMAND_ID_REG 0x0004
#define DATA_LEN_REG 0x0006
#define DATA0_REG 0x0008
#define DATA1_REG 0x000A

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))
#define ENUM_TO_STRING(enum_val) [enum_val] = #enum_val

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

typedef enum eLeptonError_t {
	LEP_OK = 0, /* Camera ok */
	LEP_COMM_OK = LEP_OK, /* Camera comm ok (same as LEP_OK) */
	LEP_ERROR = 1, /* Camera general error */
	LEP_NOT_READY = 2, /* Camera not ready error */
	LEP_RANGE_ERROR = 3, /* Camera range error */
	LEP_CHECKSUM_ERROR = 4, /* Camera checksum error */
	LEP_BAD_ARG_POINTER_ERROR = 5, /* Camera Bad argument error */
	LEP_DATA_SIZE_ERROR = 6, /* Camera byte count error */
	LEP_UNDEFINED_FUNCTION_ERROR = 7, /* Camera undefined function error */
	LEP_FUNCTION_NOT_SUPPORTED = 8, /* Camera function not yet supported error */
	LEP_DATA_OUT_OF_RANGE_ERROR = 9, /* Camera input DATA is out of valid range error */
	LEP_COMMAND_NOT_ALLOWED = 11, /* Camera unable to execute command due to current camera
	state */
	/* OTP access errors */
	LEP_OTP_WRITE_ERROR = 15, /*!< Camera OTP write error */
	LEP_OTP_READ_ERROR = 16, /* double bit error detected (uncorrectible) */
	LEP_OTP_NOT_PROGRAMMED_ERROR = 18, /* Flag read as non-zero */
	/* I2C Errors */
	LEP_ERROR_I2C_BUS_NOT_READY = 20, /* I2C Bus Error - Bus Not Avaialble */
	LEP_ERROR_I2C_BUFFER_OVERFLOW = 22, /* I2C Bus Error - Buffer Overflow */
	LEP_ERROR_I2C_ARBITRATION_LOST = 23, /* I2C Bus Error - Bus Arbitration Lost */
	LEP_ERROR_I2C_BUS_ERROR = 24, /* I2C Bus Error - General Bus Error */
	LEP_ERROR_I2C_NACK_RECEIVED = 25, /* I2C Bus Error - NACK Received */
	LEP_ERROR_I2C_FAIL = 26, /* I2C Bus Error - General Failure */
	/* Processing Errors */
	LEP_DIV_ZERO_ERROR = 80, /* Attempted div by zero */
	/* Comm Errors */
	LEP_COMM_PORT_NOT_OPEN = 101, /* Comm port not open */
	LEP_COMM_INVALID_PORT_ERROR = 102, /* Comm port no such port error */
	LEP_COMM_RANGE_ERROR = 103, /* Comm port range error */
	LEP_ERROR_CREATING_COMM = 104, /* Error creating comm */
	LEP_ERROR_STARTING_COMM = 105, /* Error starting comm */
	LEP_ERROR_CLOSING_COMM = 106, /* Error closing comm */
	LEP_COMM_CHECKSUM_ERROR = 107, /* Comm checksum error */
	LEP_COMM_NO_DEV = 108, /* No comm device */
	LEP_TIMEOUT_ERROR = 109, /* Comm timeout error */
	LEP_COMM_ERROR_WRITING_COMM = 110, /* Error writing comm */
	LEP_COMM_ERROR_READING_COMM = 111, /* Error reading comm */
	LEP_COMM_COUNT_ERROR = 112, /* Comm byte count error */
	/* Other Errors */
	LEP_OPERATION_CANCELED = 126, /* Camera operation canceled */
	LEP_UNDEFINED_ERROR_CODE = 127 /* Undefined error */
} eLeptonError_t;

static const char* lepton_error_strings[] = {
	ENUM_TO_STRING(LEP_OK),
	ENUM_TO_STRING(LEP_COMM_OK),
	ENUM_TO_STRING(LEP_ERROR),
	ENUM_TO_STRING(LEP_NOT_READY),
	ENUM_TO_STRING(LEP_RANGE_ERROR),
	ENUM_TO_STRING(LEP_CHECKSUM_ERROR),
	ENUM_TO_STRING(LEP_BAD_ARG_POINTER_ERROR),
	ENUM_TO_STRING(LEP_DATA_SIZE_ERROR),
	ENUM_TO_STRING(LEP_UNDEFINED_FUNCTION_ERROR),
	ENUM_TO_STRING(LEP_FUNCTION_NOT_SUPPORTED),
	ENUM_TO_STRING(LEP_DATA_OUT_OF_RANGE_ERROR),
	ENUM_TO_STRING(LEP_COMMAND_NOT_ALLOWED),
	ENUM_TO_STRING(LEP_OTP_WRITE_ERROR),
	ENUM_TO_STRING(LEP_OTP_READ_ERROR),
	ENUM_TO_STRING(LEP_OTP_NOT_PROGRAMMED_ERROR),
	ENUM_TO_STRING(LEP_ERROR_I2C_BUS_NOT_READY),
	ENUM_TO_STRING(LEP_ERROR_I2C_BUFFER_OVERFLOW),
	ENUM_TO_STRING(LEP_ERROR_I2C_ARBITRATION_LOST),
	ENUM_TO_STRING(LEP_ERROR_I2C_BUS_ERROR),
	ENUM_TO_STRING(LEP_ERROR_I2C_NACK_RECEIVED),
	ENUM_TO_STRING(LEP_ERROR_I2C_FAIL),
	ENUM_TO_STRING(LEP_DIV_ZERO_ERROR),
	ENUM_TO_STRING(LEP_COMM_PORT_NOT_OPEN),
	ENUM_TO_STRING(LEP_COMM_INVALID_PORT_ERROR),
	ENUM_TO_STRING(LEP_COMM_RANGE_ERROR),
	ENUM_TO_STRING(LEP_ERROR_CREATING_COMM),
	ENUM_TO_STRING(LEP_ERROR_STARTING_COMM),
	ENUM_TO_STRING(LEP_ERROR_CLOSING_COMM),
	ENUM_TO_STRING(LEP_COMM_CHECKSUM_ERROR),
	ENUM_TO_STRING(LEP_COMM_NO_DEV),
	ENUM_TO_STRING(LEP_TIMEOUT_ERROR),
	ENUM_TO_STRING(LEP_COMM_ERROR_WRITING_COMM),
	ENUM_TO_STRING(LEP_COMM_ERROR_READING_COMM),
	ENUM_TO_STRING(LEP_COMM_COUNT_ERROR),
	ENUM_TO_STRING(LEP_OPERATION_CANCELED),
	ENUM_TO_STRING(LEP_UNDEFINED_ERROR_CODE),
};

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

static bool Lepton_API_WriteRegister(uint16_t reg, uint16_t data){
	while(Lepton_API_CheckBusy() != true){

	}
	uint8_t tx_data[4] = {reg >> 8, reg & 0xFF, data >> 8, data & 0xFF};
	return HAL_I2C_Master_Transmit(&hi2c4, LEPTON_ADDRESS, tx_data, 4, 10) == HAL_OK;
}

static bool Lepton_API_ReadRegister(uint16_t reg, uint16_t *out){
	uint8_t tx_data[4] = {reg >> 8, reg & 0xFF};
	HAL_I2C_Master_Transmit(&hi2c4, LEPTON_ADDRESS, tx_data, 2, 10);
	uint8_t rx_data[2] = {0};
	if(HAL_I2C_Master_Receive(&hi2c4, LEPTON_ADDRESS, rx_data, 2, 10) != HAL_OK){
		return false;
	}
	*out = (rx_data[0] << 8) | rx_data[1];
	return true;
}

/* IN datasheet: base is command base */
static bool Lepton_API_SendCommand(eLeptonCommandType_t command, eLeptonModuleId_t module_id, uint8_t command_base){
	/* Wait for not busy */
	while(Lepton_API_CheckBusy() != true){

	}
	//Lepton_API_WriteRegister(DATA_LEN_REG, 0x01); //TODO: not needed?
	uint16_t command_word = 0x0000;
	/* Set OEM bit */
	if((module_id == eLeptonModuleOEM) || (module_id == eLeptonModuleRAD)){
		command_word |= 0x4000;
	}
	/* Set module ID, command ID and command type*/
	command_word |= ((uint8_t)module_id << 8);
	/* No shifting by two, because datasheet command_id base specifies unshifted value */
	command_word |= (command_base);
	command_word |= (uint8_t)(command);
	return Lepton_API_WriteRegister(COMMAND_ID_REG, command_word);
}

static bool Lepton_API_ReadData(){
	/* Wait for not busy */
	while(Lepton_API_CheckBusy() != true){

	}
	/* Check data length */
	uint16_t data_length = 0;
	Lepton_API_ReadRegister(DATA_LEN_REG, &data_length);
	debug("Read data length: 0x%x\r\n", data_length);
	for(uint8_t i = 0; i < data_length; i++){
		uint16_t temp_data = 0x0000;
		Lepton_API_ReadRegister(DATA_LEN_REG + i + 1, &temp_data); //TODO: WILL NOT WORK BECAUSE NO INCREMENT ON SEPERATE REQUESTS
		debug("Read data: 0x%x\r\n", temp_data);
	}
	return true;
}
//TODO: is not working?
bool Lepton_API_SetGpio(void){
	Lepton_API_SendCommand(eLeptonCommandGet, eLeptonModuleOEM, 0x54);
	Lepton_API_ReadData();

	Lepton_API_WriteRegister(DATA_LEN_REG, 0x0002);
	Lepton_API_WriteRegister(DATA0_REG, 0x0001);
	Lepton_API_SendCommand(eLeptonCommandSet, eLeptonModuleOEM, 0x54);

	Lepton_API_SendCommand(eLeptonCommandGet, eLeptonModuleOEM, 0x54);
	Lepton_API_ReadData();
	return true;
}

bool Lepton_API_EnableAGC(void){
	Lepton_API_SendCommand(eLeptonCommandGet, eLeptonModuleAGC, 0x00);
	Lepton_API_ReadData();

	Lepton_API_WriteRegister(DATA_LEN_REG, 0x0001);
	Lepton_API_WriteRegister(DATA0_REG, 0x0001);
	Lepton_API_SendCommand(eLeptonCommandSet, eLeptonModuleAGC, 0x00);

	Lepton_API_SendCommand(eLeptonCommandGet, eLeptonModuleAGC, 0x00);
	Lepton_API_ReadData();
	return true;
}

bool Lepton_API_CheckBusy(void){
	if(HAL_I2C_IsDeviceReady(&hi2c4, LEPTON_ADDRESS, 100, 1000) != HAL_OK){
		error("Lepton I2C unavailable\r\n");
		return false;
	}
	uint16_t read_val = 0;
	Lepton_API_ReadRegister(STATUS_REG, &read_val);
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
	/* Check error message 	*/
	if((read_val >> 8) != 0x00){
		error("Lepton repsonse error: %s %d\r\n", lepton_error_strings[abs((int8_t)(read_val >> 8))], (int8_t)(read_val >> 8));
	}
	return true;
}
