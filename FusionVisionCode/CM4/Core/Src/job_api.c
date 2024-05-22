///*
// * job_api.c
// *
// *  Created on: Nov 16, 2023
// *      Author: simon
// */
//
//#include "job_api.h"
//#include "debug_api.h"
//#include "string_common.h"
//#include <stddef.h>
//#include <stdlib.h>
//
//#define __DEBUG_FILE_NAME__ "JOB"
//#define QUEUE_DESC(_enum, _name, _length) [_enum] = {(sString_t){_name, sizeof(_name)}, NULL, _length}
//
//typedef struct sQueue_t {
//	sString_t name;
//	osMessageQueueId_t id;
//	size_t length;
//}sQueue_t;
//
//static sQueue_t queue_lut[eQueueLast] = {
//	QUEUE_DESC(eQueueLighting, "LIGHTING", 10),
//	QUEUE_DESC(eQueuePowerTrack, "POWER TRACK", 10)
//};
//
//bool Job_API_AddNew(eQueueEnum_t queue, sJob_t job_in){
//	if((queue >= eQueueLast) || (job_in.job >= eJobLast)){
//		return false;
//	}
//	if(queue_lut[queue].id == NULL){
//		return false;
//	}
//	if(osMessageQueuePut(queue_lut[queue].id, &job_in, 0, 0) != osOK){
//		error("Failed adding to queue %s\r\n", queue_lut[queue].name.text);
//		return false;
//	}
//	return true;
//}
//
//bool Job_API_WaitNew(eQueueEnum_t queue, const sJobCallbackDesc_t *callbacks, uint32_t callback_amount){
//	if((queue >= eQueueLast) || (callbacks == NULL) | (callback_amount == 0)){
//		return false;
//	}
//	if(queue_lut[queue].id == NULL){
//		return false;
//	}
//	sJob_t current_job;
//	if(osMessageQueueGet(queue_lut[queue].id, (void *)&current_job, 0, osWaitForever) != osOK){
//		error("Failed waiting for queue %s\r\n", queue_lut[queue].name.text);
//		return false;
//	}
//	for(uint32_t i = 0; i < callback_amount; i++){
//		if((callbacks + i)->job == current_job.job){
//			/* Found required callback */
//			((callbacks + i)->callback)(current_job.payload);
//			if(current_job.payload != NULL){
//				free(current_job.payload);
//			}
//			return true;
//		}
//	}
//	/* Couldn't find required job */
//	error("Could find required job callback\r\n");
//	return false;
//}
//
//bool Job_API_CreateQueue(eQueueEnum_t queue){
//	if(queue >= eQueueLast){
//		return false;
//	}
//	if(queue_lut[queue].id != NULL){
//		osMessageQueueDelete(queue_lut[queue].id);
//	}
//	queue_lut[queue].id = osMessageQueueNew(queue_lut[queue].length, sizeof(sJob_t), NULL);
//	if(queue_lut[queue].id == NULL){
//		error("Creating queue of %s\r\n", *queue_lut[queue].name.text);
//		return false;
//	}
//	return true;
//}
//
//bool Job_API_InitAll(void){
//	for(eQueueEnum_t queue = eQueueFirst; queue < eQueueLast; queue++){
//		if(Job_API_CreateQueue(queue) == false){
//			return false;
//		}
//	}
//	return true;
//}
