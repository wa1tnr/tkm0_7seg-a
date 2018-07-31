/*
 * apahandler.c
 *
 * Created: 3/12/2018 11:55:30 AM
 *  Author: Jake
 */ 

#include "hardware.h"
#include "apahandler.h"
#include "stepper.h"

void apa_handle_packet(uint8_t *packet, uint8_t length){
	// dirty debug reply
	
	// through packet
	int i = 0;
	int apa_handler_state = APA_HANDLER_OUTSIDE;
	
	while(i < length){ // prep for the messy double switch :|
		switch (apa_handler_state){
			case APA_HANDLER_OUTSIDE:
				if (packet[i] == APA_END_ROUTE_DELIMITER){
					apa_handler_state = APA_HANDLER_INSIDE;
				} else {
					//
				}
				i ++;
				break;
				
			case APA_HANDLER_INSIDE:
				switch (packet[i]){
					
					case DELIM_KEY_TEST:
						apa_return_packet(packet, length);
						pin_toggle(&stlb);
						i ++;
						break;
						
					case DELIM_KEY_STEPS:
					// take steps (int32_t) and speed (uint32_t) and dir (uint8_t) to step
						apa_return_packet(packet, length);
						if(i + 8 > length){
							i ++; // avoid hangup, but give up on this
						} else {
							int32_t steps = (packet[i+1] << 24) | (packet[i+2] << 16) | (packet[i+3] << 8) | packet[i+4]; // msb
							uint32_t speed = (packet[i+5] << 24) | (packet[i+6] << 16) | (packet[i+7] << 8) | packet[i+8];
							stepper_steps(&stepper, steps, speed);
							i += 9; // bring packet ptr to next key (should)
						}
						break;
					
					case DELIM_KEY_BLOCK:
						if(i + 12 > length){ // confirm: not i + 12 >= ?
							i ++; // avoid hangup
						} else {
							// in steps (discrete)
							int32_t steps = (packet[i+1] << 24) | (packet[i+2] << 16) | (packet[i+3] << 8) | packet[i+4];
							// in steps/s
							uint32_t entryspeed = (packet[i+5] << 24) | (packet[i+6] << 16) | (packet[i+7] << 8) | packet[i+8];
							// in steps/min/s
							uint32_t accel = (packet[i+9] << 24) | (packet[i+10] << 16) | (packet[i+11] << 8) | packet[i+12];
							// in steps/min
							uint32_t accellength = (packet[i+13] << 24) | (packet[i+14] << 16) | (packet[i+15] << 8) | packet[i+16];
							// in steps/min
							uint32_t deccellength = (packet[i+17] << 24) | (packet[i+18] << 16) | (packet[i+19] << 8) | packet[i+20];
							// do the business
							i += 13; // ? not 12 ?
							stepper_new_block(packet, i, &stepper, steps, entryspeed, accel, accellength, deccellength);
						}
						break;
																
					default:
						// probably an error
						i ++;
						break;
				} // end interior switch
				break;
				
			default: 
				// throw err
				break;
		} // end y/n switch
	}
}


void apa_return_packet(uint8_t *packet, uint8_t length){
	//uart_sendchar_buffered(ups[1], 120);
	//uart_sendchars_buffered(ups[1], packet, length);
	uint8_t ackpack[length];
	ackpack[0] = length;
	// find route header
	int i = 2;
	int stop = 0;
	while(i < length){
		if(packet[i] == APA_END_ROUTE_DELIMITER){
			stop = i;
			break;
		}
		i ++;
	}
	// do the business
	if(!stop){
		// error if stop == 0
		} else {
		// reverse the address header
		for(int a = stop - 1, b = 1; a >= 1; a--, b++){
			ackpack[b] = packet[a];
		}
		// fill the rest with same packet data
		ackpack[stop] = APA_END_ROUTE_DELIMITER;
		for(int u = stop; u < length; u ++){
			ackpack[u] = packet[u];
		}
		uart_sendchars_buffered(ups[ackpack[1]], ackpack, length);
		// NOW:
		// looking for justice: why no return packet on double length hop?
		// debug with 2nd ftdi
		//uart_sendchar_buffered(ups[1], 121);
		//uart_sendchars_buffered(ups[1], ackpack, length);
	}
}