/*
 * message_common.h
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#ifndef COMMON_STRING_COMMON_H_
#define COMMON_STRING_COMMON_H_

#include<stddef.h>

typedef struct sString_t {
	char *string;
	size_t length;
}sString_t;

#endif /* COMMON_STRING_COMMON_H_ */
