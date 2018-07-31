/*
* atkrouter.c
*
* Created: 6/17/2018 2:48:08 PM
* Author : Jake
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "hardware.h"
#include "fastmath.h"

// stlclk PA0
// stlerr PA1

// seems like comm to the tmc is ok, but buggy. do new boards once new phy, for fab, etc, focus on 23 and bldc.

void clock_init(void){
	OSC.XOSCCTRL = OSC_XOSCSEL_XTAL_256CLK_gc | OSC_FRQRANGE_12TO16_gc; // select external source
	OSC.CTRL = OSC_XOSCEN_bm; // enable external source
	while(!(OSC.STATUS & OSC_XOSCRDY_bm)); // wait for external
	OSC.PLLCTRL = OSC_PLLSRC_XOSC_gc | OSC_PLLFAC0_bm | OSC_PLLFAC1_bm; // select external osc for pll, do pll = source * 3
	//OSC.PLLCTRL = OSC_PLLSRC_XOSC_gc | OSC_PLLFAC1_bm; // pll = source * 2 for 32MHz std clock
	OSC.CTRL |= OSC_PLLEN_bm; // enable PLL
	while (!(OSC.STATUS & OSC_PLLRDY_bm)); // wait for PLL to be ready
	CCP = CCP_IOREG_gc; // enable protected register change
	CLK.CTRL = CLK_SCLKSEL_PLL_gc; // switch to PLL for main clock
}

void uarts_init(void){
	// UP0
	rb_init(&up0rbrx);
	rb_init(&up0rbtx);
	pin_init(&up0rxled, &PORTE, PIN4_bm, 4, 1);
	pin_init(&up0txled, &PORTE, PIN5_bm, 5, 1);
	uart_init(&up0, &USARTE1, &PORTE, PIN6_bm, PIN7_bm, &up0rbrx, &up0rbtx, &up0rxled, &up0txled);
	//PORTE.PIN7CTRL = PORT_SRLEN_bm;
	uart_start(&up0, SYSTEM_BAUDA, SYSTEM_BAUDB);
	
	ups[0] = &up0;
}

void atkps_init(void){
	atkport_init(&atkp0, 0, &up0);
}

void stephardware_init(void){
	pin_init(&spicspin, &PORTC, PIN4_bm, 4, 1);
	spi_init(&spitmc, &USARTC1, &PORTC, PIN6_bm, PIN7_bm, PIN5_bm, &spicspin);
	spi_start(&spitmc, 1, 1);
	
	pin_init(&tmcen, &PORTA, PIN3_bm, 3, 1);
	pin_init(&tmcdiag1, &PORTA, PIN4_bm, 4, 0);
	pin_init(&tmcdiag0, &PORTA, PIN5_bm, 5, 0);
	pin_init(&step, &PORTC, PIN3_bm, 3, 1);
	pin_init(&dir, &PORTC, PIN2_bm, 2, 1);
	
	// a faux
	pin_init(&tmciref, &PORTC, PIN0_bm, 0, 1);
	pin_set(&tmciref);
	
	tmc2130_init(&tmc, &spitmc, &tmcen, &tmcdiag0, &tmcdiag1);
	
	stepper_init(&stepper, &step, &dir);
}

void tickers_init(void){
	// one 'ticker' to execute steps
	// turn on TCC0, do perclk / 8
	TCC0.CTRLA = TC_CLKSEL_DIV256_gc;
	
	// compare and capture at value
	uint16_t pera = 40; // ~ 200us timer to start... 
	// write low first, bc bussing / xmega 8-bit oddities cc datasheet @ 3.11
	uint8_t peral = (uint8_t) pera;
	uint8_t perah = (uint8_t) (pera >> 8);
	TCC0.PERBUFL = peral;
	TCC0.PERBUFH = perah;
	
	// set cca interrupt on
	TCC0.INTCTRLA = TC_OVFINTLVL_HI_gc;
		
	// another ticker to execute accel
	TCC1.CTRLA = TC_CLKSEL_DIV256_gc;
	TCC1.PERBUFL = peral;
	TCC1.PERBUFH = perah;
	
	TCC1.INTCTRLA = TC_OVFINTLVL_HI_gc;
}

int main(void)
{
	clock_init();
	uarts_init();
	atkps_init();
	
	// problem step?
	// err. on linking wait and non-wait moves 
	// do wait / no wait commands: same params, ship from same code
	
	// enable interrupts
	sei();
	PMIC.CTRL |= PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;

	pin_init(&stlclk, &PORTE, PIN2_bm, 2, 1);
	pin_init(&stlerr, &PORTE, PIN1_bm, 1, 1);
	pin_set(&stlerr);
	pin_set(&stlclk);

	// stepper business
	stephardware_init();
	
	tmc2130_start(&tmc);
	tmc2130_enable(&tmc);
	
	tickers_init();

	// runtime globals
	uint32_t tck = 0;

	while (1)
	{
		atkport_scan(&atkp0, 2);
		// just... as fast as we can 
 		tck++;
    	if(!(fastModulo(tck, 4096))){
     		pin_toggle(&stlclk);
     	}
	}
}

ISR(TCC0_OVF_vect){
	stepper_updatesteps(&stepper);
}

ISR(TCC1_OVF_vect){
	stepper_updateaccel(&stepper);
}

ISR(USARTE1_RXC_vect){
	uart_rxhandler(&up0);
}

ISR(USARTE1_DRE_vect){
	uart_txhandler(&up0);
}