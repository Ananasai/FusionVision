/*
 * diagnostics_app.h
 *
 *  Created on: Dec 22, 2023
 *      Author: simon
 */

#ifndef INC_DIAGNOSTICS_APP_H_
#define INC_DIAGNOSTICS_APP_H_

#include "main.h"

typedef enum eDiagEvent_t {
	eDiagEventFirst = 0,
	eDiagEventCamera = eDiagEventFirst,
	eDiagEventProcessing,
	eDiagEventDisplay,
	eDiagEventFrame,
	eDiagEventLast
}eDiagEvent_t;

extern TIM_HandleTypeDef htim1;

void Diagnostics_APP_Start(void);
void Diagnostics_APP_RecordStart(eDiagEvent_t event);
void Diagnostics_APP_RecordEnd(eDiagEvent_t event);

#endif /* INC_DIAGNOSTICS_APP_H_ */
