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


#define WARP_ETHERNET_TYPE                0xae08

#define TYPE_INDEX                        0
#define SUBTYPE_INDEX                     1

#define TYPE_TRANSMIT                     1
#define TYPE_CONTROL                      2

#define SUBTYPE_MANAGEMENT_TRANSMIT       0
#define SUBTYPE_DATA_TRANSMIT			  1

//For type transmit
#define TRANSMIT_HEADER_INDEX                          HEADER_OFFSET
#define TRANSMIT_HEADER_LENGTH                         2 ////6 bytes bssid, 1 for flag, 1 for retry, 2 for length
#define TRANSMIT_PAYLOAD_SIZE_MSB_INDEX                0
#define TRANSMIT_PAYLOAD_SIZE_LSB_INDEX                1

//For fragment info
#define FRAGMENT_INFO_INDEX                            HEADER_OFFSET + TRANSMIT_HEADER_LENGTH
#define FRAGMENT_INFO_LENGTH                           5
#define FRAGMENT_ID_INDEX                              0
#define FRAGMENT_NUMBER_INDEX                          1
#define FRAGMENT_TOTAL_NUMBER_INDEX                    2
#define FRAGMENT_BYTE_OFFSET_MSB_INDEX                 3
#define FRAGMENT_BYTE_OFFSET_LSB_INDEX                 4

#define SUBTYPE_MAC_ADDRESS_DATABASE_MANAGEMENT        4

#define SUBTYPE_TRANSMISSION_CONTROL                   8
#define SUBTYPE_TRANSMISSION_CONTROL_INDEX             HEADER_OFFSET
#define TRANSMISSION_CONTROL_LENGTH                    12
#define TRANSMISSION_TOTAL_NUMBER_ELEMENT              0
#define TRANSMISSION_BSSID_ADDRESS_INDEX               1
#define TRANSMISSION_DISABLED_INDEX                    6 //Relative to the bssid
#define TRANSMISSION_TX_POWER_INDEX                    7 //Relative to the bssid
#define TRANSMISSION_CHANNEL_INDEX                     8 //Relative to the bssid
#define TRANSMISSION_RATE_INDEX                        9 //Relative to the bssid
#define TRANSMISSION_HW_MODE_INDEX                     10 //Relative to the bssid


#define SUBTYPE_BSSID_CONTROL                          12
#define SUBTYPE_BSSID_CONTROL_INDEX                    TRANSMIT_HEADER_LENGTH
#define BSSID_CONTROL_NUMBER_ELEMENT_INDEX             0
#define BSSID_CONTROL_BSSID_INDEX                      1
#define BSSID_CONTROL_OPERATION_CODE_INDEX             7
#define BSSID_CONTROL_STATION_MAC_INDEX                8 //And +6 for each station MAC followed

//Operation code for bssid control
#define BSSID_CONTROL_ASSOCIATE_D                      1
#define BSSID_CONTROL_STATION_LIST_FULL                2
#define BSSID_CONTROL_SYNC                             16
#define BSSID_CONTROL_DISASSOCIATE_D                   32
#define BSSID_CONTROL_CLEAR_ED                         33
#define BSSID_CONTROL_CHECK_IF_EXIST                   64
#define BSSID_CONTROL_STATION_EXIST                    65
#define BSSID_CONTROL_STATION_NOT_EXIST                66
#define BSSID_CONTROL_BSSID_STATION_CONFLICT           70
#define BSSID_CONTROL_BSSID_NOT_EXIST                  75


typedef struct {
	u8 id;
	u8 fragment_number;
	u8 total_number_fragment;
	u16 byte_offset;
	u32 length;
} fragment_struct;

typedef struct{
//	u8 flag;
//	u8 retry;
//	u16 length;
	u8 dst_mac[6];
	u8 src_mac[6];
	u16 type;
	u8 bssid[6];
} transmit_element;

void warp_protocol_initialize(void(*management_transmit_callback)(), u8* eth_send_dst);
int warp_protocol_process(dl_list* checkout, u8* data, u16 tx_length);

//Testing
u8 read_bssid_control_header(u8* warp_header, u16* length);
u8 read_mac_control_header(u8* warp_header, u16* length);

#endif /* WARP_PROTOCOL_H_ */
