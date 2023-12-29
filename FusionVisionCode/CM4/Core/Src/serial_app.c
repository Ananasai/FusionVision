/*
 * serial_app.c
 *
 *  Created on: Dec 28, 2023
 *      Author: simon
 */
#include "serial_app.h"
#include "debug_api.h"
#include "serial_commands.h"
#include <cmsis_os2.h>
#include <string.h>

#define __DEBUG_FILE_NAME__ "SRL"

#define FLAGS_RX 0x01
#define FLAGS_ALL FLAGS_RX
#define CMD_DELIMITER "\r\n"
#define CMD_DELIMITER_LEN 2
#define RX_CYCL_BUF_LEN 50
#define RX_BUFF_MAX_LEN 50

 //TODO: implement proper cyclic buffer
static uint8_t rx_cyclic_buff[RX_CYCL_BUF_LEN] = {0};
static uint8_t rx_cyclic_buff_head = 0;
static uint8_t rx_cyclic_buff_tail = 0;

static uint8_t rx_buffer[RX_BUFF_MAX_LEN] = {0};
static uint8_t rx_buffer_index = 0;
static uint8_t rx_byte = 0;
static uint8_t delimiter_index = 0;

static const osThreadAttr_t serial_thread_attr = {
	.name = "SERIAL",
	.priority = osPriorityHigh7
};

static const osEventFlagsAttr_t serial_flags_attr = {
	.name = "SERIAL"
};

static osThreadId_t serial_thread_id = NULL;
static osEventFlagsId_t serial_flags_id = NULL;

static void Serial_Thread(void *arg);
static bool Serial_Match_CMD(const sSerialCommandTable_t *cmd_table, sString_t incoming);

bool Serial_APP_Start(void){
	serial_thread_id = osThreadNew(Serial_Thread, NULL, &serial_thread_attr);
	if(serial_thread_id == NULL){
		error("Creating thread\r\n");
	}
	serial_flags_id = osEventFlagsNew(&serial_flags_attr);
	if(serial_flags_id == NULL){
		error("Creating flags\r\n");
	}
	return true;
}

static void Serial_Thread(void *arg){
	uint32_t flags = 0;
	/* Start UART RX */
	HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
	while(1){
		flags = osEventFlagsWait(serial_flags_id, FLAGS_ALL, osFlagsWaitAny, osWaitForever);
		if(flags == (uint32_t)osErrorTimeout){
			error("Flags timeout\r\n");
			continue;
		}
		if(flags == (uint32_t)osErrorResource){
			error("Flags resource\r\n");
			continue;
		}
		while(rx_cyclic_buff_tail != rx_cyclic_buff_head){
			rx_buffer[rx_buffer_index] = rx_cyclic_buff[rx_cyclic_buff_tail];
			rx_buffer_index++;
			if(rx_cyclic_buff_tail == RX_CYCL_BUF_LEN - 1){
				rx_cyclic_buff_tail = 0;
			}else{
				rx_cyclic_buff_tail++;
			}
			/* Compare delimiter */
			if(rx_buffer[rx_buffer_index-1] == CMD_DELIMITER[delimiter_index]){
				/* Found matching char */
				delimiter_index++;
				if(delimiter_index == CMD_DELIMITER_LEN){
					/* Found matching delimiter */
					if(Serial_Match_CMD(&serial_commands, (sString_t){.text = (char *)rx_buffer, .length = rx_buffer_index-CMD_DELIMITER_LEN}) == false){
						error("Invalid command\r\n");
					}
					delimiter_index = 0;
					rx_buffer_index = 0;
				}
			} else if(rx_buffer_index == RX_BUFF_MAX_LEN){
				/* RX buffer overflow */
				error("RX buffer overflow\r\n");
				memset(rx_buffer, 0, RX_BUFF_MAX_LEN);
				delimiter_index = 0;
			}
		}
	}
}

static bool Serial_Match_CMD(const sSerialCommandTable_t *cmd_table, sString_t incoming){
	for(uint32_t cmd = 0; cmd < cmd_table->length; cmd++){
		const sSerialCommand_t *current_cmd = cmd_table->commands + cmd;
		if(current_cmd->string.length != incoming.length){
			continue;
		}
		if(strncmp(current_cmd->string.text, incoming.text, current_cmd->string.length) == 0){
			/* Found match execute callback */
			if(current_cmd->callback != NULL){
				(*current_cmd->callback)();
				return true;
			}
		}
	}
	return false;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){ //TODO: handle cyclic buffer overflow
	rx_cyclic_buff[rx_cyclic_buff_head] = rx_byte;
	if(rx_cyclic_buff_head == RX_CYCL_BUF_LEN - 1){
		rx_cyclic_buff_head = 0;
	}
	else{
		rx_cyclic_buff_head++;
	}
	osEventFlagsSet(serial_flags_id, FLAGS_RX);
	HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
}
