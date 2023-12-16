/*
 * debug_api.h
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#ifndef INC_DEBUG_API_H_
#define INC_DEBUG_API_H_

#include <stdbool.h>
#include "main.h"

/* Hack to make __LINE__ into string */
#define LINE_TO_STRING(x) LINE_TO_STRING2(x)
#define LINE_TO_STRING2(x) #x
/* If this part is erroring - you forgot to add __DEBUG_FILE_NAME__ to you'r file */
#define debug(message, args...) DEBUG_API_LOG(message, "[DEBUG]\t["__DEBUG_FILE_NAME__"]\t", NULL, ##args)
#define error(message, args...) DEBUG_API_LOG(message, "[ERROR]\t["__DEBUG_FILE_NAME__"]\t", "("__FILE__" ("LINE_TO_STRING(__LINE__)")) ", ##args)

bool Debug_API_Start(UART_HandleTypeDef huart);
void DEBUG_API_LOG(char *message, char *info_string, char *error_string, ...);

#endif /* INC_DEBUG_API_H_ */
