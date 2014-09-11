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
fragment_receive_result* fragment_process(fragment_struct* info, dl_list* data, u32 data_length, u32 preprocessed_bytes);

/**********************************************************************************************************************/
//Initialization
// here is where the buffer addresses for each packet being put together will be stored
    // this is fed into the fragment_arrive array such that it can match each
    // new fragment with any fragments whose buffer locations are stored in the array,
    // add the location of fragments from new packets to the array and remove completed
    // fragments
dl_list** checked_out_queue_addr;
fragment_struct** info_addr;

/**********************************************************************************************************************/

void fragment_receiver_initialize() {
	checked_out_queue_addr = (dl_list**) wlan_mac_high_calloc(PACKET_SPACES * sizeof(dl_list*));
	info_addr = (fragment_struct**) wlan_mac_high_calloc(PACKET_SPACES * sizeof(fragment_struct*));
}

void free_fragment_receive_result(fragment_receive_result* input) {
	wlan_mac_high_free(input->info_address);
	wlan_mac_high_free(input);
}

u8* get_data_buffer_from_queue(dl_list* input, u32 preprocessed_bytes) {
	packet_bd*	tx_queue = (packet_bd*)(input->first);
	u8* output = ((tx_packet_buffer*)(tx_queue->buf_ptr))->frame;
	return (output + preprocessed_bytes);
}

fragment_struct* create_info(u8 id, u8 number, u16 byte_offset, u8 total_number_fragment) {
    fragment_struct* info = (fragment_struct*) wlan_mac_high_calloc(sizeof(fragment_struct));
    info->id = id;
    info->fragment_number = number;
    info->total_number_fragment = total_number_fragment;
    info->byte_offset = (u16) byte_offset;
    return info;
}

fragment_receive_result* fragment_receive(dl_list* check_out, u32 data_length, u32 preprocessed_bytes) {
	u8* packet_buffer = get_data_buffer_from_queue(check_out, preprocessed_bytes);

    u8 id = packet_buffer[FRAGMENT_ID_INDEX];
    u8 fragment_number = packet_buffer[FRAGMENT_NUMBER_INDEX];
    u8 total_number_fragment = packet_buffer[FRAGMENT_TOTAL_NUMBER_INDEX];
    u16 byte_offset = ((u16)(packet_buffer[FRAGMENT_BYTE_OFFSET_MSB_INDEX] << 8)) & (packet_buffer[FRAGMENT_BYTE_OFFSET_LSB_INDEX]);
    fragment_struct* fragment_info = create_info(id, fragment_number, byte_offset, total_number_fragment);

//    xil_printf("fragment id is %d\n", id);
//    xil_printf("fragment fragment_number is %d\n", fragment_number);
//    xil_printf("fragment total_number_fragment is %d\n", total_number_fragment);
//    xil_printf("fragment byte_offset is %d\n", byte_offset);
    fragment_receive_result* output = fragment_process(fragment_info, check_out, data_length - FRAGMENT_INFO_LENGTH, preprocessed_bytes + FRAGMENT_INFO_LENGTH);

    return output;
}

fragment_receive_result* fragment_process(fragment_struct* info, dl_list* checked_out_queue, u32 data_length, u32 preprocessed_bytes) {
    // assemble and store data contained in buffers being read into this function.
    // When a fragment is received and its packet still requires more fragments
    // to be complete return WAITING_FOR_FRAGMENT. Because the function receives a pointer to the
    // addresses, this does not need to be returned, but rather can be accessed directly.
    // When a packet is completely reassembled, move its location in the array to addresses[0],
    // return READY_TO_SEND so that the main program knows that
    // the first element of the addresses array contains the address to the reassembled
    // packet
    u8* data = get_data_buffer_from_queue(checked_out_queue, preprocessed_bytes);

    fragment_receive_result* frag_result = (fragment_receive_result*) wlan_mac_high_calloc(sizeof(fragment_receive_result));

    // printf("ID is %d, number is %d, offset is %d and length is %d\n", info->id, info->fragment_number, info->byte_offset, data_length);

    if (info->total_number_fragment == 1) {
        frag_result->status = RECEIVER_READY_TO_SEND;
        frag_result->packet_address = checked_out_queue;

        info->length = data_length;
        frag_result->info_address = (fragment_struct*)info;

        // printf("1\n");
    } else if (checked_out_queue_addr[0] == 0) {
        // there have been no addresses added yet, so we input the location
        // of the first data element of the incoming fragment

        // we keep track of how many fragments we are waiting for
        // by decrimenting the size. Thus, when size reaches 0, we know
        // that we have received all the fragments of this packet


        info->total_number_fragment--;
        info->length = data_length;

        info_addr[0] = info;

        checked_out_queue_addr[0] = checked_out_queue;

        frag_result->status = RECEIVER_WAITING_FOR_FRAGMENT;

        frag_result->packet_address = NULL;
        frag_result->info_address = NULL;

    } else {
        // addresses array has been initiated
        u8 i = 0;

        for (i = 0; i < PACKET_SPACES; i++) {
            // run through the array of packets, seeing if the packet number
            // of a stored fragment matches the one in the incoming fragment
            // printf("loop: %d\n", i);

            if (checked_out_queue_addr[i] == NULL){
                // we've checked all the stored packet info and the new
                // fragment isn't a match to any of them. Thus, we store
                // this fragment in the first 0 element we see

                info->total_number_fragment--;
                info->length = data_length;

                info_addr[i] = info;

                checked_out_queue_addr[i] = checked_out_queue;

                frag_result->status = RECEIVER_WAITING_FOR_FRAGMENT;

                frag_result->packet_address = NULL;
                frag_result->info_address = NULL;

                // printf("2\n");

                break;

            } else {
                // cross reference the first element of the i'th stored fragment
                // (the number of the packet it belongs to) with the first
                // element of the incoming data.

                dl_list* test_data = checked_out_queue_addr[i];
                fragment_struct* test_info = (fragment_struct*)info_addr[i];

                // printf("fragment id and number: %d %d\n", info->id, info->fragment_number);
                // printf("fragment id test info and number: %d %d\n", test_info->id, test_info->fragment_number);

                if (info->id == test_info->id) {
                    u8* test_data_buffer = get_data_buffer_from_queue(test_data, preprocessed_bytes);
                    // printf("3: %d\n", i);

                    // we've found a match, therefore we look at the respective
                    // fragment numbers to see which is the lower packet number
                    // and add the data elements from the higher fragment
                    // number to the lower fragment number

                    if (info->fragment_number < test_info->fragment_number) {
                    // printf("4 Copying to new buffer\n");

                    // incoming fragment is the lower fragment number

                        u16 rel_offset = test_info->byte_offset - info->byte_offset;
                        // find relative byte_offset of stored fragment from new
                        // fragment in order to know where to place data in
                        // new fragment
                        memmove(data + rel_offset, test_data_buffer, test_info->length);
                        info->length = test_info->byte_offset - info->byte_offset + test_info->length;

                        if (test_info->total_number_fragment - 1 == 0) {
                        // this new fragment contains the last pieces of the
                        // packet that we need. Provide the outside program
                        // with the data and info addresses of the newly-modified
                        // "data" fragment, remove its address from the array
                        // and shift up each of the addresses after it

                            frag_result->status = RECEIVER_READY_TO_SEND;

                            frag_result->packet_address = checked_out_queue;
                            checked_out_queue_addr[i] = NULL;


                            u8 k = i;

                            while (k < (PACKET_SPACES - 1) && checked_out_queue_addr[k + 1] != NULL){
                                checked_out_queue_addr[k] = checked_out_queue_addr[k+1];
                                checked_out_queue_addr[k + 1] = NULL;

                                k++;
                            }

                            frag_result->info_address = (fragment_struct*)info;

                            k = i;
                            wlan_mac_high_free(info_addr[i]);
                            while (k < (PACKET_SPACES - 1) && info_addr[k + 1] != NULL){
                                info_addr[k] = info_addr[k+1];
                                info_addr[k + 1] = NULL;

                                k++;
                            }
                        } else {
                            // decrement the size in info for management
                            // purposes and store the data and info addresses
                            // before letting the outside program know the result
                            info->total_number_fragment = test_info->total_number_fragment - 1;

                            frag_result->status = RECEIVER_WAITING_FOR_FRAGMENT;
                            frag_result->packet_address = checked_out_queue_addr[i];
                            frag_result->info_address = NULL;

                            checked_out_queue_addr[i] = checked_out_queue;
                            info_addr[i] = info;
                        }
                    } else {
                        // printf("5 Copying to old buffer\n");
                        u16 rel_offset = info->byte_offset - test_info->byte_offset;
                        memmove(test_data_buffer + rel_offset, data, data_length);

                        if (info->byte_offset - test_info->byte_offset + data_length > test_info->length) {
                            test_info->length = info->byte_offset - test_info->byte_offset + data_length;
                        }

                        if (test_info->total_number_fragment - 1 == 0){
                            test_info->total_number_fragment = info->total_number_fragment;
                            frag_result->status = RECEIVER_READY_TO_SEND;
                            frag_result->packet_address = test_data;
                            checked_out_queue_addr[i] = NULL;



                            u8 k = i;
                            while (k < (PACKET_SPACES - 1) && checked_out_queue_addr[k + 1] != NULL){
                                checked_out_queue_addr[k] = checked_out_queue_addr[k+1];
                                checked_out_queue_addr[k + 1] = NULL;
                                k++;
                            }

                            frag_result->info_address = (fragment_struct*)test_info;

                            k = i;
                            wlan_mac_high_free(info_addr[i]);
                            while (k < (PACKET_SPACES - 1) && info_addr[k + 1] != NULL){
                                info_addr[k] = info_addr[k+1];
                                info_addr[k + 1] = NULL;

                                k++;
                            }

                        } else {
                            test_info->total_number_fragment--;
                            frag_result->status = RECEIVER_WAITING_FOR_FRAGMENT;

                            frag_result->packet_address = checked_out_queue;
                            frag_result->info_address = NULL;
                        }
                    }
                    break;
                }
            }
        }
    }
    return frag_result;
}
