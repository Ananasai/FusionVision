/*
 * sync_api.h
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#ifndef INC_SYNC_API_H_
#define INC_SYNC_API_H_

#include <stdbool.h>

#define SHARED_MEM_START 0x38000000
#define SHARED_MEM_LEN 0x0000FFFF

typedef enum eSemaphore_t {
	eSemaphoreFirst = 0,
	eSemaphoreCore = eSemaphoreFirst, //DEFAULT FOR STARTING CORES DO NOT CHANGE
	eSemaphoreUart,
	eSemaphoreLast
}eSemaphore_t;

bool Sync_API_WaitSemaphore(eSemaphore_t sem);
bool Sync_API_TakeSemaphore(eSemaphore_t sem);
bool Sync_API_ReleaseSemaphore(eSemaphore_t sem);
bool Sync_API_ReleaseAll(void);

#endif /* INC_SYNC_API_H_ */
