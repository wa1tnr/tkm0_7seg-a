/*
 * pin.h
 *
 * Created: 2/5/2018 11:21:47 PM
 *  Author: Jake
 */ 


#ifndef PIN_H_
#define PIN_H_

#include "sam.h"

typedef struct {
	PortGroup *port;
	uint32_t bm; // bitmask
} pin_t;

pin_t pin_new(PortGroup *port, uint32_t pin);

void pin_output(pin_t *pin);
void pin_input(pin_t *pin);

void pin_set(pin_t *pin);
void pin_clear(pin_t *pin);
void pin_toggle(pin_t *pin);

int pin_read(pin_t *pin);

#endif /* PIN_H_ */