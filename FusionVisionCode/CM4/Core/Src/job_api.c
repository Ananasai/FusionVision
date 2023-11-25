/*
 * job_api.c
 *
 *  Created on: Nov 16, 2023
 *      Author: simon
 */

#include "job_api.h"
#include "debug_api.h"
#include "string_common.h"
#include <cmsis_os2.h>
#include <stddef.h>

#define __DEBUG_FILE_NAME__ "JOB_API"
#define QUEUE_DESC(q_enum, name, length) [q_enum] = {(sString_t){name, sizeof(name)} ,NULL, length}

typedef struct sQueue_t {
	sString_t name;
	osMessageQueueId_t id;
	size_t length;
}sQueue_t;

static sQueue_t queue_lut[eQueueLast] = {
		//QUEUE_DESC(eQueueLighting, "LIGHTING", 10)
		[eQueueLighting] = {(sString_t){"LIGHTING", sizeof("LIGHTING")}, NULL, 10}
};

bool Job_API_AddNew(eQueueEnum_t queue, sJob_t job_in){
	if((queue >= eQueueLast) || (job_in.job >= eJobLast)){
		return false;
	}
	if(queue_lut[queue].id == NULL){
		return false;
	}
	if(osMessageQueuePut(queue_lut[queue].id, &job_in, 0, 0) != osOK){
		error("Failed adding to queue %s\r\n", queue_lut[queue].name.string);
		return false;
	}
	return true;
}

bool Job_API_WaitNew(eQueueEnum_t queue, sJob_t *job_out){
	if((queue >= eQueueLast) || (job_out == NULL)){
		return false;
	}
	if(queue_lut[queue].id == NULL){
		return false;
	}
	if(osMessageQueueGet(queue_lut[queue].id, (void *)job_out, 0, osWaitForever) != osOK){
		error("Failed waiting for queue %s\r\n", queue_lut[queue].name.string);
		return false;
	}
	return true;
}

bool Job_API_CreateQueue(eQueueEnum_t queue){
	if(queue >= eQueueLast){
		return false;
	}
	if(queue_lut[queue].id != NULL){
		osMessageQueueDelete(queue_lut[queue].id);
	}
	queue_lut[queue].id = osMessageQueueNew(queue_lut[queue].length, sizeof(sJob_t), NULL);
	if(queue_lut[queue].id == NULL){
		error("Creating queue of %s\r\n", *queue_lut[queue].name.string);
		return false;
	}
	return true;
}

bool Job_API_InitAll(void){
	for(eQueueEnum_t queue = eQueueFirst; queue < eQueueLast; queue++){
		if(Job_API_CreateQueue(queue) == false){
			return false;
		}
	}
	return true;
}
