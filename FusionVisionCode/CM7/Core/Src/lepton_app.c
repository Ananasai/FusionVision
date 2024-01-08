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
#pragma GCC optimize ("O3") //TODO: remove this

#define __DEBUG_FILE_NAME__ "LPT"

#define FLAGS_RX 0x01
#define FLAGS_ALL (FLAGS_RX)

#define CIRC_BUF_LEN 164 //TODO: CHANGE
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
static uint8_t *rx_buffer; //TODO: shorten?
static uint8_t rx_buffer1[CIRC_BUF_LEN] = {0}; //TODO: shorten?
static uint8_t rx_buffer2[CIRC_BUF_LEN] = {0}; //TODO: shorten?
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
	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LEPTON_PWR_GPIO_Port, LEPTON_PWR_Pin, GPIO_PIN_RESET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(LEPTON_PWR_GPIO_Port, LEPTON_PWR_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(LEPTON_RST_GPIO_Port, LEPTON_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(LEPTON_RST_GPIO_Port, LEPTON_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(1000);
	/* Wait while Lepton boots */
	while(Lepton_API_CheckBusy() != true){
		HAL_Delay(100);
	}
	debug("Lepton active\r\n");
	//if(Lepton_API_SetGpio() == false){
	//	error("Failed set GPIO\r\n");
	//}
	//while(1){};
	//while(1){};
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

	//HAL_TIM_Base_Start_IT(&htim3);
	 */

	return true;
}
static bool first_run = true;
static bool use_buffer_1 = true;
void Lepton_APP_Run(uint8_t *flag){
	/* For first time transfers after some long period */
	if(first_run){
		first_run = false;
		HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_RESET);
		if(use_buffer_1 == true){
			HAL_SPI_Receive_DMA(&hspi4, rx_buffer1, 164);
		}
		else{
			HAL_SPI_Receive_DMA(&hspi4, rx_buffer2, 164);
		}

	}
	if(rx_byte_flag){
		rx_byte_flag = false;
		rx_buffer = use_buffer_1 ? rx_buffer1 : rx_buffer2;
		use_buffer_1 = !use_buffer_1;
		HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_RESET);
		if(use_buffer_1 == true){
			HAL_SPI_Receive_DMA(&hspi4, rx_buffer1, 164);
		}
		else{
			HAL_SPI_Receive_DMA(&hspi4, rx_buffer2, 164);
		}

		/* Check discard packet*/
		if((rx_buffer[0] & 0x0F) == 0x0F){
			return;
		}

		/* Decode data */
		uint16_t decoded_packet = rx_buffer[1];
		uint16_t decoded_segment = rx_buffer[0] >> 4;
		if((decoded_packet == 20) && (decoded_segment == 0)){
			return;
		}
		//TODO: could be different with telemetry enabled
		//uint16_t pixel_index = curr_segment_index * SEGMENT_PIXEL_AMOUNT + curr_packet_index * PACKET_PIXEL_AMOUNT;
		/* Is packet first in row or second */
		//debug("Pck: %d\r\n", decoded_packet);
		packet_left_side = decoded_packet % 2 == 0;
		uint16_t row = (3 - calculated_segment) * 30 + ((59- decoded_packet) / 2);
		uint16_t collumn = packet_left_side ? 80 : 0;
		pixel_index = row * SCREEN_WIDTH + collumn;
		/* Get every other pixel as AGC is on */
		for(uint16_t i = PACKET_DATA_LEN + 4; i > 3; i -= 2){
			*(image_buffer + pixel_index) = rx_buffer[i];
			//*(image_buffer + pixel_index) = (rx_buffer[i] | (rx_buffer[i-1] << 8) & 0x3C00) + (rx_buffer[i] | (rx_buffer[i-1] << 8) & 0x03E0) + ((rx_buffer[i] | rx_buffer[i-1] << 8) & 0x001F);
			pixel_index++;
		}
		calculated_packet++;
		if(calculated_packet == PACKET_IN_SEGMENT){
			calculated_segment++;
			calculated_packet = 0;
			if(calculated_segment == 4){
				calculated_segment = 0;
				/* Start drawing  */
				*flag = 0x01;
				HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
				//HAL_Delay(200);
				return; //TODO: clear buffer
			}
		}
	}
}



void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef * hspi){
    if(hspi == &hspi4){
    	//Circular_buffer_push(&circ_buffer, rx_byte);
    	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
    	rx_byte_flag = true;
    	/* Continue reading */
    	//if(rx_buffer_index >= PACKET_LEN){return;}
    	//if(first_run == false){
    	//	HAL_SPI_Receive_IT(&hspi4, &rx_byte, 1);
    	//}
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == LEPTON_VSYNC_Pin){
		HardFault_Handler(); //TODO: not working
	}
}
#pragma GCC pop_options
/* VSYNC EXTI in buttons file
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

}
*/
