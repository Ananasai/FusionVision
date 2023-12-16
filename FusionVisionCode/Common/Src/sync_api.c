#include "sync_api.h"
#include "main.h"
#include <stm32h7xx_hal_hsem.h>

void Sync_API_WaitSemaphore(eSemaphore_t sem){
	while(HAL_HSEM_FastTake(1) != HAL_OK){

	}
}

void Sync_API_TakeSemaphore(eSemaphore_t sem){

}

void Sync_API_ReleaseSemaphore(eSemaphore_t sem){
	HAL_HSEM_Release(1, 0);
}
