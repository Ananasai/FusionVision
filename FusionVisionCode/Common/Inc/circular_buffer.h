#ifndef INC_CIRCULAR_BUFFER_H_
#define INC_CIRCULAR_BUFFER_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct sCircularBuffer_t {
	uint8_t *buffer;
	uint32_t head;
	uint32_t tail;
	uint32_t max;
}sCircularBuffer_t;

bool Circular_buffer_create(sCircularBuffer_t *circ, uint32_t length);
bool Circular_buffer_pop(sCircularBuffer_t *circ, uint8_t *out);
bool Circular_buffer_push(sCircularBuffer_t *circ, uint8_t in);

#endif /* INC_CIRCULAR_BUFFER_H_ */
