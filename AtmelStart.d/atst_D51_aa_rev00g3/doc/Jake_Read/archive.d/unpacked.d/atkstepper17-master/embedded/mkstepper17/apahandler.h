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
#define DELIM_KEY_RESET 128 

#define DELIM_KEY_TRAPEZOID 131

void apa_handle_packet(uint8_t *packet, uint8_t length);

void apa_reply_packet(uint8_t *opPacket, uint8_t opLength, uint8_t *replyData, uint8_t replyLength);
void apa_return_packet(uint8_t *packet, uint8_t length);

#endif /* APAHANDLER_H_ */