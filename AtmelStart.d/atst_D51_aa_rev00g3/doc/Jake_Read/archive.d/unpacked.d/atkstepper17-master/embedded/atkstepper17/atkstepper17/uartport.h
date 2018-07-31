/*
 * uartport.h
 *
 * Created: 6/18/2018 12:12:43 PM
 *  Author: Jake
 */ 


#ifndef UARTPORT_H_
#define UARTPORT_H_

#include "avr/io.h"
#include "ringbuffer.h"
#include "pin.h"

typedef struct{
	USART_t *uart;
	PORT_t *port;
		
	uint8_t pinRX_bm;
	uint8_t pinTX_bm;
	
	ringbuffer_t *rbrx;
	ringbuffer_t *rbtx;
	
	pin_t *stlrx;
	pin_t *stltx;
	
}uartport_t;

void uart_init(uartport_t *up, USART_t *uart, PORT_t *port, uint8_t pinRX_bm, uint8_t pinTX_bm, ringbuffer_t *rbrx, ringbuffer_t *rbtx, pin_t *stlrx, pin_t *stltx);

void uart_start(uartport_t *up, uint8_t BAUDA, uint8_t BAUDB);

void uart_sendchar_polled(uartport_t *up, uint8_t data);
void uart_sendchar_buffered(uartport_t *up, uint8_t data);
void uart_sendchars_buffered(uartport_t *up, uint8_t *data, uint8_t length);

void uart_rxhandler(uartport_t *up);
void uart_txhandler(uartport_t *up);

#endif /* UARTPORT_H_ */