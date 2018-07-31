// wishful thinking to include any of these:
#include <atmel_start.h>
#include "driver_examples.h"
#include "driver_init.h" // gives atmel_start_pins.h for D13 and such
#include <peripheral_clk_config.h>
#include "utils.h"
#include <hal_init.h>
#include "gpio_local.h"

void init_act_LED(void) {
    gpio_set_pin_direction(D13, GPIO_DIRECTION_OUT); // feather m4 - canonical
    gpio_set_pin_direction(D12, GPIO_DIRECTION_OUT); // metro m4 kludge
}

void raise_D12(void) {
	// GPIO on PA16 // metro m4 express only

	// Set pin direction to output
	// gpio_set_pin_direction(D12, GPIO_DIRECTION_OUT);

	gpio_set_pin_level(D12,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   true);

	// gpio_set_pin_function(D12, GPIO_PIN_FUNCTION_OFF);
}

void raise_D13(void) {
	// GPIO on PA23

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
	// GPIO on PA23

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
