#include "sync_api.h"
#include "main.h"
#include <stm32h7xx_hal_hsem.h>

#ifdef CORE_CM7
#define PROCESS_ID 0
#else
#define PROCESS_ID 1
#endif

bool Sync_API_WaitSemaphore(eSemaphore_t sem){
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

bool Sync_API_ReleaseAll(void){
	HAL_HSEM_ReleaseAll(0xFFFF, PROCESS_ID);
	return true;
}
