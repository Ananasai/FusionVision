/*
 * diagnostics_app.h
 *
 *  Created on: Dec 22, 2023
 *      Author: simon
 */

#ifndef INC_DIAGNOSTICS_APP_H_
#define INC_DIAGNOSTICS_APP_H_

#include "main.h"

extern TIM_HandleTypeDef htim1;

void Diagnostics_APP_FrameStart(void);
void Diagnostics_APP_FrameEnd(void);

#endif /* INC_DIAGNOSTICS_APP_H_ */
