/*
 * apaport.c
 *
 * Created: 2/23/2018 9:17:48 AM
 *  Author: Jake
 */ 

#include "apaport.h"
#include "hardware.h"

void apaport_build(apaport_t *apap, uint8_t portnum, uartport_t *uart, pin_t *stlr, pin_t *stlb){
	apap->uart = uart;
	apap->portnum = portnum;
	apap->stlr = stlr; // dangerous because stlr is def'nd in hardware.h as well - watch your namespaces
	apap->stlb = stlb;
}

void apaport_reset(apaport_t *apap){
	apap->packet_num = 0;
	apap->packets_ready = 0;
	apap->packet_state = APAPORT_OUTSIDE_PACKET;
	apap->packet_position = 0;
	
	pin_set(apap->stlr);
	pin_set(apap->stlb);
}

void apaport_scan(apaport_t *apap, uint32_t maxpackets){
	// scan through for completely received packets
	while(apap->packets_ready <= maxpackets){
		// check that we have bytes to read out of the buffer
		if(rb_empty(apap->uart->rbrx)){
			pin_set(apap->stlr);
			break;
		}
		// pull bytes out of buffer into the packet structure
		apap->packets[apap->packet_num][apap->packet_position] = rb_get(apap->uart->rbrx);
		apap->packet_position ++;
		// now segment, point to them
		if(apap->packet_position >= apap->packets[apap->packet_num][0]){
			// length is 1st byte, like array[n] not array[n-1]
			// now volley for next pass
			// packet_num is index of head of packet buffer (just an array)
			apap->packet_num = (apap->packet_num + 1) % APAPORT_NUM_STATIC_PACKETS; // inc. and loop
			// packets_ready is the number of ready-state packets in that buffer (array)
			apap->packets_ready ++;
			// the position, in bytes, where we are currently operating.
			// at this point, we have come to the end, so we're resetting counter for the next
			apap->packet_position = 0;
		}
	} 
	// end 1st scan for packets, now we know we have apaport->packet_num packets completely received
	// now we handle those packets	
	while(apap->packets_ready > 0){
		// the particular packet index
		uint32_t p = (apap->packet_num + APAPORT_NUM_STATIC_PACKETS - apap->packets_ready) % APAPORT_NUM_STATIC_PACKETS;
		// first we shift the old pointer out (p[1] is, at the moment, the port the last node tx'd on)
		apapacket_shift_pointer(apap->packets[p], apap->portnum);
		// now p[1] is next port
		// now to handle
		// [p][0] is length of packet
		if(apap->packets[p][1] == APA_ROUTE_POINTER){
			apa_handle_packet(apap->packets[p], apap->packets[p][0]);
		} else if(apap->packets[p][1] == APA_ROUTE_FLOOD){
			// loop through bytes to find pointer and increment
			// now ship it out on all ports
			for(int i = 0; i < APAPORT_NUM_PORTS; i ++){
				if(i == apap->portnum){
					// don't flood back
				} else {
					uart_sendchars_buffered(ups[i], apap->packets[p], apap->packets[p][0]);
				}
			}
		} else {
			// packet is for a particular port,
			if(apap->packets[p][1] > APAPORT_NUM_PORTS){
				// port does not exist, throw error
				// pin_clear(&stlr);
			} else {
				uart_sendchars_buffered(ups[apap->packets[p][1]], apap->packets[p], apap->packets[p][0]);
			}
		}
		// debug reply (at the moment, reply is handled in apa_handle_packet
		// uart_sendchars_buffered(apap->uart, apap->packets[p], apap->packets[p][0]);
		apap->packets_ready --;
	}
}

void apapacket_shift_pointer(uint8_t *packet, uint8_t portnum){
	int i = 2;
	while(i < packet[0]){ // while less than length
		if(packet[i] == APA_END_ROUTE_DELIMITER){
			// put our port in tail
			packet[i-1] = portnum;
			break;
		} else {
			// shift 'em down
			packet[i-1] = packet[i];
		}
		i ++;
	}
}

void apaport_send_packet(uint8_t *address, uint8_t address_length, uint8_t *payload, uint8_t payloadlength){
	// 1st byte is port out
	// not yet implemented, using apa_return_packet ... all of these could be cleaner
}

// UNIT TESTS:
/*
 flood packets
 multiple receptions? handle in app?
 packets no end addr bar delimiter, packets no pointer, general white noise
 packets varying length
 packets wrong length ? very hard to catch w/o hella state ... timeout?
	packets no end addr delimiter?
 packets to ports not existing
 // next: write javascript terminal packet builder for unit tests!
*/