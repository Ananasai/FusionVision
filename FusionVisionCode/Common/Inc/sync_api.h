/*
 * sync_api.h
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#ifndef INC_SYNC_API_H_
#define INC_SYNC_API_H_

#include <stdbool.h>
//https://github.com/MaJerle/stm32h7-dual-core-inter-cpu-async-communication/blob/main/projects/nucleo_stm32h745_q_aync_comm/Common/Inc/common.h
#define SHARED_MEM_START 0x38000000
#define SHARED_MEM_LEN 0x0000FFFF

typedef enum eSemaphore_t {
	eSemaphoreFirst = 0,
	eSemaphoreCore = eSemaphoreFirst, //DEFAULT FOR STARTING CORES DO NOT CHANGE
	eSemaphoreUart,
	eSemaphoreButton,
	eSemaphoreLast
}eSemaphore_t;

bool Sync_API_WaitSemaphore(eSemaphore_t sem);
bool Sync_API_TakeSemaphore(eSemaphore_t sem);
bool Sync_API_ReleaseSemaphore(eSemaphore_t sem);
bool Sync_API_ReleaseAll(void);
bool Sync_API_ActivateSemaphoreIrq(eSemaphore_t sem);

#endif /* INC_SYNC_API_H_ */
