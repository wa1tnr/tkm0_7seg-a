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

typedef struct {
	pin_t *step_pin;
	pin_t *dir_pin;
	
	// currently.. why do we need so much state?
	uint32_t speed_period;
	uint32_t speed;
	uint32_t accel_period;
	uint32_t position_ticks;
	uint32_t dir_steps; // 0 or 1
	uint32_t dir_accel;
	
	// tracking time for updates
	uint32_t last_step;
	uint32_t last_accel;
	
	// targets
	uint32_t position_ticks_target;
}stepper_t;

stepper_t stepper_new(pin_t *step_pin, pin_t *dir_pin);

void stepper_reset(stepper_t *stepper);

void stepper_steps(stepper_t *stepper, int32_t steps);
void stepper_speed(stepper_t *stepper, uint32_t speed);

void stepper_segment(stepper_t *stepper, uint32_t startspeed, int32_t accel, int32_t steps);

void stepper_update(stepper_t *stepper);

void stepper_current_position(stepper_t *steppper, float *position);
void stepper_current_speed(stepper_t *stepper, float *speed);
void stepper_currently(stepper_t *stepper, float *position, float *speed);

/*
step to-do
 - everything is broken and untested
 - refactor all for:
  - dir-by-flag, assume no dir change per segment
  - speed & periods unfuck
  - update checks accel rate as well
*/

#endif /* STEPPER_H_ */