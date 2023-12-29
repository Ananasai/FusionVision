#include "sync_api.h"
#include "main.h"
#include <stm32h7xx_hal_hsem.h>

#ifdef CORE_CM7
#define PROCESS_ID 0
#else
#define PROCESS_ID 1
#endif

void (*semaphore_callback_lut[eSemaphoreLast])(void) = {0};


bool Sync_API_WaitSemaphore(eSemaphore_t sem){
	//while(HAL_HSEM_IsSemTaken(sem) == 1){ //TODO: not working

	//}
	while(HAL_HSEM_Take(sem, PROCESS_ID) != HAL_OK){

	}
	return true;
}

bool Sync_API_TakeSemaphore(eSemaphore_t sem){
	return HAL_HSEM_Take(sem, PROCESS_ID) == HAL_OK ? true : false;
}

bool Sync_API_ReleaseSemaphore(eSemaphore_t sem){
	HAL_HSEM_Release(sem, PROCESS_ID);
	return true;
}

bool Sync_API_ReleaseSemaphoreAll(void){
	HAL_HSEM_ReleaseAll(0xFFFF, PROCESS_ID);
	return true;
}

bool Sync_API_ActivateSemaphoreIrq(eSemaphore_t sem, void (*new_callback)(void)){
	semaphore_callback_lut[sem] = new_callback;
	HAL_HSEM_ActivateNotification(0x1 << sem);
	return true;
}

/* HSEM released IRQ, call desired callback*/
void HAL_HSEM_FreeCallback(uint32_t sem_mask){
	for(uint8_t i = 0; i < eSemaphoreLast; i++){
		if(((sem_mask >> i) & 0x01) == 0x01){
			if(semaphore_callback_lut[i] == NULL){
				break;
			}
			(*semaphore_callback_lut[i])();
			break;
		}
	}
	/* Reeactivate semaphore irq */
	HAL_HSEM_ActivateNotification(sem_mask);
}

