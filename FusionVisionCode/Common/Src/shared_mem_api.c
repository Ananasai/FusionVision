#include "shared_mem_api.h"
#include "sync_api.h"
#include <string.h>

bool Shared_mem_API_Read(uint32_t address, volatile void *out, size_t length){
	if((address < SHARED_MEM_START) || (address > SHARED_MEM_START + SHARED_MEM_LEN)){
		return false;
	}
	if(Sync_API_WaitSemaphore(eSemaphoreSharedMem) == false){
		return false;
	}
	memcpy((void *)out,(void *)address, length);
	//*out = *((uint32_t*)address);
	Sync_API_ReleaseSemaphore(eSemaphoreSharedMem);
	return true;
}

bool Shared_mem_API_Write(uint32_t address, volatile const void *data, size_t length){
	if((address < SHARED_MEM_START) || (address + length > SHARED_MEM_START + SHARED_MEM_LEN) || (data == NULL) || (length == 0)){
		return false;
	}
	if(Sync_API_WaitSemaphore(eSemaphoreSharedMem) == false){
		return false;
	}
	memcpy((void *)address, (void *)data, length);
	//*((volatile uint32_t*)address) = *data;
	//memcpy((void*)address, data, length); //TODO: MULTIPLE WRITES
	Sync_API_ReleaseSemaphore(eSemaphoreSharedMem);
	return true;
}
