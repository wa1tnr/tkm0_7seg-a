/*
 * ringbuffer.c
 *
 * Created: 2/7/2018 11:39:44 AM
 *  Author: Jake
 */ 

#include "ringbuffer.h"

uint8_t rb_init(ringbuffer_t *rb){
	rb->size = RINGBUFFER_SIZE; // stuck with this, due to not having malloc, wall of skill
	//rb->buffer = malloc(size);
	rb_reset(rb);
	return 1;
}

uint8_t rb_reset(ringbuffer_t *rb){
	if(rb){
		rb->head = 0;
		rb->tail = 0;
		return 1;
	} else {
		return 0;
	}
}

uint8_t rb_empty(ringbuffer_t *rb){
	return (rb->head == rb->tail);
}

uint8_t rb_full(ringbuffer_t *rb){
	// read from tail, update at head
	// if head is 'just behind' tail (in ring) we have no extra space: the
	return ((rb->head + 1) % rb->size) == rb->tail;
}

uint8_t rb_freespace(ringbuffer_t *rb){
	if(rb->head >= rb->tail){
		return rb->size - (rb->head - rb->tail);
	} else {
		return rb->tail - rb->head - 1;
	}
}

uint8_t rb_putchar(ringbuffer_t *rb, uint8_t data){
	rb->buffer[rb->head] = data;
	rb->head = (rb->head + 1) % rb->size; // increment and loop about
	return 1;
}

uint8_t rb_putdata(ringbuffer_t *rb, uint8_t *data, uint8_t size){
	/*
	if(rb_freespace(rb) >= size){
		// rb_freespace, not working?
		return 0;
	} else {
		*/
	for(int i = 0; i < size; i ++){
		rb_putchar(rb, data[i]);
	}
	
	return 1;
	//}
}

uint8_t rb_get(ringbuffer_t *rb){
	uint8_t data = rb->buffer[rb->tail];
	rb->tail = (rb->tail + 1) % rb->size;
	return data;
}

