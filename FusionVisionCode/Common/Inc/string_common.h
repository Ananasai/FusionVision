/*
 * debug_api.h
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#ifndef INC_STRING_COMMON_H_
#define INC_STRING_COMMON_H_

#include <stdint.h>

typedef struct sString_t {
	char *text;
	uint32_t length;
}sString_t;

#endif /* INC_STRING_COMMON_H_ */
