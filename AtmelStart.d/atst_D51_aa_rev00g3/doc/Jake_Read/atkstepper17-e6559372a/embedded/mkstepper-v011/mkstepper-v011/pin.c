/*
 * pin.c
 *
 * Created: 2/5/2018 11:21:37 PM
 *  Author: Jake
 */ 

#include "pin.h"
#include "sam.h"

pin_t pin_new(PortGroup *port, uint32_t pin_number){
	pin_t pin;
	pin.port = port;
	pin.bm = (uint32_t)(1 << pin_number);
	return pin;
}


void pin_output(pin_t *pin){
	pin->port->DIRSET.reg = pin->bm;
	pin->port->OUTCLR.reg = pin->bm;
}

void pin_input(pin_t *pin){
	pin->port->DIRCLR.reg = pin->bm;
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