/*
 * fragment_receiver.c
 *
 *  Created on: 2014-09-08
 *      Author: Hoai Phuoc Truong
 */

#include <string.h>
#include <stdlib.h>
#include "xintc.h"
#include "xil_types.h"
#include "wlan_mac_high.h"
#include "warp_protocol.h"
#include "fragment_receiver.h"

// the number of spaces in the below arrays for info and data
#define PACKET_SPACES                    20


/***************************************Type definition *****************************************************************/

/***************************************Forward declaration ************************************************************/

/*
Return status READY_TO_SEND if all fragments have arrived (including this one)
Return status WAITING_FOR_FIRST_FRAGMENT if at least a fragment (including this one) has arrived, but the first fragment has not arrived
Return status WAITING_FOR_FRAGMENT if at least a fragment (including this one) has arrived, and the first fragment has arrived

Return dl_list* packet_address is the pointer to the data buffer that is NO LONGER USED by the manager. Return null if nothing to return.
When completely reassembled, return pointer to dl_list* having the buffer containing all data.

At all time, there should be only 1 frame buffer for each fragment id. This means
    If the fragment ID does not have a data buffer, use the pointer passed in as the buffer.
    If the fragment ID already had a data buffer, copy the newly arrived data into the buffer.

Note: Do not create (using malloc/ direct declaration) data buffer. Only use the buffer u8* data in the input.
Creating buffer for management purpose (using malloc/ direct declaration) is

If returned status is READ_TO_SEND, then all data must have been assembled in u8* frame_data in the receive_result struct
Can safely assume that the pointer passed in will be able to hold all data
*/
void fragment_process(fragment_struct* info, packet_bd* checked_out_queue, u32 data_length, u32 preprocessed_bytes, fragment_receive_result* result);

/**********************************************************************************************************************/
//Initialization
// here is where the buffer addresses for each packet being put together will be stored
    // this is fed into the fragment_arrive array such that it can match each
    // new fragment with any fragments whose buffer locations are stored in the array,
    // add the location of fragments from new packets to the array and remove completed
    // fragments
packet_bd** checked_out_queue_addr;
//u8 data_buffer[20][2048];
fragment_struct** info_addr;

/**********************************************************************************************************************/

void fragment_receiver_initialize() {
	checked_out_queue_addr = (packet_bd**) wlan_mac_high_calloc(PACKET_SPACES * sizeof(packet_bd*));
	info_addr = (fragment_struct**) wlan_mac_high_calloc(PACKET_SPACES * sizeof(fragment_struct*));
}

void free_fragment_receive_result(fragment_receive_result* input) {
	if (input->info_address != NULL) {
		wlan_mac_high_free(input->info_address);
	}
	wlan_mac_high_free(input);
}

u8* get_data_buffer_from_queue(packet_bd* input, u32 preprocessed_bytes) {
	u8* output = ((tx_packet_buffer*)(input->buf_ptr))->frame;
	return (output + preprocessed_bytes);
}

void update_database(u8 index, fragment_struct* new_info, packet_bd* new_queue) {
    info_addr[index] = new_info;
    checked_out_queue_addr[index] = new_queue;
}

void pugre_database() {
    u8 i;
    for (i = 0; i < PACKET_SPACES; i++) {
        update_database(i, NULL, NULL);
    }
}

fragment_struct* create_info(u8 id, u8 number, u16 byte_offset, u8 total_number_fragment) {
    fragment_struct* info = (fragment_struct*) wlan_mac_high_calloc(sizeof(fragment_struct));
    info->id = id;
    info->fragment_number = number;
    info->total_number_fragment = total_number_fragment;
    info->byte_offset = (u16) byte_offset;
    return info;
}

void queue_copy_fragment(packet_bd* destination, packet_bd* source, u16 offset_dest, u16 offset_source, u16 length) {
	memmove(get_data_buffer_from_queue(destination, offset_dest), get_data_buffer_from_queue(source, offset_source), length);
}

u8 find_packet_id(u8 id) {
    u8 i;
    u8 output = PACKET_SPACES; //Intentionally put invalid value so that if there is no space left the caller would know
    for (i = 0; i < PACKET_SPACES; i++) {
    	fragment_struct* current = info_addr[i];
        if (current != NULL) {
            if (current->id == id) {
                return i;
            }
        } else {
            output = i;
        }
    }
    return output;
}

void assemble_result(u8 status, packet_bd* packet_address, fragment_struct* info_address, fragment_receive_result* result) {
    result->status = status;
    result->packet_address = packet_address;
    result->info_address = info_address;
}

void fragment_receive(packet_bd* check_out, u32 data_length, u32 preprocessed_bytes, fragment_receive_result* result) {
	u8* packet_buffer = get_data_buffer_from_queue(check_out, preprocessed_bytes);

    u8 id = packet_buffer[FRAGMENT_ID_INDEX];
    u8 fragment_number = packet_buffer[FRAGMENT_NUMBER_INDEX];
    u8 total_number_fragment = packet_buffer[FRAGMENT_TOTAL_NUMBER_INDEX];
    u16 byte_offset = (packet_buffer[FRAGMENT_BYTE_OFFSET_MSB_INDEX] << 8) + (packet_buffer[FRAGMENT_BYTE_OFFSET_LSB_INDEX]);
    fragment_struct* fragment_info = create_info(id, fragment_number, byte_offset, total_number_fragment);

//    xil_printf("fragment id is %d. Sending in %d\n", id, info_of_the_fragment->id);
//    xil_printf("fragment fragment_number is %d. Sending in %d\n", fragment_number, info_of_the_fragment->fragment_number);
//    xil_printf("fragment total_number_fragment is %d. Sending in %d\n", total_number_fragment, info_of_the_fragment->total_number_fragment);
//    xil_printf("fragment byte_offset is %d. Sending in %d\n", byte_offset, info_of_the_fragment->byte_offset);
//    xil_printf("addr of info is %d\n", info_of_the_fragment);
    fragment_process(fragment_info, check_out, data_length - FRAGMENT_INFO_LENGTH, preprocessed_bytes + FRAGMENT_INFO_LENGTH, result);
}

//void print_packeett(void* packet, u16 tx_length) {
//	u16 i = 0;
//	u8* tx_pkt = packet;
//	xil_printf("packet length: %d\n", tx_length);
//	while (i < tx_length) {
//		xil_printf(" %02x ", (u8)tx_pkt[i]);
//		i++;
//	}
//	xil_printf("\n");
//}

void fragment_process(fragment_struct* new_info, packet_bd* incoming_packet, u32 data_length, u32 preprocessed_bytes, fragment_receive_result* result) {
    // assemble and store data contained in buffers being read into this function.
    // When a fragment is received and its packet still requires more fragments
    // to be complete return WAITING_FOR_FRAGMENT. Because the function receives a pointer to the
    // addresses, this does not need to be returned, but rather can be accessed directly.
    // When a packet is completely reassembled, move its location in the array to addresses[0],
    // return READY_TO_SEND so that the main program knows that
    // the first element of the addresses array contains the address to the reassembled
    // packet
//	xil_printf("ID is %d, number is %d, total is %d, offset is %d and length is %d with input queue %d\n", new_info->id, new_info->fragment_number, new_info->total_number_fragment, new_info->byte_offset, data_length, incoming_packet);
//	xil_printf("data buffer starts at %x\n", get_data_buffer_from_queue(incoming_packet, 0));

	if (new_info->total_number_fragment == 1) {
		//Bounce back immediately
		new_info->length = data_length;
		assemble_result(RECEIVER_READY_TO_SEND, incoming_packet, new_info, result);
	} else {
		result->status = RECEIVER_WAITING_FOR_FRAGMENT;
		u8 found = find_packet_id(new_info->id);

		if (found == PACKET_SPACES) {//Database full...
			xil_printf("Database is full!!! Something went wrong???\n");
			pugre_database();
			//Try again
			fragment_process(new_info, incoming_packet, data_length, preprocessed_bytes, result);
		} else if (info_addr[found] == NULL) {
			//This is the first fragment of this given ID
//			xil_printf("First fragment of the id...\n");
			//Move data to the right position
			queue_copy_fragment(incoming_packet, incoming_packet, preprocessed_bytes, preprocessed_bytes + new_info->byte_offset, data_length);

			//Update info
			new_info->total_number_fragment--;
			new_info->length = data_length;
			new_info->byte_offset = 0; //Not really necessary

			//Save to database
			update_database(found, new_info, incoming_packet);

			//Assemble output
			assemble_result(RECEIVER_WAITING_FOR_FRAGMENT, NULL, NULL, result);
		} else {
			//A previous fragment has already arrived
			fragment_struct* previous_info = info_addr[found];
			packet_bd* previous_queue = checked_out_queue_addr[found];

//			xil_printf("Copy to old queue index number %d\n", found);
			//Move data from incoming queue to old queue
			queue_copy_fragment(previous_queue, incoming_packet, preprocessed_bytes + new_info->byte_offset, preprocessed_bytes, data_length);

			//Update information
			previous_info->length += data_length;
			previous_info->total_number_fragment--;

			//Check if finished and assemble result
			if (previous_info->total_number_fragment == 0) {
				assemble_result(RECEIVER_READY_TO_SEND, previous_queue, previous_info, result);
				update_database(found, NULL, NULL);
			} else {
				assemble_result(RECEIVER_WAITING_FOR_FRAGMENT, NULL, NULL, result);
			}

			//Free unused packet_bd
			queue_checkin_packet_bd(incoming_packet);

			//Clean up
			free(new_info);
		}
	}
}
