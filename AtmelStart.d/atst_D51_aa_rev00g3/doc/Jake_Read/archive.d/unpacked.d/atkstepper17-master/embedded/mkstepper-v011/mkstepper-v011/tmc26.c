/*
 * tmc2660.c
 *
 * Created: 2/7/2018 10:17:39 PM
 *  Author: Jake
 */ 

#include "tmc26.h"

tmc26_t tmc26_new(spiport_t *spi, pin_t *en, pin_t *sg){
	tmc26_t tmc;
	
	tmc.spi = spi;
	tmc.en_pin = en;
	tmc.sg_pin = sg;
	
	return tmc;
}

void tmc26_write(tmc26_t *tmc, uint32_t word){
	// takes a 20-bit TMC ready word and writes it on the SPI port, using three 8-bit words
	//word = word << 4; // go left for 4 empty bits at the end of byte 3 (20 not 24 bit word)
	uint8_t bytes[3];
	bytes[0] = word >> 16 | 0b11110000; // top 4 & mask for visibility
	bytes[1] = word >> 8; // middle 8
	bytes[2] = word; // last 4 and 0's
	spi_txchars_polled(tmc->spi, bytes, 3);
}

void tmc26_init(tmc26_t *tmc){
	pin_output(tmc->en_pin);
	//pin_input(tmc->sg_pin);
	tmc26_disable(tmc); // turnt off
	
	// address, slope control hi and lo to minimum, short to ground protection on, short to gnd timer 3.2us,
	// enable step/dir, sense resistor full scale current voltage is 0.16mv, readback stallguard2 data, reserved
	uint32_t drvconf = 0b11100000000000110000; 
	tmc26_write(tmc, drvconf);
	
	// address, sgfilt off, threshold value, current scaling (5-bit value appended)
	uint32_t sgthresh_mask = 0b00000111111100000000;
	int32_t sgthres_val = 60;
	uint32_t cscale_mask = 0b00000000000000011111;
	uint32_t cscale_val = 12;
	uint32_t sgcsconf = 0b11010000000000000000 | ((sgthres_val << 8) & sgthresh_mask) | (cscale_val & cscale_mask);
	tmc26_write(tmc, sgcsconf);
	
	// turning coolstep off
	uint32_t smarten = 0b10100000000000000000;
	tmc26_write(tmc, smarten);
	
	// times, delays, cycle mode
	uint32_t chopconf = 0b10011000001000010011;
	tmc26_write(tmc, chopconf);
	
	// 9th bit is intpol, 8th is dedge, last 4 are microstepping
	// 0101 8
	// 0100 16
	// 0011 32
	// 0010 64
	// 0001 128
	// 0000 256
	uint32_t drvctrl = 0b00000000001100000100;
	tmc26_write(tmc, drvctrl);
}

void tmc26_update(tmc26_t *tmc){
	uint32_t smarten = 0b10100000000000000000;
	tmc26_write(tmc, smarten);
}

void tmc26_enable(tmc26_t *tmc){
	pin_clear(tmc->en_pin);
}

void tmc26_disable(tmc26_t *tmc){
	pin_set(tmc->en_pin);
}