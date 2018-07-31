/*
 * stepper.c
 *
 * Created: 2/17/2018 5:39:34 PM
 *  Author: Jake
 */ 

#include "stepper.h"
#include "hardware.h"

stepper_t stepper_new(pin_t *step_pin, pin_t *dir_pin){
	stepper_t stepper;
	
	stepper.step_pin = step_pin;
	stepper.dir_pin = dir_pin;
		
	stepper.speed_period = 0;
	stepper.position_ticks = 0;
	
	stepper.position_ticks_target = 0;
	
	return stepper;
}

void stepper_reset(stepper_t *stepper){
	stepper->speed_period = 0;
	stepper->position_ticks = 0;
	stepper->position_ticks_target = 0;
	stepper->dir_steps = 1;
}

void stepper_steps(stepper_t *stepper, int32_t position){
	stepper->position_ticks_target = abs(position);
	stepper->position_ticks = 0;
	if(position < 0){
		stepper->dir_steps = 0;
		pin_clear(stepper->dir_pin);
	} else {
		stepper->dir_steps = 1;
		pin_set(stepper->dir_pin);
	}
}

void stepper_speed(stepper_t *stepper, uint32_t speed){
	// minimum of 1 step / s
	// if speed is 0, we leave speed at minimum and set state to not step?
	(speed < 1) ? speed = 1 : (0);
	stepper->speed = speed;
	// timing not accurate to real-world yet
	// timer is 48MHz clock on a DIV16, so we have 3 000 000 ticks / s (333ns resolution, wow)
	stepper->speed_period = 3000000 / speed;
}

void stepper_segment(stepper_t *stepper, uint32_t startspeed, int32_t accel, int32_t steps){
	if(accel < 0){
		stepper->dir_accel = 0;
	} else {
		stepper->dir_accel = 1;
	}
	stepper->accel_period = 3000000 / abs(accel); // should check for / by 0
	stepper_speed(stepper, startspeed);
	stepper_steps(stepper, steps);
	TICKER_SYNC;
	uint32_t now = TICKER;
	stepper->last_accel = now;
	stepper->last_step = now;
}

void stepper_update(stepper_t *stepper){	
	if(stepper->position_ticks_target > stepper->position_ticks){ // still have somewhere to go
		TICKER_SYNC;
		uint32_t now = TICKER;
		// check if it's time to accel
		if(now - stepper->last_accel > stepper->accel_period){
			if(stepper->dir_accel){
				uint32_t newspeed = stepper->speed + 1;
				stepper_speed(stepper, newspeed);
			} else {
				uint32_t newspeed = stepper->speed - 1;
				stepper_speed(stepper, newspeed);
			}
			stepper->last_accel = now;
		}
		// check if it's time to step
		if(now - stepper->last_step > stepper->speed_period){
			pin_toggle(stepper->step_pin);
			stepper->position_ticks ++;
			stepper->last_step = now;
		} // end step cycle
		
	}// end step if not there cycle
}