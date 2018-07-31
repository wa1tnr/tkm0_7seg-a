/*
 * uartport.c
 *
 * Created: 2/6/2018 10:48:26 AM
 *  Author: Jake
 */ 

#include "uartport.h"
#include "hardware.h"

uartport_t uart_new(Sercom *com, PortGroup *port, ringbuffer_t *rbrx, ringbuffer_t *rbtx, uint32_t rx_pin, uint32_t tx_pin, uint32_t apbx, uint32_t peripheral){
	uartport_t uart;
	
	uart.com = com;
	uart.port = port;
	
	uart.rbrx = rbrx;
	uart.rbtx = rbtx;
	
	uart.pinrx = rx_pin;
	uart.pinrx_bm = (uint32_t)(1 << rx_pin);
	uart.pintx = tx_pin;
	uart.pintx_bm = (uint32_t)(1 << tx_pin);
	uart.apbx = apbx;
	uart.peripheral = peripheral;
	// add ringbuffers
	return uart;
}

void uart_init(uartport_t *uart, uint32_t gclknum, uint32_t gclkidcore, uint16_t baud){
	// rx pin setups
	uart->port->DIRCLR.reg = uart->pinrx_bm;
	uart->port->PINCFG[uart->pinrx].bit.PMUXEN = 1;
	if(uart->pinrx % 2){ // yes if odd
		uart->port->PMUX[uart->pinrx >> 1].reg |= PORT_PMUX_PMUXO(uart->peripheral);
		} else {
		uart->port->PMUX[uart->pinrx >> 1].reg |= PORT_PMUX_PMUXE(uart->peripheral);
	}
	// tx pin setups
	uart->port->DIRSET.reg = uart->pintx_bm;
	uart->port->PINCFG[uart->pintx].bit.PMUXEN = 1;	
	if(uart->pintx % 2){ // yes if odd
		uart->port->PMUX[uart->pintx >> 1].reg |= PORT_PMUX_PMUXO(uart->peripheral);
		} else {
		uart->port->PMUX[uart->pintx >> 1].reg |= PORT_PMUX_PMUXE(uart->peripheral);
	}
	
	// unmask the clock
	// -> have to do this manually b/c unfavourable api
	GCLK->GENCTRL[gclknum].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_DFLL) | GCLK_GENCTRL_GENEN;
	while(GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL(gclknum));
	GCLK->PCHCTRL[gclkidcore].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN(gclknum);
	// now the sercom
	while(uart->com->USART.SYNCBUSY.bit.ENABLE);
	uart->com->USART.CTRLA.bit.ENABLE = 0;
	while(uart->com->USART.SYNCBUSY.bit.SWRST);
	uart->com->USART.CTRLA.bit.SWRST = 1;
	while(uart->com->USART.SYNCBUSY.bit.SWRST);
	while(uart->com->USART.SYNCBUSY.bit.SWRST || SERCOM5->USART.SYNCBUSY.bit.ENABLE);
	
	uart->com->USART.CTRLA.reg = SERCOM_USART_CTRLA_MODE(1) | SERCOM_USART_CTRLA_DORD | SERCOM_USART_CTRLA_RXPO(1) | SERCOM_USART_CTRLA_TXPO(0);
	while(uart->com->USART.SYNCBUSY.bit.CTRLB);
	uart->com->USART.CTRLB.reg = SERCOM_USART_CTRLB_RXEN | SERCOM_USART_CTRLB_TXEN | SERCOM_USART_CTRLB_CHSIZE(0);
	/*
	BAUD = 65536*(1-S*(fBAUD/fref))
	where S is samples per bit, 16 for async uart
	where fBAUD is the rate that you want
	where fref is the peripheral clock from GCLK, in this case (and most) 48MHz
	*/
	uart->com->USART.BAUD.reg = baud;
	while(uart->com->USART.SYNCBUSY.bit.ENABLE);
	uart->com->USART.CTRLA.bit.ENABLE = 1;
	
	uart->com->USART.INTENSET.bit.RXC = 1; // set receive interrupt on, see 34.6.4.2
}

void uart_sendchar_polled(uartport_t *uart, uint8_t data){
	while(!uart->com->USART.INTFLAG.bit.DRE);
	uart->com->USART.DATA.reg = data;
}

void uart_sendchar_buffered(uartport_t *uart, uint8_t data){
	rb_putchar(uart->rbtx, data); // dump it in there
	uart->com->USART.INTENSET.bit.DRE = 1; // set up the volley
}

void uart_sendchars_buffered(uartport_t *uart, uint8_t *data, uint8_t length){
	rb_putdata(uart->rbtx, data, length);
	uart->com->USART.INTENSET.bit.DRE = 1;
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