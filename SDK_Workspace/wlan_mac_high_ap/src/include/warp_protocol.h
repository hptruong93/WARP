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
#include "wlan_mac_misc_util.h"

#define TYPE_INDEX                        0
#define SUBTYPE_INDEX                     1

#define HEADER_OFFSET                     2

#define TYPE_TRANSMIT                     1
#define TYPE_CONTROL                      2

#define SUBTYPE_MANAGEMENT_TRANSMIT       0
#define SUBTYPE_DATA_TRANSMIT			  1

//For fragment info
#define FRAGMENT_INFO_LENGTH                          5
#define FRAGMENT_ID_INDEX                             0
#define FRAGMENT_NUMBER_INDEX                         1
#define FRAGMENT_TOTAL_NUMBER_INDEX                   2
#define FRAGMENT_BYTE_OFFSET_MSB                      3
#define FRAGMENT_BYTE_OFFSET_LSB                      4


#define SUBTYPE_TRANSMISSION_CONTROL      1
#define SUBTYPE_MAC_ADDRESS_CONTROL       2


typedef struct {
	u8 id;
	u8 fragment_number;
	u8 total_number_fragment;
	u16 byte_offset;
	u32 length;
} fragment_struct;

typedef struct{
	u8 flag;
	u8 retry;
	u16 length;
	u8 dst_mac[6];
	u8 src_mac[6];
	u8 bssid[6];
} transmit_element;

void warp_protocol_initialize();
int warp_protocol_process(dl_list* checkout, u8* data, u16 tx_length);
void warp_protocol_set_management_transmit_callback(void(*callback)());
void warp_protocol_set_data_transmit_callback(void(*callback)());

void setup_mac_address_control(u8* buffer, u8 operation, u8* mac_addr);

#endif /* WARP_PROTOCOL_H_ */
