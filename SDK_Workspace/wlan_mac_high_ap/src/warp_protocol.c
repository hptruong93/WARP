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

#define WARP_PROTOCOL_ENABLED
//#define WARP_PROTOCOL_DEBUG

#define TYPE_INDEX                        0
#define SUBTYPE_INDEX                     1

#define HEADER_OFFSET                     2

#define TYPE_TRANSMIT                     1
#define TYPE_CONTROL                      2

#define SUBTYPE_TRANSMISSION_CONTROL      1
#define SUBTYPE_MAC_ADDRESS_CONTROL       2

function_ptr_t warp_protocol_transmit_callback;
u8 retry;

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

u8 read_transmit_header(u8* packet, u16* length) {
	retry = packet[HEADER_OFFSET + RETRY_INDEX];
	*length = HEADER_OFFSET + TRANSMIT_HEADER_LENGTH + packet[HEADER_OFFSET + PAYLOAD_SIZE_INDEX];
#ifdef WARP_PROTOCOL_DEBUG
	xil_printf("Retry %d\n", retry);
#endif

	return TRANSMIT_HEADER_LENGTH;
}

u8 read_transmission_control_header(u8* packet, u16* length) {
	return TRANSMISSION_CONTROL_LENGTH;
}

u8 read_mac_control_header(u8* packet, u16* length) {
#ifdef WARP_PROTOCOL_DEBUG
	u8 operation_code = packet[HEADER_OFFSET + OPERATION_CODE_INDEX];
	xil_printf("op code = %d\n", operation_code);

	u8 mac_addr[6];
	memcpy((void*) &(mac_addr[0]), packet + HEADER_OFFSET + 1, 6);

	print_mac(&mac_addr[0]);
#endif

	wlan_mac_high_mac_manage_control(packet + HEADER_OFFSET);
	return MAC_ADDRESS_CONTROL_LENGTH;
}

void print_packett(void* packet, u16 tx_length) {
	u16 i = 0;
	u8* tx_pkt = packet;
	xil_printf("packet length: %d\n", tx_length);
	while (i < tx_length) {
		xil_printf(" %02x ", (u8)tx_pkt[i]);
		i++;
	}
	xil_printf("\n");
}

int warp_protocol_process(dl_list* checkout, u8* packet, u16 tx_length) {
	packet_bd*	tx_queue;
	tx_queue = (packet_bd*)(checkout->first);

#ifdef WARP_PROTOCOL_ENABLED
#ifdef WARP_PROTOCOL_DEBUG
	xil_printf("Start reading warp protocol. Type is %d and subtype is %d \n", type, subtype);
#endif

	retry = 0;
	u16 shift_amount = 0;

	switch (packet[TYPE_INDEX]) {
	case TYPE_TRANSMIT:
#ifdef WARP_PROTOCOL_DEBUG
		xil_printf("Transmit\n");
#endif
		shift_amount = read_transmit_header(packet, &tx_length);
		break;
	case TYPE_CONTROL:
		switch (packet[SUBTYPE_INDEX]) {
		case SUBTYPE_TRANSMISSION_CONTROL:
#ifdef WARP_PROTOCOL_DEBUG
			xil_printf("Transmission control\n");
			xil_printf("Transmission subtype supported yet!\n");
#endif
			shift_amount = read_transmission_control_header(packet, &tx_length);
			break;
		case SUBTYPE_MAC_ADDRESS_CONTROL:
#ifdef WARP_PROTOCOL_DEBUG
			xil_printf("MAC address control\n");
#endif
			shift_amount = read_mac_control_header(packet, &tx_length);
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

	tx_length = tx_length - (HEADER_OFFSET + shift_amount);
	memmove((void*) ((tx_packet_buffer*)(tx_queue->buf_ptr))->frame, (void*) (packet + HEADER_OFFSET + shift_amount), tx_length);
	warp_protocol_transmit_callback(checkout, tx_queue, tx_length, retry);
#else
	memmove((void*) ((tx_packet_buffer*)(tx_queue->buf_ptr))->frame, (void*) (packet), tx_length);
	warp_protocol_transmit_callback(checkout, tx_queue, tx_length, 0);
#endif
	return 0; //Success
}
