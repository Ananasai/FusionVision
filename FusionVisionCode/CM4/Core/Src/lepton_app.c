/*
 * lepton_app.c
 *
 *  Created on: Dec 30, 2023
 *      Author: simon
 */

#include "lepton_app.h"
#include "debug_api.h"
#include "cmsis_os2.h"

#define __DEBUG_FILE_NAME__ "LPT"

#define FLAGS_RX 0x01
#define FLAGS_ALL (FLAGS_RX)

#define RX_BUFF_LEN 1000
#define PACKET_LEN 164
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

static void Lepton_Thread(void *arg);

static const osThreadAttr_t lepton_thread_attr = {
	.name = "Lepton",
	.priority = osPriorityHigh
};

static const osEventFlagsAttr_t lepton_flags_attr = {
		.name = "Lepton"
};

static osThreadId_t lepton_thread_id = NULL;
static osEventFlagsId_t lepton_flags_id = NULL;

static uint8_t rx_byte = 0x00;
static uint8_t rx_index = 0;
static uint8_t rx_buffer[RX_BUFF_LEN] = {0};
static uint16_t curr_packet_index = 0;
static uint16_t curr_segment_index = 0;

static uint8_t pixels[FRAME_WIDTH * FRAME_HEIGHT] = {0};

bool Lepton_APP_Start(void){
	lepton_thread_id = osThreadNew(Lepton_Thread, NULL, &lepton_thread_attr);
	if(lepton_thread_id == NULL){
		error("Creating lepton thread\r\n");
		return false;
	}
	lepton_flags_id = osEventFlagsNew(&lepton_flags_attr);
	if(lepton_flags_id == NULL){
		error("Creating lepton flags\r\n");
		return false;
	}
	return true;
}

static void Lepton_Thread(void *arg){
	uint32_t flags = 0;
	/* Lepton initialisation */
	/* Datasheet page 17 */
	HAL_GPIO_WritePin(LEPTON_PWR_GPIO_Port, LEPTON_PWR_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LEPTON_RST_GPIO_Port, LEPTON_RST_Pin, GPIO_PIN_RESET);
	osDelay(1);
	HAL_GPIO_WritePin(LEPTON_RST_GPIO_Port, LEPTON_RST_Pin, GPIO_PIN_SET);
	/* Start receive SPI */
	HAL_SPI_Receive_IT(&hspi4, &rx_byte, 1);
	while(1){
		flags = osEventFlagsWait(lepton_flags_id, FLAGS_ALL, osFlagsWaitAny, osWaitForever);
		if(flags == FLAGS_RX){
			/* Received packet */
			/* Checked discard packet */
			if((rx_buffer[0] & 0x0F) == 0x0F){
				curr_packet_index = 0;
				curr_segment_index = 0;
				continue;
			}
			uint32_t packet_number = rx_buffer[1];
			curr_packet_index = packet_number;
			if(packet_number == 20){
				uint32_t segment_number = rx_buffer[0] >> 4;
				if(segment_number != 0){
					curr_segment_index = segment_number - 1;
				}
			}else{
				uint16_t pixel_index = curr_segment_index * SEGMENT_PIXEL_AMOUNT + curr_packet_index * PACKET_PIXEL_AMOUNT;
				/* Get every other pixel as AGC is on */
				for(uint16_t i = 5; i < 160; i += 2){
					pixels[pixel_index] = rx_buffer[i];
					pixel_index++;
				}
			}
			if(curr_packet_index == PACKET_IN_SEGMENT - 1){
				curr_segment_index++;
			}

		}
	}
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef * hspi)
{
    if(hspi == &hspi4){
    	rx_buffer[rx_index] = rx_byte;
    	if(rx_index == PACKET_LEN){
    		/* Received full packet */
    		osEventFlagsSet(lepton_flags_id, FLAGS_RX);
    	}
    	/* Continue reading */
    	HAL_SPI_Receive_IT(&hspi4, &rx_byte, 1);
    }
}

/* Called on vsync rising edge */
void Lepton_APP_VsyncIrq(void){

}

/* VSYNC EXTI in buttons file
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

}
*/
