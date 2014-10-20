/*
 * fragment_sender.c
 *
 *  Created on: 2014-09-10
 *      Author: Hoai Phuoc Truong
 */

#include "stdlib.h"
#include "string.h"
#include "xil_types.h"
#include "xintc.h"

#include "wlan_mac_eth_util.h"
#include "wlan_mac_high.h"
#include "fragment_sender.h"
#include "warp_protocol.h"

#define MAX_ETHERNET_LENGTH                           1475

static u8 eth_dst[6]		= { 0x00, 0x0D, 0xB9, 0x34, 0x17, 0x29 };//The PC Engine ethernet MAC
static u8 eth_mac_addr[6] =   { 0x40, 0xd8, 0x5, 0x04, 0x22, 0x84 };

void fragment_sender_initialize(u8* warp_eth_mac_addr) {
	memcpy(eth_mac_addr, warp_eth_mac_addr, 6);
}

u8 generate_fragment_id() {
	static u8 fragment_id_count = 0;
	fragment_id_count = (fragment_id_count + 1) % 255;
	if (fragment_id_count == 255) {
		fragment_id_count = 0;
	}
	return fragment_id_count;
}

int eth_pkt_send(void* data, u16 length, u8* warp_protocol_layer, u8 warp_protocol_layer_length) {
	int status;
	u32 eth_tx_len;
	//u16 ipv4_pkt_len, udp_pkt_len;
	u8* eth_tx_ptr;
//	dl_list checkout;
//	packet_bd* queue_entry;
	ethernet_header* eth_hdr;
	//ipv4_header* ip_hdr;
	//udp_header* udp_hdr;

	eth_tx_len = sizeof(ethernet_header) + warp_protocol_layer_length + length;
//	queue_checkout(&checkout, 1);

//	if (checkout.length == 1) {
//		queue_entry = (packet_bd*)(checkout.first);
		//Assume that the buffer has enough space in front
		//In fact, the received frame will have PHY_RX_PKT_BUF_MPDU_OFFSET bytes in front. This information must have been processed and incoporated into warp header
		//Should warp header + sizeof(ethernet_header) >  PHY_RX_PKT_BUF_MPDU_OFFSET, another approach should be consider so that memory is used safely.
		eth_tx_ptr = data - sizeof(ethernet_header) - warp_protocol_layer_length;//;(u8*)queue_entry->buf_ptr;

		//ethernet header
		eth_hdr = (ethernet_header*)eth_tx_ptr;
		memcpy((void*) eth_hdr->address_destination, (void*)&eth_dst[0], 6);
		memcpy((void*) eth_hdr->address_source, (void*)&eth_mac_addr[0], 6);
		eth_hdr->type = 44552;//Magic??? It's 0x8ae

		//copy warp_header;
		memcpy((void*)(eth_tx_ptr + sizeof(ethernet_header)), (void*) (warp_protocol_layer), warp_protocol_layer_length);
		//copy payload
//		memcpy((void*)(eth_tx_ptr + sizeof(ethernet_header) + warp_protocol_layer_length) , (void*) data, length);

		//send and then free memory
		status = wlan_eth_dma_send(eth_tx_ptr, eth_tx_len);
//		queue_checkin(&checkout);
		if(status != 0) {xil_printf("Error in wlan_mac_send_eth! Err = %d\n", status); return -1;}
//	} else {
//		xil_printf("unable to allocate memory for eth");
//	}
	return 0;
}

//Type has to be TYPE_TRANSMIT
void fragmentational_send(u8 subtype, u8* packet, u16 length) {
//	static u8 warp_header[] = { 1, 0, 0x40, 0xD8, 0x55, 0x04, 0x22, 0x84, 0, 0, 0, 0, 99, 1, 1, 0, 0 };
//
//	u8 fragment_id = generate_fragment_id();
//	warp_header[SUBTYPE_INDEX] = subtype;
//	warp_header[FRAGMENT_INFO_INDEX + FRAGMENT_ID_INDEX] = fragment_id;
//
//	if (length + sizeof(warp_header) <= MAX_ETHERNET_LENGTH) {
//		//Send in one shot
//		warp_header[TRANSMIT_HEADER_INDEX + TRANSMIT_PAYLOAD_SIZE_MSB_INDEX] = ((length + FRAGMENT_INFO_LENGTH) >> 8) & 0xff;
//		warp_header[TRANSMIT_HEADER_INDEX + TRANSMIT_PAYLOAD_SIZE_LSB_INDEX] = ((length + FRAGMENT_INFO_LENGTH)) & 0xff;
//
//		warp_header[FRAGMENT_INFO_INDEX + FRAGMENT_NUMBER_INDEX] = 1;
//		warp_header[FRAGMENT_INFO_INDEX + FRAGMENT_TOTAL_NUMBER_INDEX] = 1;
//
//		warp_header[FRAGMENT_INFO_INDEX + FRAGMENT_BYTE_OFFSET_LSB_INDEX] = 0;
//		warp_header[FRAGMENT_INFO_INDEX + FRAGMENT_BYTE_OFFSET_MSB_INDEX] = 0;
//
//		eth_pkt_send((void*) packet, length, &(warp_header[0]), sizeof(warp_header));
//	} else {
//		xil_printf("exceed %d %d %d\n", length, sizeof(warp_header), MAX_ETHERNET_LENGTH - sizeof(warp_header));
//		xil_printf("ID is %d\n", fragment_id);
//		//Fragment the packet
//		u16 fragment_max_length = MAX_ETHERNET_LENGTH - sizeof(warp_header);
//
//		u8 total_number_fragment = (length % fragment_max_length == 0) ? (length / fragment_max_length) : (length / fragment_max_length + 1);
//		warp_header[FRAGMENT_INFO_INDEX + FRAGMENT_TOTAL_NUMBER_INDEX] = total_number_fragment;
//
//		u8 fragment_count = 0;
//		for (; fragment_count < total_number_fragment; fragment_count++) {
//			//Resolve new fragment number
//			warp_header[FRAGMENT_INFO_INDEX + FRAGMENT_NUMBER_INDEX] = fragment_count;
//
//			//Resolve new offset
//			u16 byte_offset = fragment_count * fragment_max_length;
//			warp_header[FRAGMENT_INFO_INDEX + FRAGMENT_BYTE_OFFSET_MSB_INDEX] = (byte_offset >> 8) & 0xff;
//			warp_header[FRAGMENT_INFO_INDEX + FRAGMENT_BYTE_OFFSET_LSB_INDEX] = byte_offset & 0xff;
//
//			//Resolve new length
//			u16 fragment_length = 1;
//			if (byte_offset + fragment_max_length <= length) {
//				fragment_length = fragment_max_length;
//			} else {
//				fragment_length = length - byte_offset;
//			}
//
//			fragment_length += FRAGMENT_INFO_LENGTH;
//			xil_printf("Offset is %d. Length is %d\n", byte_offset, fragment_length);
//			warp_header[TRANSMIT_HEADER_INDEX + TRANSMIT_PAYLOAD_SIZE_MSB_INDEX] = (fragment_length >> 8) & 0xff;
//			warp_header[TRANSMIT_HEADER_INDEX + TRANSMIT_PAYLOAD_SIZE_LSB_INDEX] = fragment_length & 0xff;
//
//			//Send
//			eth_pkt_send((void*) (packet + byte_offset), fragment_length, &(warp_header[0]), sizeof(warp_header));
//		}
//	}
}
