/*
 * transmit_header.h
 *
 *  Created on: 2014-07-10
 *      Author: Hoai Phuoc Truong
 */

#ifndef TRANSMIT_ELEMENT_H_
#define TRANSMIT_ELEMENT_H_

#define TRANSMIT_MANAGEMENT_HEADER_LENGTH  7
#define TRANSMIT_DATA_HEADER_LENGTH		   (7 + 6)

#define POWER_INDEX                        0
#define RATE_INDEX                         1
#define CHANNEL_INDEX                      2
#define FLAG_INDEX                         3
#define RETRY_INDEX                        4
#define PAYLOAD_SIZE_MSB_INDEX			   5
#define PAYLOAD_SIZE_LSB_INDEX			   6
#define BSSID_INDEX						   7

#define DEFAULT_TRANSMIT_POWER			   0
#define DEFAULT_TRANSMIT_RATE			   1
#define DEFAULT_TRANSMIT_CHANNEL		   1
#define DEFAULT_TRANSMIT_FLAG			   0
#define DEFAULT_TRANSMIT_RETRY			   0

typedef struct{
	u8 power;
	u8 rate;
	u8 channel;
	u8 flag;
	u8 retry;
	u16 length;
	u8 dst_mac[6];
	u8 src_mac[6];
	u8 bssid[6];
} transmit_element;

void interpret_management_transmit_element(u8* packet, transmit_element* transmit_info);
void interpret_data_transmit_element(u8* packet, transmit_element* transmit_info);
void clear_transmit_element(transmit_element* element);
u8 verify_transmit_element(transmit_element* transmit_element);

#endif /* TRANSMIT_HEADER_H_ */
