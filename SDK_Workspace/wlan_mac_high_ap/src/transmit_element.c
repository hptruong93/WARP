/*
 * transmit_element.c
 *
 *  Created on: 2014-07-25
 *      Author: Hoai Phuoc Truong
 */

#include <string.h>
#include "xil_types.h"

#include "warp_protocol.h"
#include "transmit_element.h"

void interpret_transmit_element(u8* packet, transmit_element* transmit_info) {
	memcpy(&(transmit_info->bssid[0]), &(packet[TRANSMIT_BSSID_INDEX]), 6);
	transmit_info->flag = packet[TRANSMIT_FLAG_INDEX];
	transmit_info->retry = packet[TRANSMIT_RETRY_INDEX];
	transmit_info->length = (packet[TRANSMIT_PAYLOAD_SIZE_MSB_INDEX] << 8) + packet[TRANSMIT_PAYLOAD_SIZE_LSB_INDEX];
}

void clear_transmit_element(transmit_element* element) {
	element->flag = DEFAULT_TRANSMIT_FLAG;

	element->retry = DEFAULT_TRANSMIT_RETRY;
	element->length = 0;
	memset(element->bssid, 0, 6);
	memset(element->src_mac, 0, 6);
	memset(element->dst_mac, 0, 6);
}


u8 verify_transmit_element(transmit_element* transmit_element) {
	if (transmit_element->retry > 7) {
		return 1;
	}

	return 0;
}
