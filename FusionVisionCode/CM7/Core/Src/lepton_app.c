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
#include "shared_mem_api.h"
#include "shared_param_api.h"
#include <string.h>
#include "common.h"

//#pragma GCC push_options
//#pragma GCC optimize ("O3") //TODO: remove this

#define __DEBUG_FILE_NAME__ "LPT"

#define PACKET_FULL_LEN 164
#define PACKET_DATA_LEN 160
#define PACKET_IN_SEGMENT 60

#define CIRC_BUF_PACKET_LEN 20

typedef enum eLeptonFlag {
	eLeptonFlagFirst = 0,
	eLeptonFlagPacketOverflow,
	eLeptonFlagPacketReceived,
	eLeptonFlagLostSynchronization,
	eLeptonFlagLast
}eLeptonFlag_t;

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

static uint8_t lepton_flags = 0x00;

static uint8_t *rx_buffer;
static uint8_t rx_buffer1[PACKET_FULL_LEN] = {0};
static uint8_t rx_buffer2[PACKET_FULL_LEN] = {0};

static void Lepton_APP_ResetWatchdog(void) {
	HAL_TIM_Base_Stop_IT(&htim4);
	htim4.Instance->CNT = 0;
	HAL_TIM_Base_Start_IT(&htim4);
}

static void Lepton_APP_StartReceive(void) {
	static bool use_buffer_1 = true;
	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_RESET);
	if(use_buffer_1 == true){
		HAL_SPI_Receive_DMA(&hspi4, rx_buffer1, 164);
	}
	else{
		HAL_SPI_Receive_DMA(&hspi4, rx_buffer2, 164);
	}
	rx_buffer = use_buffer_1 ? rx_buffer1 : rx_buffer2;
	use_buffer_1 = !use_buffer_1;
}

static uint32_t min_captured_temperature = 0xFF;
static uint32_t max_captured_temperature = 0;
static void Lepton_APP_DecodeAndDrawFromBuffer(uint8_t *buffer, uint8_t segment, uint8_t packet) {
	uint16_t row = 119 - (30 * (segment-1)) - ((uint8_t)(packet / 2));
	uint16_t collumn_start = packet % 2 == 0 ? 80 : 0;
	uint32_t pixel_index = row * 160 + collumn_start;
	for(uint16_t i = PACKET_DATA_LEN + 2; i > 3; i -= 2){
		/* RAW14 - MSB in buffer[i], LSB in buffer[i-1] */
		uint16_t full_value = (buffer[i] << 8) | buffer[i-1];
		uint8_t reduced_value = full_value >> 6;
		*(uint8_t *)(SHARED_TERMO_BUF_START + pixel_index) = reduced_value;
		if((segment != 58) && (segment != 59) && (reduced_value != 0)) {
			if(reduced_value > max_captured_temperature) {
				max_captured_temperature = reduced_value;
			}
			else if(reduced_value < min_captured_temperature) {
				min_captured_temperature = reduced_value;
			}
		}
		//*(image_buffer + pixel_index) = full_value >> 6;
		pixel_index += 1;
	}
}

static void Letpon_APP_Resync(void) {
	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
	HAL_Delay(200);
	Lepton_APP_ResetWatchdog();
}

bool Lepton_APP_Start(void) {
	/* Lepton initialisation page 17 */
	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LEPTON_PWR_GPIO_Port, LEPTON_PWR_Pin, GPIO_PIN_RESET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(LEPTON_PWR_GPIO_Port, LEPTON_PWR_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(LEPTON_RST_GPIO_Port, LEPTON_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(LEPTON_RST_GPIO_Port, LEPTON_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(100);
	/* Wait while Lepton boots */
	//while(Lepton_API_CheckBusy() != true){
	//	HAL_Delay(100);
	//}

	//if(Lepton_API_EnableAGC() == false){
	//	error("Failed set AGC\r\n");
	//}

	debug("Lepton active\r\n");

	Lepton_APP_StartReceive();
	Lepton_APP_ResetWatchdog();
	//if(Lepton_API_SetGpio() == false){
	//	error("Failed set GPIO\r\n");
	//}
	return true;
}

//TODO: use start buffer for all rx_buffer
/* Circular buffer for receiving packets */
uint8_t circ_buffer[PACKET_FULL_LEN * CIRC_BUF_PACKET_LEN] = {0};
uint8_t circ_buffer_index = 0;

uint16_t decoded_segment = 0;
void Lepton_APP_Run(void){
	if(READ_FLAG(lepton_flags, eLeptonFlagLostSynchronization)) {
		/* Restores synchronization if no new frame is generated in 1s period */
		error("Lepton lost synchronization\r\n");
		CLEAR_FLAG(lepton_flags, eLeptonFlagLostSynchronization);
		CLEAR_FLAG(lepton_flags, eLeptonFlagPacketOverflow);
		CLEAR_FLAG(lepton_flags, eLeptonFlagPacketReceived);
		Letpon_APP_Resync();
		Lepton_APP_StartReceive();
	}
	if(READ_FLAG(lepton_flags, eLeptonFlagPacketOverflow)) {
		/* Packet reading overflow */
		error("Lepton reading overflow\r\n");
		CLEAR_FLAG(lepton_flags, eLeptonFlagPacketReceived);
		CLEAR_FLAG(lepton_flags, eLeptonFlagPacketOverflow);
		Letpon_APP_Resync();
		Lepton_APP_StartReceive();
	}
	if(READ_FLAG(lepton_flags, eLeptonFlagPacketReceived)){
		CLEAR_FLAG(lepton_flags, eLeptonFlagPacketReceived);
		Lepton_APP_StartReceive();
		/* Check discard packet*/
		if((rx_buffer[0] & 0x0F) == 0x0F){
			return;
		}

		/* Decode data */
		uint16_t decoded_packet = rx_buffer[1];
		if(decoded_packet == 20){
			decoded_segment = (rx_buffer[0] >> 4) & 0x0F;
			if(decoded_segment == 0){
				/* Whole frame invalid */
				return;
			} else if(decoded_segment > 4){
				decoded_segment = 0;
				return;
			} else {
				/* Valid segment */
				//debug("Seg: %hu %hu \r\n", decoded_segment, start_buffer_idx);
				/* Display last 20 packets */
				uint8_t packet_index = 0;
				for(uint8_t i = circ_buffer_index; i < CIRC_BUF_PACKET_LEN; i++) {
					Lepton_APP_DecodeAndDrawFromBuffer(circ_buffer + i * PACKET_FULL_LEN, decoded_segment, packet_index);
					packet_index++;
				}
				for(uint8_t i = 0; i < circ_buffer_index; i++) {
					Lepton_APP_DecodeAndDrawFromBuffer(circ_buffer + i * PACKET_FULL_LEN, decoded_segment, packet_index);
					packet_index++;
				}
			}
		} else if (decoded_packet < 20) {
			/* Received packet without knowing from which segment */
			/* Save to circular buffer for further use */
			//TODO: copy only decoded data?
			memcpy(circ_buffer + circ_buffer_index * PACKET_FULL_LEN, rx_buffer, PACKET_FULL_LEN);
			circ_buffer_index = (circ_buffer_index + 1) % CIRC_BUF_PACKET_LEN;
		} else if (decoded_packet >= 60){
			return;
		}

		if(decoded_segment != 0) {
			Lepton_APP_DecodeAndDrawFromBuffer(rx_buffer, decoded_segment, decoded_packet);
		}

		if(decoded_packet == PACKET_IN_SEGMENT - 2){
			if(decoded_segment == 4){
				/* Can start drawing */
				HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
				Lepton_APP_ResetWatchdog();
			}
			decoded_segment = 0;
			/* Save min/max temperature of the frame */
			if(min_captured_temperature == 0xFF) {
				min_captured_temperature = 1;
			}
			if(max_captured_temperature == 0 || max_captured_temperature == 255) {
				max_captured_temperature = 0xFF-10; //TODO test
			}
			Shared_param_API_Write(eSharedParamMinCapturedTemperature, &min_captured_temperature);
			Shared_param_API_Write(eSharedParamMaxCapturedTemperature, &max_captured_temperature);
			min_captured_temperature = 0xFF;
			max_captured_temperature = 0;
		}
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim){
	if(htim == &htim4){
		/* Lost synchronization timer */
		SET_FLAG(lepton_flags, eLeptonFlagLostSynchronization);
	}
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef * hspi){
    if(hspi == &hspi4){
    	HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
    	if(READ_FLAG(lepton_flags, eLeptonFlagPacketReceived)) {
    		SET_FLAG(lepton_flags, eLeptonFlagPacketOverflow);
    	} else {
    		SET_FLAG(lepton_flags, eLeptonFlagPacketReceived);
    	}
    }
}

/* Sadly VSYNC not configurable */
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
//	if(GPIO_Pin == LEPTON_VSYNC_Pin){
//		HardFault_Handler(); //TODO: not working
//	}
//}
//#pragma GCC pop_options
