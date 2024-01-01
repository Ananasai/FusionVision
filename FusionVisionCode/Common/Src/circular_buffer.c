#include "circular_buffer.h"
#include <stddef.h>
#include <stdlib.h>

static inline void Advance_pointer(uint32_t length, uint32_t *pointer){
	if(*pointer == length-1){
		*pointer = 0;
	}else{
		(*pointer)++;
	}
}

bool Circular_buffer_create(sCircularBuffer_t *circ, uint32_t length){
	if(circ == NULL){
		return false;
	}
	circ->max = length;
	circ->buffer = (uint8_t *)calloc(length, sizeof(uint8_t));
	circ->head = 0;
	circ->tail = 0;
	return true;
}

bool Circular_buffer_pop(sCircularBuffer_t *circ, uint8_t *out){
	if((circ == NULL) || (out == NULL)){
		return false;
	}
	if(circ->head == circ->tail){
		return false;
	}
	*out = *(circ->buffer + circ->tail);
	Advance_pointer(circ->max, &(circ->tail));
	return true;
}

bool Circular_buffer_push(sCircularBuffer_t *circ, uint8_t in){
	if(circ == NULL){
		return false;
	}
	*(circ->buffer+circ->head) = in;
	Advance_pointer(circ->max, &(circ->head));
	return true;
}

//TODO: circ buffer deinit
//TODO: handle overflow, make variable in struct if full
