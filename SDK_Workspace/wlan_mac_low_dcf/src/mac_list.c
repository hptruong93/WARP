/*
 * mac_list.c
 *
 *  Created on: 2014-07-11
 *      Author: Hoai Phuoc Truong
 */

#include "mac_list.h"
#include "wlan_mac_misc_util.h"
#include <string.h>

#define LIST_SIZE         8

u8 mac_list[LIST_SIZE][6];
u16 exist_flag = 0;

void set_flag(u8 bit) {
	exist_flag |= 1 << bit;
}

void reset_flag(u8 bit) {
	exist_flag &= ~(1 << bit);
}

void reset_all_flag() {
	exist_flag = 0;
}

u8 check_flag(u8 bit) {
	return exist_flag & (1 << bit);
}

u8 mac_list_add_mac(u8* mac_addr) {
	u8 i = mac_list_check_mac(mac_addr);

	if (i == MAC_EXISTED) {
		return MAC_EXISTED;
	} else {
		for (i = 0; i < LIST_SIZE; i++) {
			if (!check_flag(i)) {
				memcpy(mac_list[i], mac_addr, 6);
				return MAC_ADDED;
			}
		}
		return MAC_LIST_FULL;
	}
}

u8 mac_list_remove_mac(u8* mac_addr) {
	u8 i;
	for (i = 0; i < LIST_SIZE; i++) {
		if (check_flag(i)) {//If there is a mac address at this position
			if (wlan_addr_eq(mac_list[i], mac_addr)) {
				reset_flag(i);
				return MAC_REMOVED;
			}
		}
	}

	return MAC_NOT_EXISTED;
}

u8 mac_list_check_mac(u8* mac_addr) {
	u8 i;
	for (i = 0; i < LIST_SIZE; i++) {
		if (check_flag(i)) {//If there is a mac address at this position
			if (wlan_addr_eq(mac_list[i], mac_addr)) {
				return MAC_EXISTED;
			}
		}
	}

	return MAC_NOT_EXISTED;
}

u8 mac_list_remove_all() {
	reset_all_flag();
	return 0;
}

u8 mac_list_manage_mac(u8 operation_code, u8* mac_addr) {
	switch (operation_code) {
	case MAC_ADD:
		return mac_list_add_mac(mac_addr);
		break;
	case MAC_REMOVE:
		return mac_list_remove_mac(mac_addr);
		break;
	case MAC_REMOVE_ALL:
		return mac_list_remove_all();
		break;
	case MAC_CHECK_IF_EXIST:
		return mac_list_check_mac(mac_addr);
		break;
	}
	return 0;
}
