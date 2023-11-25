/*
 * adc_app.c
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#include "adc_app.h"
#include "debug_api.h"
#include <cmsis_os2.h>
#include <main.h>

#define __DEBUG_FILE_NAME__ "ADC"

extern ADC_HandleTypeDef hadc1;
static uint16_t adc_val = 0;

bool Adc_APP_Start(void){
	if(HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK){
		error("Failed ADC calibration \r\n");
		return false;
	}
	if(HAL_ADC_Start_IT(&hadc1) != HAL_OK){
		error("Couldn't start ADC1\r\n");
	}
	return true;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  adc_val = HAL_ADC_GetValue(&hadc1);
  debug("%d", adc_val);
}
