/*
* usb-adafruit-cdc.c
*
* Created: 5/3/2018 6:39:05 PM
* Author : Jake
*/

#include <stdio.h>
#include "sam.h"

#include "hardware.h"
#include "tmc2130.h"

void clock_init(void)
{
	
	/* Set 1 Flash Wait State for 48MHz */
	NVMCTRL->CTRLA.reg |= NVMCTRL_CTRLA_RWS(0);

	/* ----------------------------------------------------------------------------------------------
	* 1) Enable XOSC32K clock (External on-board 32.768Hz oscillator)
	*/
	OSC32KCTRL->OSCULP32K.reg = OSC32KCTRL_OSCULP32K_EN32K;
	/*
	while( (OSC32KCTRL->STATUS.reg & OSC32KCTRL_STATUS_XOSC32KRDY) == 0 ){
	// wait ready
	}
	*/
	
	//OSC32KCTRL->RTCCTRL.bit.RTCSEL = OSC32KCTRL_RTCCTRL_RTCSEL_ULP1K;


	/* Software reset the module to ensure it is re-initialized correctly */
	/* Note: Due to synchronization, there is a delay from writing CTRL.SWRST until the reset is complete.
	* CTRL.SWRST and STATUS.SYNCBUSY will both be cleared when the reset is complete
	*/
	GCLK->CTRLA.bit.SWRST = 1;
	while ( GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_SWRST ){
		/* wait for reset to complete */
	}
	
	/* ----------------------------------------------------------------------------------------------
	* 2) Put XOSC32K as source of Generic Clock Generator 3
	*/
	GCLK->GENCTRL[3].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_OSCULP32K) | //generic clock gen 3
	GCLK_GENCTRL_GENEN;

	while ( GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL3 ){
		/* Wait for synchronization */
	}
	
	/* ----------------------------------------------------------------------------------------------
	* 3) Put Generic Clock Generator 3 as source for Generic Clock Gen 0 (DFLL48M reference)
	*/
	GCLK->GENCTRL[0].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_OSCULP32K) | GCLK_GENCTRL_GENEN;
	
	/* ----------------------------------------------------------------------------------------------
	* 4) Enable DFLL48M clock
	*/

	/* DFLL Configuration in Open Loop mode */

	OSCCTRL->DFLLCTRLA.reg = 0;
	//GCLK->PCHCTRL[OSCCTRL_GCLK_ID_DFLL48].reg = (1 << GCLK_PCHCTRL_CHEN_Pos) | GCLK_PCHCTRL_GEN(GCLK_PCHCTRL_GEN_GCLK3_Val);

	OSCCTRL->DFLLMUL.reg = OSCCTRL_DFLLMUL_CSTEP( 0x1 ) |
	OSCCTRL_DFLLMUL_FSTEP( 0x1 ) |
	OSCCTRL_DFLLMUL_MUL( 0 );

	while ( OSCCTRL->DFLLSYNC.reg & OSCCTRL_DFLLSYNC_DFLLMUL )
	{
		/* Wait for synchronization */
	}
	
	OSCCTRL->DFLLCTRLB.reg = 0;
	while ( OSCCTRL->DFLLSYNC.reg & OSCCTRL_DFLLSYNC_DFLLCTRLB )
	{
		/* Wait for synchronization */
	}
	
	OSCCTRL->DFLLCTRLA.reg |= OSCCTRL_DFLLCTRLA_ENABLE;
	while ( OSCCTRL->DFLLSYNC.reg & OSCCTRL_DFLLSYNC_ENABLE )
	{
		/* Wait for synchronization */
	}
	
	OSCCTRL->DFLLVAL.reg = OSCCTRL->DFLLVAL.reg;
	while( OSCCTRL->DFLLSYNC.bit.DFLLVAL );
	
	OSCCTRL->DFLLCTRLB.reg = OSCCTRL_DFLLCTRLB_WAITLOCK |
	OSCCTRL_DFLLCTRLB_CCDIS;
	
	while ( !OSCCTRL->STATUS.bit.DFLLRDY )
	{
		/* Wait for synchronization */
	}

	/* ----------------------------------------------------------------------------------------------
	* 5) Switch Generic Clock Generator 0 to DFLL48M. CPU will run at 48MHz.
	*/
	GCLK->GENCTRL[1].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_DFLL) |
	GCLK_GENCTRL_IDC |
	GCLK_GENCTRL_OE |
	GCLK_GENCTRL_GENEN;

	while ( GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL0 )
	{
		/* Wait for synchronization */
	}
	
	// now we want a DPLL0 for MCLK
	
	// a reference, from the DFLL, for the DPLL0
	GCLK->GENCTRL[5].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_DFLL_Val) | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_DIV(24u);
	while(GCLK->SYNCBUSY.bit.GENCTRL5);
	
	// the DPLL setup
	GCLK->PCHCTRL[OSCCTRL_GCLK_ID_FDPLL0].reg = (1 << GCLK_PCHCTRL_CHEN_Pos) | GCLK_PCHCTRL_GEN(GCLK_PCHCTRL_GEN_GCLK5_Val);
	OSCCTRL->Dpll[0].DPLLRATIO.reg = OSCCTRL_DPLLRATIO_LDRFRAC(0x00) | OSCCTRL_DPLLRATIO_LDR(59);
	while(OSCCTRL->Dpll[0].DPLLSYNCBUSY.bit.DPLLRATIO);
	OSCCTRL->Dpll[0].DPLLCTRLB.reg = OSCCTRL_DPLLCTRLB_REFCLK_GCLK | OSCCTRL_DPLLCTRLB_LBYPASS;
	OSCCTRL->Dpll[0].DPLLCTRLA.reg = OSCCTRL_DPLLCTRLA_ENABLE;
	while(OSCCTRL->Dpll[0].DPLLSTATUS.bit.CLKRDY == 0 || OSCCTRL->Dpll[0].DPLLSTATUS.bit.LOCK == 0);
	// set clock to use dpll0
	
	// this would switch the CPU clock to the DPLL0
	GCLK->GENCTRL[0].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_DPLL0) | GCLK_GENCTRL_IDC | GCLK_GENCTRL_GENEN;
	while(GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL0);
	
	/* Turn on the digital interface clock */
	//MCLK->APBAMASK.reg |= MCLK_APBAMASK_GCLK;

	/*
	* Now that all system clocks are configured, we can set CLKDIV .
	* These values are normally the ones present after Reset.
	*/
	MCLK->CPUDIV.reg = MCLK_CPUDIV_DIV_DIV1;
}

void lights_init(void){
	stlTicker = pin_new(1, 13);
	pin_output(&stlTicker);
	pin_set(&stlTicker);
	
	stlErr = pin_new(1, 12);
	pin_output(&stlErr);
	pin_set(&stlErr);
	
	up0stlb = pin_new(1, 14);
	pin_output(&up0stlb);
	pin_set(&up0stlb);
	up0stlr = pin_new(1, 15);
	pin_output(&up0stlr);
	pin_set(&up0stlr);
}

void lights_flash(void){
	// yikes!
	pin_toggle(&stlTicker);
	pin_toggle(&stlErr);
	pin_toggle(&up0stlb);
	pin_toggle(&up0stlr);
}

void uarts_init(void){
	// don't forget to also add the handler
	NVIC_EnableIRQ(SERCOM4_0_IRQn);
	NVIC_EnableIRQ(SERCOM4_2_IRQn);
	// init rbs
	rb_init(&up0rbrx);
	rb_init(&up0rbtx);
	// init uart
	up0 = uart_new(SERCOM4, &PORT->Group[0], &up0rbrx, &up0rbtx, &up0stlr, &up0stlb, 12, 13);
	MCLK->APBDMASK.bit.SERCOM4_ = 1;
	uart_init(&up0, 8, SERCOM4_GCLK_ID_CORE, BAUD_SYSTEM, HARDWARE_ON_PERIPHERAL_D);
	
	ups[0] = &up0;
}

void apaports_init(void){
	apap0 = apaport_new(0, &up0, &up0stlr, &up0stlb);
	apaport_reset(&apap0);
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
	
	// enable the IRQ
	NVIC_EnableIRQ(TC0_IRQn);
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
	
	// enable the IRQ
	NVIC_EnableIRQ(TC2_IRQn);
}

// CURRENTLY:
/*
 - hardware reconfigure, req all
  - tmc->2130 and SPI
  - tmc->en,diag,iref_pwm
  - step timer back online 
*/

int main(void)
{
	/* Initialize the SAM system */
	SystemInit();
	// clock setup to run main CPU at 120MHz, and DFLL48M setup from internal osc, should run USB
	clock_init();
	// enable interrupts in the system
	__enable_irq();
	
	lights_init();
	
	// init uartports
	uarts_init();
	
	// init apaports
	apaports_init();
	
	// init stepper structs
	
	spi_tmc2130 = spi_new(SERCOM0, &PORT->Group[0], 4, 7, 5, 6, HARDWARE_ON_PERIPHERAL_D);
	MCLK->APBAMASK.bit.SERCOM0_ = 1;
	spi_init(&spi_tmc2130, 8, SERCOM0_GCLK_ID_CORE, 16, 0, 2, 0, 1, 1, 0);
	
	en_pin = pin_new(0, 2);
	pin_output(&en_pin);
	pin_clear(&en_pin);
	step_pin = pin_new(1, 8);
	pin_output(&step_pin);
	dir_pin = pin_new(1, 9);
	pin_output(&dir_pin);
	
	diag0 = pin_new(1, 4);
	diag1 = pin_new(0, 3);
	
	tmc_driver = tmc2130_new(&spi_tmc2130, &en_pin, &diag0, &diag1);
	
	// setup timers
	
	ticker_init();
	
	stepticker_init();
	
	// the stepper object
	
	stepper = stepper_new(&step_pin, &dir_pin);
	stepper_reset(&stepper);
	
	// a ticker to look for hangouts
	SysTick_Config(8000000);
	
	// wakeup and say hello
	for(int i = 0; i < 600000; i ++){
		if(!(i % 190000)){
			lights_flash();
		}
	}
	
	uint32_t stick = 0;
	
	tmc2130_init(&tmc_driver);
	
	while (1)
	{
		
		apaport_scan(&apap0, 3);
		
		/*
		if(!(stick % 200)){
			stick = 0;
			pin_toggle(&step_pin);
			uart_sendchar_buffered(&up0, 85);
		}
		stick ++;
		*/
		/*
		while(!rb_empty(up0.rbrx)){
		uart_sendchar_buffered(&up0, rb_get(up0.rbrx));
		}
		
		while(!rb_empty(up1.rbrx)){
		uart_sendchar_buffered(&up1, rb_get(up1.rbrx));
		}
		*/
	}
}

void SysTick_Handler(void){
	pin_toggle(&stlTicker);
}

void SERCOM4_0_Handler(void){
	uart_txhandler(&up0);
}

void SERCOM4_2_Handler(void){
	uart_rxhandler(&up0);
}


void TC0_Handler(void){ // fires rarely, for counting overflows of time-ticker
	TC0->COUNT32.INTFLAG.bit.OVF = 1; // to clear it
	overflows ++;
}

void TC2_Handler(void){ // fires every 8.3us, for step checking
	TC2->COUNT32.INTFLAG.bit.MC0 = 1;
	TC2->COUNT32.INTFLAG.bit.MC1 = 1;
	stepper_update(&stepper);
}