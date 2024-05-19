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
#include <string.h>

//#pragma GCC push_options
//#pragma GCC optimize ("O3") //TODO: remove this

#define __DEBUG_FILE_NAME__ "LPT"


#define PACKET_FULL_LEN 164
#define PACKET_DATA_LEN 160
#define PACKET_IN_SEGMENT 60
#define SEGMENT_DATA_LEN (PACKET_DATA_LEN * PACKET_IN_SEGMENT)

#define PACKET_PIXEL_AMOUNT 80
#define SEGMENT_PIXEL_AMOUNT (PACKET_PIXEL_AMOUNT * PACKET_IN_SEGMENT)

#define LEPTON_FRAME_WIDTH 160
#define LEPTON_FRAME_HEIGHT 120

// Macro to set a flag
#define SET_FLAG(variable, flag)    ((variable) |= (1U << (flag)))

// Macro to clear a flag
#define CLEAR_FLAG(variable, flag)  ((variable) &= ~(1U << (flag)))

// Macro to read a flag (returns 1 if the flag is set, 0 otherwise)
#define READ_FLAG(variable, flag)   (((variable) >> (flag)) & 1U)

typedef enum eLeptonFlag {
	eLeptonFlagFirst = 0,
	eLeptonFlagPacketOverflow,
	eLeptonFlagPacketReceived,
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

static uint16_t *image_buffer = {0};

static uint8_t calculated_segment = 0;
static uint8_t calculated_packet = 0;

static void Lepton_APP_StartReceive(void){
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

static void Lepton_APP_DecodeAndDrawFromBuffer(uint8_t *buffer, uint8_t segment, uint8_t packet) {
	uint16_t row = 119 - (30 * (segment-1)) - ((uint8_t)(packet / 2));
	uint16_t collumn_start = packet % 2 == 0 ? 80 : 0;
	uint32_t pixel_index = row * SCREEN_WIDTH + collumn_start;
	for(uint16_t i = PACKET_DATA_LEN + 2; i > 3; i -= 2){
		if(pixel_index > 480*320){
			error("OUT OF BOUNDS row: %d, col %d, seg %d, pack %d\r\n", row, collumn_start, segment, packet);
			return;
		}
		/* RAW14 - MSB in buffer[i], LSB in buffer[i-1] */
		uint16_t full_value = (buffer[i] << 8) | buffer[i-1];
		*(image_buffer + pixel_index) = full_value >> 6;
		pixel_index += 1;
	}
}

bool Lepton_APP_Start(uint16_t *new_image_buffer){
	if(new_image_buffer == NULL){
		return false;
	}
	image_buffer = new_image_buffer;
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
	//if(Lepton_API_SetGpio() == false){
	//	error("Failed set GPIO\r\n");
	//}
	return true;
}

//TODO: use start buffer for all rx_buffer
uint8_t start_buffer[PACKET_FULL_LEN * 20] = {0};
uint8_t start_buffer_idx = 0;

uint16_t decoded_segment = 0;
void Lepton_APP_Run(uint8_t *flag){
	if(READ_FLAG(lepton_flags, eLeptonFlagPacketOverflow)) {
		/* Packet reading overflow */
		error("Lepton reading overflow\r\n");
		CLEAR_FLAG(lepton_flags, eLeptonFlagPacketReceived);
		CLEAR_FLAG(lepton_flags, eLeptonFlagPacketOverflow);

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
				for(uint8_t i = start_buffer_idx; i < 20; i++) {
					Lepton_APP_DecodeAndDrawFromBuffer(start_buffer + i * PACKET_FULL_LEN, decoded_segment, packet_index);
					packet_index++;
				}
				for(uint8_t i = 0; i < start_buffer_idx; i++) {
					Lepton_APP_DecodeAndDrawFromBuffer(start_buffer + i * PACKET_FULL_LEN, decoded_segment, packet_index);
					packet_index++;
				}
			}
		} else if (decoded_packet < 20) {
			/* Received packet without knowing from which segment */
			/* Save to circular buffer for further use */
			//TODO: copy only decoded data?
			memcpy(start_buffer + start_buffer_idx * PACKET_FULL_LEN, rx_buffer, PACKET_FULL_LEN);
			start_buffer_idx = (start_buffer_idx + 1) % 20;
		} else if (decoded_packet >= 60){
			return;
		}

		if(decoded_segment != 0) {
			Lepton_APP_DecodeAndDrawFromBuffer(rx_buffer, decoded_segment, decoded_packet);
		}


//		if(decoded_packet == PACKET_IN_SEGMENT - 1) {
//			debug("P %u s %u\r\n", decoded_packet, decoded_segment);
//			if(decoded_segment == 4) {
//				*flag = 0x01;
//				HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
//			}
//			decoded_segment = 0;
//		}

		calculated_packet++;
		if(calculated_packet == PACKET_IN_SEGMENT){
			//debug("P %u s %u\r\n", decoded_packet, decoded_segment);

			calculated_segment++;
			calculated_packet = 0;
			if(calculated_segment == 4){
				//debug("P %u s %u\r\n", decoded_packet, decoded_segment);
				calculated_segment = 0;
				/* Start drawing  */
				//debug("Done\r\n");
				*flag = 0x01;
				HAL_GPIO_WritePin(SPI4_CS_GPIO_Port, SPI4_CS_Pin, GPIO_PIN_SET);
			}
			decoded_segment = 0;
		}
	}
}

//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim){
//	if(htim == &htim4){
//		//debug("a\r\n");
//		//Lepton_APP_StartReceive();
//		continue_rx_flag = true;
//	}
//}

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
