/*
 * atkport.h
 *
 * Created: 2/23/2018 9:17:34 AM
 *  Author: Jake
 */ 


#ifndef ATKPORT_H_
#define ATKPORT_H_

#include "uartport.h"
#include "pin.h"

#define ATKPORT_NUM_STATIC_PACKETS 8
#define ATKPORT_NUM_PORTS 6

#define ATKPORT_OUTSIDE_PACKET 0
#define ATKPORT_INSIDE_PACKET 1

#define ATK_END_ROUTE_DELIMITER 255
#define ATK_ROUTE_POINTER 254
#define ATK_ROUTE_FLOOD 253

typedef struct{
	uartport_t *uart;
	
	uint8_t portnum; // which port are we
	
	uint32_t packet_num;
	uint32_t packet_position;
	uint32_t packets_ready;
	uint32_t packet_state;
	uint8_t packets[ATKPORT_NUM_STATIC_PACKETS][256]; // packets for handling by app
}atkport_t;

void atkport_init(atkport_t *atkp, uint8_t portnum, uartport_t *uart);

void atkport_reset(atkport_t *atkp);

void atkport_scan(atkport_t *atkp, uint32_t maxpackets);

void atkpacket_shift_pointer(uint8_t *packet, uint8_t portnum);

void atkport_send_packet(uint8_t *address, uint8_t address_length, uint8_t *payload, uint8_t payloadlength);

#endif /* atkPORT_H_ */