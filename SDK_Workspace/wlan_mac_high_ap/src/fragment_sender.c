/*
 * fragment_sender.c
 *
 *  Created on: 2014-09-10
 *      Author: Hoai Phuoc Truong
 */

#include "xil_types.h"
#include "xintc.h"
#include "fragment_sender.h"
#include "warp_protocol.h"

#define MAX_ETHERNET_LENGTH                           1475

function_ptr_t fragment_send_eth_callback;

void set_eth_pkt_send_callback(void(*callback)()) {
	fragment_send_eth_callback = (function_ptr_t) callback;
}

void fragment_sender_initialize(void(*callback)()) {
	set_eth_pkt_send_callback(callback);
}

u8 generate_fragment_id() {
	static u8 fragment_id_count = 0;
	fragment_id_count = (fragment_id_count + 1) % 255;
	if (fragment_id_count == 255) {
		fragment_id_count = 0;
	}
	return fragment_id_count;
}

//Type has to be TYPE_TRANSMIT
void fragmentational_send(u8 subtype, u8* packet, u16 length) {
	static u8 warp_header[] = { 1, 0, 0x40, 0xD8, 0x55, 0x04, 0x22, 0x84, 0, 0, 0, 0, 99, 1, 1, 0, 0 };

	u8 fragment_id = generate_fragment_id();
	warp_header[SUBTYPE_INDEX] = subtype;
	warp_header[FRAGMENT_INFO_INDEX + FRAGMENT_ID_INDEX] = fragment_id;

	if (length + sizeof(warp_header) <= MAX_ETHERNET_LENGTH) {
		//Send in one shot
		warp_header[TRANSMIT_HEADER_INDEX + TRANSMIT_PAYLOAD_SIZE_MSB_INDEX] = ((length + FRAGMENT_INFO_LENGTH) >> 8) & 0xff;
		warp_header[TRANSMIT_HEADER_INDEX + TRANSMIT_PAYLOAD_SIZE_LSB_INDEX] = ((length + FRAGMENT_INFO_LENGTH)) & 0xff;

		warp_header[FRAGMENT_INFO_INDEX + FRAGMENT_NUMBER_INDEX] = 1;
		warp_header[FRAGMENT_INFO_INDEX + FRAGMENT_TOTAL_NUMBER_INDEX] = 1;

		fragment_send_eth_callback((void*) packet, length, &(warp_header[0]), sizeof(warp_header));
	} else {
		xil_printf("exceed %d %d %d\n", length, sizeof(warp_header), MAX_ETHERNET_LENGTH - sizeof(warp_header));
		xil_printf("ID is %d\n", fragment_id);
		//Fragment the packet
		u16 fragment_max_length = MAX_ETHERNET_LENGTH - sizeof(warp_header);

		u8 total_number_fragment = (length % fragment_max_length == 0) ? (length / fragment_max_length) : (length / fragment_max_length + 1);
		warp_header[FRAGMENT_INFO_INDEX + FRAGMENT_TOTAL_NUMBER_INDEX] = total_number_fragment;

		u8 fragment_count = 0;
		for (; fragment_count < total_number_fragment; fragment_count++) {
			//Resolve new fragment number
			warp_header[FRAGMENT_INFO_INDEX + FRAGMENT_NUMBER_INDEX] = fragment_count;

			//Resolve new offset
			u16 byte_offset = fragment_count * fragment_max_length;
			warp_header[FRAGMENT_INFO_INDEX + FRAGMENT_BYTE_OFFSET_MSB_INDEX] = (byte_offset >> 8) & 0xff;
			warp_header[FRAGMENT_INFO_INDEX + FRAGMENT_BYTE_OFFSET_LSB_INDEX] = byte_offset & 0xff;

			//Resolve new length
			u16 fragment_length = 1;
			if (byte_offset + fragment_max_length <= length) {
				fragment_length = fragment_max_length;
			} else {
				fragment_length = length - byte_offset;
			}

			fragment_length += FRAGMENT_INFO_LENGTH;
			xil_printf("Offset is %d. Length is %d\n", byte_offset, fragment_length);
			warp_header[TRANSMIT_HEADER_INDEX + TRANSMIT_PAYLOAD_SIZE_MSB_INDEX] = (fragment_length >> 8) & 0xff;
			warp_header[TRANSMIT_HEADER_INDEX + TRANSMIT_PAYLOAD_SIZE_LSB_INDEX] = fragment_length & 0xff;

			//Send
			fragment_send_eth_callback((void*) (packet + byte_offset), fragment_length, &(warp_header[0]), sizeof(warp_header));
		}
	}
}
