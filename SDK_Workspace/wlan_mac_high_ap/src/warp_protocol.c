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

void warp_protocol_initialize(void(*management_transmit_callback)(), void(*data_transmit_callback)(), u8* eth_send_dst) {
	warp_protocol_set_management_transmit_callback(management_transmit_callback);
	warp_protocol_set_data_transmit_callback(data_transmit_callback);
	fragment_receiver_initialize();
	fragment_sender_initialize(eth_send_dst);
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
	ethernet_header* ether_header = (ethernet_header*) packet;
	u8* warp_header = packet + sizeof(ethernet_header);

	if (ether_header->type == WARP_ETHERNET_TYPE) {
		switch (warp_header[TYPE_INDEX]) {
		case TYPE_TRANSMIT:
#ifdef WARP_PROTOCOL_DEBUG
			xil_printf("Transmit\n");
#endif
			;
			memcpy(transmit_info.src_mac, ether_header->address_source, 6);
			memcpy(transmit_info .dst_mac, ether_header->address_destination, 6);
			transmit_info.type = ether_header->type;

			void* mpdu_start_ptr = (void*)((tx_packet_buffer*)(tx_queue->buf_ptr))->frame;
			tx_length -= HEADER_OFFSET + TRANSMIT_HEADER_LENGTH;
			memmove(mpdu_start_ptr, warp_header + HEADER_OFFSET + TRANSMIT_HEADER_LENGTH, tx_length);

			warp_protocol_management_transmit_callback(checkout, tx_length, &transmit_info);
			return 0;
		break;
		case TYPE_CONTROL:
			switch (warp_header[SUBTYPE_INDEX]) {
			case SUBTYPE_TRANSMISSION_CONTROL:
#ifdef WARP_PROTOCOL_DEBUG
				xil_printf("Transmission control\n");
				xil_printf("Transmission subtype supported yet!\n");
#endif
				read_transmission_control_header(warp_header, &tx_length);
			break;
			case SUBTYPE_MAC_ADDRESS_CONTROL:
#ifdef WARP_PROTOCOL_DEBUG
				xil_printf("MAC address control\n");
#endif
				read_mac_control_header(warp_header, &tx_length);
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
	} else {
		memcpy(transmit_info.src_mac, ether_header->address_source, 6);
		memcpy(transmit_info.dst_mac, ether_header->address_destination, 6);
		transmit_info.type = ether_header->type;

		tx_length = wlan_eth_encap((void*) ((tx_packet_buffer*) (tx_queue->buf_ptr))->frame, ether_header->address_destination, ether_header->address_source, packet, tx_length + sizeof(ethernet_header));
		warp_protocol_management_transmit_callback(checkout, tx_length, &transmit_info);
		return 0;
	}

	queue_checkin(checkout);
	return 0; //Success
}
