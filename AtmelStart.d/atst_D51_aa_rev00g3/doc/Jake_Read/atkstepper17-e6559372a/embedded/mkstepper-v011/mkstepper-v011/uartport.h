/*
 * uartport.h
 *
 * Created: 2/6/2018 10:47:56 AM
 *  Author: Jake
 */ 

#ifndef UARTPORT_H_
#define UARTPORT_H_

#include "sam.h"
#include "ringbuffer.h"
#include "pin.h"

#define HARDWARE_IS_APBA 0
#define HARDWARE_IS_APBB 1
#define HARDWARE_IS_APBC 2
#define HARDWARE_IS_APBD 3

#define HARDWARE_ON_PERIPHERAL_A 0x0
#define HARDWARE_ON_PERIPHERAL_B 0x1
#define HARDWARE_ON_PERIPHERAL_C 0x2
#define HARDWARE_ON_PERIPHERAL_D 0x3

typedef struct{
	Sercom *com;
	PortGroup *port;
	
	uint32_t pinrx;
	uint32_t pinrx_bm;
	uint32_t pintx;
	uint32_t pintx_bm;
	
	ringbuffer_t *rbrx;
	ringbuffer_t *rbtx;
	
	pin_t *stlr;
	pin_t *stlb;
	
	uint32_t apbx;
	uint32_t peripheral;
	uint16_t baud;
}uartport_t;

uartport_t uart_new(Sercom *com, PortGroup *port, ringbuffer_t *rbrx, ringbuffer_t *rbtx, uint32_t rx_pin, uint32_t tx_pin, uint32_t apbx, uint32_t peripheral);

void uart_init(uartport_t *uart, uint32_t gclknum, uint32_t gclkidcore, uint16_t baud);

void uart_sendchar_polled(uartport_t *uart, uint8_t data);
void uart_sendchar_buffered(uartport_t *uart, uint8_t data);
void uart_sendchars_buffered(uartport_t *uart, uint8_t *data, uint8_t length);

void uart_rxhandler(uartport_t *uart);
void uart_txhandler(uartport_t *uart);

#endif /* UARTPORT_H_ */