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

bool UI_APP_DrawAll(uint16_t *image_buffer);
bool UI_APP_Printout(uint16_t *image_buffer);

#endif /* INC_UI_APP_H_ */
