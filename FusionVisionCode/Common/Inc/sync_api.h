/*
 * sync_api.h
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#ifndef INC_SYNC_API_H_
#define INC_SYNC_API_H_

typedef enum eSemaphore_t {
	eSemaphoreFirst = 0,
	eSemaphoreUart = eSemaphoreFirst,
	eSemaphoreLast
}eSemaphore_t;

void Sync_API_WaitSemaphore(eSemaphore_t sem);
void Sync_API_TakeSemaphore(eSemaphore_t sem);
void Sync_API_ReleaseSemaphore(eSemaphore_t sem);

#endif /* INC_SYNC_API_H_ */
