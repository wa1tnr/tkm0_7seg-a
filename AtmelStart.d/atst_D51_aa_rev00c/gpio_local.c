#include "driver_init.h" // gives atmel_start_pins.h for D13 and such
// #include <peripheral_clk_config.h> // modified  -  wa1tnr 27 JUL 2018
// #include <utils.h>
// #include <hal_init.h>
// #include <hpl_gclk_base.h>
// #include <hpl_pm_base.h>

void init_act_LED(void) {
    gpio_set_pin_direction(D13, GPIO_DIRECTION_OUT);
}

/*
 26 void init_act_LED(void) { // PIN_ACTIVITY_LED
 27 //    gpio_init(PORTA, PIN_ACTIVITY_LED, 1); // PA17
 28 }
*/

void raise_D13(void) {
	// GPIO on PA17

	// Set pin direction to output
	// gpio_set_pin_direction(D13, GPIO_DIRECTION_OUT);

	gpio_set_pin_level(D13,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   true);

	// gpio_set_pin_function(D13, GPIO_PIN_FUNCTION_OFF);
}

void lower_D13(void) {
	// GPIO on PA17

	// Set pin direction to output
	// gpio_set_pin_direction(D13, GPIO_DIRECTION_OUT);

	gpio_set_pin_level(D13,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// gpio_set_pin_function(D13, GPIO_PIN_FUNCTION_OFF);
}
