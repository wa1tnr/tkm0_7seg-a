/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_init.h"
#include <peripheral_clk_config.h>
#include <utils.h>
#include <hal_init.h>

void system_init(void)
{
	init_mcu();

	// GPIO on PA23

	// Set pin direction to output
	gpio_set_pin_direction(D13, GPIO_DIRECTION_OUT);

	gpio_set_pin_level(D13,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   true); // please

	// gpio_set_pin_function(D13, GPIO_PIN_FUNCTION_OFF); // would turn that pin off
}
