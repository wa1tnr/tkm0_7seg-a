/*
 * pin.c
 *
 * Created: 2/5/2018 11:21:37 PM
 *  Author: Jake
 */ 

#include "pin.h"
#include "sam.h"

pin_t pin_new(uint32_t portNumber, uint32_t pin_number){
	pin_t pin;
	pin.port = &PORT->Group[portNumber];
	pin.bm = (uint32_t)(1 << pin_number);
	pin.pos = pin_number;
	return pin;
}

void pin_output(pin_t *pin){
	pin->port->DIRSET.reg = pin->bm;
	pin->port->OUTCLR.reg = pin->bm;
}

void pin_input(pin_t *pin){
	pin->port->DIRCLR.reg = pin->bm;
	pin->port->PINCFG[pin->pos].bit.INEN = 1;
}

void pin_pullen(pin_t *pin){
	pin->port->PINCFG[pin->pos].bit.PULLEN = 1;
}

void pin_set(pin_t *pin){
	pin->port->OUTSET.reg = pin->bm;
}

void pin_clear(pin_t *pin){
	pin->port->OUTCLR.reg = pin->bm;
}

void pin_toggle(pin_t *pin){
	pin->port->OUTTGL.reg = pin->bm;
}

int pin_read(pin_t *pin){
	return pin->port->IN.reg & pin->bm;
}
