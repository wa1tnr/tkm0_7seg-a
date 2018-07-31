/*
 * atkhandler.h
 *
 * Created: 3/12/2018 11:55:40 AM
 *  Author: Jake
 */ 


#ifndef ATKHANDLER_H_
#define ATKHANDLER_H_

#include <avr/io.h>

#define ATK_HANDLER_OUTSIDE 0
#define ATK_HANDLER_INSIDE 1

#define DELIM_KEY_TEST 127 // toggles a light, to test network
#define DELIM_KEY_RESET 128 

#define DELIM_KEY_TRAPEZOID 131 
#define DELIM_KEY_WAIT 132

void atk_handle_packet(uint8_t *packet, uint8_t length);

void atk_reply_packet(uint8_t *opPacket, uint8_t *replyData, uint8_t replyLength);
void atk_return_packet(uint8_t *packet, uint8_t length);

#endif /* atkHANDLER_H_ */