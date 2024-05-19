/*
 * debug_api.h
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#ifndef INC_SHARED_PARAM_API_H_
#define INC_SHARED_PARAM_API_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum eSharedParamEnum {
	eSharedParamFirst = 0,
	eSharedParamEdgeThreshold = eSharedParamFirst,
	eSharedParamActiveUiPanelIndex,
	eSharedParamActiveUiButtonIndex,
	eSharedParamScreenState,
	eSharedParamScreenOptim,
	eSharedParamEdgeAlgorithm,
	eSharedParamBatteryLevel,
	eSharedParamTermoState,
	eSharedParamTermoThreshold,
	eSharedParamLast
}eSharedParamEnum_t;

typedef enum eScreenState {
	eScreenStateFirst = 0,
	eScreenStatePassthrough = eScreenStateFirst,
	eScreenStateProcessed,
	eScreenStateLast
}eScreenState_t;

typedef enum eScreenOptim {
	eScreenOptimFirst = 0,
	eScreenOptimNone = eScreenOptimFirst,
	eScreenOptimInterlacedProcessing,
	eScreenOptimInterlacedAll,
	eScreenOptimLast
}eScreenOptim_t;

typedef enum eEdgeAlgorithm {
	eEdgeAlgorithmFirst = 0,
	eEdgeAlgorithmSobel = eEdgeAlgorithmFirst,
	eEdgeAlgorithmRoberts,
	eEdgeAlgorithmLast
}eEdgeAlgorithm_t;

typedef enum eTermoState {
	eTermoStateFirst = 0,
	eTermoStateDisabled = eTermoStateFirst,
	eTermoStatePassthrough,
	eTermoStateThreshold,
	eTermoStateEdge,
	eTermoStateLast
}eTermoState_t;

typedef struct sSharedParam_t{
	char *name;
	size_t size; //BYTES
	uint32_t default_val;
	uint32_t min;
	uint32_t max;
}sSharedParam_t;

bool Shared_param_API_Init(void);
bool Shared_param_API_GetDesc(eSharedParamEnum_t param, sSharedParam_t *out);
bool Shared_param_API_Read(eSharedParamEnum_t param, volatile void* out);
bool Shared_param_API_Write(eSharedParamEnum_t param, volatile uint32_t* in);

#endif /* INC_SHARED_PARAM_API_H_ */
