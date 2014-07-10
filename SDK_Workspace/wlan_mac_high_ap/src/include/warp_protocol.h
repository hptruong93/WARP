/*
 * warp_protocol.h
 *
 *  Created on: 2014-07-09
 *      Author: Hoai Phuoc Truong
 */

#ifndef WARP_PROTOCOL_H_
#define WARP_PROTOCOL_H_

#include "xil_types.h"
#include "wlan_mac_dl_list.h"

int warp_protocol_process(dl_list* checkout, u16 tx_length);

#endif /* WARP_PROTOCOL_H_ */
