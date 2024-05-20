#include "shared_param_api.h"
#include "shared_mem_api.h"
#include "sync_api.h"
#include <string.h>

#define NEW_PARAM(_enum, _name, _size, _default, _min, _max) [_enum] = {.name = _name, .size = _size, .default_val = _default, .max = _max, .min = _min}

#define ARR_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

    //TODO: cache resources if they were not modified
static const sSharedParam_t shared_param_lut[eSharedParamLast] = {
	NEW_PARAM(eSharedParamEdgeThreshold, "Edge", sizeof(uint32_t), 1500, 0, 10000),
	NEW_PARAM(eSharedParamActiveUiPanelIndex, "UI panel index", sizeof(uint32_t), 0, 0, 999),
	NEW_PARAM(eSharedParamActiveUiButtonIndex, "UI button index", sizeof(uint32_t), 0, 0, 999),
	NEW_PARAM(eSharedParamScreenState, "Screen", sizeof(uint32_t), eScreenStatePassthrough, eScreenStateFirst, eScreenStateLast),
	NEW_PARAM(eSharedParamScreenOptim, "Optimisation", sizeof(uint32_t), eScreenOptimNone, eScreenOptimFirst, eScreenOptimLast),
	NEW_PARAM(eSharedParamEdgeAlgorithm, "Algorithm", sizeof(uint32_t), eEdgeAlgorithmSobel, eEdgeAlgorithmFirst, eEdgeAlgorithmLast),
	NEW_PARAM(eSharedParamBatteryLevel, "Battery", sizeof(uint32_t), 69, 0, 100),
	NEW_PARAM(eSharedParamTermoState, "Termo", sizeof(uint32_t), eTermoStateEdge, eTermoStateFirst, eTermoStateLast),
	NEW_PARAM(eSharedParamTermoThreshold, "Termo threshold", sizeof(uint32_t), 100, 0, 9999)
	//NEW_PARAM(eSharedParamIrLighting, "Lighting", sizeof(uint32_t), 0, 0, 1),
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
		/* Set all buffer to default value of uint32_t type */
		//TODO: doesnt work with shared_param_api_write, why?
		uint32_t default_value = shared_param_lut[param].default_val;
		for(uint32_t i = 0; i < shared_param_lut[param].size/4; i++){
			memcpy((void *)shared_param_address_lut[param], &default_value, 4);
		}
	}
	return true;
}

bool Shared_param_API_GetDesc(eSharedParamEnum_t param, sSharedParam_t *out){
	if(param >= eSharedParamLast){
		return false;
	}
	*out = shared_param_lut[param];
	return true;
}

bool Shared_param_API_Read(eSharedParamEnum_t param, volatile void* out){ //TODO: could implement caching
	if(param >= eSharedParamLast){
		return false;
	}
	return Shared_mem_API_Read(shared_param_address_lut[param], (volatile void *)out, shared_param_lut[param].size);
}

bool Shared_param_API_Write(eSharedParamEnum_t param, volatile uint32_t* in){
	if((param >= eSharedParamLast) || (in == NULL)){
		return false;
	}
	if(Shared_mem_API_Write(shared_param_address_lut[param], in, shared_param_lut[param].size) == false){
		return false;
	}
	/* Indicate to M7 that new visual configuration written */
	if(param >= eSharedParamFirst){
		Sync_API_TakeSemaphore(eSemaphoreUiUpdate);
		Sync_API_ReleaseSemaphore(eSemaphoreUiUpdate);
	}
	return true;
}
