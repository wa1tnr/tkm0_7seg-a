#include <atmel_start.h>
#include "driver_examples.h"
#include "driver_init.h"
#include "utils.h"
#include "gpio_local.h"


/* Many changes: by wa1tnr, July 2018 */

void uSec(void) {
    for (volatile int i = 1; i < 2; i++) { // needs calibration
        // nothing
    }
}

void short_timer(void) { // human blinkie timescale
    uint32_t on_time  = 2140111222; // it's 2147 something ;)
    for(on_time = 21401122; on_time > 0; on_time--) {
        uSec();
    }
}

void blink_LED(void) {
    raise_D13();
    raise_D12();
    short_timer(); // should be first instance of use of this timer
    lower_D13();
    // lower_D12();
    short_timer();
}

void flicker_LED(void) {
    raise_D13();
    raise_D12();
    short_timer();
    lower_D13();
    short_timer();
}

void blinkLEDfast(void) {
    for(int i = 15; i > 0; i--) {
        blink_LED();
    }
}

void activity_LED_demo(void) {
    init_act_LED();
    blinkLEDfast();
}

void long_long_timer(void) {
    for (int j=201444; j>1; j--){
        for (int i=201444; i>1; i--){
            short_timer();
        } // fake delay
    } // faker delay haha
}

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
        raise_D13(); // LED ON - why not?
        raise_D12(); // metro m4 kludged

        long_long_timer();

        // activity_LED_demo();

        // lower_D13();

        // short_timer();

	/* Replace with your application code */
	while (1) {
            // flicker_LED();
            // short_timer();
	}
}
