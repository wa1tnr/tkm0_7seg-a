/*
* atkhandler.c
*
* Created: 3/12/2018 11:55:30 AM
*  Author: Jake
*/

#include "hardware.h"
#include "atkhandler.h"

int32_t steps;

void atk_handle_packet(uint8_t *packet, uint8_t length){
	// dirty debug reply
	//atk_return_packet(packet, length);
	// through packet
	int i = 0;
	int atk_handler_state = ATK_HANDLER_OUTSIDE;
	uint8_t testReply[4] = {127, 12, 24, 48};
		
	while(i < length){ // prep for the messy double switch :|
		switch (atk_handler_state){
			case ATK_HANDLER_OUTSIDE:
				if (packet[i] == ATK_END_ROUTE_DELIMITER){
					atk_handler_state = ATK_HANDLER_INSIDE;
				} 
				i ++;
				break;
			
			case ATK_HANDLER_INSIDE:
				switch (packet[i]){
					case DELIM_KEY_TEST:
						pin_toggle(&stlerr);
						// see the packet, make sure you're not doing something else with this light
						atk_reply_packet(packet, testReply, 4);
						i ++;
						break;
					
					case DELIM_KEY_RESET:
						CCP = CCP_IOREG_gc;
						RST.CTRL = RST_SWRST_bm;
						i ++;
						break;
					
					case DELIM_KEY_TRAPEZOID:
						//pin_toggle(&stlerr);
						if(i + 12 > length){
							i ++;
						} else {
							// to move or not to move
							uint8_t is_wait = 0;
							// in steps (discrete)
							steps = ((int32_t)packet[i+1] << 24) | ((int32_t)packet[i+2] << 16) | ((int32_t)packet[i+3] << 8) | (int32_t)packet[i+4];
							// in steps/s
							uint32_t entryspeed = ((int32_t)packet[i+5] << 24) | ((int32_t)packet[i+6] << 16) | ((int32_t)packet[i+7] << 8) | (int32_t)packet[i+8];
							// in steps/min/s
							uint32_t accel = ((int32_t)packet[i+9] << 24) | ((int32_t)packet[i+10] << 16) | ((int32_t)packet[i+11] << 8) | (int32_t)packet[i+12];
							// in steps/min
							uint32_t accellength = ((int32_t)packet[i+13] << 24) | ((int32_t)packet[i+14] << 16) | ((int32_t)packet[i+15] << 8) | (int32_t)packet[i+16];
							// in steps/min
							uint32_t deccellength = ((int32_t)packet[i+17] << 24) | ((int32_t)packet[i+18] << 16) | (int32_t)(packet[i+19] << 8) | (int32_t)packet[i+20];
							// do the business
							i += 21;
							stepper_new_block(packet, &stepper, is_wait, steps, entryspeed, accel, accellength, deccellength);
						}
						break;
					
					case DELIM_KEY_WAIT:
						if(i + 8 > length){
							i ++;
						} else {
							// to move or not to move
							uint8_t is_wait = 1;
							// in steps (discrete)
							steps = ((int32_t)packet[i+1] << 24) | ((int32_t)packet[i+2] << 16) | ((int32_t)packet[i+3] << 8) | (int32_t)packet[i+4];
							// in steps/s
							uint32_t entryspeed = ((int32_t)packet[i+5] << 24) | ((int32_t)packet[i+6] << 16) | ((int32_t)packet[i+7] << 8) | (int32_t)packet[i+8];
							// in steps/min/s
							uint32_t accel = ((int32_t)packet[i+9] << 24) | ((int32_t)packet[i+10] << 16) | ((int32_t)packet[i+11] << 8) | (int32_t)packet[i+12];
							// in steps/min
							uint32_t accellength = ((int32_t)packet[i+13] << 24) | ((int32_t)packet[i+14] << 16) | ((int32_t)packet[i+15] << 8) | (int32_t)packet[i+16];
							// in steps/min
							uint32_t deccellength = ((int32_t)packet[i+17] << 24) | ((int32_t)packet[i+18] << 16) | (int32_t)(packet[i+19] << 8) | (int32_t)packet[i+20];
							// do the business
							i += 21;
							stepper_new_block(packet, &stepper, is_wait, steps, entryspeed, accel, accellength, deccellength);
						}
											
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

uint8_t ackPack[256];

void atk_reply_packet(uint8_t *opPacket, uint8_t *replyData, uint8_t replyLength){
	// find address chunk in opPacket
	int i = 2;
	int stop = 0;
	while(i < opPacket[0]){
		if(opPacket[i] == ATK_END_ROUTE_DELIMITER){
			stop = i;
			break;
		}
		i ++;
	}
	
	uint8_t ackLength = stop + replyLength + 1;
	ackPack[0] = ackLength;
	
	if(stop){
		// reverse the address header
		for(int a = stop - 1, b = 1; a >= 1; a--, b++){
			ackPack[b] = opPacket[a];
		}
		// and append the end block
		ackPack[stop] = ATK_END_ROUTE_DELIMITER;
		// now fill with provided reply data
		for(int u = 0; u <= replyLength; u ++){
			ackPack[u + stop + 1] = replyData[u];
		}
		// checking the port exists, send it out
		if(ackPack[1] >= SYSTEM_NUM_UPS){
			ackPack[1] = SYSTEM_NUM_UPS - 1;
		}
		uart_sendchars_buffered(ups[ackPack[1]], ackPack, ackPack[0]);//ups[ackPack[1]], ackPack, ackLength);
	}
}

void atk_return_packet(uint8_t *packet, uint8_t length){
	//uart_sendchar_buffered(ups[1], 120);
	//uart_sendchars_buffered(ups[1], packet, length);
	// using this placeholder 
	ackPack[0] = length;
	// find route header
	int i = 2;
	int stop = 0;
	while(i < length){
		if(packet[i] == ATK_END_ROUTE_DELIMITER){
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
		ackPack[stop] = ATK_END_ROUTE_DELIMITER;
		for(int u = stop; u < length; u ++){
			ackPack[u] = packet[u];
		}
		// checking the packet exists, send it out
		if(ackPack[1] >= SYSTEM_NUM_UPS){
			ackPack[1] = SYSTEM_NUM_UPS - 1;
		}
		uart_sendchars_buffered(ups[ackPack[1]], ackPack, ackPack[0]);
		// NOW:
		// looking for justice: why no return packet on double length hop?
		// debug with 2nd ftdi
		//uart_sendchar_buffered(ups[1], 121);
		//uart_sendchars_buffered(ups[1], ackpack, length);
	}
}