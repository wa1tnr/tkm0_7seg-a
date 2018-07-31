/*
 * tmc2660.h
 *
 * Created: 2/7/2018 10:17:49 PM
 *  Author: Jake
 */ 


#ifndef TMC2660_H_
#define TMC2660_H_

#include "sam.h"
#include "spiport.h"
#include "pin.h"

// TODO: adding updates (microstep, current)
// in that, calculating current
// TODO: reading stallguard, understanding if is already doing closed loop?

typedef struct{
	spiport_t *spi;
	
	pin_t *en_pin;
	pin_t *sg_pin;
}tmc26_t;

tmc26_t tmc26_new(spiport_t *spi, pin_t *en, pin_t *sg);

void tmc26_write(tmc26_t *tmc, uint32_t word);

void tmc26_init(tmc26_t *tmc);

void tmc26_update(tmc26_t *tmc);

void tmc26_enable(tmc26_t *tmc);

void tmc26_disable(tmc26_t *tmc);

#endif /* TMC2660_H_ */