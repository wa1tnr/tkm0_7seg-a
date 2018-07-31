/*
 * pin.c
 *
 * Created: 6/18/2018 12:22:50 PM
 *  Author: Jake
 */ 

#include "pin.h"

void pin_init(pin_t *pin, PORT_t *port, uint8_t pin_bm, uint8_t pin_pos, uint8_t inout){
	pin->port = port;
	pin->pin_bm = pin_bm;
	pin->pin_pos = pin_pos;
	if(inout){
		pin_output(pin);
	} else {
		pin_input(pin);
	}
}

void pin_output(pin_t *pin){
	pin->port->DIRSET = pin->pin_bm;
}

void pin_set(pin_t *pin){
	pin->port->OUTSET = pin->pin_bm;
}
void pin_clear(pin_t *pin){
	pin->port->OUTCLR = pin->pin_bm;
}

void pin_toggle(pin_t *pin){
	pin->port->OUTTGL = pin->pin_bm;
}

void pin_input(pin_t *pin){
	pin->port->DIRCLR = pin->pin_bm;
}

void pin_pullup(pin_t *pin){
	// ? how to access PIN0CTRL, etc?
}
void pin_pulldown(pin_t *pin){
	// 
}

uint8_t pin_read(pin_t *pin){
	return (pin->port->IN & pin->pin_bm);
}