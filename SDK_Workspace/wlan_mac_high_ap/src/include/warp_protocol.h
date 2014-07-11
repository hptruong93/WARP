/*
 * warp_protocol.h
 *
 *  Created on: 2014-07-09
 *      Author: Hoai Phuoc Truong
 */

#ifndef WARP_PROTOCOL_H_
#define WARP_PROTOCOL_H_

#include "xil_types.h"
#include "wlan_mac_queue.h"

int warp_protocol_process(dl_list* checkout, u16 tx_length);
void warp_protocol_set_transmit_callback(void(*callback)());

void setup_transmit_header(u8* buffer, u8 power, u8 rate, u8 channel, u8 flag, u8 retry);
void setup_mac_address_control(u8* buffer, u8 operation, u8* mac_addr);

#endif /* WARP_PROTOCOL_H_ */
