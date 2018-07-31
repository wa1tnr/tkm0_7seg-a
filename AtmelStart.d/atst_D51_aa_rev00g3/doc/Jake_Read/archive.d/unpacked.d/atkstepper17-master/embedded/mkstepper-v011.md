# MK Stepper Embedded Doc

My goal is to 
 - verify that I can communicate over both UART ports to the board 
  - now, wrap gpio and uart in tiny packages
 - verify that I can speak SPI to the TMC2660, maybe the AS5147D.
  - then, wrap spi into a package
 - fiddle through tmc2660 config and send some steps
 - send a key:value to the board that is a 'step command' - a # of steps to take over a timespan, and setup a queue of these

## Ring Testing the ATSAMD51

OK, first I setup the systick timer to fire every 500 cycles. This is handy - I use the interrupt to toggle a pin, then I can get a rough estimate of where the main clock is running.

```C
int main(void)
{
    /* Initialize the SAM system */
    SystemInit();
	
	// setup blinking
	
	PORT->Group[1].DIRSET.reg |= (uint32_t)(1 << 13);
	PORT->Group[1].DIRSET.reg |= (uint32_t)(1 << 14);
	PORT->Group[1].OUTSET.reg |= (uint32_t)(1 << 13);
	PORT->Group[1].OUTSET.reg |= (uint32_t)(1 << 14);
	
	// setup ring
	
	PORT->Group[1].DIRSET.reg |= (uint32_t)(1 << 2);
	PORT->Group[1].OUTSET.reg |= (uint32_t)(1 << 2);
	
	PORT->Group[1].DIRCLR.reg = (uint32_t)(1 << 3);
	PORT->Group[1].PINCFG[3].reg |= PORT_PINCFG_INEN;
	PORT->Group[1].PINCFG[3].reg &= ~PORT_PINCFG_PULLEN;
	
	SysTick_Config(500);

    while (1) 
    {

    }
}

void SysTick_Handler(void){
	PORT->Group[1].OUTTGL.reg = (uint32_t)(1 << 2);
	PORT->Group[1].OUTTGL.reg = (uint32_t)(1 << 14); // blink STLB
}
```

I see 48kHz on the scope, and I know I've got a 1000x multiplier on that wave (as the scope is counting positive clock edges only, the toggle does postive-to-negative etc). So these chips set up to run at 48MHz unless we tell them otherwise. Let's try that. I'm following adafruit's bootloader code, mostly.

Also, [this](https://github.com/ataradov/mcu-starter-projects/blob/master/samd21/hal_gpio.h) was nice GPIO reference for the structures used in the ATSAMD series (21 and 51). 

OK, this clock is baffling me. I'm trapped at 48MHz ... going to move on for now. Somehow I want to set up the DFLL (digital frequency locked loop .. ?) to run on the 32.768kHz xtal (as a reference) with a big ol' multiplier to bring it up to 120MHz. To be honest I'm not really sure if even this is correct.

In any case, here's some code for a ring test, for when I get the clock sorted. On a 48MHz clock it runs at 1.6MHz. Uses the setup above... 

```C
    while (1) 
    {
		//PORT->Group[1].OUTTGL.reg = (uint32_t)(1 << 2);
		if(PORT->Group[1].IN.reg & (1 << 3)){
			PORT->Group[1].OUTCLR.reg = (uint32_t)(1 << 2);
		} else {
			PORT->Group[1].OUTSET.reg = (uint32_t)(1 << 2);
		}
    }
```

## USART on the ATSAMD51

Altogether more registers than I'd like to manage, but here we are.

```C

int main(void)
{
    /* Initialize the SAM system */
    SystemInit();
	
	//clock_init();
	
	/*setup blinking
	STLR		PB13
	STLB		PB14
	*/
	PORT->Group[1].DIRSET.reg |= (uint32_t)(1 << 13);
	PORT->Group[1].DIRSET.reg |= (uint32_t)(1 << 14);
	PORT->Group[1].OUTSET.reg |= (uint32_t)(1 << 13);
	PORT->Group[1].OUTSET.reg |= (uint32_t)(1 << 14);
	
	SysTick_Config(5000000);
	
	/* setup UARTs
	NP1RX		PA12 / SER4-1
	NP1TX		PA13 / SER4-0

	NP2RX		PB03 / SER5-1 / Peripheral D
	NP2TX		PB02 / SER5-0 / Peripheral D
	*/
	
	// setup pins for peripheral
	PORT->Group[1].DIRCLR.reg = (uint32_t)(1 << 3); // rx is input
	PORT->Group[1].DIRSET.reg = (uint32_t)(1 << 2); // tx output
	PORT->Group[1].PINCFG[3].bit.PMUXEN = 1;
	PORT->Group[1].PMUX[3>>1].reg |= PORT_PMUX_PMUXE(0x3); 
	PORT->Group[1].PMUX[3>>3].reg |= PORT_PMUX_PMUXO(0x3);
	PORT->Group[1].PINCFG[2].bit.PMUXEN = 1;
	PORT->Group[1].PMUX[2>>1].reg |= PORT_PMUX_PMUXE(0x3);
	PORT->Group[1].PMUX[2>>3].reg |= PORT_PMUX_PMUXO(0x3);
	// unmask clocks
	MCLK->APBDMASK.reg |= MCLK_APBDMASK_SERCOM5;
	// generate clocks to, starting with clock 6 (arbitrary choice, lower # held for system things)
	// datasheet says normally one gclk per peripheral shrugman knows why
	GCLK->GENCTRL[6].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_DFLL) | GCLK_GENCTRL_GENEN;
	while(GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL6);
	GCLK->PCHCTRL[SERCOM5_GCLK_ID_CORE].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN_GCLK6;
	// now the sercom
	while(SERCOM5->USART.SYNCBUSY.bit.ENABLE);
	SERCOM5->USART.CTRLA.bit.ENABLE = 0;
	while(SERCOM5->USART.SYNCBUSY.bit.SWRST);
	SERCOM5->USART.CTRLA.bit.SWRST = 1;
	while(SERCOM5->USART.SYNCBUSY.bit.SWRST);
	while(SERCOM5->USART.SYNCBUSY.bit.SWRST || SERCOM5->USART.SYNCBUSY.bit.ENABLE);
	// now reset and ready, do config
	SERCOM5->USART.CTRLA.reg = SERCOM_USART_CTRLA_MODE(1) | SERCOM_USART_CTRLA_DORD | SERCOM_USART_CTRLA_RXPO(1) | SERCOM_USART_CTRLA_TXPO(0);
	while(SERCOM5->USART.SYNCBUSY.bit.CTRLB);
	SERCOM5->USART.CTRLB.reg = SERCOM_USART_CTRLB_RXEN | SERCOM_USART_CTRLB_TXEN | SERCOM_USART_CTRLB_CHSIZE(0);
	/*
	BAUD = 65536*(1-S*(fBAUD/fref))
	where S is samples per bit, 16 for async uart
	where fBAUD is the rate that you want
	where fref is the peripheral clock from GCLK, in this case (and most) 48MHz
	*/
	SERCOM5->USART.BAUD.reg = 45402;
	while(SERCOM5->USART.SYNCBUSY.bit.ENABLE);
	SERCOM5->USART.CTRLA.bit.ENABLE = 1;
	
	
    while (1) 
    {
		while(!SERCOM5->USART.INTFLAG.bit.DRE);
		SERCOM5->USART.DATA.reg = (uint8_t)170;
    }
}

void SysTick_Handler(void){
	PORT->Group[1].OUTTGL.reg = (uint32_t)(1 << 14); // blink STLB
}
```

## Interrupts on the ATSAMD51

To configure an interrupt on the ATSAMD51, roughly, we do this:

```C

int main(void){

	__enable_irq(); // globally enables the NVIC

	// turns on a particular 'irq line' or 'irq number'
	// for example, in the SERCOM->UART section of the datasheet, 
	// see 34.8.8 - the interrupt 'flag' sections. these are analagous to 'line numbers'
	// and are linked to 'interrupt numbers' per 10.2.2
	NVIC_EnableIRQ(SERCOM4_2_IRQn); 
}

// we can then handle the interrupt:

void SERCOM4_2_Handler(void){
	// we must clear the flag, often this is done by reading the flag register, like this:
	// SERCOM4->USART->INTFLAG.bit.RXC = 1; // odd, writing '1' is normally the way to clear it
	// however, for this particular register, we read the data from the peripheral to clear the interrupt.
	uint8_t data = SERCOM4->USART.DATA.reg;
	pin_clear(&stlr); // indicate
	// then we would presumably do something
}

```

TODO: make ATSAMD51 doc page for all of this bringup !

## Async-happy USART

To handle USART on these systems, I implement a (hopefully) bulletproof and interrupt-friendly architecture.

TX and RX both have interrupt handlers - the RX dumps bytes into a ringbuffer for the application to poll later on (polling the ringbuffer, not the UART hardware) and the TX buffer is set up so that the application can dump a block of bytes into the transmitter at once, without waiting for anything to transmit. 

```C

int main(void)
{
    /* Initialize the SAM system */
    SystemInit();
	SysTick_Config(5000000);
	
	//clock_init();
	
	// lights
	
	stlb = pin_new(&PORT->Group[1], 14);
	pin_output(&stlb);
	pin_set(&stlb);
	stlr = pin_new(&PORT->Group[1], 13);
	pin_output(&stlr);
	pin_set(&stlr);
	
	// ready interrupt system
	__enable_irq();
	NVIC_EnableIRQ(SERCOM4_0_IRQn); //up1tx
	NVIC_EnableIRQ(SERCOM4_2_IRQn); //up1rx
	NVIC_EnableIRQ(SERCOM5_0_IRQn);
	NVIC_EnableIRQ(SERCOM5_2_IRQn);
	
	// ringbuffers (for uart ports)
	rb_init(&up1_rbrx);
	rb_init(&up1_rbtx);
	rb_init(&up2_rbrx);
	rb_init(&up2_rbtx);
	
	// uarts (ports)
	// TODO: have used PMUXO and PMUXE incorrectly: only set one of these, based on whether / not pin is even / odd !
	
	up1 = uart_new(SERCOM4, &PORT->Group[0], &up1_rbrx, &up1_rbtx, 12, 13, HARDWARE_IS_APBD, HARDWARE_ON_PERIPHERAL_D); 
	MCLK->APBDMASK.reg |= MCLK_APBDMASK_SERCOM4;
	uart_init(&up1, 6, SERCOM4_GCLK_ID_CORE, 63018); // baud: 45402 for 921600, 63018 for 115200
	up2 = uart_new(SERCOM5, &PORT->Group[1], &up2_rbrx, &up2_rbtx, 3, 2, HARDWARE_IS_APBD, HARDWARE_ON_PERIPHERAL_D);
	MCLK->APBDMASK.reg |= MCLK_APBDMASK_SERCOM5;
	uart_init(&up2, 7, SERCOM5_GCLK_ID_CORE, 63018);
	
	
	// SPI
	// TMC_MOSI		PA07 / SER0-3
	// TMC_MISO		PA04 / SER0-0
	// TMC_SCK		PA05 / SER0-1
	// TMC_CSN		PA06 / SER0-2
	
	// spi
	spi_tmc = spi_new(SERCOM0, &PORT->Group[0], 4, 7, 5, 6, HARDWARE_IS_APBA, HARDWARE_ON_PERIPHERAL_D);
	MCLK->APBAMASK.reg |= MCLK_APBAMASK_SERCOM0;
	spi_init(&spi_tmc, 8, SERCOM0_GCLK_ID_CORE, 126, 0, 2);
	
	// -> DO SPI, talk to the TMC !
		
    while (1) 
    {
		// spi_txchar_polled(&spitmc, 'x');
		// find TMC registers now, try to read!
		
    }
}

void SysTick_Handler(void){
	pin_toggle(&stlb);
	while(!rb_empty(up1.rbrx)){
		uart_sendchar_buffered(&up1, rb_get(up1.rbrx));
	}
}

void SERCOM4_0_Handler(void){
	uart_txhandler(&up1);
}

void SERCOM4_2_Handler(void){
	uart_rxhandler(&up1);
}

void SERCOM5_0_Handler(void){
	uart_txhandler(&up2);
}

void SERCOM5_2_Handler(void){
	uart_rxhandler(&up2);
}
```

and 

```C

void uart_sendchar_polled(uartport_t *uart, uint8_t data){
	while(!uart->com->USART.INTFLAG.bit.DRE);
	uart->com->USART.DATA.reg = data;
}

void uart_sendchar_buffered(uartport_t *uart, uint8_t data){
	rb_putchar(uart->rbtx, data); // dump it in there
	uart->com->USART.INTENSET.bit.DRE = 1; // set up the volley
}

void uart_rxhandler(uartport_t *uart){
	uint8_t data = uart->com->USART.DATA.reg;
	rb_putchar(uart->rbrx, data);
}

void uart_txhandler(uartport_t *uart){
	if(!rb_empty(uart->rbtx)){
		uart->com->USART.DATA.reg = rb_get(uart->rbtx);
	} else {
		uart->com->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
	}
}
```

## Trinamic Bit-Fiddling

Next I'm going to try to get some steps to happen. I guess we're ready for this!

I found [this](https://github.com/trinamic/TMC26XStepper) library, which will save me from re-writing all of the registers and addresses. Bless.

SPI commands and responses are all 20-bits long. Each 20-bit command is met with a 20-bit status word. Those 20-bit 'commands' are registers - the first three bits of each word denote the address of which register we are re-writing. This means that we have to keep track of each entire register-state in the microcontroller, or should. 

Most likely, operation will consist of starting up with sending all 5 registers sequentially in the setup config, and then occasionally polling one to get the response (which, it looks like, contains useful things like relative loads!). 

Later on, when I try to make a closed loop stepper with the AS5147, I'll be writing coil currents directly into these registers (I think?) to commutate the motor based on encoder input and a tiny PID loop. That, later - but for now I see that in the DRVCTRL register has two 8-bit words for Current in A and Current in B. Noice.

Another thing I like is that I can set both step edges to signify a step. Normally, step drivers have a 'minimum on time' and only count the positive edge, so the microcode has to do a tiny wait (normally not long enough to warrant an interrupt-to-turn-off). This way I'll just toggle the step pin whenever I want to step, and I can forget about it.

So, first I'll try to figure out how to send a 20-bit word with the SPI peripheral, and verify that I get any kind of response from the TMC.

The ATSAMD51 only allows an 8, 9, or 32 bit word, so 32 it is - I'll just keep the last 12 set to 0 and hope that suffices.

OK, here's a classic debugging moment: I take ~45 minutes (or was it two hours? would rather not count) trying to get my SPI setup correctly. Nothing returns. Welp. I pull my hair out. I take a break, decide to plug in some motor power, because why not? Of course, now, data returns. Heck.

To solve that 20-bit word problem, I'm taking over manual control of the chip select pin, issuing 3 8-bit words (which I'll have to bit-splice myself) and then sending those on one CSN low cycle.

Great, now we unfux the hacked together code, and try to send some data, then some steps, I guess?

I think I'm actually going to try writing this library from the datasheet. Let's see how that goes.

OK, great news. This morning Will and Sam reminded me that I can just write binary with the prefix '0b' and GCC will make bits out of that. How wonderful. Here's my TMC setup:

```C
/*
 * tmc2660.c
 *
 * Created: 2/7/2018 10:17:39 PM
 *  Author: Jake
 */ 

#include "tmc2660.h"

tmc2260_t tmc2660_new(spiport_t *spi, pin_t *step, pin_t *dir, pin_t *en, pin_t *sg){
	tmc2260_t tmc;
	
	tmc.spi = spi;
	tmc.step_pin = step;
	tmc.dir_pin = dir;
	tmc.en_pin = en;
	tmc.sg_pin = sg;
	
	return tmc;
}

void tmc2660_write(tmc2260_t *tmc, uint32_t word){
	// takes a 20-bit TMC ready word and writes it on the SPI port, using three 8-bit words
	//word = word << 4; // go left for 4 empty bits at the end of byte 3 (20 not 24 bit word)
	uint8_t bytes[3];
	bytes[0] = word >> 16 | 0b11110000; // top 4 & mask for visibility
	bytes[1] = word >> 8; // middle 8
	bytes[2] = word; // last 4 and 0's
	spi_txchars_polled(tmc->spi, bytes, 3);
}

void tmc2660_init(tmc2260_t *tmc){
	// address, slope control hi and lo to minimum, short to ground protection on, short to gnd timer 3.2us,
	// enable step/dir, sense resistor full scale current voltage is 0.305mv, readback stallguard2 data, reserved
	uint32_t drvconf = 0b11100000000000010000; 
	tmc2660_write(tmc, drvconf);
	
	// address, sgfilt off, threshold value, current scaling (5-bit value appended)
	uint32_t sgthresh_mask = 0b00000111111100000000;
	int32_t sgthres_val = 60;
	uint32_t cscale_mask = 0b00000000000000011111;
	uint32_t cscale_val = 12;
	uint32_t sgcsconf = 0b11010000000000000000 | ((sgthres_val << 8) & sgthresh_mask) | (cscale_val & cscale_mask);
	tmc2660_write(tmc, sgcsconf);
	
	// turning coolstep off
	uint32_t smarten = 0b10100000000000000000;
	tmc2660_write(tmc, smarten);
	
	// times, delays, cycle mode
	uint32_t chopconf = 0b10010010100000010111;
	tmc2660_write(tmc, chopconf);
	
	// 9th bit is intpol, 8th is dedge, last 4 are microstepping
	uint32_t drvctrl = 0b00000000000100000001;
	tmc2660_write(tmc, drvctrl);
}


void tmc2660_update(tmc2260_t *tmc){
	uint32_t smarten = 0b10100000000000000000;
	tmc2660_write(tmc, smarten);
}
```

Here's a capture from the logic analyzer, from boot. I write 5 addresses with the settings above (init) and then I'm polling the driver periodically for stallguard data, by just re-writing a register (I'm re-writing the smarten register, because it's disabled and so I feel safer about not accidentally changing anything). 

![spi](https://gitlab.cba.mit.edu/jakeread/mkstepper/raw/master/images/spi-tmc-fromboot.png)

It took me a minute to figure out how to properly align the 20-bit word within three 8-bit bytes that the SPI peripheral on the ATSAMD51 can handle. There's also a 32 bit extension, which it might make sense to write to, instead. ~ or ~ use a buffer and txinterrupt to fire the series of bytes away. At the moment, I'm polling that flag (txready) before each new byte, meaning when I send a register the processor is waiting around until they're all on the line, and this is non-ideal. However, I was able to push the SPI speed up to 6.25MBPS, so the wait is only 5us (for one register). That's OK. 

![spi](https://gitlab.cba.mit.edu/jakeread/mkstepper/raw/master/images/spi-tmc-speed.png)

I'm running at 0.8A current and everything is nice and cool. When I drive this up to 1.9A, things start to heat up. Next time I draw the board I can likely do a bit better at dissipating heat - a bit helpful step would be to bring the copper thickness past 1oz, that way I can comfortably drive NEMA23s. What would be cooler (haha) would be a little bit of implementation on the coolStep feature, which intelligently sets current in a closed-loop manner, while watching the stallGuard setting - that all happens in the TMC.

The driver's not dead silent, but I expect if I were to play with the settings it might get better. It's certainly better than most drivers I've used. 

# Step Timing

In any case! Towards making these kits of parts do CNC things, I want to be able to send commands to this board like "go x steps at y speed" - with a buffer of these commands, I should be able to successfully sync network steppers in a motion control application. There will of course be the tricky problem of synchronization (!) and to be honest I don't have a good plan for that yet.

Pending success today on 'x steps at y speed' - or maybe better is to do 'x steps over y time' - same thing really, but the latter logic probably better for whatever is upstream, breaking motion into discrete chunks of time before networking them - I can spend some time tomorrow at drafting that architecture.

OK, so. Lit review. AKA asking Sam, Will, and looking at other step-motion-controller softwares online. I'm familiar with Smoothie and Marlin.

I believe most people do this with timers or 'tickers' that run around 1kHz to periodically check and step-or-not-step based on elapsed time, steps left, etc. First thing would be to write a quick table to check how often I'll have to do this in order to reliably step at certain speeds, with this gnarly 128 or 256 microstepping in the TMC. 

Looks like this needs to be pretty fast. In retrospect, that 1kHz ticker I was thinking about was running an acceleration update loop. The actual step ticker has to be much faster ... for 200RPM (which does about 190mm/s on an 18mm diameter pulley) while driving 64 microsteps (more than enough) we have a 43kHz tick. So I'll be writing a ~ 10us ticker, I think. Not unreasonable in microcontroller land. 

## Planning

What's the rest of the plan? I'll need a timer, for sure. I want to run that on the real-time clock and use the kHz crystal I put on the board for that. 

Let's see - there's kind of a lot of different ways to do this. Basically, we are just changing position at set time. Forgive me for getting undergraduate here. My intuition is to make this really simple: we track position, and tell each motor to go to positions at certain times. On the next level up, we do acceleration planning. This would require sending messages across the network at around 1kHz, probably more. 

Maybe a better way is for each block to have an 'end of block' speed and position - and the motors are responsible for ramping up or down over the course of a segment. Then we set two timers: one to check steps to be made against a speed, one to check if the speed needs to be updated. But I'll need to do some head-scratching to figure how to make all of those ends meet: get here at this time and be doing this speed at the end. Feels over-constrained. 

Actually I think this is ideal, on the D-t chart I'm sending commands to get to a point with the curve having a particular slope at that point. I'm drawing bezier type curves that inform a point-to-point S-t chart:

![ptst-naive](https://gitlab.cba.mit.edu/jakeread/mkstepper/raw/master/images/wb-ptst-naive.jpg)

Or is this naive? It is: the S-t points integrate into the P-t points, so by adding the tangent to the P-t information I'm over-constraining the solution for S-t. However, I should be able to decouple those things in whatever planning-level maths I'm doing, and reliably send 'ramp linear to be at this speed at this time' - trouble is that then I'm in time-series, and have to count on the last steps integrating properly into position. Hmmmm. 

So if I send the over-constrained information, I can let the stepper motor error correct by adding some juice of its own, or taking some away, over the time of the block:

![ptst-integrate](https://gitlab.cba.mit.edu/jakeread/mkstepper/raw/master/images/wb-ptst-integrate.jpg)

So we end up doing error correction on the 3rd order for acceleration on the 2nd order. Sounds like I might be making this into a more-complicated-than-it-need-be exercise. Rather than trying to add that next bezier, juts do this: add a point, midpoint to current S-t plan, such that the integral matches. Low cpu overhead, good little error correction, still get to feel secure about positioning, while maintaining the ability to do networked acceleration without flooding hella position messages. 

![ptst-integrate](https://gitlab.cba.mit.edu/jakeread/mkstepper/raw/master/images/wb-ptst-integrate-simple.jpg)

I can check for bounds: like that mid-point (or any combination thereof) driving the acceleration past some max, or I can just count on the controller one level up being a benevolent / brilliant overlord, and go for gold no matter the circumstance. 

Cornering, etc, is actually kind of interesting. See [this note on 'junction deviation'](https://onehossshay.wordpress.com/2011/09/24/improving_grbl_cornering_algorithm/) from the Grbl squad. 

So, here's the plan:
 - get command (although this was a nice discussion, to implement with speed I'm going to take the input 'go, for this speed, for this duration' first)
 - update parameters for loops: current speed (defining an integral to check against) and block start time.
 - build a fast realtime timer to run both of these things,
 - one interrupt updates the time value, an overflowing 32bit integer?
 - one interrupt runs the ticker, checking:
  - do we need to make a step? if yes, do, and update position
  - is the block over? ok.

OK, ENOUGH CHAT amirite? Let's get down to bones and write a timer.

## Timers and Interrupts

My first sad discovery is that with a 32.768kHz clock source for the Real Time Counter, the best I can do is 30us clock intervals. I should have known! haha. I would like to run the time-incrementing on this clock. I can either run a faster timer and then try to calibrate / sync them ... sounds tricky. Or I can just use a faster clock and commit to putting a reliable crystal on the next board, for ~12MHz source (which I'm still struggling to figure out how to make this chip actually chooch at 150MHz?). 

I can alternately just run the step ticker at 30us.

In either case, I'm not making any progress just sitting here.

OK: for speed I'm going to use the SysTick timer to do time counting (i.e updating a micros() type function) and then drive another timer at 10us or so to check for steps.

Here's some code that's working. I originally tried this with a 16 bit timer, to no luck, and I have no idea why.

```C

// from MAIN:
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
	TC0->COUNT32.CTRLA.reg = TC_CTRLA_MODE_COUNT32 | TC_CTRLA_PRESCSYNC_PRESC;
	TC1->COUNT32.CTRLA.reg = TC_CTRLA_MODE_COUNT32 | TC_CTRLA_PRESCSYNC_PRESC;
	// allow interrupt to trigger on this event (overflow)
	TC0->COUNT32.INTENSET.bit.OVF = 1;
	// enable, sync for enable write
	TC0->COUNT32.CTRLA.bit.ENABLE = 1;
	while(TC0->COUNT32.SYNCBUSY.bit.ENABLE);
	TC1->COUNT32.CTRLA.bit.ENABLE = 1;
	while(TC0->COUNT32.SYNCBUSY.bit.ENABLE);
	// counter is there, can read COUNT, it's not updating, so a clock is probably off. 

// and later, to read COUNT and send to UART

void SysTick_Handler(void){
	lpcnt ++;
	pin_toggle(&stlb);
	/*
	while(!rb_empty(up1.rbrx)){
		uart_sendchar_buffered(&up1, rb_get(up1.rbrx));
	}
	*/
	TC0->COUNT32.CTRLBSET.reg = TCC_CTRLBSET_CMD_READSYNC;
	uint32_t thecount = TC0->COUNT32.COUNT.reg;
	uint8_t onemicro = thecount;
	uint8_t twomicro = thecount >> 8;
	uint8_t threemicro = thecount >> 16;
	uint8_t fourmicro = thecount >> 24;
	uart_sendchar_buffered(&up1, onemicro);
	uart_sendchar_buffered(&up1, twomicro);
	uart_sendchar_buffered(&up1, threemicro);
	uart_sendchar_buffered(&up1, fourmicro);
	
	uart_sendchar_buffered(&up1, lpcnt);
	uint8_t stop = TC1->COUNT8.STATUS.bit.STOP;
	uart_sendchar_buffered(&up1, stop);
}
```

OK, this is done now - both timers, and a step to position @ speed command. Next is networks for this board.

## Network Code

Packet Handling and Passing, oh my!

I'll take most of this from the code I've already written for the Router.

Done that, just have to debug one apaport and get the lights a flashing. I think this project should be the major touchstone for network code / all around infrastructure as it'll probably get the most love.

## Step Linking and Acceleration 

So, I want to be able to send the command:
 - Start Speed : Steps to take : End Speed
 - or Steps to Take : End Speed and assume link from last block
And do linear interpretation.

I should also, and as a starting exercise, re-write the relative speed and relative steps packet handlers to do ... relative, and real speeds in steps / s. 