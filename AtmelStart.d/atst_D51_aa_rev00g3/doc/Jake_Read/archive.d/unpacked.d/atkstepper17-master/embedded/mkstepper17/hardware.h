/*
 * hardware.h
 *
 * Created: 5/18/2018 11:01:12 AM
 *  Author: Jake
 */ 

#ifndef HARDWARE_H_
#define HARDWARE_H_

#include "pin.h"
#include "ringbuffer.h"
#include "uartport.h"
#include "apaport.h"
#include "apahandler.h"

#include "spiport.h"
#include "tmc2130.h"
#include "stepper.h"

// For if-case init

#define HARDWARE_ON_PERIPHERAL_A 0x0
#define HARDWARE_ON_PERIPHERAL_B 0x1
#define HARDWARE_ON_PERIPHERAL_C 0x2
#define HARDWARE_ON_PERIPHERAL_D 0x3

// use BAUD_SYSTEM 63019 for FTDI-Limited 115200 baudrate // ~ 4.2us bit period
// use BAUD_SYSTEM 60502 for 230400 
// use BAUD_SYSTEM 55469 for 460800
// use BAUD_SYSTEM 45403 for 921600

#define BAUD_SYSTEM 63019

/*
UP0RX		PA12	SER4-1
UP0TX		PA13	SER4-0
UP0STLB		PB14	(on receive)
UP0STLR		PB15	(on transmit)
*/

ringbuffer_t up0rbrx;
ringbuffer_t up0rbtx;

uartport_t up0;

apaport_t apap0;

pin_t up0stlb; // on receive
pin_t up0stlr; // on transmit

// pointers to uartports
#define NUM_UPS 1
uartport_t *ups[NUM_UPS];

/* STATUS LIGHTS

STLTICKER		PB13
STLERR			PB12
*/

pin_t stlTicker; // green!
pin_t stlErr; // blue!

/* TIMER

*/

uint32_t overflows;

#define STEPTICKER_SYNC (TC2->COUNT32.CTRLBSET.reg = TCC_CTRLBSET_CMD_READSYNC)
#define STEPTICKER (TC2->COUNT32.COUNT.reg)

#define TICKER_SYNC (TC0->COUNT32.CTRLBSET.reg = TCC_CTRLBSET_CMD_READSYNC)
#define TICKER (TC0->COUNT32.COUNT.reg)

/* STEPPER THINGS

TMC_SCK			PA05	SER0-1
TMC_CSN			PA06	SER0-2
TMC_MOSI		PA07	SER0-3
TMC_MISO		PA04	SER0-0

TMC_IREF_PWM	PA01	TC2-1

TMC_EN			PA02
TMC_STEP		PB08
TMC_DIR			PB09

TMC_DIAG0		PB04
TMC_DIAG1		PA03

*/

spiport_t spi_tmc2130;

pin_t step_pin;
pin_t dir_pin;
pin_t en_pin;
pin_t sg_pin;

// as configured, both are open drain (active low)

pin_t diag0; // overtemp prewarning
pin_t diag1; // stall

// and write new pwm_t for duty?

tmc2130_t tmc_driver;

stepper_t stepper;


#endif /* HARDWARE_H_ */