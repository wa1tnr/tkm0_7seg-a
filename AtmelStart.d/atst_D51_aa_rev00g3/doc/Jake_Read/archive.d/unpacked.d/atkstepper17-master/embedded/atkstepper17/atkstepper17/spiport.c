/*
 * spiport.c
 *
 * Created: 2/7/2018 10:51:42 AM
 *  Author: Jake
 */ 

#include "spiport.h"

void spi_init(spiport_t *spi, USART_t *com, PORT_t *port, uint8_t miso_bm, uint8_t mosi_bm, uint8_t sck_bm, pin_t *csn){
	spi->com = com;
	spi->port = port;
	
	spi->miso_bm = miso_bm;
	spi->mosi_bm = mosi_bm;
	spi->sck_bm = sck_bm;
	spi->csn = csn;
}

void spi_start(spiport_t *spi, uint8_t cpha, uint8_t cpol){
	spi->port->DIRSET = spi->mosi_bm;
	spi->port->DIRSET = spi->sck_bm;
	spi->port->DIRCLR = spi->miso_bm;
	
	// want BSEL 7 and BSCALE -4
	spi->com->BAUDCTRLA = 0; //130;
	spi->com->BAUDCTRLB = 1; //USART_BSCALE3_bm | USART_BSCALE2_bm | USART_BSCALE1_bm | USART_BSCALE0_bm;
	
	spi->com->CTRLB = USART_TXEN_bm | USART_RXEN_bm;
	
	spi->com->CTRLC = USART_CMODE_MSPI_gc; // set to master spi mode
	
	if(cpha != 0){
		spi->com->CTRLC |= (1 << 1); // in master spi mode, this bit is clock polarity
	}
	
	if(cpol != 0){
		spi->port->PIN5CTRL |= PORT_INVEN_bm;
	}
}

void spi_txchar_polled(spiport_t *spi, uint8_t data){
	while(!(spi->com->STATUS & USART_DREIF_bm));
	spi->com->DATA = data;
}

void spi_txchars_polled(spiport_t *spi, uint8_t *data, uint8_t length){
	pin_clear(spi->csn);
	for(int i = 0; i < length; i ++){
		spi_txchar_polled(spi, data[i]);
		spi->com->STATUS |= USART_TXCIF_bm; // clear transmit complete flag 
	}
	while(!(spi->com->STATUS & USART_TXCIF_bm)); // wait for complete before
	pin_set(spi->csn);
}

void spi_txrxchar_polled(spiport_t *spi, uint8_t data, uint8_t *rxdata){
	// how to read?
}