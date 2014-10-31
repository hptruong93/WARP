/*
 * warp_protocol.c
 *
 *  Created on: 2014-07-10
 *      Author: Hoai Phuoc Truong
 */

#include <string.h>
#include "xil_types.h"
#include "xintc.h"
#include "warp_protocol.h"
#include "fragment_sender.h"

#include "wlan_mac_dl_list.h"
#include "wlan_mac_queue.h"
#include "wlan_mac_misc_util.h"
#include "wlan_mac_eth_util.h"
#include "wlan_mac_high.h"
#include "wlan_mac_ap.h"

#include "transmit_element.h"
#include "mac_address_control.h"
#include "transmission_control.h"

//#define WARP_PROTOCOL_DEBUG

function_ptr_t warp_protocol_management_transmit_callback;
function_ptr_t warp_protocol_data_transmit_callback;
transmit_element transmit_info;

void warp_protocol_set_management_transmit_callback(void(*callback)()) {
	warp_protocol_management_transmit_callback = (function_ptr_t)callback;
}

void warp_protocol_set_data_transmit_callback(void(*callback)()) {
	warp_protocol_data_transmit_callback = (function_ptr_t)callback;
}

void print_packett(void* packet, u16 tx_length) {
	u16 i = 0;
	u8* tx_pkt = packet;
	xil_printf("packet length: %d at %d\n", tx_length, packet);
	while (i < tx_length) {
		xil_printf(" %02x ", (u8)tx_pkt[i]);
		i++;
	}
	xil_printf("\n");
}

void warp_protocol_initialize(void(*management_transmit_callback)(), u8* eth_send_dst) {
	warp_protocol_set_management_transmit_callback(management_transmit_callback);
	fragment_sender_initialize(eth_send_dst);
}

#ifdef WARP_PROTOCOL_DEBUG
void print_mac(u8* address) {
	u8 i;
	for (i = 0; i < 5; i++) {
		xil_printf("%02x:", address[i]);
	}
	xil_printf("%02x\n", address[i]);
}
#endif

u8 read_transmit_header(u8* warp_header, u16* length) {
//	u8* element = warp_header + TRANSMIT_HEADER_LENGTH;

	return TRANSMIT_HEADER_LENGTH;
}

u8 read_transmission_control_header(u8* warp_header, u16* length) {
	u8* transmission_control_data = warp_header + TRANSMIT_HEADER_LENGTH;
	u8 total_number_element = transmission_control_data[TRANSMISSION_TOTAL_NUMBER_ELEMENT];
	u8* current_bssid = transmission_control_data + TRANSMISSION_BSSID_ADDRESS_INDEX;
	u8 i = 0;

	for (i = 0; i < total_number_element; i++) {
		u8* bssid = current_bssid;
		u8 disabled = current_bssid[TRANSMISSION_DISABLED_INDEX];
		u8 tx_power = current_bssid[TRANSMISSION_TX_POWER_INDEX];
		u8 channel = current_bssid[TRANSMISSION_CHANNEL_INDEX];
		u8 rate = current_bssid[TRANSMISSION_RATE_INDEX];
		u8 hw_mode = current_bssid[TRANSMISSION_HW_MODE_INDEX];

		xil_printf("Request to configure radio with bssid\n");
		print_packett(bssid, 6);
		xil_printf("Disabled: %d, tx_power: %d, channel: %d, rate: %d, hw_mode: %d\n", disabled, tx_power, channel, rate, hw_mode);

		//Configure radio here
		//Finished configuring radio

		static u8 reply[TRANSMIT_HEADER_LENGTH + TRANSMISSION_CONTROL_LENGTH];
		reply[TYPE_INDEX] = TYPE_CONTROL;
		reply[SUBTYPE_INDEX] = SUBTYPE_TRANSMISSION_CONTROL;

		u8* reply_data = reply + TRANSMISSION_CONTROL_LENGTH;
		reply_data[TRANSMISSION_TOTAL_NUMBER_ELEMENT] = 1;
		reply_data++;
		memcpy(reply_data, bssid, 6);
		reply_data[TRANSMISSION_DISABLED_INDEX] = disabled;
		reply_data[TRANSMISSION_TX_POWER_INDEX] = tx_power;
		reply_data[TRANSMISSION_CHANNEL_INDEX] = channel;
		reply_data[TRANSMISSION_RATE_INDEX] = rate;
		reply_data[TRANSMISSION_HW_MODE_INDEX] = hw_mode;

		xil_printf("Sending reply to request...");
		eth_pkt_send(reply, TRANSMIT_HEADER_LENGTH + TRANSMISSION_CONTROL_LENGTH, NULL, 0);

		current_bssid += TRANSMISSION_CONTROL_LENGTH - 1; //1 is the total number in the front
	}

	return 1 + total_number_element * (TRANSMISSION_CONTROL_LENGTH - 1);
}

u8 read_mac_control_header(u8* warp_header, u16* length) {
#ifdef WARP_PROTOCOL_DEBUG
	u8 operation_code = packet[TRANSMIT_HEADER_LENGTH + OPERATION_CODE_INDEX];
	xil_printf("op code = %d\n", operation_code);

	u8 mac_addr[6];
	memcpy((void*) &(mac_addr[0]), warp_header + HEADER_OFFSET + 1, 6);

	print_mac(&mac_addr[0]);
#endif

	wlan_mac_high_mac_manage_control(warp_header + TRANSMIT_HEADER_LENGTH);
	return MAC_ADDRESS_CONTROL_LENGTH;
}

u8 read_bssid_control_header(u8* warp_header, u16* length) {
	u8* element = warp_header + TRANSMIT_HEADER_LENGTH;
	u8 total_number_station_mac = element[BSSID_CONTROL_NUMBER_ELEMENT_INDEX];
	u8* bssid = element + BSSID_CONTROL_BSSID_INDEX;
	u8 operation_code = element[BSSID_CONTROL_OPERATION_CODE_INDEX];
	u8 i;

	xil_printf("Bssid is\n");
	print_packett(bssid, 6);

	if (operation_code == BSSID_CONTROL_SYNC) {
		u8 number_of_stations = association_table.length;
		u8 i, station_in_bssid;
		u8* reply = wlan_mac_high_malloc(sizeof(ethernet_header) + TRANSMIT_HEADER_LENGTH + 1 + 6 + 1 + number_of_stations * 6);
		if (reply == NULL) {
			xil_printf("Error allocating memory. Request ignored...");
			return 255;
		}

		u8* bssid_control_data = reply + sizeof(ethernet_header) + TRANSMIT_HEADER_LENGTH;
		memcpy(bssid_control_data + BSSID_CONTROL_BSSID_INDEX, bssid, 6);
		bssid_control_data[BSSID_CONTROL_OPERATION_CODE_INDEX] = BSSID_CONTROL_SYNC;

		u8* current_addr_pointer = bssid_control_data + BSSID_CONTROL_STATION_MAC_INDEX;
		station_info* curr_station_info = (station_info*)(association_table.first);
		station_in_bssid = 0;

		for(i = 0; i < association_table.length; i++){
			if (wlan_addr_eq(curr_station_info->bssid, bssid)) {
				station_in_bssid++;
				memcpy(current_addr_pointer, curr_station_info->bssid, 6);
				current_addr_pointer += 6;
			}
			curr_station_info = station_info_next(curr_station_info);
		}

		bssid_control_data[BSSID_CONTROL_NUMBER_ELEMENT_INDEX] = station_in_bssid;
		reply += sizeof(ethernet_header);

		reply[TYPE_INDEX] = TYPE_CONTROL;
		reply[SUBTYPE_INDEX] = SUBTYPE_BSSID_CONTROL;

		xil_printf("Sending reply to sync command. %s station(s) found for this bssid.\n", station_in_bssid);
		eth_pkt_send(reply, TRANSMIT_HEADER_LENGTH + 1 + 6 + 1 + 6 * station_in_bssid, NULL, 0);
		return 1 + 6 + 1;
	} else if (operation_code == BSSID_CONTROL_CLEAR_ED) {
		station_info* curr_station_info = (station_info*)(association_table.first);

		u8 remove_count = 0;
		for(i = 0; i < association_table.length; i++){
			station_info* next_station = station_info_next(curr_station_info);
			if (wlan_addr_eq(curr_station_info->bssid, bssid)) {
				//Remove station
				remove_association(&association_table, &statistics_table, curr_station_info->bssid);
				remove_count++;
			}
			curr_station_info = next_station;
		}

		u8 reply[TRANSMIT_HEADER_LENGTH + 1 + 6 + 1];
		reply[TYPE_INDEX] = TYPE_CONTROL;
		reply[SUBTYPE_INDEX] = SUBTYPE_BSSID_CONTROL;

		u8* bssid_control_data = reply + TRANSMIT_HEADER_LENGTH;
		bssid_control_data[BSSID_CONTROL_NUMBER_ELEMENT_INDEX] = remove_count;
		memcpy(bssid_control_data + BSSID_CONTROL_BSSID_INDEX, bssid, 6);
		bssid_control_data[BSSID_CONTROL_OPERATION_CODE_INDEX] = BSSID_CONTROL_CLEAR_ED;

		xil_printf("Sending reply to clear command. %s station(s) found for this bssid.\n", remove_count);
		eth_pkt_send(reply, sizeof(reply), NULL, 0);
		return 1 + 6 + 1;
	} else {
		for (i = 0; i < total_number_station_mac; i++) {
			u8 reply[TRANSMIT_HEADER_LENGTH + 1 + 6 + 1 + 16*6];
			reply[TRANSMIT_HEADER_LENGTH + BSSID_CONTROL_NUMBER_ELEMENT_INDEX] = 1;

			u8* station_mac = element + BSSID_CONTROL_STATION_MAC_INDEX;

			xil_printf("Operation code is %d\n", operation_code);
			xil_printf("Station mac is\n");
			print_packett(station_mac, 6);

			u8 reply_code = 0;
			if (operation_code == BSSID_CONTROL_ASSOCIATE_D) {
				station_info* new_station =  wlan_mac_high_find_station_info_ADDR(&association_table, station_mac);
				if (new_station == NULL) {//If not exists
					new_station = add_association(&association_table, &statistics_table, station_mac);
					if (new_station == NULL) {//If cannot add to list
						reply_code = BSSID_CONTROL_STATION_LIST_FULL;
					} else {//Successfully added to list
						memcpy(new_station->bssid, bssid, 6);
						reply_code = BSSID_CONTROL_ASSOCIATE_D;
					}
				} else {//Station already exists
					if (wlan_addr_eq(new_station->bssid, bssid)) {//If there is no change in bssid
						reply_code = BSSID_CONTROL_STATION_EXIST;
					} else {//Change bssid
						memcpy(new_station->bssid, bssid, 6);
						reply_code = BSSID_CONTROL_ASSOCIATE_D;
					}
				}
			} else if (operation_code == BSSID_CONTROL_DISASSOCIATE_D) {
				station_info* new_station = wlan_mac_high_find_station_info_ADDR(&association_table, station_mac);
				if (new_station == NULL) {//Station not exists
					reply_code = BSSID_CONTROL_STATION_NOT_EXIST;
				} else {//Station exists
					if (!(wlan_addr_eq(bssid, new_station->bssid))) {//Conflict in request. Do NOT remove association
						xil_printf("Conflicting request...");
						reply_code = BSSID_CONTROL_BSSID_STATION_CONFLICT;
					} else {//Valid request, remove association
						xil_printf("Address inside is %x\n", &association_table);
						if (remove_association(&association_table, &statistics_table, station_mac) != 0) {
							xil_printf("Something wrong??? Cannot remove association...");
						}
						reply_code = BSSID_CONTROL_DISASSOCIATE_D;
					}
				}
			} else if (operation_code == BSSID_CONTROL_CHECK_IF_EXIST) {
				station_info* new_station = wlan_mac_high_find_station_info_ADDR(&association_table, station_mac);
				if (new_station == NULL) {
					reply_code = BSSID_CONTROL_STATION_NOT_EXIST;
				} else {
					if (wlan_addr_eq(new_station->bssid, bssid)) {//Same value
						reply_code = BSSID_CONTROL_STATION_EXIST;
					} else {//Conflicting values
						reply_code = BSSID_CONTROL_BSSID_STATION_CONFLICT;
					}
				}
			}

			memcpy(reply + SUBTYPE_BSSID_CONTROL_INDEX + BSSID_CONTROL_BSSID_INDEX, bssid, 6);
			memcpy(reply + SUBTYPE_BSSID_CONTROL_INDEX + BSSID_CONTROL_STATION_MAC_INDEX, station_mac, 6);
			reply[sizeof(ethernet_header) + SUBTYPE_BSSID_CONTROL_INDEX + BSSID_CONTROL_OPERATION_CODE_INDEX] = reply_code;

			reply[TYPE_INDEX] = TYPE_CONTROL;
			reply[SUBTYPE_INDEX] = SUBTYPE_BSSID_CONTROL;

			xil_printf("Sending reply with code is %d\n", reply_code);
			eth_pkt_send(reply, TRANSMIT_HEADER_LENGTH + 1 + 6 + 1 + 6, NULL, 0);

			element += 6;
		}
		return 1 + 6 + 1 + total_number_station_mac * 6;
	}
}


int warp_protocol_process(dl_list* checkout, u8* packet, u16 tx_length) {
	packet_bd* tx_queue = (packet_bd*)(checkout->first);

#ifdef WARP_PROTOCOL_DEBUG
	xil_printf("Start reading warp protocol. Type is %d and subtype is %d \n", type, subtype);
#endif

	clear_transmit_element(&transmit_info);
	ethernet_header* ether_header = (ethernet_header*) packet;
	u8* warp_header = packet + sizeof(ethernet_header);

	if (ether_header->type == WARP_ETHERNET_TYPE) {
		switch (warp_header[TYPE_INDEX]) {
		case TYPE_TRANSMIT://Management packets
#ifdef WARP_PROTOCOL_DEBUG
			xil_printf("Transmit\n");
#endif
			memcpy(transmit_info.src_mac, ether_header->address_source, 6);
			memcpy(transmit_info .dst_mac, ether_header->address_destination, 6);
			transmit_info.type = ether_header->type;

			void* mpdu_start_ptr = (void*)((tx_packet_buffer*)(tx_queue->buf_ptr))->frame;
			tx_length -= TRANSMIT_HEADER_LENGTH + TRANSMIT_HEADER_LENGTH;
			memmove(mpdu_start_ptr, warp_header + TRANSMIT_HEADER_LENGTH + TRANSMIT_HEADER_LENGTH, tx_length);

			warp_protocol_management_transmit_callback(checkout, tx_length, &transmit_info);
			return 0;
		break;
		case TYPE_CONTROL:
			switch (warp_header[SUBTYPE_INDEX]) {
			case SUBTYPE_TRANSMISSION_CONTROL:
#ifdef WARP_PROTOCOL_DEBUG
				xil_printf("Transmission control\n");
				xil_printf("Transmission subtype supported yet!\n");
#endif
				read_transmission_control_header(warp_header, &tx_length);
			break;
			case SUBTYPE_MAC_ADDRESS_DATABASE_MANAGEMENT:
#ifdef WARP_PROTOCOL_DEBUG
				xil_printf("MAC address control\n");
#endif
				read_mac_control_header(warp_header, &tx_length);
			break;
			case SUBTYPE_BSSID_CONTROL:
				read_bssid_control_header(warp_header, &tx_length);
			break;
			default:
#ifdef WARP_PROTOCOL_DEBUG
				xil_printf("Subtype not supported yet.\n");
#endif
			break;
			}
		break;
		default: //Do nothing
#ifdef WARP_PROTOCOL_DEBUG
			xil_printf("Type not supported yet.\n");
#endif
		break;
		}
	} else {//Data packets
		memcpy(transmit_info.src_mac, ether_header->address_source, 6);
		memcpy(transmit_info.dst_mac, ether_header->address_destination, 6);
		transmit_info.type = ether_header->type;

		tx_length = wlan_eth_encap((void*) ((tx_packet_buffer*) (tx_queue->buf_ptr))->frame, ether_header->address_destination, ether_header->address_source, packet, tx_length + sizeof(ethernet_header));
		warp_protocol_management_transmit_callback(checkout, tx_length, &transmit_info);
		return 0;
	}

	queue_checkin(checkout);
	return 0; //Success
}
