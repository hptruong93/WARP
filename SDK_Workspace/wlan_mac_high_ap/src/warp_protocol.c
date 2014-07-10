/*
 * warp_protocol.c
 *
 *  Created on: 2014-07-10
 *      Author: Hoai Phuoc Truong
 */

#include "xil_types.h"
#include "xintc.h"

#include "warp_protocol.h"

#include "wlan_mac_dl_list.h"
#include "wlan_mac_queue.h"
#include "wlan_mac_misc_util.h"

#include "transmit_header.h"
#include "mac_address_control.h"
#include "transmission_control.h"


#define TYPE_INDEX                        0
#define SUBTYPE_INDEX                     1

#define HEADER_OFFSET                     2

#define TYPE_TRANSMIT                     1
#define TYPE_CONTROL                      2

#define SUBTYPE_NO_ACK_TRANSMIT           0
#define SUBTYPE_ACK_TRANSMIT              1

#define SUBTYPE_TRANSMISSION_CONTROL      1
#define SUBTYPE_MAC_ADDRESS_CONTROL       2

void print_mac(u8* address) {
	u8 i;
	for (i = 0; i < 5; i++) {
		xil_printf("%02x:", address[i]);
	}
	xil_printf("%02x", address[i]);
}

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

u8 read_transmit_header(u8* packet, u16 length, u8 ack) {
	u8 retry = *(packet + HEADER_OFFSET + RETRY_INDEX);
	shift_back(packet, length, HEADER_OFFSET + TRANSMIT_HEADER_LENGTH);
	return TRANSMIT_HEADER_LENGTH;
}

u8 read_transmission_control_header(u8* packet, u16 length) {
	shift_back(packet, length, HEADER_OFFSET + TRANSMISSION_CONTROL_LENGTH);
	return TRANSMISSION_CONTROL_LENGTH;
}

u8 read_mac_control_header(u8* packet, u16 length) {
	u8 operation_code = packet[HEADER_OFFSET + OPERATION_CODE_INDEX];
	xil_printf("op code = %d", operation_code);

	u8 mac_addr[6];
	u8 i;
	for (i = 0; i < 6; i++) {
		mac_addr[i] = packet[i + 1];
	}
	print_mac(&mac_addr[0]);
	shift_back(packet, length, HEADER_OFFSET + MAC_ADDRESS_CONTROL_LENGTH);
	return MAC_ADDRESS_CONTROL_LENGTH;
}

int warp_protocol_process(dl_list* checkout, u16 tx_length) {
	packet_bd*	tx_queue;
	tx_queue = (packet_bd*)(checkout->first);
	u8* packet = ((tx_packet_buffer*)(tx_queue->buf_ptr))->frame;

	u8 type = packet[TYPE_INDEX];
	u8 subtype = packet[SUBTYPE_INDEX];

	switch (type) {
	case TYPE_TRANSMIT:
		switch (subtype) {
		case SUBTYPE_NO_ACK_TRANSMIT:
			read_transmit_header(packet, tx_length, 0);
			eth_pkt_transmit(checkout, tx_length);
			break;
		case SUBTYPE_ACK_TRANSMIT:
			read_transmit_header(packet, tx_length, 1);
			break;
		default:
			break;
		}
		break;
	case TYPE_CONTROL:
		switch (subtype) {
		case SUBTYPE_TRANSMISSION_CONTROL:
			read_transmission_control_header(packet, tx_length);
			break;
		case SUBTYPE_MAC_ADDRESS_CONTROL:
			read_mac_control_header(packet, tx_length);
			break;
		default:
			break;
		}
		break;
	default: //Do nothing
		break;
	}

	return 0; //Success
}
