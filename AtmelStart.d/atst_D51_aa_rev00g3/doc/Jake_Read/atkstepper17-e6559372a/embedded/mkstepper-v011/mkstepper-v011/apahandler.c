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
	apa_return_packet(packet, length);
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
						pin_toggle(&stlb);
						i ++;
						break;
						
					case DELIM_KEY_POSITION_STEPS:
					// should set 0 accel 
						if(i + 4 > length){
							i ++; // avoid hangup
						} else {
							int32_t steps = (packet[i+1] << 24) | (packet[i+2] << 16) | (packet[i+3] << 8) | packet[i+4]; // msb
							stepper_steps(&stepper, steps);
							i += 5; // bring packet ptr to next key (should)
						}
						break;
					
					case DELIM_KEY_SPEED_STEPS:
					// should set 0 accel
						if(i + 4 > length){
							i ++; // avoiding hangup
						} else {
							uint32_t speed = (packet[i+1] << 24) | (packet[i+2] << 16) | (packet[i+3] << 8) | packet[i+4]; // msb
							stepper_speed(&stepper, speed);
							i += 5; // bring packet ptr to next key (should)
						}
						break;
					
					case DELIM_KEY_SEGMENT:
						if(i + 12 > length){ // confirm: not i + 12 >= ?
							i ++; // avoid hangup
						} else {
							// in steps / s
							uint32_t startspeed = (packet[i+1] << 24) | (packet[i+2] << 16) | (packet[i+3] << 8) | packet[i+4];
							// in steps / s^2
							int32_t accel = (packet[i+5] << 24) | (packet[i+6] << 16) | (packet[i+7] << 8) | packet[i+8];
							// in steps
							int32_t steps = (packet[i+9] << 24) | (packet[i+10] << 16) | (packet[i+11] << 8) | packet[i+12];
							stepper_segment(&stepper, startspeed, accel, steps);
							i += 13; // ? not 12 ?
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