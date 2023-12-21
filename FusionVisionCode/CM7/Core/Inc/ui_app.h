/*
 * ui_app.h
 *
 *  Created on: Dec 16, 2023
 *      Author: simon
 */

#ifndef INC_UI_APP_H_
#define INC_UI_APP_H_

#include <stdbool.h>
#include <stdint.h>
#include "main.h"

extern RTC_HandleTypeDef hrtc;

bool UI_APP_Init(uint16_t *new_image_buffer);
bool UI_APP_DrawAll(void);
bool UI_APP_Printout(void);

#endif /* INC_UI_APP_H_ */
