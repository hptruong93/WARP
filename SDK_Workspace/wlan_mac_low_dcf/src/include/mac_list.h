/*
 * mac_list.h
 *
 *  Created on: 2014-07-11
 *      Author: Hoai Phuoc Truong
 */

#ifndef MAC_LIST_H_
#define MAC_LIST_H_

#include "xil_types.h"

#define MAC_ADD           		1
#define MAC_ADDED         		2
#define MAC_LIST_FULL     		3

#define MAC_REMOVE        		32
#define MAC_REMOVE_ALL			33
#define MAC_REMOVED       		34

#define MAC_CHECK_IF_EXIST 	 	64
#define MAC_EXISTED        		65
#define MAC_NOT_EXISTED    		66

u8 mac_list_manage_mac(u8 operation_code, u8* mac_addr);
u8 mac_list_add_mac(u8* mac_addr);
u8 mac_list_remove_mac(u8* mac_addr);
u8 mac_list_check_mac(u8* mac_addr);

u8 mac_list_remove_all();

#endif /* MAC_LIST_H_ */
