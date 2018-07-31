/*
 * pin.h
 *
 * Created: 6/18/2018 12:22:58 PM
 *  Author: Jake
 */ 


#ifndef PIN_H_
#define PIN_H_

#include "avr/io.h"

typedef struct {
	PORT_t *port;
	uint8_t pin_bm;
	uint8_t pin_pos;
}pin_t;

void pin_init(pin_t *pin, PORT_t *port, uint8_t pin_bm, uint8_t pin_pos, uint8_t inout);

void pin_output(pin_t *pin);
void pin_set(pin_t *pin);
void pin_clear(pin_t *pin);
void pin_toggle(pin_t *pin);

void pin_input(pin_t *pin);
void pin_pullup(pin_t *pin);
void pin_pulldown(pin_t *pin);

uint8_t pin_read(pin_t *pin);


#endif /* PIN_H_ */