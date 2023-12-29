/*
 * adc_app.h
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#ifndef INC_ADC_APP_H_
#define INC_ADC_APP_H_

#include <stdbool.h>
#include "main.h"

extern ADC_HandleTypeDef hadc1;

typedef enum eAdcChannel_t {
	eAdcChannelFirst = 0,
	eAdcChannelCurrent = eAdcChannelFirst,
	eAdcChannelLuminance,
	eAdcChannelLast
}eAdcChannel_t;

bool Adc_APP_Start(void);

#endif /* INC_ADC_APP_H_ */
