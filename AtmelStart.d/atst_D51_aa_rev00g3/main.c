// Tue Jul 31 13:45:08 UTC 2018

// main.c

#include "atmel_start.h"
#include "gpio_local.h"

/* Many changes: by wa1tnr, July 2018 */

void uSec(void) {
    for (volatile int i = 1; i < 2; i++) { // needs calibration
        // nothing
    }
}

void short_timer(void) { // human blinkie timescale
    uint32_t on_time  = 2140111222; // it's 2147 something ;)
    for(on_time =       21401122; on_time > 0; on_time--) { // 21.4 million
        uSec();
    }
}

void raise_LED_pins(void) { // multiple target boards
    // raise_D13(); raise_D12();
    void raise_D13-metro();   // Metro M4 Express
    void raise_D13-feather(); // Feather M4 Express
}

void lower_LED_pins(void) {
    // lower_D13(); lower_D12();
    void lower_D13-metro();
    void lower_D13-feather();
}

void blink_LED(void) {
    // raise_D13(); raise_D12();
    raise_LED_pins(); // Feather M4 or Metro M4
    short_timer(); // should be first instance of use of this timer
    lower_D13();
    lower_D12();
    short_timer();
}

void flicker_LED(void) {
    raise_D13();
    raise_D12();
    short_timer();
    lower_D13();
    lower_D12();
    short_timer();
}

void blinkLEDfast(void) {
    for(int i = 15; i > 0; i--) {
        blink_LED();
    }
}

void activity_LED_demo(void) {
    // init_act_LED();
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
        init_act_LED();
        raise_D13(); // LED ON - why not?
        raise_D12(); // LED ON - why not?
        short_timer();
        delay_ms(3500); // 3.5 seconds
        NVIC_SystemReset(); // warm boot - lights may modulate as with pushbutton reset?
        lower_D13();
        lower_D12();

        long_long_timer();

        activity_LED_demo();

        lower_D13();
        lower_D12();

        short_timer();

	while (1) {
            flicker_LED();
            short_timer();
            // short_timer();
	}
}
