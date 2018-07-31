/*
 * stepper.c
 *
 * Created: 2/17/2018 5:39:34 PM
 *  Author: Jake
 */ 

#include "stepper.h"
#include "hardware.h"
#include "apaport.h"
#include "apahandler.h"

stepper_t stepper_new(pin_t *step_pin, pin_t *dir_pin){
	stepper_t stepper;
	
	stepper.step_pin = step_pin;
	stepper.dir_pin = dir_pin;
	
	stepper_reset(&stepper);
	
	return stepper;
}

void stepper_reset(stepper_t *stepper){
	stepper->one_minute = 160980000; // calib'd # for timer ticks per minute (speeds, accels all in steps/min or steps/min/s)
	stepper->speed_period = 0;
	stepper->accel_period = 0;
	
	stepper->blockhead = 0;
	stepper->blocktail = 0;
	stepper->blocksize = BLOCKS_QUEUE_SIZE;
	
	stepper->speed = 0;
	
	stepper->last_step = 0;
	stepper->last_accel = 0;
	
	stepper->position_ticks = 0;
	stepper->position_accel_to = 0;
	stepper->position_deccel_from = 0;
	stepper->position_ticks_end = 0;
}

void stepper_steps(stepper_t *stepper, int32_t steps, uint32_t speed){
	// TODO: update to ship a block
	// set speed period
	(speed < 1) ? speed = 1 : (0); // avoid 0 division, 1 step / s seems like reasonable lower bound step rate
	stepper->speed = speed;
	stepper->speed_period = stepper->one_minute / stepper->speed;
	
	// set direction
	if(steps < 0){
		pin_clear(stepper->dir_pin);
	} else {
		pin_set(stepper->dir_pin);
	}
		
	// set new position information
	stepper->position_ticks_end = abs(steps);
	stepper->position_accel_to = 0;
	stepper->position_deccel_from = stepper->position_ticks_end; // defines flat line block
	stepper->position_ticks = 0;
	
	// reset times
	TICKER_SYNC;
	unsigned long now = TICKER;
	stepper->last_accel = now;
	stepper->last_step = now;
}

void stepper_new_block(uint8_t *packet, stepper_t *stepper, int32_t steps, uint32_t entryspeed, uint32_t accel, uint32_t accellength, uint32_t deccellength){
	// does assignments and adds to queue
	
	// track this address so that we can ack to it when the move is complete
	// a sloppy copy, I'm sure
	for(int i = 0; i < packet[1]; i ++){
		stepper->block[stepper->blockhead].packet[i] = packet[i];
	}
		
	// TODO: should block the execution of this block while we do this, so that we 
	// have an interrupt safe ringbuffer
	
	// enforce no div/0
	(entryspeed < 1) ? entryspeed = 1 : (0);
	// going to have to catch blocks which cause deceleration to 0 during deceleration phases !
	stepper->block[stepper->blockhead].entry_speed = entryspeed;

	// do starting speed period
	stepper->block[stepper->blockhead].accel_period = stepper->one_minute / accel;
	
	// set dir
	if(steps < 0){
		stepper->block[stepper->blockhead].dir = 0;
	} else {
		stepper->block[stepper->blockhead].dir = 1;
	}
	
	// do lengths
	stepper->block[stepper->blockhead].position_end = abs(steps);
	stepper->block[stepper->blockhead].position_accel_to = accellength;
	stepper->block[stepper->blockhead].position_deccel_from = deccellength;
	
	// ready set
	stepper->block[stepper->blockhead].is_new = 1;
	
	// increment block head ptr: should catch full queue HERE but not bothering
	stepper->blockhead = (stepper->blockhead + 1) % stepper->blocksize;
}

void stepper_update(stepper_t *stepper){
	if(stepper->blockhead == stepper->blocktail){
		// pin_clear(&stlb);
		// bail, no steps to make, ringbuffer is empty
	} else if(stepper->block[stepper->blocktail].position_end > stepper->position_ticks){ 
		// we have somewhere to go
		TICKER_SYNC;
		unsigned long now = TICKER;
		
		if(stepper->block[stepper->blocktail].is_new){
			// if we're just starting this block, set the speed
			stepper->speed = stepper->block[stepper->blocktail].entry_speed;
			stepper->speed_period = stepper->one_minute / stepper->speed;
			
			// and set the dir
			if(stepper->block[stepper->blocktail].dir > 0){
				pin_set(stepper->dir_pin);
			} else {
				pin_clear(stepper->dir_pin);
			}
			
			// and distance was 0'd after last move
			
			// and then clear this flag
			stepper->block[stepper->blocktail].is_new = 0; 
		}
		
		if(stepper->position_ticks < stepper->block[stepper->blocktail].position_accel_to){
			// we're accelerating!
			if(now - stepper->last_accel > stepper->block[stepper->blocktail].accel_period){
				stepper->speed += 1;
				stepper->speed_period = stepper->one_minute / stepper->speed;
				stepper->last_accel = now;
			}
		} else if(stepper->position_ticks > stepper->block[stepper->blocktail].position_deccel_from){
			if(now - stepper->last_accel > stepper->block[stepper->blocktail].accel_period){
				stepper->speed -= 1;
				(stepper->speed < 1) ? stepper->speed = 1 : (0); // assert no 0's
				stepper->speed_period = stepper->one_minute / stepper->speed;
				stepper->last_accel = now;
			}
		} else {
			// no accel or deccel
		}
		
		// check if it's time to step
		if(now - stepper->last_step >= stepper->speed_period){
			pin_toggle(stepper->step_pin);
			stepper->position_ticks ++;
			stepper->last_step = now; 
			//stepper->last_step + stepper->speed_period; // last speed_period if accelerating
			// not sure why that wasn't working: for now, take for granted that over the course of many steps,
			// we tend do equal amounts undershooting speed on all motors
		} // end step cycle
		
	} else {
		// looks a lot like we're done here
		
		// send a reply for windowed transmission
		// this is dirty because we're passing the packet (referenced here in the ringbuffer) by reference,
		// properly return_packet should take a copy of the packet so that we can be done with it now, but *it* goes faster than *this* (almost for sure...)
		// so, we shrugman for now
		// apa_return_packet(stepper->block[stepper->blocktail].packet, 25);
		uint8_t reply[2] = {12,24};
		apa_reply_packet(stepper->block[stepper->blocktail].packet, 25, reply, 2);
		
		// increment ringbuffer along
		stepper->blocktail = (stepper->blocktail + 1) % stepper->blocksize;
		stepper->position_ticks = 0; // clear so that we evaluate new block as having steps to make
	}
}