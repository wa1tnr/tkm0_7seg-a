/*
 * spiport.h
 *
 * Created: 2/7/2018 10:51:52 AM
 *  Author: Jake
 */ 


#ifndef SPIPORT_H_
#define SPIPORT_H_

#include "sam.h"

// TODO: cleaning settings / init values for prettiness, ease of use on variable devices

typedef struct{
	Sercom *com;
	PortGroup *port;
	
	uint32_t miso_pin;
	uint32_t miso_bm;
	uint32_t mosi_pin;
	uint32_t mosi_bm;
	uint32_t sck_pin;
	uint32_t sck_bm;
	uint32_t csn_pin;
	uint32_t csn_bm;
	
	uint32_t apbx;
	uint32_t peripheral;
	uint32_t baud;
}spiport_t;

spiport_t spi_new(Sercom *com, PortGroup *port, uint32_t miso_pin, uint32_t mosi_pin, uint32_t sck_pin, uint32_t csn_pin, uint32_t apbx, uint32_t peripheral);

void spi_init(spiport_t *spi, uint32_t gclknum, uint32_t gclkidcore, uint8_t baud, uint8_t dipo, uint8_t dopo, uint8_t csnhardware, uint8_t cpha, uint8_t cpol, uint8_t lsbfirst); // bits: 0: 8, 1: 32

void spi_txchar_polled(spiport_t *spi, uint8_t data);
void spi_txchars_polled(spiport_t *spi, uint8_t *data, uint8_t length);
void spi_txrxchar_polled(spiport_t *spi, uint8_t data, uint8_t *rxdata);

#endif /* SPIPORT_H_ */