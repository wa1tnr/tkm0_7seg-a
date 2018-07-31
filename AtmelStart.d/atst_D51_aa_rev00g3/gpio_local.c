// wishful thinking to include any of these:
// #include <atmel_start.h>
// #include "driver_examples.h"
#include "driver_init.h" // gives atmel_start_pins.h for D13 and such
// #include <peripheral_clk_config.h>
// #include "utils.h"
// #include <hal_init.h>
// #include "gpio_local.h"

void init_act_LED(void) {
    gpio_set_pin_direction(PA23, GPIO_DIRECTION_OUT);
    gpio_set_pin_direction(PA16, GPIO_DIRECTION_OUT);
}

void raise_D13_metro(void) { // Metro M4 Express
    gpio_set_pin_level(PA16, true);
}

void raise_D13_feather(void) { // Feather M4 Express
    gpio_set_pin_level(PA23, true);
}

void lower_D13_metro(void) {
    gpio_set_pin_level(PA16, false);
}

void lower_D13_feather(void) {
    gpio_set_pin_level(PA23, false);
}
