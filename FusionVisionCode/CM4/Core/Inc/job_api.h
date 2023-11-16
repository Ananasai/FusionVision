/*
 * job_api.h
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#ifndef INC_JOB_API_H_
#define INC_JOB_API_H_

#include "job_common.h"
#include <stdbool.h>

bool Job_API_AddNew(eQueueEnum_t queue, sJob_t job_in);
bool Job_API_WaitNew(eQueueEnum_t queue, sJob_t *job_out);
bool Job_API_CreateQueue(eQueueEnum_t queue);
bool Job_API_InitAll(void);

#endif /* INC_JOB_API_H_ */
