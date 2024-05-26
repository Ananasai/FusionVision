#include "sync_api.h"
#include "main.h"
#include <stm32h7xx_hal_hsem.h>

#ifdef CORE_CM7
#define PROCESS_ID 0
#else
#define PROCESS_ID 1
#endif

/*
 * Array of callbacks to semaphore take IRQs. //TODO: make several callbacks available.
 */
void (*semaphore_callback_lut[eSemaphoreLast])(void) = {0};

/*
 * Wait forever for semaphore to be free.
 */
bool Sync_API_WaitSemaphore(eSemaphore_t sem){ //TODO; implement timeout?
	while(HAL_HSEM_Take(sem, PROCESS_ID) != HAL_OK){

	}
	return true;
}

/*
 * Take semaphore.
 * Returns true if taken succesfully.
 */
bool Sync_API_TakeSemaphore(eSemaphore_t sem){
	return HAL_HSEM_Take(sem, PROCESS_ID) == HAL_OK ? true : false;
}

/*
 * Releases taken semaphore.
 */
bool Sync_API_ReleaseSemaphore(eSemaphore_t sem){
	HAL_HSEM_Release(sem, PROCESS_ID);
	return true;
}

/*
 * Releases all semaphores of the current core.
 */
bool Sync_API_ReleaseSemaphoreAll(void){
	HAL_HSEM_ReleaseAll(0xFFFF, PROCESS_ID);
	return true;
}

/*
 * Register a callback if a specific semaphore is released.
 * Currently only one callback is supported.
 */
bool Sync_API_ActivateSemaphoreIrq(eSemaphore_t sem, void (*new_callback)(void)){
	semaphore_callback_lut[sem] = new_callback;
	HAL_HSEM_ActivateNotification(0x1 << sem);
	return true;
}

/*
 * Semaphore released IRQ.
 */
void HAL_HSEM_FreeCallback(uint32_t sem_mask){
	/* Check which semaphore was released */
	for(uint8_t i = 0; i < eSemaphoreLast; i++){
		if(((sem_mask >> i) & 0x01) == 0x01){
			/* i semaphore released - call callback if one is defined */
			if(semaphore_callback_lut[i] == NULL){
				break;
			}
			(*semaphore_callback_lut[i])();
			break;
		}
	}
	/* Reeactivate semaphore IRQ */
	HAL_HSEM_ActivateNotification(sem_mask);
}

