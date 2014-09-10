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
#include "fragment_receiver.h"

#include "wlan_mac_dl_list.h"
#include "wlan_mac_queue.h"
#include "wlan_mac_misc_util.h"
#include "wlan_mac_eth_util.h"
#include "wlan_mac_high.h"

#include "transmit_element.h"
#include "mac_address_control.h"
#include "transmission_control.h"

//#define WARP_PROTOCOL_DEBUG

function_ptr_t warp_protocol_management_transmit_callback;
function_ptr_t warp_protocol_data_transmit_callback;
transmit_element transmit_info;

void warp_protocol_initialize(void* management_transmit_callback, void* data_transmit_callback) {
	warp_protocol_set_management_transmit_callback(management_transmit_callback);
	warp_protocol_set_data_transmit_callback(data_transmit_callback);
	fragment_receiver_initialize();
}

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

u8 read_transmit_header(u8* packet, u16* length) {
	interpret_transmit_element(packet + HEADER_OFFSET, &transmit_info);
	*length = HEADER_OFFSET + TRANSMIT_HEADER_LENGTH + transmit_info.length;
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
	packet_bd*	tx_queue = (packet_bd*)(checkout->first);

#ifdef WARP_PROTOCOL_DEBUG
	xil_printf("Start reading warp protocol. Type is %d and subtype is %d \n", type, subtype);
#endif

	clear_transmit_element(&transmit_info);
	u16 shift_amount = 0;

	switch (packet[TYPE_INDEX]) {
	case TYPE_TRANSMIT:
#ifdef WARP_PROTOCOL_DEBUG
		xil_printf("Transmit\n");
#endif

		;

		u32 ethernet_discrepancy = packet - ((tx_packet_buffer*)(tx_queue->buf_ptr))->frame;
		shift_amount = HEADER_OFFSET + read_transmit_header(packet, &tx_length);

		fragment_receive_result* receive_result = fragment_receive(checkout, tx_length - shift_amount, ethernet_discrepancy + shift_amount);
		shift_amount += FRAGMENT_INFO_LENGTH;
		if (receive_result->status == RECEIVER_READY_TO_SEND) {
			dl_list* current = receive_result->packet_address;


			if (current != checkout) {
				//Check in the dl_list* checkout now since the packet has been assembled in dl_list* current.
				//Otherwise, packet is assembled into the current dl_list* checkout and will be checked in of once the packet is sent.
				queue_checkin(checkout);
			}
			tx_queue = (packet_bd*)(current->first);

			tx_length = receive_result->info_address->length;
			free_fragment_receive_result(receive_result);
			if (packet[SUBTYPE_INDEX] == SUBTYPE_MANAGEMENT_TRANSMIT) {
				memmove((void*) ((tx_packet_buffer*)(tx_queue->buf_ptr))->frame, (void*) (packet + shift_amount), tx_length);
				warp_protocol_management_transmit_callback(current, tx_queue, tx_length, &transmit_info);

				return 0;
			}
//			else if (packet[SUBTYPE_INDEX] == SUBTYPE_DATA_TRANSMIT) {
//				u32 new_tx_length = wlan_eth_encap(((tx_packet_buffer*)(tx_queue->buf_ptr))->frame, transmit_info.dst_mac, transmit_info.src_mac, packet + shift_amount , tx_length);
//
//				if (new_tx_length > 0) {
//					memmove((void*) (packet), (void*) (packet + HEADER_OFFSET + shift_amount + 14), tx_length - 14);
//
//					//print_packett(packet, tx_length - 14);
//
//					warp_protocol_data_transmit_callback(checkout, tx_queue, new_tx_length, &transmit_info);
//					return 0;
//				} else {
//					xil_printf("Cannot insert data header...\n");
//				}
//			}

			//print_packett(((tx_packet_buffer*)(tx_queue->buf_ptr))->frame + ethernet_discrepancy + shift_amount, tx_length);
		} else {
			if (receive_result->packet_address != NULL) {
				queue_checkin(receive_result->packet_address);
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

	//queue_checkin(checkout);
	return 0; //Success
}
