/*
 * apaport.h
 *
 * Created: 2/23/2018 9:17:34 AM
 *  Author: Jake
 */ 


#ifndef APAPORT_H_
#define APAPORT_H_

#include "apahandler.h"
#include "uartport.h"
#include "pin.h"

#define APAPORT_NUM_STATIC_PACKETS 4
#define APAPORT_NUM_PORTS 2

#define APAPORT_OUTSIDE_PACKET 0
#define APAPORT_INSIDE_PACKET 1

#define APA_END_ROUTE_DELIMITER 255
#define APA_ROUTE_POINTER 254
#define APA_ROUTE_FLOOD 253

typedef struct{
	uartport_t *uart;
	pin_t *stlr;
	pin_t *stlb;
	
	uint8_t portnum; // which port are we
	
	uint32_t packet_num;
	uint32_t packet_position;
	uint32_t packets_ready;
	uint32_t packet_state;
	uint8_t packets[APAPORT_NUM_STATIC_PACKETS][256]; // packets for handling by app
}apaport_t;

void apaport_build(apaport_t *apap, uint8_t portnum, uartport_t *uart, pin_t *stlr, pin_t *stlb);

void apaport_reset(apaport_t *apap);

void apaport_scan(apaport_t *apap, uint32_t maxpackets);

void apapacket_shift_pointer(uint8_t *packet, uint8_t portnum);

#endif /* APAPORT_H_ */