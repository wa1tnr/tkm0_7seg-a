/*
 * stepper.h
 *
 * Created: 2/17/2018 5:39:45 PM
 *  Author: Jake
 */ 


#ifndef STEPPER_H_
#define STEPPER_H_

#include "sam.h"
#include "pin.h"

#define BLOCKS_QUEUE_SIZE 16

// one movement
typedef struct {
	// from whence you came
	uint8_t packet[8]; // C quesion: how to do this properly with malloc() ? malloc() on embedded sys?
	uint8_t packet_length;
	
	// tracking
	uint8_t is_new;
	
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
	unsigned long one_minute;
	unsigned long speed_period;
	unsigned long accel_period;
	
	// tracking time for updates
	unsigned long last_step;
	unsigned long last_accel;
	
	// have to track speed to update accel
	uint32_t speed;
	
	// targets
	uint32_t position_ticks;
	uint32_t position_accel_to;
	uint32_t position_deccel_from;
	uint32_t position_ticks_end;
}stepper_t;

stepper_t stepper_new(pin_t *step_pin, pin_t *dir_pin);

void stepper_reset(stepper_t *stepper);

// steps discrete, mm/min
void stepper_steps(stepper_t *stepper, int32_t steps, uint32_t speed);

// steps discrete, mm/min, mm/min/s (mm/s better but we want more discrete resolution)
void stepper_new_block(uint8_t *packet, uint8_t packet_length, stepper_t *stepper, int32_t steps, uint32_t entryspeed, uint32_t accel, uint32_t accellength, uint32_t deccellength);

void stepper_update(stepper_t *stepper);

/*
step to-do
// block ringbuffer, pull and reply blocks - architecture for network functions, generally?
*/

#endif /* STEPPER_H_ */