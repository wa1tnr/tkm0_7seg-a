/*
 * spiport.h
 *
 * Created: 2/7/2018 10:51:52 AM
 *  Author: Jake
 */ 


#ifndef SPIPORT_H_
#define SPIPORT_H_

#include <avr/io.h>
#include "pin.h"

typedef struct{
	USART_t *com;
	PORT_t *port;
	
	uint8_t miso_bm;
	uint8_t mosi_bm;
	uint8_t sck_bm;
	
	pin_t *csn;
}spiport_t;

void spi_init(spiport_t *spi, USART_t *com, PORT_t *port, uint8_t miso_bm, uint8_t mosi_bm, uint8_t sck_bm, pin_t *csn);

void spi_start(spiport_t *spi, uint8_t cpha, uint8_t cpol); // bits: 0: 8, 1: 32

void spi_txchar_polled(spiport_t *spi, uint8_t data);
void spi_txchars_polled(spiport_t *spi, uint8_t *data, uint8_t length);
void spi_txrxchar_polled(spiport_t *spi, uint8_t data, uint8_t *rxdata);


#endif /* SPIPORT_H_ */