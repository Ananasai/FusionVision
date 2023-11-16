/*
 * job_common.h
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#ifndef COMMON_JOB_COMMON_H_
#define COMMON_JOB_COMMON_H_

typedef enum eQueueEnum_t {
	eQueueFirst = 0,
	eQueueLighting,
	eQueueLast
}eQueueEnum_t;

typedef enum eJobEnum_t {
	eJobFirst = 0,
	eJobLast
}eJobEnum_t;

typedef struct sJob_t {
	eJobEnum_t job;
	void *payload;
}sJob_t;

#endif /* COMMON_JOB_COMMON_H_ */
