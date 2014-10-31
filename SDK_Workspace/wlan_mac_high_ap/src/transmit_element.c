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

}

void clear_transmit_element(transmit_element* element) {
	element->type = 0;
	memset(element->bssid, 0, 6);
	memset(element->src_mac, 0, 6);
	memset(element->dst_mac, 0, 6);
}
