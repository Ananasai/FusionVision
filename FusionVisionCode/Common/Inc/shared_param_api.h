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

typedef enum eSharedParamEnum_t {
	eSharedParamFirst = 0,
	eSharedParamEdgeThreshold = eSharedParamFirst,
	eSharedParamActiveUiPanelIndex,
	eSharedParamActiveUiButtonIndex,
	eSharedParamScreenState,
	eSharedParamScreenOptim,
	eSharedParamEdgeAlgorithm,
	eSharedParamBatteryLevel,
	eSharedParamLast
}eSharedParamEnum_t;

typedef enum eScreenState_t {
	eScreenStateFirst = 0,
	eScreenStatePassthrough = eScreenStateFirst,
	eScreenStateProcessed,
	eScreenStateLast
}eScreenState_t;

typedef enum eScreenOptim_t {
	eScreenOptimFirst = 0,
	eScreenOptimNone = eScreenOptimFirst,
	eScreenOptimInterlacedProcessing,
	eScreenOptimInterlacedAll,
	eScreenOptimLast
}eScreenOptim_t;

typedef enum eEdgeAlgorithm_t {
	eEdgeAlgorithmFirst = 0,
	eEdgeAlgorithmSobel,
	eEdgeAlgorithmRoberts,
	eEdgeAlgorithmLast
}eEdgeAlgorithm_t;

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
