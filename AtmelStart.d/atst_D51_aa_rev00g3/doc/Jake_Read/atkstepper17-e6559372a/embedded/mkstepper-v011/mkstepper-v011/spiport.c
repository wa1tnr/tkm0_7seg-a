/*
 * spiport.c
 *
 * Created: 2/7/2018 10:51:42 AM
 *  Author: Jake
 */ 

#include "spiport.h"

spiport_t spi_new(Sercom *com, PortGroup *port, uint32_t miso_pin, uint32_t mosi_pin, uint32_t sck_pin, uint32_t csn_pin, uint32_t apbx, uint32_t peripheral){
	spiport_t spi;
	
	spi.com = com;
	spi.port = port;
	
	spi.miso_pin = miso_pin;
	spi.miso_bm = (uint32_t)(1 << miso_pin);
	spi.mosi_pin = mosi_pin;
	spi.mosi_bm = (uint32_t)(1 << mosi_pin);
	spi.sck_pin = sck_pin;
	spi.sck_bm = (uint32_t)(1 << sck_pin);
	spi.csn_pin = csn_pin;
	spi.csn_bm = (uint32_t)(1 << csn_pin);
	
	spi.apbx = apbx;
	spi.peripheral = peripheral;
	
	return spi;
}

void spi_init(spiport_t *spi, uint32_t gclknum, uint32_t gclkidcore, uint8_t baud, uint8_t dipo, uint8_t dopo, uint8_t csnhardware, uint8_t cpha, uint8_t cpol, uint8_t lsbfirst){
	// to add to this lib: doc, cleaning, options properly enumerated: do when doing AS5147
	// clk is unmasked (external to this lib)
	// do pin configs
	spi->port->DIRCLR.reg |= spi->miso_bm;
	spi->port->PINCFG[spi->miso_pin].bit.PMUXEN = 1;
	spi->port->DIRSET.reg |= spi->mosi_bm | spi->sck_bm | spi->csn_bm;
	spi->port->PINCFG[spi->mosi_pin].bit.PMUXEN = 1;
	spi->port->PINCFG[spi->sck_pin].bit.PMUXEN = 1;
	
	if(csnhardware){
		spi->port->PINCFG[spi->csn_pin].bit.PMUXEN = 1;
		if(spi->csn_pin % 2){ // yes if odd
			spi->port->PMUX[spi->csn_pin >> 1].reg |= PORT_PMUX_PMUXO(spi->peripheral);
			} else {
			spi->port->PMUX[spi->csn_pin >> 1].reg |= PORT_PMUX_PMUXE(spi->peripheral);
		}
	}else{
		spi->port->OUTSET.reg = spi->csn_bm; // set hi to start! - this should properly depend on our CPHA !
	}
	
	if(spi->miso_pin % 2){ // yes if odd
		spi->port->PMUX[spi->miso_pin >> 1].reg |= PORT_PMUX_PMUXO(spi->peripheral);
	} else {
		spi->port->PMUX[spi->miso_pin >> 1].reg |= PORT_PMUX_PMUXE(spi->peripheral);
	}
	
	if(spi->mosi_pin % 2){ // yes if odd
		spi->port->PMUX[spi->mosi_pin >> 1].reg |= PORT_PMUX_PMUXO(spi->peripheral);
		} else {
		spi->port->PMUX[spi->mosi_pin >> 1].reg |= PORT_PMUX_PMUXE(spi->peripheral);
	}
	
	if(spi->sck_pin % 2){ // yes if odd
		spi->port->PMUX[spi->sck_pin >> 1].reg |= PORT_PMUX_PMUXO(spi->peripheral);
		} else {
		spi->port->PMUX[spi->sck_pin >> 1].reg |= PORT_PMUX_PMUXE(spi->peripheral);
	}
	
	// build a clock for
	GCLK->GENCTRL[gclknum].reg = GCLK_GENCTRL_SRC(GCLK_GENCTRL_SRC_DFLL) | GCLK_GENCTRL_GENEN;
	while(GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL(gclknum));
	GCLK->PCHCTRL[gclkidcore].reg = GCLK_PCHCTRL_CHEN | GCLK_PCHCTRL_GEN(gclknum);
	
	// now some SERCOM
	// tmc spi is clock inactive hi
	// tmc spi latches data on the rising edge of sck and drives data out on the next falling edge
	spi->com->SPI.CTRLA.bit.ENABLE = 0;
	// master, data in pinout, data out pinout
	spi->com->SPI.CTRLA.reg |= SERCOM_SPI_CTRLA_MODE(0x03);
	if(cpol){
		spi->com->SPI.CTRLA.reg |= SERCOM_SPI_CTRLA_CPOL;
	}
	if(cpha){
		spi->com->SPI.CTRLA.reg	|= SERCOM_SPI_CTRLA_CPHA;
	}
	if(lsbfirst){
		SERCOM0->SPI.CTRLA.reg |= SERCOM_SPI_CTRLA_DORD; // 0 MSB, 1 LSB
	}
	spi->com->SPI.CTRLA.reg |= SERCOM_SPI_CTRLA_DIPO(dipo) | SERCOM_SPI_CTRLA_DOPO(dopo);
	// these to defaults, but here for show
	//SERCOM0->SPI.CTRLB.reg |= SERCOM_SPI_CTRLB_CHSIZE(0x0); // 8 bits character - 0x0, so no need to set
	// BAUD
	// f_baud = f_ref / (2 * (BAUD +1)) so BAUD = f_ref / (2 * f_baud) - 1
	spi->com->SPI.BAUD.reg |= SERCOM_SPI_BAUD_BAUD(baud);
	// use hardware slave select, enable receiver
	spi->com->SPI.CTRLB.reg |= SERCOM_SPI_CTRLB_RXEN; //SERCOM_SPI_CTRLB_MSSEN | 
	// 8 or 32 bits
	// do if(bits)
	//spi->com->SPI.CTRLC.reg |= SERCOM_SPI_CTRLC_DATA32B;
	//spi->com->SPI.LENGTH.reg |= SERCOM_SPI_LENGTH_LENEN | SERCOM_SPI_LENGTH_LEN(20);
	
	// turnt it up
	spi->com->SPI.CTRLA.bit.ENABLE = 1;
}

void spi_txchar_polled(spiport_t *spi, uint8_t data){
	while(!(spi->com->SPI.INTFLAG.bit.DRE));
	spi->com->SPI.DATA.reg = SERCOM_SPI_DATA_DATA(data);
}

void spi_txchars_polled(spiport_t *spi, uint8_t *data, uint8_t length){
	spi->port->OUTCLR.reg = spi->csn_bm;
	for(int i = 0; i < length; i ++){
		spi_txchar_polled(spi, data[i]);
	}
	while(!spi->com->SPI.INTFLAG.bit.TXC); // wait for complete before
	spi->port->OUTSET.reg = spi->csn_bm;
}

void spi_txrxchar_polled(spiport_t *spi, uint8_t data, uint8_t *rxdata){
	// how to read?
}