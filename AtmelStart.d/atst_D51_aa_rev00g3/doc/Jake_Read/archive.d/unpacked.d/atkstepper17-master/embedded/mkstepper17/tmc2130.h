/*
 * tmc2660.h
 *
 * Created: 2/7/2018 10:17:49 PM
 *  Author: Jake
 */ 


#ifndef TMC2130_H_
#define TMC2130_H_

#include "sam.h"
#include "spiport.h"
#include "pin.h"

// TODO: adding updates (microstep, current)
// in that, calculating current
// TODO: reading stallguard, understanding if is already doing closed loop?
// CURRENTLY: reading TMC2130 datasheet and re-writing setup etc,
// then do turning the motor, then do pack-acks with stallguard data ?

typedef struct{
	spiport_t *spi;
	
	pin_t *en_pin;
	pin_t *diag0_pin;
	pin_t *diag1_pin;
}tmc2130_t;

tmc2130_t tmc2130_new(spiport_t *spi, pin_t *en, pin_t *diag0, pin_t *diag1);

void tmc2130_write(tmc2130_t *tmc, uint8_t address, uint32_t word);

void tmc2130_init(tmc2130_t *tmc);

void tmc2130_setCurrents(tmc2130_t *tmc, uint8_t ihold, uint8_t irun);

void tmc2130_update(tmc2130_t *tmc);

void tmc2130_enable(tmc2130_t *tmc);

void tmc2130_disable(tmc2130_t *tmc);

#endif /* TMC2660_H_ */