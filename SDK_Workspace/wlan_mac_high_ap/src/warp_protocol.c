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
#include "fragment_sender.h"

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

void warp_protocol_set_management_transmit_callback(void(*callback)()) {
	warp_protocol_management_transmit_callback = (function_ptr_t)callback;
}

void warp_protocol_set_data_transmit_callback(void(*callback)()) {
	warp_protocol_data_transmit_callback = (function_ptr_t)callback;
}

void warp_protocol_initialize(void(*management_transmit_callback)(), void(*data_transmit_callback)(), void(*eth_send_callback)()) {
	warp_protocol_set_management_transmit_callback(management_transmit_callback);
	warp_protocol_set_data_transmit_callback(data_transmit_callback);
	fragment_receiver_initialize();
	fragment_sender_initialize(eth_send_callback);
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
	xil_printf("packet length: %d at %d\n", tx_length, packet);
	while (i < tx_length) {
		xil_printf(" %02x ", (u8)tx_pkt[i]);
		i++;
	}
	xil_printf("\n");
}

int warp_protocol_process(dl_list* checkout, u8* packet, u16 tx_length) {
	packet_bd* tx_queue = (packet_bd*)(checkout->first);

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

		fragment_receive_result* receive_result = (fragment_receive_result*) wlan_mac_high_calloc(sizeof(fragment_receive_result));
		fragment_receive(tx_queue, tx_length - shift_amount, ethernet_discrepancy + shift_amount, receive_result);
		shift_amount += FRAGMENT_INFO_LENGTH;

//		xil_printf("Status is %d\n", receive_result->status);
		if (receive_result->status == RECEIVER_READY_TO_SEND) {
			packet_bd* current = receive_result->packet_address;
			u8* data_buffer = ((tx_packet_buffer*)(current->buf_ptr))->frame;
			tx_length = receive_result->info_address->length;

			//Create queue
			dl_list new_queue;
			queue_checkout(&new_queue, 0);
			dl_node_insertBeginning(&new_queue, (dl_node*)current); //See wlan_mac_queue.h for the (dl_node*) cast

			free_fragment_receive_result(receive_result);
			if (packet[SUBTYPE_INDEX] == SUBTYPE_MANAGEMENT_TRANSMIT) {
				memmove((void*) data_buffer, (void*) (data_buffer + ethernet_discrepancy + shift_amount), tx_length);
				warp_protocol_management_transmit_callback(&new_queue, current, tx_length, &transmit_info);
				return 0;
			} else if (packet[SUBTYPE_INDEX] == SUBTYPE_DATA_TRANSMIT) {
//				xil_printf("Before -----------------------------------------------------\n");
//				print_packett(data_buffer, tx_length + ethernet_discrepancy);
				u32 new_tx_length = wlan_eth_encap(data_buffer, transmit_info.dst_mac, transmit_info.src_mac, data_buffer + ethernet_discrepancy + shift_amount , tx_length);

//				if (transmit_info.dst_mac[5] == 0x8c) {
//					xil_printf("After -----------------------------------------------------\n");
//					print_packett(data_buffer, tx_length + ethernet_discrepancy);
//				}

				if (new_tx_length > 0) {
					memmove((void*) (data_buffer + ethernet_discrepancy), (void*) (data_buffer + ethernet_discrepancy + shift_amount + 14), tx_length - 14);
//					if (transmit_info.dst_mac[5] == 0x8c) {
//						print_packett(data_buffer, new_tx_length);
//						xil_printf("End -----------------------------------------------------\n\n");
//					}

					//Create queue
					dl_list new_queue;
					queue_checkout(&new_queue, 0);
					dl_node_insertBeginning(&new_queue, (dl_node*)current); //See wlan_mac_queue.h for the (dl_node*) cast

					warp_protocol_data_transmit_callback(&new_queue, current, new_tx_length, &transmit_info);
					return 0;
				} else {
					xil_printf("Cannot insert data header...\n");
					queue_checkin_packet_bd(current);
				}
			}

			//print_packett(((tx_packet_buffer*)(tx_queue->buf_ptr))->frame + ethernet_discrepancy + shift_amount, tx_length);
		} else {
			free_fragment_receive_result(receive_result);
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
