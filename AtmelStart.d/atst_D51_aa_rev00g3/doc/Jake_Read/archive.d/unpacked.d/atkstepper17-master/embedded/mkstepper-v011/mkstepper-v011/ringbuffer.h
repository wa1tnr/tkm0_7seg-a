/*
 * ringbuffer.h
 *
 * Created: 2/7/2018 11:39:54 AM
 *  Author: Jake
 */ 

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

/*
a ringbuffer,
s/o https://github.com/dhess/c-ringbuf
s/o https://embeddedartistry.com/blog/2017/4/6/circular-buffers-in-cc
s/o https://www.downtowndougbrown.com/2013/01/microcontrollers-interrupt-safe-ring-buffers/
*/

#include "sam.h"

#include <stdlib.h> // for size_t

#define RINGBUFFER_SIZE 256

typedef struct{
	uint8_t buffer[256]; // static! big enough
	size_t head;
	size_t tail;
	size_t size;
} ringbuffer_t;

uint8_t rb_init(ringbuffer_t *rb);

uint8_t rb_reset(ringbuffer_t *rb);

uint8_t rb_empty(ringbuffer_t *rb);
uint8_t rb_full(ringbuffer_t *rb);
uint8_t rb_freespace(ringbuffer_t *rb);

uint8_t rb_putchar(ringbuffer_t *rb, uint8_t data);
uint8_t rb_putdata(ringbuffer_t *rb, uint8_t *data, uint8_t size);

uint8_t rb_get(ringbuffer_t *rb);

#endif /* RINGBUFFER_H_ */