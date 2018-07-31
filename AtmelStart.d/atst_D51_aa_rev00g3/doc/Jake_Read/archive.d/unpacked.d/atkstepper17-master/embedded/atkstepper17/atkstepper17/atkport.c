/*
 * atkport.c
 *
 * Created: 2/23/2018 9:17:48 AM
 *  Author: Jake
 */ 

#include "atkport.h"
#include "hardware.h"
#include "fastmath.h"

void atkport_init(atkport_t *atkp, uint8_t portnum, uartport_t *uart){
	atkp->uart = uart;
	atkp->portnum = portnum;
	
	atkport_reset(atkp);
}

void atkport_reset(atkport_t *atkp){
	atkp->packet_num = 0;
	atkp->packets_ready = 0;
	atkp->packet_state = ATKPORT_OUTSIDE_PACKET;
	atkp->packet_position = 0;
	
	pin_set(atkp->uart->stlrx);
	pin_set(atkp->uart->stltx);
}

void atkport_scan(atkport_t *atkp, uint32_t maxpackets){
	// scan through for completely received packets
	while(atkp->packets_ready <= maxpackets){
		// check that we have bytes to read out of the buffer
		if(atkp->uart->rbrx->head == atkp->uart->rbrx->tail){ // direct access, rather than through rbempty() shaves 2us
			pin_set(atkp->uart->stlrx);
			break;
		} else {
			// pull bytes out of buffer into the packet structure
			atkp->packets[atkp->packet_num][atkp->packet_position] = rb_get(atkp->uart->rbrx);
			atkp->packet_position ++;
			// now segment, point to them
			if(atkp->packet_position >= atkp->packets[atkp->packet_num][0]){
				// length is 1st byte, like array[n] not array[n-1]
				// now volley for next pass
				// packet_num is index of head of packet buffer (just an array)
				atkp->packet_num = fastModulo((atkp->packet_num + 1), ATKPORT_NUM_STATIC_PACKETS); // inc. and loop
				// packets_ready is the number of ready-state packets in that buffer (array)
				atkp->packets_ready ++;
				// the position, in bytes, where we are currently operating.
				// at this point, we have come to the end, so we're resetting counter for the next
				atkp->packet_position = 0;
			}
		}
	} 
	// end 1st scan for packets, now we know we have atkport->packet_num packets completely received
	// now we handle those packets	
	while(atkp->packets_ready > 0){
		// the particular packet index
		uint32_t p = fastModulo((atkp->packet_num + ATKPORT_NUM_STATIC_PACKETS - atkp->packets_ready), ATKPORT_NUM_STATIC_PACKETS); //(atkp->packet_num + ATKPORT_NUM_STATIC_PACKETS - atkp->packets_ready) % ATKPORT_NUM_STATIC_PACKETS;
		// first we shift the old pointer out (p[1] is, at the moment, the port the last node tx'd on)
		atkpacket_shift_pointer(atkp->packets[p], atkp->portnum);
		// now p[1] is next port
		// now to handle
		// [p][0] is length of packet
		if(atkp->packets[p][1] == ATK_ROUTE_POINTER){
			atk_handle_packet(atkp->packets[p], atkp->packets[p][0]);
		} else if(atkp->packets[p][1] == ATK_ROUTE_FLOOD){
			// loop through bytes to find pointer and increment
			// now ship it out on all ports
			for(int i = 0; i < ATKPORT_NUM_PORTS; i ++){
				if(i == atkp->portnum){
					// don't flood back
				} else {
					uart_sendchars_buffered(ups[i], atkp->packets[p], atkp->packets[p][0]);
				}
			}
		} else {
			// packet is for a particular port,
			if(atkp->packets[p][1] > ATKPORT_NUM_PORTS){
				// port does not exist, throw error
				// pin_clear(&stlerr);
			} else {
				// debug option
				//uart_sendchars_buffered(&upU, atkp->packets[p], atkp->packets[p][0]);
				uart_sendchars_buffered(ups[atkp->packets[p][1]], atkp->packets[p], atkp->packets[p][0]);
			}
		}
		// debug reply (at the moment, reply is handled in atk_handle_packet
		// uart_sendchars_buffered(atkp->uart, atkp->packets[p], atkp->packets[p][0]);
		atkp->packets_ready --;
	}
}

void atkpacket_shift_pointer(uint8_t *packet, uint8_t portnum){
	int i = 2;
	while(i < packet[0]){ // while less than length
		if(packet[i] == ATK_END_ROUTE_DELIMITER){
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

void atkport_send_packet(uint8_t *address, uint8_t address_length, uint8_t *payload, uint8_t payloadlength){
	// 1st byte is port out
	// not yet implemented, using atk_return_packet ... all of these could be cleaner
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