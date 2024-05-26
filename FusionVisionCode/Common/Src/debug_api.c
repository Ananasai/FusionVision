#include "debug_api.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <main.h>
#include "sync_api.h"

#define __DEBUG_FILE_NAME__ "DEBUG_API"
#define MAX_LOG_MESSAGE_LEN 300
#define UART_TIMEOUT 100

static char send_buffer[MAX_LOG_MESSAGE_LEN] = {0};
static uint32_t current_copy_index = 0;

static UART_HandleTypeDef huart3;

bool Debug_API_Start(UART_HandleTypeDef huart){
	huart3 = huart;
	return true;
}

//TODO: check max message length
void DEBUG_API_LOG (char *message, char *info_string, char *error_string, ...) {
    if (message == NULL) {
        return;
    }
    Sync_API_WaitSemaphore(eSemaphoreUart);
    current_copy_index = 0;
    if(info_string != NULL){
		memcpy(send_buffer + current_copy_index, info_string, strnlen(info_string, MAX_LOG_MESSAGE_LEN));
		current_copy_index += strnlen(info_string, MAX_LOG_MESSAGE_LEN);
    }
    if (error_string != NULL) {
        memcpy(send_buffer + current_copy_index, error_string, strnlen(error_string, MAX_LOG_MESSAGE_LEN - current_copy_index));
        current_copy_index += strnlen(error_string, MAX_LOG_MESSAGE_LEN - current_copy_index);
    }
    va_list argptr;
    va_start(argptr, error_string);
    size_t buffer_length = vsnprintf(send_buffer + current_copy_index, MAX_LOG_MESSAGE_LEN - current_copy_index - 1, message, argptr) + current_copy_index;
    va_end(argptr);
    HAL_UART_Transmit(&huart3, (uint8_t *)send_buffer, buffer_length, UART_TIMEOUT);
    Sync_API_ReleaseSemaphore(eSemaphoreUart);
}

