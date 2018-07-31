/*
 * hardware.h
 *
 * Created: 6/18/2018 12:18:05 PM
 *  Author: Jake
 */ 


#ifndef HARDWARE_H_
#define HARDWARE_H_

#include "pin.h"
#include "ringbuffer.h"
#include "uartport.h"
#include "spiport.h"
#include "atkport.h"
#include "atkhandler.h"
#include "tmc2130.h"
#include "stepper.h"

// results in 1MBaud
#define SYSTEM_BAUDA 3
#define SYSTEM_BAUDB 0
#define SYSTEM_NUM_UPS 1

pin_t stlclk;
pin_t stlerr;

// UP0

ringbuffer_t up0rbrx;
ringbuffer_t up0rbtx;

uartport_t up0;

pin_t up0rxled;
pin_t up0txled;

atkport_t atkp0;

// UPS

uartport_t *ups[SYSTEM_NUM_UPS];

// Stepper
// SPI

spiport_t spitmc;
pin_t spicspin;

pin_t tmcen;
pin_t tmcdiag0;
pin_t tmcdiag1;
pin_t step;
pin_t dir;

pin_t tmciref;

tmc2130_t tmc;

stepper_t stepper;

unsigned long timebase;

#endif /* HARDWARE_H_ */