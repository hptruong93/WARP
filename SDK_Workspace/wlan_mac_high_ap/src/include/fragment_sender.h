/*
 * fragment_sender.h
 *
 *  Created on: 2014-09-10
 *      Author: Hoai Phuoc Truong
 */

#ifndef FRAGMENT_SENDER_H_
#define FRAGMENT_SENDER_H_

u8 generate_fragment_id();
void fragmentational_send(u8 subtype, u8* packet, u16 length);
void fragment_sender_initialize(u8* warp_eth_mac_addr);

#endif /* FRAGMENT_SENDER_H_ */
