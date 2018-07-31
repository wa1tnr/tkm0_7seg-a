/*
* stepper.c
*
* Created: 2/17/2018 5:39:34 PM
*  Author: Jake
*/

#include "stepper.h"
#include "hardware.h"
#include "atkport.h"
#include "atkhandler.h"
#include "fastmath.h"

void stepper_init(stepper_t *stepper, pin_t *step_pin, pin_t *dir_pin){
	stepper->step_pin = step_pin;
	stepper->dir_pin = dir_pin;
	
	stepper_reset(stepper);
}

void stepper_reset(stepper_t *stepper){
	stepper->blockhead = 0;
	stepper->blocktail = 0;
	stepper->blocksize = BLOCKS_QUEUE_SIZE;

	stepper->speed_period = 0;
	stepper->accel_period = 0;
	
	stepper->speed = 0;
	
	stepper->position_ticks = 0;
	stepper->position_accel_to = 0;
	stepper->position_deccel_from = 0;
}

void stepper_steps(stepper_t *stepper, int32_t steps, uint32_t speed){
	// not implemented atm
}

void stepper_new_block(uint8_t *packet, stepper_t *stepper, uint8_t is_wait, int32_t steps, uint32_t entryspeed, uint32_t accel, uint32_t accellength, uint32_t deccellength){
	// does assignments and adds to queue
	// track this address so that we can ack to it when the move is complete
	// a sloppy copy, I'm sure
	for(int i = 0; i < packet[0]; i ++){
		stepper->block[stepper->blockhead].packet[i] = packet[i];
	}
	
	// a real move, or a wait placeholder?
	stepper->block[stepper->blockhead].is_wait = is_wait;
	
	// TODO: should block the execution of this block while we do this, so that we
	// have an interrupt safe ringbuffer
	
	// enforce no div/0
	(entryspeed < 3) ? entryspeed = 3 : (0); //(0) is NOP: 3 is min steps/s due to timer resolution
	(entryspeed > 187500) ? entryspeed = 187500 : (0); // no faster than this pls, else below timer resolution
	// going to have to catch blocks which cause deceleration to 0 during deceleration phases !
	stepper->block[stepper->blockhead].entry_speed = entryspeed;
	
	// do starting speed period
	(accel < 3) ? accel = 3 : (0);
	(accel > 187500) ? accel = 187500 : (0);
	stepper->block[stepper->blockhead].accel_period = STEPTICKER_ONE_SECOND / accel;
	
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
	
	// if there are currently no steps to make, we're not sure about the current step frequency, we'll set the period
	// otherwise we're taking for granted that we've set this properly following the last move
	if(stepper->blockhead == stepper->blocktail){
		uint16_t newper = STEPTICKER_ONE_SECOND / entryspeed;
		stepticker_restart();
		stepticker_newperiod(newper);
		uint16_t accper = STEPTICKER_ONE_SECOND / accel;
		accelticker_restart();
		accelticker_newperiod(accper);
		pin_clear(&stlerr);
	}
	// increment block head ptr: should catch full queue HERE but not bothering
	stepper->blockhead = fastModulo((stepper->blockhead + 1), BLOCKS_QUEUE_SIZE);
}

void stepper_updatesteps(stepper_t *stepper){
	if(stepper->blockhead == stepper->blocktail){
		// no steps to make, ringbuffer is empty
		pin_set(&stlerr);
		} else if(stepper->block[stepper->blocktail].position_end > stepper->position_ticks){
		// we have somewhere to go
		if(stepper->block[stepper->blocktail].is_new){
			// if we're just starting this block, set the speed
			stepper->speed = stepper->block[stepper->blocktail].entry_speed;
			stepper->speed_period = STEPTICKER_ONE_SECOND / stepper->speed;
			stepticker_restart(); // restart sets cc register to zero, so we do this first
			stepticker_newperiod(stepper->speed_period);
			
			// and set the accel ticker
			accelticker_restart(); 
			accelticker_newperiod(stepper->block[stepper->blocktail].accel_period);
			
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
		
		// if there's steps to make, and this timer is firing, it's time to step!
		if(!stepper->block[stepper->blocktail].is_wait){
			pin_toggle(stepper->step_pin);
		}
		
		stepper->position_ticks ++;
		
		if(stepper->position_ticks < stepper->block[stepper->blocktail].position_accel_to){
			stepper->accelstate = STEP_ACCELSTATE_ACCEL;
			} else if (stepper->position_ticks > stepper->block[stepper->blocktail].position_deccel_from){
			stepper->accelstate = STEP_ACCELSTATE_DECEL;
			} else {
			stepper->accelstate = STEP_ACCELSTATE_CRUISE;
		}
		
		} else {
		// looks a lot like we're done here
		// send a reply for windowed transmission
		// this is dirty because we're passing the packet (referenced here in the ringbuffer) by reference,
		// properly return_packet should take a copy of the packet so that we can be done with it now, but *it* goes faster than *this* (almost for sure...)
		// so, we shrugman for now
		// apa_return_packet(stepper->block[stepper->blocktail].packet, 25);
		uint8_t reply[12];
		int8_t replyLength;
		if(stepper->block[stepper->blocktail].is_wait){
			reply[0] = DELIM_KEY_WAIT;
			reply[1] = 12;
			replyLength = 2;
		} else {
			reply[0] = DELIM_KEY_TRAPEZOID;
			int32_t stepsTaken = stepper->block[stepper->blocktail].position_end;
			if(!stepper->block[stepper->blocktail].dir){
				stepsTaken = -1 * stepsTaken;
			}
			reply[1] = stepsTaken >> 24;
			reply[2] = stepsTaken >> 16;
			reply[3] = stepsTaken >> 8;
			reply[4] = stepsTaken;
			replyLength = 5;
		}
		atk_reply_packet(stepper->block[stepper->blocktail].packet, reply, replyLength);
		// increment ringbuffer along
		stepper->blocktail = fastModulo((stepper->blocktail + 1), BLOCKS_QUEUE_SIZE);
		stepper->position_ticks = 0; // clear so that we evaluate new block as having steps to make
	}
}

void stepper_updateaccel(stepper_t *stepper){
	// check for acceleration or deceleration
	switch (stepper->accelstate){
		case STEP_ACCELSTATE_ACCEL:
			stepper->speed ++;
			(stepper->speed > 187500) ? stepper->speed = 187500 : (0); // max speed due to timer res
			stepper->speed_period = STEPTICKER_ONE_SECOND / stepper->speed;
			stepticker_newperiod(stepper->speed_period);
			break;
		case STEP_ACCELSTATE_DECEL:
			stepper->speed --;
			(stepper->speed < 3) ? stepper->speed = 3 : (0); // min speed due to timer res
			stepper->speed_period = STEPTICKER_ONE_SECOND / stepper->speed;
			stepticker_newperiod(stepper->speed_period);
			break;
		case STEP_ACCELSTATE_CRUISE:
			(0);
			break;
		default:
			(0);
			break;
	}
}

void stepticker_newperiod(uint16_t per){
	TCC0.PERBUFL = (uint8_t) per;
	TCC0.PERBUFH = (uint8_t) (per >> 8);
}

void stepticker_restart(void){
	TCC0.CTRLFSET = TC_CMD_RESTART_gc;
}

void accelticker_newperiod(uint16_t per){
	TCC1.PERBUFL = (uint8_t) per;
	TCC1.PERBUFH = (uint8_t) (per >> 8);
}

void accelticker_restart(void){
	TCC1.CTRLFSET = TC_CMD_RESTART_gc;
}