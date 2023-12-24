#include "shared_param_api.h"
#include "shared_mem_api.h"
#include "sync_api.h"
#include <string.h>

#define NEW_PARAM(_enum, _name, _size, _default) [_enum] = {.name = _name, .size = _size, .default_val = _default}

#define ARR_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef struct sSharedParam_t{
	char *name;
	size_t size; //BYTES
	uint32_t default_val;
}sSharedParam_t;

static const sSharedParam_t shared_param_lut[eSharedParamLast] = {
	NEW_PARAM(eSharedParamEdgeThreshold, "Edge threshold", sizeof(uint32_t), 5),
	NEW_PARAM(eSharedParamActiveUiPanelIndex, "UI panel index", sizeof(uint32_t), 0),
	NEW_PARAM(eSharedParamActiveUiButtonIndex, "UI button index", sizeof(uint32_t), 0),
	NEW_PARAM(eSharedParamScreenState, "Screen state", sizeof(uint32_t), 1),
	NEW_PARAM(eSharedParamScreenOptim, "Screen optimisation", sizeof(uint32_t), 1)
};

static uint32_t shared_param_address_lut[eSharedParamLast] = { 0 };

/* Init shared parameter addresses in shared SRAM region */
bool Shared_param_API_Init(void){
	uint32_t last_address = SHARED_MEM_START;
	for(eSharedParamEnum_t param = 0; param < ARR_LENGTH(shared_param_lut); param++){
		shared_param_address_lut[param] = last_address;
		last_address += shared_param_lut[param].size;
		if(last_address >= SHARED_MEM_START + SHARED_MEM_LEN){
			return false;
		}
		//TODO: change for non uint32_t type
		uint32_t default_value = shared_param_lut[param].default_val;
		*((volatile uint32_t*)shared_param_address_lut[param]) = default_value;
		//Shared_param_API_Write(param, &default_value, shared_param_lut[param].size);
		/* Clear buffer */
		//memset((void *)shared_param_address_lut[param], 0, shared_param_lut[param].size);
	}
	return true;
}

bool Shared_param_API_Read(eSharedParamEnum_t param, volatile void* out){
	if(param >= eSharedParamLast){
		return false;
	}
	return Shared_mem_API_Read(shared_param_address_lut[param], (volatile void *)out);
}

bool Shared_param_API_Write(eSharedParamEnum_t param, volatile uint32_t* in, size_t length){
	if((param >= eSharedParamLast) || (in == NULL)){
		return false;
	}
	if(length > shared_param_lut[param].size){
		return false;
	}
	if(Shared_mem_API_Write(shared_param_address_lut[param], in, length) == false){
		return false;
	}
	/* Indicate to M7 that new visual configuration written */
	if(param > eSharedParamFirst){
		Sync_API_TakeSemaphore(eSemaphoreUiUpdate);
		Sync_API_ReleaseSemaphore(eSemaphoreUiUpdate);
	}
	return true;
}
