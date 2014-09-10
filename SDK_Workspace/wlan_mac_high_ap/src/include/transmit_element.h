/*
 * transmit_header.h
 *
 *  Created on: 2014-07-10
 *      Author: Hoai Phuoc Truong
 */

#ifndef TRANSMIT_ELEMENT_H_
#define TRANSMIT_ELEMENT_H_

#include "warp_protocol.h"

#define TRANSMIT_HEADER_LENGTH             10

#define FLAG_INDEX                         6
#define RETRY_INDEX                        7
#define PAYLOAD_SIZE_MSB_INDEX			   8
#define PAYLOAD_SIZE_LSB_INDEX			   9
#define BSSID_INDEX						   0

#define DEFAULT_TRANSMIT_FLAG			   0
#define DEFAULT_TRANSMIT_RETRY			   0

void interpret_transmit_element(u8* packet, transmit_element* transmit_info);
void clear_transmit_element(transmit_element* element);
u8 verify_transmit_element(transmit_element* transmit_element);

#endif /* TRANSMIT_HEADER_H_ */
