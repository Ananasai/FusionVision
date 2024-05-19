/*
 * debug_api.h
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#ifndef INC_SHARED_MEM_API_H_
#define INC_SHARED_MEM_API_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

//https://github.com/MaJerle/stm32h7-dual-core-inter-cpu-async-communication/blob/main/projects/nucleo_stm32h745_q_aync_comm/Common/Inc/common.h
#define SHARED_MEM_START 0x38000000
#define SHARED_MEM_LEN 0x0000FFFF

#define SHARED_TERMO_BUF_START (SHARED_MEM_START + 100)


bool Shared_mem_API_Read(uint32_t address, volatile void *out, size_t length);
bool Shared_mem_API_Write(uint32_t address, volatile const void *data, size_t length);

#endif /* INC_SHARED_MEM_API_H_ */
