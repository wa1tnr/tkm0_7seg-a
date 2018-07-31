/*
 * hardware.h
 *
 * Created: 2/5/2018 11:50:30 PM
 *  Author: Jake
 */ 


#ifndef HARDWARE_H_
#define HARDWARE_H_

#include "pin.h"
#include "uartport.h"
#include "apaport.h"
#include "spiport.h"
#include "ringbuffer.h"
#include "tmc26.h"
#include "stepper.h"

// LIGHTS

pin_t stlr;
pin_t stlb;

// PORTS

pin_t up1_stlr;
pin_t up1_stlb;

ringbuffer_t up1_rbrx;
ringbuffer_t up1_rbtx;

uartport_t up0;
uartport_t up1;

// array of ptrs to uartports
// inits in main.c
#define NUM_UPS 2
uartport_t *ups[NUM_UPS];

apaport_t apap0;
apaport_t apap1;

// TIMERS

uint32_t overflows;

uint32_t blindcounter;

#define STEPTICKER_SYNC (TC2->COUNT32.CTRLBSET.reg = TCC_CTRLBSET_CMD_READSYNC)
#define STEPTICKER (TC2->COUNT32.COUNT.reg)

#define TICKER_SYNC (TC0->COUNT32.CTRLBSET.reg = TCC_CTRLBSET_CMD_READSYNC)
#define TICKER (TC0->COUNT32.COUNT.reg)

// TMC26x

spiport_t spi_tmc;

pin_t step_pin;
pin_t dir_pin;
pin_t en_pin;
pin_t sg_pin;

tmc26_t tmc;

// STEPPER

stepper_t stepper;

// DEBUG

pin_t tickup0tx;

#endif /* HARDWARE_H_ */