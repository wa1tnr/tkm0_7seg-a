/*
 * uartport.c
 *
 * Created: 6/18/2018 12:12:32 PM
 *  Author: Jake
 */ 

#include "uartport.h"
#include "hardware.h"

void uart_init(uartport_t *up, USART_t *uart, PORT_t *port, uint8_t pinRX_bm, uint8_t pinTX_bm, ringbuffer_t *rbrx, ringbuffer_t *rbtx, pin_t *stlrx, pin_t *stltx){
	up->uart = uart;
	up->port = port;
	up->pinRX_bm = pinRX_bm;
	up->pinTX_bm = pinTX_bm;
	up->rbrx = rbrx;
	up->rbtx = rbtx;
	up->stlrx = stlrx;
	up->stltx = stltx;
}

void uart_start(uartport_t *up, uint8_t BAUDA, uint8_t BAUDB){
	up->uart->BAUDCTRLA = BAUDA;
	up->uart->BAUDCTRLB = BAUDB;
	
	up->uart->CTRLA |= USART_RXCINTLVL_MED_gc | USART_TXCINTLVL_OFF_gc | USART_DREINTLVL_OFF_gc;
	
	up->uart->CTRLB = USART_TXEN_bm | USART_RXEN_bm;
	
	// should try that parity
	up->uart->CTRLC = USART_CMODE_ASYNCHRONOUS_gc | USART_PMODE_DISABLED_gc | USART_CHSIZE_8BIT_gc;
	
	up->port->DIRCLR = up->pinRX_bm;
	up->port->DIRSET = up->pinTX_bm;
	
	pin_set(up->stlrx);
	pin_set(up->stltx);
}

void uart_sendchar_polled(uartport_t *up, uint8_t data){
	while(!(up->uart->STATUS & USART_DREIF_bm));
	up->uart->DATA = data;
}

void uart_sendchar_buffered(uartport_t *up, uint8_t data){
	rb_putchar(up->rbtx, data);
	pin_clear(up->stltx);
	up->uart->CTRLA |= USART_DREINTLVL_MED_gc;
}

void uart_sendchars_buffered(uartport_t *up, uint8_t *data, uint8_t length){
	rb_putdata(up->rbtx, data, length);
	pin_clear(up->stltx);
	up->uart->CTRLA |= USART_DREINTLVL_MED_gc;
}

void uart_rxhandler(uartport_t *up){
	uint8_t data = up->uart->DATA;
	rb_putchar(up->rbrx, data);
	pin_clear(up->stlrx);
}

void uart_txhandler(uartport_t *up){
	if(!rb_empty(up->rbtx)){
		up->uart->DATA = rb_get(up->rbtx);
	} else {
		up->uart->CTRLA = USART_DREINTLVL_OFF_gc | USART_RXCINTLVL_MED_gc;
		pin_set(up->stltx);
	}
}