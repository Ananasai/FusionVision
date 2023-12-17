#include "shared_mem_api.h"
#include <string.h>

bool Shared_mem_API_Read(uint32_t address, volatile uint32_t *out){
	if((address < SHARED_MEM_START) || (address > SHARED_MEM_START + SHARED_MEM_LEN)){
		return false;
	}
	*out = *((uint32_t*)address);
	return true;
}

bool Shared_mem_API_Write(uint32_t address, volatile const uint32_t *data, size_t length){
	if((address < SHARED_MEM_START) || (address + length > SHARED_MEM_START + SHARED_MEM_LEN) || (data == NULL) || (length == 0)){
		return false;
	}
	*((volatile uint32_t*)address) = *data;
	//memcpy((void*)address, data, length); //TODO: MULTIPLE WRITES
	return true;
}
