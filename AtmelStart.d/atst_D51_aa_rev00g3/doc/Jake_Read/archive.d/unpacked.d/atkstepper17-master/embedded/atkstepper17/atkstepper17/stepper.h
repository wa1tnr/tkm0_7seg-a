/*
 * stepper.h
 *
 * Created: 2/17/2018 5:39:45 PM
 *  Author: Jake
 */ 


#ifndef STEPPER_H_
#define STEPPER_H_

#include "pin.h"

#define BLOCKS_QUEUE_SIZE 16
#define STEPTICKER_ONE_SECOND 187500 // one tick of timer is 5.3-- us
#define STEP_ACCELSTATE_CRUISE 0
#define STEP_ACCELSTATE_ACCEL 1
#define STEP_ACCELSTATE_DECEL 2

// one movement
typedef struct {
	// from whence you came
	uint8_t packet[256]; // C quesion: how to do this properly with malloc() ? malloc() on embedded sys?
	
	// tracking
	uint8_t is_new;
	uint8_t is_wait;
	
	// for what you do
	uint8_t dir; // 0 or 1
	uint32_t position_end; // in steps
	uint32_t entry_speed;
	uint32_t accel_period;
	uint32_t position_accel_to;
	uint32_t position_deccel_from;
}block_t;

// the stepper
typedef struct {
	pin_t *step_pin;
	pin_t *dir_pin;
	
	// block ringbuffer
	block_t block[BLOCKS_QUEUE_SIZE];
	uint8_t blockhead;
	uint8_t blocktail;
	uint8_t blocksize;
		
	// tracking time periods
	uint16_t speed_period; // meaning we have a min. step speed of STEPTICKER_ONE_SECOND / 2^16 ( ~ 2.86 s/s)
	uint16_t accel_period;
	
	// tracking time for updates
	uint8_t accelstate;
	
	// have to track speed to update accel
	uint32_t speed;
	
	// targets
	uint32_t position_ticks;
	uint32_t position_accel_to;
	uint32_t position_deccel_from;
}stepper_t;

void stepper_init(stepper_t *stepper, pin_t *step_pin, pin_t *dir_pin);

void stepper_reset(stepper_t *stepper);

// steps discrete, mm/min
void stepper_steps(stepper_t *stepper, int32_t steps, uint32_t speed);

// steps discrete, mm/min, mm/min/s (mm/s better but we want more discrete resolution)
void stepper_new_block(uint8_t *packet, stepper_t *stepper, uint8_t is_wait, int32_t steps, uint32_t entryspeed, uint32_t accel, uint32_t accellength, uint32_t deccellength);

void stepper_updatesteps(stepper_t *stepper);

void stepper_updateaccel(stepper_t *stepper);

void stepticker_newperiod(uint16_t per);

void stepticker_restart(void);

void accelticker_newperiod(uint16_t per);

void accelticker_restart(void);

/*
step to-do
// block ringbuffer, pull and reply blocks - architecture for network functions, generally?
// doc this https://embeddedgurus.com/stack-overflow/2011/02/efficient-c-tip-13-use-the-modulus-operator-with-caution/ 
*/

#endif /* STEPPER_H_ */