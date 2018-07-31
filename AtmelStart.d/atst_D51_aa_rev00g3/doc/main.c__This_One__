/*
 * mkstepper-v011.c
 *
 * Created: 2/5/2018 12:00:35 PM
 * Author : Jake
 */ 


#include "sam.h"
#include "pin.h"
#include "uartport.h"
#include "spiport.h"
#include "apaport.h"

#include "hardware.h"

/* PINS 

STLR		PB13
STLB		PB14

NP1RX		PA12 / SER4-1
NP1TX		PA13 / SER4-0

NP2RX		PB03 / SER5-1
NP2TX		PB02 / SER5-0

TMC_SG		PB08
TMC_EN		PB07
STEP		PB08
DIR			PB09

TMC_MOSI	PA07 / SER0-3
TMC_MISO	PA04 / SER0-0
TMC_SCK		PA05 / SER0-1
TMC_CSN		PA06 / SER0-2

AS_MOSI		PA11 / SER2-3
AS_MISO		PA08 / SER2-1
AS_SCK		PA09 / SER2-0
AS_CSN		PA10 / SER2-2

*/

void clock_init(void){
	
	// on Reset, the DFLL48< source clock is on and running at 48MHz
	// GCLK0 uses DFLL48M as a source and generates GCLK_MAIN
	// we want to use OSCCTRL to (1) set the DFLL48M to run on a reference clock, in closed-loop mode
	// (20 then to prescale the DFLL48M such that it runs at 120MHz
	
	// generic clock channel 0 is the reference for the DFLL - we'll try to set that up first
	
	// for 120mhz do https://github.com/adafruit/ArduinoCore-samd/blob/samd51/cores/arduino/startup.c
	
	// something?
	NVMCTRL->CTRLA.reg |= NVMCTRL_CTRLA_RWS(0);
	
	// reset
	GCLK->CTRLA.bit.SWRST = 1;
	while(GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_SWRST);
	
	// Setup internal reference to gclk gen 3
	GCLK->GENCTRL[3].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_OSCULP32K) | GCLK_GENCTRL_GENEN;
	while(GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL3);
	GCLK->GENCTRL[0].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_OSCULP32K) | GCLK_GENCTRL_GENEN;
	while(GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL0);
	
	// enable DFLL48M clock
	OSCCTRL->DFLLCTRLA.reg = 0;
	OSCCTRL->DFLLMUL.reg = OSCCTRL_DFLLMUL_CSTEP(0x1) | OSCCTRL_DFLLMUL_FSTEP(0x1) | OSCCTRL_DFLLMUL_MUL(0);
	while(OSCCTRL->DFLLSYNC.reg & OSCCTRL_DFLLSYNC_DFLLMUL);
	OSCCTRL->DFLLCTRLB.reg = 0;
	while(OSCCTRL->DFLLSYNC.reg & OSCCTRL_DFLLSYNC_DFLLCTRLB);
	OSCCTRL->DFLLCTRLA.reg |= OSCCTRL_DFLLCTRLA_ENABLE;
	while(OSCCTRL->DFLLSYNC.reg & OSCCTRL_DFLLSYNC_ENABLE);
	OSCCTRL->DFLLCTRLB.reg = OSCCTRL_DFLLCTRLB_WAITLOCK | OSCCTRL_DFLLCTRLB_CCDIS | OSCCTRL_DFLLCTRLB_USBCRM;
	while(!OSCCTRL->STATUS.bit.DFLLRDY);
	
	// this is generating a reference for our 120mhz
	GCLK->GENCTRL[5].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_DFLL_Val) | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_DIV(24u);
	while(GCLK->SYNCBUSY.bit.GENCTRL5);
	
	// setup DPLL0 to 120MHz
	GCLK->PCHCTRL[OSCCTRL_GCLK_ID_FDPLL0].reg = (1 << GCLK_PCHCTRL_CHEN_Pos) | GCLK_PCHCTRL_GEN(GCLK_PCHCTRL_GEN_GCLK5_Val);
	OSCCTRL->Dpll[0].DPLLRATIO.reg = OSCCTRL_DPLLRATIO_LDRFRAC(0x00) | OSCCTRL_DPLLRATIO_LDR(59);
	while(OSCCTRL->Dpll[0].DPLLSYNCBUSY.bit.DPLLRATIO);
	OSCCTRL->Dpll[0].DPLLCTRLB.reg = OSCCTRL_DPLLCTRLB_REFCLK_GCLK | OSCCTRL_DPLLCTRLB_LBYPASS;
	OSCCTRL->Dpll[0].DPLLCTRLA.reg = OSCCTRL_DPLLCTRLA_ENABLE;
	while(OSCCTRL->Dpll[0].DPLLSTATUS.bit.CLKRDY == 0 || OSCCTRL->Dpll[0].DPLLSTATUS.bit.LOCK == 0);
	// set clock to use dpll0
	
	GCLK->GENCTRL[0].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_DPLL0) | GCLK_GENCTRL_IDC | GCLK_GENCTRL_GENEN;
	while(GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL0);
	
	MCLK->CPUDIV.reg = MCLK_CPUDIV_DIV_DIV1;
}

void ticker_init(void){
	// Timers: in 32 bit mode we pair two - obscure datasheet reading later, they pair in a predefined way: 0 with 1...
	// a word of warning: with the same code, a 16-bit timer was not working. I am mystified.
	TC0->COUNT32.CTRLA.bit.ENABLE = 0;
	TC1->COUNT32.CTRLA.bit.ENABLE = 0;
	// unmask clocks
	MCLK->APBAMASK.reg |= MCLK_APBAMASK_TC0 | MCLK_APBAMASK_TC1; // at 15.8.9
	// generate a gclk
	GCLK->GENCTRL[10].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_DFLL) | GCLK_GENCTRL_GENEN;
	while(GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL(10));
	// ship gclk to their channels
	GCLK->PCHCTRL[TC0_GCLK_ID].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN(10);
	GCLK->PCHCTRL[TC1_GCLK_ID].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN(10);
	// turn on in mode, presync
	TC0->COUNT32.CTRLA.reg = TC_CTRLA_MODE_COUNT32 | TC_CTRLA_PRESCSYNC_PRESC | TC_CTRLA_PRESCALER_DIV16;
	TC1->COUNT32.CTRLA.reg = TC_CTRLA_MODE_COUNT32 | TC_CTRLA_PRESCSYNC_PRESC | TC_CTRLA_PRESCALER_DIV16;
	// allow interrupt to trigger on this event (overflow)
	TC0->COUNT32.INTENSET.bit.OVF = 1;
	// enable, sync for enable write
	TC0->COUNT32.CTRLA.bit.ENABLE = 1;
	while(TC0->COUNT32.SYNCBUSY.bit.ENABLE);
	TC1->COUNT32.CTRLA.bit.ENABLE = 1;
	while(TC0->COUNT32.SYNCBUSY.bit.ENABLE);
	// counter is there, can read COUNT, it's not updating, so a clock is probably off.
	// this for counting
	overflows = 0;
}

void stepticker_init(void){
	// Timers: in 32 bit mode we pair two - obscure datasheet reading later, they pair in a predefined way: 0 with 1...
	// a word of warning: with the same code, a 16-bit timer was not working. I am mystified.
	TC2->COUNT32.CTRLA.bit.ENABLE = 0;
	TC3->COUNT32.CTRLA.bit.ENABLE = 0;
	// unmask clocks
	MCLK->APBBMASK.reg |= MCLK_APBBMASK_TC2 | MCLK_APBBMASK_TC3; // at 15.8.9
	// generate a gclk
	GCLK->GENCTRL[11].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_DFLL) | GCLK_GENCTRL_GENEN;
	while(GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL(11));
	// ship gclk to their channels
	GCLK->PCHCTRL[TC2_GCLK_ID].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN(11);
	GCLK->PCHCTRL[TC3_GCLK_ID].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN(11);
	// turn on in mode, presync
	TC2->COUNT32.CTRLA.reg = TC_CTRLA_MODE_COUNT32 | TC_CTRLA_PRESCSYNC_PRESC | TC_CTRLA_PRESCALER_DIV16 | TC_CTRLA_CAPTEN0;// | TC_CTRLA_CAPTEN1;
	TC3->COUNT32.CTRLA.reg = TC_CTRLA_MODE_COUNT32 | TC_CTRLA_PRESCSYNC_PRESC | TC_CTRLA_PRESCALER_DIV16 | TC_CTRLA_CAPTEN0;// | TC_CTRLA_CAPTEN1;
	// do frequency match
	TC2->COUNT32.WAVE.reg = TC_WAVE_WAVEGEN_MFRQ;
	TC3->COUNT32.WAVE.reg = TC_WAVE_WAVEGEN_MFRQ;
	// allow interrupt to trigger on this event (compare channel 0)
	TC2->COUNT32.INTENSET.bit.MC0 = 1;
	TC2->COUNT32.INTENSET.bit.MC1 = 1; // don't know why, but had to turn this on to get the interrupts
	// set period
	while(TC2->COUNT32.SYNCBUSY.bit.CC0);
	TC2->COUNT32.CC[0].reg = 10;
	// enable, sync for enable write
	TC2->COUNT32.CTRLA.bit.ENABLE = 1;
	while(TC2->COUNT32.SYNCBUSY.bit.ENABLE);
	TC3->COUNT32.CTRLA.bit.ENABLE = 1;
	while(TC3->COUNT32.SYNCBUSY.bit.ENABLE);
}

int main(void)
{
    /* Initialize the SAM system */
    SystemInit();
	clock_init();
	SysTick_Config(4000000);
	
	// lights
	stlb = pin_new(&PORT->Group[1], 14);
	pin_output(&stlb);
	pin_set(&stlb);
	stlr = pin_new(&PORT->Group[1], 13);
	pin_output(&stlr);
	pin_set(&stlr);
	
	// ready interrupt system
	__enable_irq();
	NVIC_EnableIRQ(SERCOM4_0_IRQn); //up0tx
	NVIC_EnableIRQ(SERCOM4_2_IRQn); //up0rx
	NVIC_EnableIRQ(SERCOM5_0_IRQn);
	NVIC_EnableIRQ(SERCOM5_2_IRQn);
	NVIC_EnableIRQ(TC0_IRQn);
	NVIC_EnableIRQ(TC2_IRQn);
	
	// ringbuffers (for uart ports)
	rb_init(&up0_rbrx);
	rb_init(&up0_rbtx);
	rb_init(&up1_rbrx);
	rb_init(&up1_rbtx);
	
	/*
	pin_t testpin = pin_new(&PORT->Group[0], 12);
	pin_output(&testpin);
	pin_set(&testpin);
	
	while(1){
		pin_toggle(&testpin);
	}
	*/
	
	// uarts (ports)
	// there's some chance we should really be using one gclk channel for all of the peripherals
	// and running that at 100MHz or so 
	
	up0 = uart_new(SERCOM4, &PORT->Group[0], &up0_rbrx, &up0_rbtx, 12, 13, HARDWARE_IS_APBD, HARDWARE_ON_PERIPHERAL_D); 
	MCLK->APBDMASK.reg |= MCLK_APBDMASK_SERCOM4;
	uart_init(&up0, 7, SERCOM4_GCLK_ID_CORE, 62675); // baud: 45402 for 921600, 63018 for 115200
	
	up1 = uart_new(SERCOM5, &PORT->Group[1], &up1_rbrx, &up1_rbtx, 3, 2, HARDWARE_IS_APBD, HARDWARE_ON_PERIPHERAL_D);
	MCLK->APBDMASK.reg |= MCLK_APBDMASK_SERCOM5;
	uart_init(&up1, 7, SERCOM5_GCLK_ID_CORE, 62675);
	
	ups[0] = &up0;
	ups[1] = &up1;
	
	up0_stlr = pin_new(&PORT->Group[0], 19);
	pin_output(&up0_stlr);
	pin_set(&up0_stlr);
	up0_stlb = pin_new(&PORT->Group[0], 17);
	pin_output(&up0_stlb);
	pin_set(&up0_stlb);
	up1_stlr = pin_new(&PORT->Group[0], 21);
	pin_output(&up1_stlr);
	pin_set(&up1_stlr);
	up1_stlb = pin_new(&PORT->Group[0], 23);
	pin_output(&up1_stlb);
	pin_set(&up1_stlb);
	
	apaport_build(&apap0, 0, &up0, &up0_stlr, &up0_stlb);
	apaport_reset(&apap0);
	apaport_build(&apap1, 1, &up1, &up1_stlr, &up1_stlb);
	apaport_reset(&apap1);
	
	// SPI
	// TMC_MOSI		PA07 / SER0-3
	// TMC_MISO		PA04 / SER0-0
	// TMC_SCK		PA05 / SER0-1
	// TMC_CSN		PA06 / SER0-2
	
	// spi
	spi_tmc = spi_new(SERCOM0, &PORT->Group[0], 4, 7, 5, 6, HARDWARE_IS_APBA, HARDWARE_ON_PERIPHERAL_D);
	MCLK->APBAMASK.reg |= MCLK_APBAMASK_SERCOM0;
	spi_init(&spi_tmc, 8, SERCOM0_GCLK_ID_CORE, 8, 0, 2, 0, 1, 1, 0);
	
	// TMC
	// STEP			PB08
	// DIR			PB09
	// EN			PB07
	// SG			PB06
	
	en_pin = pin_new(&PORT->Group[1], 7);
	sg_pin = pin_new(&PORT->Group[1], 6);
	
	tmc = tmc26_new(&spi_tmc, &en_pin, &sg_pin);
	// set registers
	tmc26_init(&tmc);
	// enable
	tmc26_enable(&tmc);
	
	ticker_init(); // for counting
	// 32 bit timer, each tick is 1/3 of one us.
	// to read, do TICKER_SYNC, and then var = TICKER as defined
	// overflows in 17.9 minutes
	
	stepticker_init(); // fires interrupt to check if we need to take a step
	// 32 bit timer at 48mhz / 64 and fires on top of 12 for a 16us period: 48 small ticks of 1/3us within each period
	
	step_pin = pin_new(&PORT->Group[1], 8);
	dir_pin = pin_new(&PORT->Group[1], 9);
	pin_output(&step_pin);
	pin_output(&dir_pin);
	
	stepper = stepper_new(&step_pin, &dir_pin);
	stepper_reset(&stepper);
		
	uint8_t lpcnt = 0;
	uint8_t stpcnt = 0;
	
    while (1) 
    {
		apaport_scan(&apap0, 2);
		apaport_scan(&apap1, 2);
		
		/*
		now: step timer should not accumulate error !
		*/
			
		/*
		if(stepper.position_ticks_target == stepper.position_ticks){
			stpcnt ++;
			//pin_clear(&stlr);
			//stepper_targetposition_steps(&stepper, 1000 * (stpcnt %2));
			//stepper_targetspeed_steps(&stepper, 500);
			//stepper_goto(&stepper, 360 * (stpcnt % 2), 1200);
		} else {
			//pin_set(&stlr);
		}
		*/
    }
}

/*
next steps (haha)
- do steps, steps/s speed checking, and direction
- unit test all commands on all ports using mkterminal

- this quickly so that you can mods compose

further reading:	
- quadratic interpolation for position @ time w/ velocity @ endpoint step commands

minors:
- bring baud rate to 1M, at least! what the heck, FTDI!
*/

void SysTick_Handler(void){
	// slow ticker
	pin_toggle(&stlb);
	//pin_toggle(&stlr);
	
	//uart_sendchar_buffered(&up0, 120);

	/*
	if(stepper.position_ticks_target == stepper.position_ticks){
		stpcnt ++;
		stepper_targetposition_steps(&stepper, 500 * (stpcnt %2));
		//stepper_targetspeed_steps(&stepper, 10000);
		//stepper_goto(&stepper, 360 * (stpcnt % 2), 1200);
	}
	*/
}

void SERCOM4_0_Handler(void){
	uart_txhandler(&up0);
}

void SERCOM4_2_Handler(void){
	uart_rxhandler(&up0);
	pin_clear(&stlb);
}

void SERCOM5_0_Handler(void){
	uart_txhandler(&up1);
}

void SERCOM5_2_Handler(void){
	uart_rxhandler(&up1);
	pin_clear(&stlb);
}

void TC0_Handler(void){ // fires rarely, for counting overflows of time-ticker
	TC0->COUNT32.INTFLAG.bit.OVF = 1; // to clear it
	overflows ++;
}

void TC2_Handler(void){ // fires every 16us, for step checking
	TC2->COUNT32.INTFLAG.bit.MC0 = 1;
	TC2->COUNT32.INTFLAG.bit.MC1 = 1;
	stepper_update(&stepper);
}