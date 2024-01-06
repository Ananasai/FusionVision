/*
 * lepton_app.c
 *
 *  Created on: Dec 30, 2023
 *      Author: simon
 */

#include "lepton_app.h"
#include "debug_api.h"
#include "circular_buffer.h"
#include "fonts.h"
#include "lepton_api.h"

#pragma GCC push_options
#pragma GCC optimize ("O0") //TODO: remove this

#define __DEBUG_FILE_NAME__ "LPT"

#define FLAGS_RX 0x01
#define FLAGS_ALL (FLAGS_RX)

#define CIRC_BUF_LEN 34000
#define PACKET_DATA_LEN 160
#define PACKET_IN_SEGMENT 60
#define SEGMENT_DATA_LEN (PACKET_DATA_LEN * PACKET_IN_SEGMENT)

#define PACKET_PIXEL_AMOUNT 80
#define SEGMENT_PIXEL_AMOUNT (PACKET_PIXEL_AMOUNT * PACKET_IN_SEGMENT)

#define FRAME_WIDTH 160
#define FRAME_HEIGHT 120

/* DATASHEET: https://cdn.sparkfun.com/assets/f/6/3/4/c/Lepton_Engineering_Datasheet_Rev200.pdf */
/* I2C registers: https://cdn.sparkfun.com/assets/0/6/d/2/e/16465-FLIRLepton-SoftwareIDD.pdf */

/* https://github.com/groupgets/LeptonModule/tree/master/software/ThermalView/src
 * https://electronics.stackexchange.com/questions/665011/pixel-offset-in-image-from-flir-lepton-3-5-vospi
 * https://hackaday.io/project/159615-lepton-35-thermal-imaging-camera/log/149225-vospi
 * */

/* Telemetry - as header
 * Radiometry enabled - Tlinear enabled
 * AGC - enabled
 * Video output - RAW14
 * GPIO - VSYNC enabled
 */


/* GPIO
 * VSYNC - HIGH impulse when new frame generated
 * SPI4_CS - active low
 * PWR_DWN_L - active low shutdown
 * RESET_L - active low reset
 * */
static bool rx_byte_flag = false;

static uint8_t rx_byte = 0x00;
static uint8_t rx_buffer[CIRC_BUF_LEN] = {0}; //TODO: shorten?
static uint32_t rx_buffer_index = 0;
static uint16_t curr_packet_index = 0;
static uint16_t curr_segment_index = 0;

static sCircularBuffer_t circ_buffer;

static uint16_t *image_buffer = {0};

static uint16_t pixel_index = 0;
static bool packet_left_side = false;

static uint8_t calculated_segment = 0;
static uint8_t calculated_packet = 0;

bool Lepton_APP_Start(uint16_t *new_image_buffer){
	if(new_image_buffer == NULL){
		return false;
	}
	image_buffer = new_image_buffer;
	if(Circular_buffer_create(&circ_buffer, CIRC_BUF_LEN) == false){
		error("Creating ciruclar buffer\r\n");
		return false;
	}
	/* Lepton initialisation page 17 */
	HAL_GPIO_WritePin(LEPTON_PWR_GPIO_Port, LEPTON_PWR_Pin, GPIO_PIN_RESET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(LEPTON_PWR_GPIO_Port, LEPTON_PWR_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(LEPTON_RST_GPIO_Port, LEPTON_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(LEPTON_RST_GPIO_Port, LEPTON_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(1000);

	while(Lepton_API_CheckBusy() != true){
		debug("Failed\r\n");
		HAL_Delay(100);
	}
	debug("Resp\r\n");

	while(1){};
	/*
	Lepton_API_ReadReg(0x2);

	Lepton_API_SendCommand(SYS, 0x28 >> 2 , GET);
	debug("SYS Camera Customer Serial Number 0x%x\r\n", Lepton_API_ReadData());

	Lepton_API_SendCommand(SYS, 0x2 , GET);
	debug("SYS Flir Serial Number 0x%x\r\n", Lepton_API_ReadData());

	Lepton_API_SendCommand(SYS, 0x0C >> 2 , GET);
	debug("SYS Camera Uptime 0x%x\r\n", Lepton_API_ReadData());

	Lepton_API_SendCommand(SYS, 0x14 >> 2 , GET);
	debug("SYS Fpa Temperature Kelvin 0x%x\r\n", Lepton_API_ReadData());

	Lepton_API_SendCommand(SYS, 0x10 >> 2 , GET);
	debug("SYS Aux Temperature Kelvin 0x%x\r\n", Lepton_API_ReadData());

	Lepton_API_SendCommand(OEM, 0x14 >> 2 , GET);
	debug("OEM Chip Mask Revision 0x%x\r\n", Lepton_API_ReadData());

	//Serial.println("OEM Part Number");
	//lepton_command(OEM, 0x1C >> 2 , GET);
	//read_data();
	Lepton_API_SendCommand(OEM, 0x20 >> 2 , GET);
	debug("OEM Camera Software Revision 0x%x\r\n", Lepton_API_ReadData());

	HAL_Delay(1000);
	HAL_SPI_Receive_IT(&hspi4, &rx_byte, 1);
	//HAL_TIM_Base_Start_IT(&htim3);
	 */

	return true;
}

void Lepton_APP_Run(uint8_t *flag){
	if(rx_byte_flag){
		rx_byte_flag = false;
		bool packet_received = false;
		/* Received byte */
		uint8_t circ_byte = 0;
		while(Circular_buffer_pop(&circ_buffer, &circ_byte)) {
			rx_buffer[rx_buffer_index] = circ_byte;
			rx_buffer_index++;
			if(rx_buffer_index == PACKET_DATA_LEN*50){
				packet_received = true;
				*flag = 0x01;
				///* Start reading from zero */
				rx_buffer_index = 0;
				break;
			}
		}
		for(uint16_t i = 0; i < PACKET_DATA_LEN*50; i++){
			DEBUG_API_LOG("0x%x ", NULL, NULL, rx_buffer[i]);
			if(i % PACKET_DATA_LEN == 0){
				DEBUG_API_LOG("\r\n", NULL, NULL);
			}
		}
		while(1){};

		return; //TODO: DEBUG
		if(packet_received){
			if(circ_buffer.overflow){
				error("Lepton buffer overflow\r\n");
				circ_buffer.overflow = false;
			}
			/* Checked discard packet */
			if((rx_buffer[0] & 0x0F) == 0x0F){
				curr_packet_index = 0;
				curr_segment_index = 0;
				return;
			}
			/* Valid packet */
			uint32_t packet_number = rx_buffer[1];
			curr_packet_index = packet_number;
			calculated_packet++;
			debug("Pack: %d %d\r\n", curr_packet_index, calculated_packet);
			if((packet_number == 20) || (calculated_packet == 20)){
				uint32_t segment_number = rx_buffer[0] >> 4;
				/* Check if discard or invalid packet */
				if(segment_number == 0){
					curr_packet_index = 0;
					curr_segment_index = 0;
					return;
				}
				curr_segment_index = segment_number - 1;
				debug("Seg: %d %d\r\n", curr_segment_index, calculated_segment);
			}
			//TODO: could be different with telemetry enabled
			//uint16_t pixel_index = curr_segment_index * SEGMENT_PIXEL_AMOUNT + curr_packet_index * PACKET_PIXEL_AMOUNT;
			/* Is packet first in row or second */
			uint32_t magic = 55;
			packet_left_side = curr_packet_index % 2 == 0;
			pixel_index = curr_segment_index * SCREEN_WIDTH * 30;
			uint32_t magic2 = 69;
			if(packet_left_side == false){
				pixel_index += PACKET_PIXEL_AMOUNT;
			}
			/* Get every other pixel as AGC is on */
			for(uint16_t i = 4; i < PACKET_DATA_LEN; i += 2){
				*(image_buffer + pixel_index) = rx_buffer[i];
				pixel_index++;
			}
			if(curr_packet_index == PACKET_IN_SEGMENT - 1){
				curr_segment_index++;
			}
		}
	}
}

#pragma GCC pop_options

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef * hspi){
    if(hspi == &hspi4){
    	Circular_buffer_push(&circ_buffer, rx_byte);
    	rx_byte_flag = true;
    	/* Continue reading */
    	//if(rx_buffer_index >= PACKET_LEN){return;}
    	HAL_SPI_Receive_IT(&hspi4, &rx_byte, 1);
    }
}

// Callback: timer has rolled over
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim == &htim3){
		//if(test_data_index == sizeof(test_data)){
			return;
		//}
		//rx_byte = test_data[test_data_index];
		//test_data_index++;
		//Circular_buffer_push(&circ_buffer, rx_byte);
		//rx_byte_flag = true;
		/* Continue reading */
		//HAL_SPI_Receive_IT(&hspi4, &rx_byte, 1);

	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == LEPTON_VSYNC_Pin){

	}
}

/* VSYNC EXTI in buttons file
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

}
*/
