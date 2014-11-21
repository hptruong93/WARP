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

static u8 eth_dst[6]		= {0x00, 0x0a, 0xcd, 0x21, 0x0b, 0x64};//{ 0x00, 0x0D, 0xB9, 0x34, 0x17, 0x29 };//The PC Engine ethernet MAC --REMEMBER to sync this with wlan_mac_ap.c
static u8 eth_mac_addr[6] =   { 0x40, 0xd8, 0x5, 0x04, 0x22, 0x84 };

void fragment_sender_initialize(u8* warp_eth_mac_addr) {
	memcpy(eth_mac_addr, warp_eth_mac_addr, 6);
}

int eth_pkt_send(void* data, u16 length, u8* warp_protocol_layer, u8 warp_protocol_layer_length) {
	int status;
	u32 eth_tx_len;
	//u16 ipv4_pkt_len, udp_pkt_len;
	u8* eth_tx_ptr;
	dl_list checkout;
	packet_bd* queue_entry;
	ethernet_header* eth_hdr;
	//ipv4_header* ip_hdr;
	//udp_header* udp_hdr;

	eth_tx_len = sizeof(ethernet_header) + warp_protocol_layer_length + length;
	queue_checkout(&checkout, 1);

	if (checkout.length == 1) {
		queue_entry = (packet_bd*)(checkout.first);
		eth_tx_ptr = (u8*)queue_entry->buf_ptr;//data - sizeof(ethernet_header) - warp_protocol_layer_length;

		//ethernet header
		eth_hdr = (ethernet_header*)eth_tx_ptr;
		memcpy((void*) eth_hdr->address_destination, (void*)&eth_dst[0], 6);
		memcpy((void*) eth_hdr->address_source, (void*)&eth_mac_addr[0], 6);
		eth_hdr->type = 0xae08;//Magic??? It's 0x8ae

		if (warp_protocol_layer_length != 0) {
			//copy warp_header;
			memcpy((void*)(eth_tx_ptr + sizeof(ethernet_header)), (void*) (warp_protocol_layer), warp_protocol_layer_length);
		}

		//copy payload
		memcpy((void*)(eth_tx_ptr + sizeof(ethernet_header) + warp_protocol_layer_length) , (void*) data, length);

		//send and then free memory
		status = wlan_eth_dma_send(eth_tx_ptr, eth_tx_len);
		queue_checkin(&checkout);
		if(status != 0) {xil_printf("Error in wlan_mac_send_eth! Err = %d\n", status); return -1;}
	} else {
		xil_printf("unable to allocate memory for eth");
	}
	return 0;
}
