/*
 * debug_api.c
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#include "debug_api.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <main.h>

#define __DEBUG_FILE_NAME__ "DEBUG_API"
#define MAX_LOG_MESSAGE_LEN 300
#define UART_TIMEOUT 100

extern UART_HandleTypeDef huart3; //TODO: check if works

static char send_buffer[MAX_LOG_MESSAGE_LEN] = {0};
static uint32_t current_copy_index = 0;
//TODO: ADD MUTEX
void DEBUG_API_LOG (char *message, char *info_string, char *error_string, ...) {
    if ((message == NULL) || (info_string == NULL)) {
        return;
    }
    current_copy_index = 0;
    memcpy(send_buffer + current_copy_index, info_string, strnlen(info_string, MAX_LOG_MESSAGE_LEN));
    current_copy_index += strnlen(info_string, MAX_LOG_MESSAGE_LEN);
    if (error_string != NULL) {
        memcpy(send_buffer + current_copy_index, error_string, strnlen(error_string, MAX_LOG_MESSAGE_LEN - current_copy_index));
        current_copy_index += strnlen(error_string, MAX_LOG_MESSAGE_LEN - current_copy_index);
    }
    va_list argptr;
    va_start(argptr, error_string);
    size_t buffer_length = vsnprintf(send_buffer + current_copy_index, MAX_LOG_MESSAGE_LEN - current_copy_index - 1, message, argptr) + current_copy_index;
    va_end(argptr);
    HAL_UART_Transmit(&huart3, (uint8_t *)send_buffer, buffer_length, UART_TIMEOUT);
}

