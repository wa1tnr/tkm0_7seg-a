// Sun Jul 29 01:36:04 UTC 2018
// main.c

// atmel start program - dotstar demo
// author: wa1tnr

// LICENSE is LGPL v2.1

#include <atmel_start.h>
// #include "thisproject.h" // local include for main.c
#include "gpio_local.h"
// #include "dotstar.h"

/* Many changes: by wa1tnr, July 2018 */

void uSec(void) {
    for (volatile int i = 1; i < 2; i++) { // needs calibration
        // nothing
    }
}

void short_timer(void) { // human blinkie timescale
    uint32_t on_time  = 2140111222; // it's 2147 something ;)
    // for(on_time = 21401; on_time > 0; on_time--) {
    // for(on_time = 214011; on_time > 0; on_time--) {
    for(on_time = 2142011; on_time > 0; on_time--) {
        uSec();
    }
}

void vy_short_timer(void) { // human blinkie timescale
    uint32_t on_time  = 2140111222; // it's 2147 something ;)
    for(on_time = 344; on_time > 0; on_time--) {
        uSec();
    }
}

// this is what you lengthen to get a visibly slowed animation:

void uber_short_timer(void) { // digital clock/data timescale
    uint32_t on_time  = 2140111222; // it's 2147 something ;)
    // for(on_time = 23; on_time > 0; on_time--) { // worked well, but slowly
    for(on_time = 3; on_time > 0; on_time--) {
        uSec();
    }
}

void long_timer(void) {
    for(int i =  9; i > 0; i--) {
        short_timer();
    }
}

void blink_LED(void) {
    raise_D13();
    short_timer();
    lower_D13();
    short_timer();
}

void flicker_LED(void) {
    raise_D13();
    vy_short_timer();
    lower_D13();
    vy_short_timer();
}

void blinkLEDfast(void) {
    // for(int i = 15; i > 0; i--) {
    for(int i = 152000; i > 0; i--) {
        blink_LED();
    }
}

void activity_LED_demo(void) {
    init_act_LED();
    blinkLEDfast();
}

void loop_vy_short_timer(void) {
    for(int i = 12; i > 0; i--) {
        vy_short_timer();
    }
}

void medium_timer(void) {
    for(int i = (7); i > 0; i--) {
        short_timer();
    }
}

void med_short_timer(void) {
    for(int i = (3); i > 0; i--) {
        short_timer();
    }
}

void fleck_timer(void) {
    for(int i = (2); i > 0; i--) {
        loop_vy_short_timer();
    }
}

void done_eight_times(void) {
    for(int i = (8); i > 0; i--) {
        blink_LED();
        short_timer(); short_timer();
    }
}

void init_act_LED(void);

int main(void) {
    /* Initializes MCU, drivers and middleware */
    atmel_start_init();
    init_act_LED();
    // activity_LED_demo();
    raise_D13();    // red LED D13 - OFF
    while(1);
    // init_act_LED();

    /* Pimoroni Blinkt demo */
    // DS_sends_demo();

    // USB_0_PORT_init(); // redundant?

    // testing of D7 logic using physical D13 (the LED)
    // usual use of D13 commented out during those tests.
    // activity_LED_demo();

    // D7 logic good - using D13
    // demo_D7_clock();
    // demo_D6_data();

    raise_D13();    // red LED D13 - OFF
    // medium_timer(); // wait a bit
    // raise_D13();    // red LED D13 - OFF
    // lower_D13();    // red LED D13 - OFF
    // medium_timer(); // wait a bit

    // sanity check on loop counts:
    // done_eight_times();
    while (1) {
         raise_D13();    // red LED D13 - OFF
        // flicker_LED(); // very short duration red LED pulse
        // long_timer();  // lots of time between pulses
    }
}
