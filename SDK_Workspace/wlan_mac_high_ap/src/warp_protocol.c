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
#include "wlan_mac_eth_util.h"
#include "wlan_mac_high.h"

#include "transmit_element.h"
#include "mac_address_control.h"
#include "transmission_control.h"

#define WARP_PROTOCOL_ENABLED
//#define WARP_PROTOCOL_DEBUG

function_ptr_t warp_protocol_management_transmit_callback;
function_ptr_t warp_protocol_data_transmit_callback;
function_ptr_t warp_protocol_transmit_callback;
transmit_element transmit_info;

void warp_protocol_set_management_transmit_callback(void(*callback)()) {
	warp_protocol_management_transmit_callback = (function_ptr_t)callback;
}

void warp_protocol_set_data_transmit_callback(void(*callback)()) {
	warp_protocol_data_transmit_callback = (function_ptr_t)callback;
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

u8 read_management_transmit_header(u8* packet, u16* length) {
	interpret_management_transmit_element(packet + HEADER_OFFSET, &transmit_info);
	*length = HEADER_OFFSET + TRANSMIT_MANAGEMENT_HEADER_LENGTH + transmit_info.length;

	return TRANSMIT_MANAGEMENT_HEADER_LENGTH;
}

u8 read_data_transmit_header(u8* packet, u16* length) {
	interpret_data_transmit_element(packet + HEADER_OFFSET, &transmit_info);
	*length = HEADER_OFFSET + TRANSMIT_DATA_HEADER_LENGTH + transmit_info.length;

	return TRANSMIT_DATA_HEADER_LENGTH;
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

void print_mac(u8* mac) {
	u8 i = 0;
	for (i = 0; i < 6; i++) {
		xil_printf("%02x-", mac[i]);
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

	clear_transmit_element(&transmit_info);
	u16 shift_amount = 0;

	warp_protocol_transmit_callback = warp_protocol_management_transmit_callback;

	switch (packet[TYPE_INDEX]) {
	case TYPE_TRANSMIT:
#ifdef WARP_PROTOCOL_DEBUG
		xil_printf("Transmit\n");
#endif
		if (packet[SUBTYPE_INDEX] == SUBTYPE_MANAGEMENT_TRANSMIT) {
			shift_amount = read_management_transmit_header(packet, &tx_length);
			warp_protocol_transmit_callback = warp_protocol_management_transmit_callback;

			tx_length = tx_length - (HEADER_OFFSET + shift_amount);
			memmove((void*) ((tx_packet_buffer*)(tx_queue->buf_ptr))->frame, (void*) (packet + HEADER_OFFSET + shift_amount), tx_length);
			warp_protocol_management_transmit_callback(checkout, tx_queue, tx_length, &transmit_info);

			return 0;
		} else if (packet[SUBTYPE_INDEX] == SUBTYPE_DATA_TRANSMIT) {
			shift_amount = read_data_transmit_header(packet, &tx_length);
			warp_protocol_transmit_callback = warp_protocol_data_transmit_callback;

			tx_length = tx_length - (HEADER_OFFSET + shift_amount);

			u32 new_tx_length = wlan_eth_encap(((tx_packet_buffer*)(tx_queue->buf_ptr))->frame, transmit_info.dst_mac, transmit_info.src_mac, packet + HEADER_OFFSET + shift_amount, tx_length);

			if (new_tx_length > 0) {
				memmove((void*) (packet), (void*) (packet + HEADER_OFFSET + shift_amount + 14), tx_length - 14);

				//print_packett(packet, tx_length - 14);

				warp_protocol_data_transmit_callback(checkout, tx_queue, new_tx_length, &transmit_info);
				return 0;
			} else {
				xil_printf("Cannot insert data header...\n");
			}
		}
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

	queue_checkin(checkout);
#else
	memmove((void*) ((tx_packet_buffer*)(tx_queue->buf_ptr))->frame, (void*) (packet), tx_length);
	clear_transmit_element(&transmit_info);
	warp_protocol_management_transmit_callback(checkout, tx_queue, tx_length, &transmit_info);
#endif
	return 0; //Success
}
