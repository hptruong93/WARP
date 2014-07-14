/*
 * warp_protocol.c
 *
 *  Created on: 2014-07-10
 *      Author: Hoai Phuoc Truong
 */

#include <string.h>
#include "xil_types.h"
#include "xintc.h"
#include "warp_protocol.h"

#include "wlan_mac_dl_list.h"
#include "wlan_mac_queue.h"
#include "wlan_mac_misc_util.h"
#include "wlan_mac_high.h"

#include "transmit_header.h"
#include "mac_address_control.h"
#include "transmission_control.h"


//#define WARP_PROTOCOL_DEBUG

#define TYPE_INDEX                        0
#define SUBTYPE_INDEX                     1

#define HEADER_OFFSET                     2

#define TYPE_TRANSMIT                     1
#define TYPE_CONTROL                      2

#define SUBTYPE_TRANSMISSION_CONTROL      1
#define SUBTYPE_MAC_ADDRESS_CONTROL       2

function_ptr_t warp_protocol_transmit_callback;

void warp_protocol_set_transmit_callback(void(*callback)()) {
	warp_protocol_transmit_callback = (function_ptr_t)callback;
}

#ifdef WARP_PROTOCOL_DEBUG
void print_mac(u8* address) {
	u8 i;
	for (i = 0; i < 5; i++) {
		xil_printf("%02x:", address[i]);
	}
	xil_printf("%02x\n", address[i]);
}
#endif

void shift_back(u8* data, u16 length, signed char amount) {
	if (amount != 0) {
		if (amount > 0) {
			if (length - amount < 0) {
				return;
			}
		} else if (length + amount < 0) {
			return;
		}

		unsigned int i;
		for (i = 0; i < length - amount; i++) {//Shift things by amount bytes
			*(data + i) = *(data + i + amount);
		}
	}
}

u8 read_transmit_header(u8* packet, u16 length) {
	u8 retry = *(packet + HEADER_OFFSET + RETRY_INDEX);
	shift_back(packet, length, HEADER_OFFSET + TRANSMIT_HEADER_LENGTH);
	return retry;
}

u8 read_transmission_control_header(u8* packet, u16 length) {
	shift_back(packet, length, HEADER_OFFSET + TRANSMISSION_CONTROL_LENGTH);
	return TRANSMISSION_CONTROL_LENGTH;
}

u8 read_mac_control_header(u8* packet, u16 length) {
#ifdef WARP_PROTOCOL_DEBUG
	u8 operation_code = packet[HEADER_OFFSET + OPERATION_CODE_INDEX];
	xil_printf("op code = %d\n", operation_code);

	u8 mac_addr[6];
	memcpy((void*) &(mac_addr[0]), packet + HEADER_OFFSET + 1, 6);

	print_mac(&mac_addr[0]);
#endif

	wlan_mac_high_mac_manage_control(packet + HEADER_OFFSET);

	shift_back(packet, length, HEADER_OFFSET + MAC_ADDRESS_CONTROL_LENGTH);
	return MAC_ADDRESS_CONTROL_LENGTH;
}

int warp_protocol_process(dl_list* checkout, u16 tx_length) {
	packet_bd*	tx_queue;
	tx_queue = (packet_bd*)(checkout->first);
	u8* packet = ((tx_packet_buffer*)(tx_queue->buf_ptr))->frame;

	u8 type = packet[TYPE_INDEX];
	u8 subtype = packet[SUBTYPE_INDEX];

#ifdef WARP_PROTOCOL_DEBUG
	//xil_printf("Start reading warp protocol. Type is %d and subtype is %d \n", type, subtype);
#endif

	u8 retry = 0;

	switch (type) {
	case TYPE_TRANSMIT:
#ifdef WARP_PROTOCOL_DEBUG
		//xil_printf("Transmit\n");
#endif
		retry = read_transmit_header(packet, tx_length);
		break;
	case TYPE_CONTROL:
		switch (subtype) {
		case SUBTYPE_TRANSMISSION_CONTROL:
#ifdef WARP_PROTOCOL_DEBUG
			xil_printf("Transmission control\n");
			xil_printf("Transmission subtype supported yet!\n");
#endif
			read_transmission_control_header(packet, tx_length);
			break;
		case SUBTYPE_MAC_ADDRESS_CONTROL:
#ifdef WARP_PROTOCOL_DEBUG
			xil_printf("MAC address control\n");
#endif
			read_mac_control_header(packet, tx_length);
			break;
		default:
#ifdef WARP_PROTOCOL_DEBUG
			xil_printf("Subtype not supported yet.\n");
#endif
			break;
		}
		break;
	default: //Do nothing
#ifdef WARP_PROTOCOL_DEBUG
			xil_printf("Type not supported yet.\n");
#endif
		break;
	}

	warp_protocol_transmit_callback(checkout, tx_queue, tx_length, retry);
	return 0; //Success
}
