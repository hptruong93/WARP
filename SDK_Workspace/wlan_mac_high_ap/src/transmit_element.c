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

void interpret_management_transmit_element(u8* packet, transmit_element* transmit_info) {
	transmit_info->power = packet[POWER_INDEX];
	transmit_info->rate = packet[RATE_INDEX];
	transmit_info->channel = packet[CHANNEL_INDEX];
	transmit_info->flag = packet[FLAG_INDEX];

	transmit_info->retry = packet[RETRY_INDEX];
	transmit_info->length = (packet[PAYLOAD_SIZE_MSB_INDEX] << 8) + packet[PAYLOAD_SIZE_LSB_INDEX];
}

void interpret_data_transmit_element(u8* packet, transmit_element* transmit_info) {
	transmit_info->power = packet[POWER_INDEX];
	transmit_info->rate = packet[RATE_INDEX];
	transmit_info->channel = packet[CHANNEL_INDEX];
	transmit_info->flag = packet[FLAG_INDEX];

	transmit_info->retry = packet[RETRY_INDEX];
	transmit_info->length = (packet[PAYLOAD_SIZE_MSB_INDEX] << 8) + packet[PAYLOAD_SIZE_LSB_INDEX];

	memcpy(&(transmit_info->bssid[0]), &(packet[BSSID_INDEX]), 6);
}

void clear_transmit_element(transmit_element* element) {
	element->power = DEFAULT_TRANSMIT_POWER;
	element->rate = DEFAULT_TRANSMIT_RATE;
	element->channel = DEFAULT_TRANSMIT_CHANNEL;
	element->flag = DEFAULT_TRANSMIT_FLAG;

	element->retry = DEFAULT_TRANSMIT_RETRY;
	element->length = 0;
}


u8 verify_transmit_element(transmit_element* transmit_element) {
	if (transmit_element->channel > 11) {
		return 1;
	}

	if (transmit_element->retry > 7) {
		return 1;
	}

	return 0;
}
