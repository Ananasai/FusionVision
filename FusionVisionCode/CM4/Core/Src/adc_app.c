///*
// * adc_app.c
// *
// *  Created on: Nov 16, 2023
// *      Author: simon
// */
//
//#include "adc_app.h"
//#include "debug_api.h"
//#include "string_common.h"
//#include "job_api.h"
//#include <string.h>
//#include <stdlib.h>
//
//#define __DEBUG_FILE_NAME__ "ADC"
//#define ADC_CHANNEL(_enum, _name, _target_time, _channel, _queue) [_enum] = {.name = (sString_t){.text = _name, .length = sizeof(_name)}, .target_time = _target_time, .ticks = 0, .channel = _channel, .result_queue = _queue}
//
//#define ADC_TIMER_TICKS 1
//
//static const osTimerAttr_t adc_timer_attr = {
//	.name = "ADC"
//};
//
//static osTimerId_t adc_timer_id = NULL;
//
//typedef struct sAdcChannelDesc_t {
//	sString_t name;
//	uint32_t target_time;
//	uint32_t ticks;
//	uint32_t channel;
//	eQueueEnum_t result_queue;
//}sAdcChannelDesc_t;
//
//static sAdcChannelDesc_t adc_channel_lut[eAdcChannelLast] = {
//	ADC_CHANNEL(eAdcChannelCurrent, "Current", 1, ADC_CHANNEL_15, eQueuePowerTrack),
//	ADC_CHANNEL(eAdcChannelLuminance, "Luminance", 1000, ADC_CHANNEL_16, eQueueLighting),
//};
//
//static void Adc_APP_Timer(void *arg);
//
//bool Adc_APP_Start(void){
//	if(HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK){
//		error("Failed ADC calibration \r\n");
//		return false;
//	}
//	adc_timer_id = osTimerNew(Adc_APP_Timer, osTimerPeriodic, NULL, &adc_timer_attr);
//	if(adc_timer_id == NULL){
//		error("Creating adc timer\r\n");
//		return false;
//	}
//	if(osTimerStart(adc_timer_id, ADC_TIMER_TICKS) != osOK){
//		error("Starting adc timer\r\n");
//		return false;
//	}
//	return true;
//}
//
//static void Adc_APP_Timer(void *arg){
//	for(eAdcChannel_t channel = eAdcChannelFirst; channel < eAdcChannelLast; channel++){
//		adc_channel_lut[channel].ticks++;
//		if(adc_channel_lut[channel].ticks == adc_channel_lut[channel].target_time){
//			/* Sample selected channel */
//			ADC_ChannelConfTypeDef channel_config = {0};
//			channel_config.Channel = adc_channel_lut[channel].channel;
//			channel_config.Rank = ADC_REGULAR_RANK_1;
//			channel_config.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
//			if (HAL_ADC_ConfigChannel(&hadc1, &channel_config) != HAL_OK) {
//				HAL_ADC_Stop(&hadc1);
//				error("In selecting channel: %s\r\n", adc_channel_lut[channel].name);
//				return;
//			}
//			HAL_ADC_Start(&hadc1);
//			if (HAL_ADC_PollForConversion(&hadc1, 1) != HAL_OK) {
//				HAL_ADC_Stop(&hadc1);
//				error("In polling channel: %s\r\n", adc_channel_lut[channel].name);
//				return;
//			}
//			uint16_t adc_value = HAL_ADC_GetValue(&hadc1);
//			HAL_ADC_Stop(&hadc1);
//			float *adc_value_v = malloc(sizeof(float));
//			if(adc_value_v == NULL){
//				error("In allocating value\r\n");
//				return;
//			}
//			*adc_value_v = adc_value / 65535.0f * 3.3f;
//			Job_API_AddNew(adc_channel_lut[channel].result_queue, (sJob_t){.job = eJobRecAdc, .payload = adc_value_v});
//			adc_channel_lut[channel].ticks = 0;
//		}
//	}
//}
