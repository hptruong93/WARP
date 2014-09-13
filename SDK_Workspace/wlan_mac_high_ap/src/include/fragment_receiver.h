/*
 * fragment_receiver.h
 *
 *  Created on: 2014-09-08
 *      Author: Hoai Phuoc Truong
 */

#ifndef FRAGMENT_RECEIVER_H_
#define FRAGMENT_RECEIVER_H_

#include "warp_protocol.h"

#define RECEIVER_READY_TO_SEND                           1
#define RECEIVER_WAITING_FOR_FIRST_FRAGMENT              2
#define RECEIVER_WAITING_FOR_FRAGMENT                    3

typedef struct {
    u8 status;
    dl_list* packet_address;
    fragment_struct* info_address;
} fragment_receive_result;

void fragment_receiver_initialize();

//The buffer will starts with fragment info. WARP should not distinguish between management and data packets
//After use, must call free_fragment_receive_result of the output to avoid memory leak.
void fragment_receive(dl_list* check_out, u32 data_length, u32 preprocessed_bytes, fragment_receive_result* result);

void free_fragment_receive_result(fragment_receive_result* input);

#endif /* FRAGMENT_RECEIVER_H_ */
