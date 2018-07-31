/*
* apahandler.c
*
* Created: 3/12/2018 11:55:30 AM
*  Author: Jake
*/

#include "hardware.h"
#include "apahandler.h"

void apa_handle_packet(uint8_t *packet, uint8_t length){
	// dirty debug reply
	//apa_return_packet(packet, length);
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
						// see the packet, make sure you're not doing something else with this light
						pin_toggle(&stlErr);
						//apa_return_packet(packet, length);
						// test reply-with
						uint8_t reply[4] = {127,12,24,48};
						apa_reply_packet(packet, length, reply, 4);
						i ++;
						break;
				
					case DELIM_KEY_RESET:
						WDT->CONFIG.bit.PER = 0;
						WDT->CTRLA.bit.ENABLE = 1;
						// chip will reset in 8ms (uses WDT on clock of 1kHz, min timout of 8 cycles)
						break;
						
					case DELIM_KEY_TRAPEZOID:
						if(i + 21 > length){ // confirm: not i + 12 >= ?
							i ++; // avoid hangup
							pin_clear(&stlTicker);
						} else {
							pin_toggle(&stlErr);
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
							i += 21; // ? not 12 ?
							stepper_new_block(packet, &stepper, steps, entryspeed, accel, accellength, deccellength);
						}
						break;
				
					default:
						// probably an error
						i ++;
						break;
					} // end interior switch
					break;
			
			default :
			// throw err
			break;
		} // end y/n switch
	}
}

// HERE return with address header, or from old packet with new data

void apa_reply_packet(uint8_t *opPacket, uint8_t opLength, uint8_t *replyData, uint8_t replyLength){
	// find address chunk in opPacket
	int i = 2;
	int stop = 0;
	while(i < opLength){
		if(opPacket[i] == APA_END_ROUTE_DELIMITER){
			stop = i;
			break;
		}
		i ++;
	}
	
	uint8_t ackLength = stop + replyLength + 1;
	uint8_t ackPack[ackLength];
	ackPack[0] = ackLength;
	
	if(stop){
		// reverse the address header
		for(int a = stop - 1, b = 1; a >= 1; a--, b++){
			ackPack[b] = opPacket[a];
		}
		// and append the end block
		ackPack[stop] = APA_END_ROUTE_DELIMITER;
		// now fill with provided reply data
		for(int u = 0; u <= replyLength; u ++){
			ackPack[u + stop + 1] = replyData[u];
		}
		// checking the port exists, send it out
		if(ackPack[1] >= NUM_UPS){
			ackPack[1] = NUM_UPS - 1;
		}
		uart_sendchars_buffered(ups[ackPack[1]], ackPack, ackLength);
	}
	
}

void apa_return_packet(uint8_t *packet, uint8_t length){
	//uart_sendchar_buffered(ups[1], 120);
	//uart_sendchars_buffered(ups[1], packet, length);
	uint8_t ackPack[length];
	ackPack[0] = length;
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
	if(stop){
		// reverse the address header
		for(int a = stop - 1, b = 1; a >= 1; a--, b++){
			ackPack[b] = packet[a];
		}
		// fill the rest with same packet data
		ackPack[stop] = APA_END_ROUTE_DELIMITER;
		for(int u = stop; u < length; u ++){
			ackPack[u] = packet[u];
		}
		// checking the packet exists, send it out
		if(ackPack[1] >= NUM_UPS){
			ackPack[1] = NUM_UPS - 1;
		}
		uart_sendchars_buffered(ups[ackPack[1]], ackPack, length);
		// NOW:
		// looking for justice: why no return packet on double length hop?
		// debug with 2nd ftdi
		//uart_sendchar_buffered(ups[1], 121);
		//uart_sendchars_buffered(ups[1], ackpack, length);
	}
}