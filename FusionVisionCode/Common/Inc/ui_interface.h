/*
 * sync_api.h
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#ifndef INC_UI_INTERFACE_H_
#define INC_UI_INTERFACE_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct sUiLabel_t {
	char *content;
	size_t length;
}sUiLabel_t;

typedef struct sUiPanel_t {
	sUiLabel_t *labels;
	size_t label_amount;
}sUiPanel_t;

bool UI_Interface_GetCurrentPanel(uint32_t panel_id, sUiPanel_t *out);

#endif /* INC_UI_INTERFACE_H_ */
