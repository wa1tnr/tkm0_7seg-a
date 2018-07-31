/*
 * apahandler.h
 *
 * Created: 3/12/2018 11:55:40 AM
 *  Author: Jake
 */ 


#ifndef APAHANDLER_H_
#define APAHANDLER_H_

#include "sam.h"

#define APA_HANDLER_OUTSIDE 0
#define APA_HANDLER_INSIDE 1

#define DELIM_KEY_TEST 127 // toggles a light, to test network
#define DELIM_KEY_STEPS 128 // steps (steps) uint32_t, speed (steps/s) uint32_t, dir uint8_t
#define DELIM_KEY_BLOCK 129 // is 32 bit int

void apa_handle_packet(uint8_t *packet, uint8_t length);
void apa_return_packet(uint8_t *packet, uint8_t length);

#endif /* APAHANDLER_H_ */