/** @file wlan_mac_ap.c
 *  @brief Access Point
 *
 *  This contains code for the 802.11 Access Point.
 *
 *  @copyright Copyright 2014, Mango Communications. All rights reserved.
 *          Distributed under the Mango Communications Reference Design License
 *				See LICENSE.txt included in the design archive or
 *				at http://mangocomm.com/802.11/license
 *
 *  @author Chris Hunter (chunter [at] mangocomm.com)
 *  @author Patrick Murphy (murphpo [at] mangocomm.com)
 *  @author Erik Welsh (welsh [at] mangocomm.com)
 *  @bug No known bugs
 */


/***************************** Include Files *********************************/

//Xilinx SDK includes
#include "xparameters.h"
#include "stdio.h"
#include "stdlib.h"
#include "xtmrctr.h"
#include "xio.h"
#include "string.h"
#include "xintc.h"

//WARP includes
#include "wlan_mac_ipc_util.h"
#include "wlan_mac_misc_util.h"
#include "wlan_mac_802_11_defs.h"
#include "wlan_mac_queue.h"
#include "wlan_mac_ltg.h"
#include "wlan_mac_high.h"
#include "wlan_mac_packet_types.h"
#include "wlan_mac_eth_util.h"
#include "wlan_mac_event_log.h"
#include "wlan_mac_events.h"
#include "wlan_mac_ap.h"
#include "ascii_characters.h"
#include "wlan_mac_schedule.h"
#include "wlan_mac_dl_list.h"
#include "warp_protocol.h"
#include "fragment_sender.h"

// WLAN Exp includes
#include "wlan_exp_common.h"
#include "wlan_exp_node.h"
#include "wlan_exp_node_ap.h"
#include "wlan_exp_transport.h"

/*************************** Constant Definitions ****************************/

#define  WLAN_EXP_ETH                  WN_ETH_B
#define  WLAN_EXP_TYPE                 WARPNET_TYPE_80211_BASE + WARPNET_TYPE_80211_AP

#define  WLAN_CHANNEL                  2
#define  TX_GAIN_TARGET				   45


/*********************** Global Variable Definitions *************************/



/*************************** Variable Definitions ****************************/

// SSID variables
static char default_AP_SSID[] = "WARP-AP";
char*       access_point_ssid;
u8 print_enable = 1;

// Common TX header for 802.11 packets
mac_header_80211_common tx_header_common;
ipv4_header ipv4_common;

// Control variables
u8 allow_assoc;
u8 perma_assoc_mode;
u8 default_unicast_rate;
u8 default_tx_gain_target;

// Association table variable
dl_list		 association_table;
dl_list		 statistics_table;

// Tx queue variables;
u32			 max_queue_size;

// AP channel
u32 		 mac_param_chan;

// AP MAC address / Broadcast address
static u8 eeprom_mac_addr[6];
static u8 eth_mac_addr[6];
static u8 bcast_addr[6]      = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

u32 ip_dest = 0x0202a8c0;
u32 ip_src = 0x0102a8c0;

// Misc
u32 animation_schedule_id;

#ifdef WARP_PC_INTERFACE_TEST
static unsigned long eth_rx = 0;
static unsigned long wlan_tx = 0;
static unsigned long memory_issue_cnt = 0;
#endif

/*************************** Functions Prototypes ****************************/


#ifdef WLAN_USE_UART_MENU

void uart_rx(u8 rxByte);

#else

void uart_rx(u8 rxByte){ };

#endif

/******************************** Functions **********************************/


int main(){
	xil_printf("\f----- wlan_mac_ap -----\n");
	xil_printf("Compiled %s %s\n", __DATE__, __TIME__);

	//This function should be executed first. It will zero out memory, and if that
	//memory is used before calling this function, unexpected results may happen.
	wlan_mac_high_heap_init();
	wlan_mac_high_init();

    // Set Global variables
	perma_assoc_mode     = 0;
	default_unicast_rate = WLAN_MAC_RATE_18M;
	default_tx_gain_target = TX_GAIN_TARGET;

#ifdef USE_WARPNET_WLAN_EXP
	//node_info_set_max_assn( MAX_ASSOCIATIONS );
	wlan_mac_exp_configure(WLAN_EXP_TYPE, WLAN_EXP_ETH);
#endif

#ifdef WLAN_USE_UART_MENU
	xil_printf("\nAt any time, press the Esc key in your terminal to access the AP menu\n");
#endif

#ifdef USE_WARPNET_WLAN_EXP
	// Set AP processing callbacks
	node_set_process_callback( (void *)wlan_exp_node_ap_processCmd );
#endif
	dl_list_init(&association_table);
	dl_list_init(&statistics_table);

	max_queue_size = min((queue_total_size()- eth_bd_total_size()) / (association_table.length+1),MAX_PER_FLOW_QUEUE);
	// Initialize callbacks
	wlan_mac_util_set_eth_rx_callback(       (void*)ethernet_receive);
	wlan_mac_high_set_mpdu_tx_done_callback( (void*)mpdu_transmit_done);
	wlan_mac_high_set_mpdu_rx_callback(      (void*)mpdu_rx_process);
	wlan_mac_high_set_fcs_bad_rx_callback(   (void*)bad_fcs_rx_process);
	wlan_mac_high_set_pb_u_callback(         (void*)up_button);
	wlan_mac_high_set_uart_rx_callback(      (void*)uart_rx);
	wlan_mac_high_set_check_queue_callback(  (void*)check_tx_queue);
    //wlan_mac_ltg_sched_set_callback(         (void*)ltg_event);
    wlan_mac_util_set_transmit_callback(	 (void*)warp_protocol_process);

    wlan_mac_util_set_eth_encap_mode(ENCAP_MODE_AP);

    // Initialize interrupts
    wlan_mac_high_interrupt_init();

    // Wait for CPU Low to initialize
	while( wlan_mac_high_is_cpu_low_initialized() == 0 ){
		xil_printf("waiting on CPU_LOW to boot\n");
	};

	// CPU Low will pass HW information to CPU High as part of the boot process
	//   - Get necessary HW information
	memcpy((void*) &(eth_mac_addr[0]), (void*) wlan_mac_high_get_eth_mac_addr(), 6);
	memcpy((void*) &(eeprom_mac_addr[0]), (void*) wlan_mac_high_get_eeprom_mac_addr(), 6);
	set_eth_mac_addr((void*) &(eeprom_mac_addr[0]));
	warp_protocol_initialize((void*)send_frame_to_wifi, eeprom_mac_addr);

    // Set Header information
	tx_header_common.address_2 = &(eeprom_mac_addr[0]);
	tx_header_common.seq_num   = 0;

	// IPv4 common header

	ipv4_common.ver_ihl = 0x45;
	ipv4_common.tos = 0x00;
	ipv4_common.id = 0x0100;
	ipv4_common.flags_fragOffset = 0x0000;
	ipv4_common.ttl = 0x40;
	//default protocol: UDP
	ipv4_common.prot = IPV4_PROT_UDP;
	memcpy((void*) &ipv4_common.ip_dest, (void*) &ip_dest, 4);
	memcpy((void*) &ipv4_common.ip_src, (void*) &ip_src, 4);

    // Initialize hex display
//	wlan_mac_high_write_hex_display(0);

	// Set up channel
	mac_param_chan = WLAN_CHANNEL;
	wlan_mac_high_set_channel( mac_param_chan );

	// Set SSID
	access_point_ssid = wlan_mac_high_malloc(strlen(default_AP_SSID)+1);
	strcpy(access_point_ssid,default_AP_SSID);

    // Schedule all events
	//wlan_mac_schedule_event_repeated(SCHEDULE_COARSE, BEACON_INTERVAL_US, SCHEDULE_REPEAT_FOREVER, (void*)beacon_transmit);
	//wlan_mac_schedule_event(SCHEDULE_COARSE, ASSOCIATION_CHECK_INTERVAL_US, (void*)association_timestamp_check);

	//animation_schedule_id = wlan_mac_schedule_event_repeated(SCHEDULE_COARSE, ANIMATION_RATE_US, SCHEDULE_REPEAT_FOREVER, (void*)animate_hex);

	//enable_associations( ASSOCIATION_ALLOW_PERMANENT );

	// Print AP information to the terminal
    xil_printf("WLAN MAC AP boot complete: \n");
    xil_printf("  SSID    : %s \n", access_point_ssid);
    xil_printf("  Channel : %d \n", mac_param_chan);
	xil_printf("  MAC Addr: %02x-%02x-%02x-%02x-%02x-%02x\n\n",eeprom_mac_addr[0],eeprom_mac_addr[1],eeprom_mac_addr[2],eeprom_mac_addr[3],eeprom_mac_addr[4],eeprom_mac_addr[5]);

	wlan_mac_high_interrupt_start();

	while(1){
		//The design is entirely interrupt based. When no events need to be processed, the processor
		//will spin in this loop until an interrupt happens

#ifdef USE_WARPNET_WLAN_EXP
		wlan_mac_high_interrupt_stop();
		transport_poll( WLAN_EXP_ETH );
		wlan_mac_high_interrupt_start();
#endif
	}
	return -1;
}

void check_tx_queue(){
	if( wlan_mac_high_is_cpu_low_ready() ){
		if(wlan_mac_queue_poll(0)){
			return;
		}
	}
}

void mpdu_transmit_done(tx_frame_info* tx_mpdu){
	tx_event* tx_event_log_entry;
	station_info* station;

	void * mpdu = (void*)tx_mpdu + PHY_TX_PKT_BUF_MPDU_OFFSET;
	u8* mpdu_ptr_u8 = (u8*)mpdu;
	mac_header_80211* tx_80211_header;
	tx_80211_header = (mac_header_80211*)((void *)mpdu_ptr_u8);

	tx_event_log_entry = get_next_empty_tx_event();

	if(tx_event_log_entry != NULL){
		tx_event_log_entry->state                    = tx_mpdu->state_verbose;
		tx_event_log_entry->AID                      = 0;
		tx_event_log_entry->power                    = 0; //TODO
		tx_event_log_entry->length                   = tx_mpdu->length;
		tx_event_log_entry->rate                     = tx_mpdu->rate;
		tx_event_log_entry->mac_type                 = tx_80211_header->frame_control_1;
		tx_event_log_entry->seq                      = ((tx_80211_header->sequence_control)>>4)&0xFFF;
		tx_event_log_entry->retry_count              = tx_mpdu->retry_count;
		tx_event_log_entry->tx_mpdu_accept_timestamp = tx_mpdu->tx_mpdu_accept_timestamp;
		tx_event_log_entry->tx_mpdu_done_timestamp   = tx_mpdu->tx_mpdu_done_timestamp;
	}

	if(tx_mpdu->AID != 0){
		station = wlan_mac_high_find_station_info_AID(&association_table, tx_mpdu->AID);
		if(station != NULL){
			if(tx_event_log_entry != NULL) tx_event_log_entry->AID = station->AID;
			//Process this TX MPDU DONE event to update any statistics used in rate adaptation
			wlan_mac_high_process_tx_done(tx_mpdu, station);
		}
	}
}

void up_button(){

	switch ( get_associations_status() ) {

        case ASSOCIATION_ALLOW_NONE:
    		// AP is currently not allowing any associations to take place
//        	animation_schedule_id = wlan_mac_schedule_event_repeated(SCHEDULE_COARSE, ANIMATION_RATE_US, SCHEDULE_REPEAT_FOREVER, (void*)animate_hex);
    		enable_associations( ASSOCIATION_ALLOW_TEMPORARY );
    		wlan_mac_schedule_event(SCHEDULE_COARSE,ASSOCIATION_ALLOW_INTERVAL_US, (void*)disable_associations);
        break;

        case ASSOCIATION_ALLOW_TEMPORARY:
    		// AP is currently allowing associations, but only for the small allow window.
    		//   Go into permanent allow association mode.
    		enable_associations( ASSOCIATION_ALLOW_PERMANENT );
    		xil_printf("Allowing associations indefinitely\n");
        break;

        case ASSOCIATION_ALLOW_PERMANENT:
    		// AP is permanently allowing associations. Toggle everything off.
    		enable_associations( ASSOCIATION_ALLOW_TEMPORARY );
    		disable_associations();
        break;
	}
}

void ltg_event(u32 id, void* callback_arg){
	dl_list checkout;
	packet_bd* tx_queue;
	u32 tx_length;
	u32 payload_length = 0;
	u8* mpdu_ptr_u8;
	llc_header* llc_hdr;
	station_info* station;

	switch(((ltg_pyld_hdr*)callback_arg)->type){
		case LTG_PYLD_TYPE_FIXED:
			payload_length = ((ltg_pyld_fixed*)callback_arg)->length;
		break;
		case LTG_PYLD_TYPE_UNIFORM_RAND:
			payload_length = (rand()%(((ltg_pyld_uniform_rand*)(callback_arg))->max_length - ((ltg_pyld_uniform_rand*)(callback_arg))->min_length))+((ltg_pyld_uniform_rand*)(callback_arg))->min_length;
		break;
		default:
		break;

	}

	station = wlan_mac_high_find_station_info_AID(&association_table, LTG_ID_TO_AID(id));

	if(station != NULL){
		//The AID <-> LTG ID connection is arbitrary. In this design, we use the LTG_ID_TO_AID
		//macro to map multiple different LTG IDs onto an AID for a specific station. This allows
		//multiple LTG flows to target a single user in the network.

		//We implement a soft limit on the size of the queue allowed for any
		//given station. This avoids the scenario where multiple backlogged
		//LTG flows favor a single user and starve everyone else.
		if(queue_num_queued(station->AID) < max_queue_size){
			//Send a Data packet to this station
			//Checkout 1 element from the queue;
			queue_checkout(&checkout,1);

			if(checkout.length == 1){ //There was at least 1 free queue element
				tx_queue = (packet_bd*)(checkout.first);

				wlan_mac_high_setup_tx_header( &tx_header_common, station->addr, &(eeprom_mac_addr[0]), eeprom_mac_addr );

				mpdu_ptr_u8 = (u8*)((tx_packet_buffer*)(tx_queue->buf_ptr))->frame;
				tx_length = wlan_create_data_frame((void*)((tx_packet_buffer*)(tx_queue->buf_ptr))->frame, &tx_header_common, MAC_FRAME_CTRL2_FLAG_FROM_DS);

				mpdu_ptr_u8 += sizeof(mac_header_80211);
				llc_hdr = (llc_header*)(mpdu_ptr_u8);

				//Prepare the MPDU LLC header
				llc_hdr->dsap = LLC_SNAP;
				llc_hdr->ssap = LLC_SNAP;
				llc_hdr->control_field = LLC_CNTRL_UNNUMBERED;
				bzero((void *)(llc_hdr->org_code), 3); //Org Code 0x000000: Encapsulated Ethernet
				llc_hdr->type = LLC_TYPE_CUSTOM;

				tx_length += sizeof(llc_header);
				tx_length += payload_length;

				wlan_mac_high_setup_tx_queue ( tx_queue, (void*)station, tx_length, MAX_RETRY, default_tx_gain_target,
								 (TX_MPDU_FLAGS_FILL_DURATION | TX_MPDU_FLAGS_REQ_TO) );

				enqueue_after_end(station->AID, &checkout);
				check_tx_queue();
			}
		}
	}
}

int ethernet_receive(dl_list* tx_queue_list, u8* eth_dest, u8* eth_src, u16 tx_length){
	return 0;
	//Receives the pre-encapsulated Ethernet frames
	station_info* station;
	//80211
	packet_bd* tx_queue = (packet_bd*)(tx_queue_list->first);

	wlan_mac_high_setup_tx_header( &tx_header_common, (u8*)(&(eth_dest[0])), &(eeprom_mac_addr[0]), (u8*)(&(eth_src[0])) );
	wlan_create_data_frame((void*)((tx_packet_buffer*)(tx_queue->buf_ptr))->frame, &tx_header_common, MAC_FRAME_CTRL2_FLAG_FROM_DS);

	if(wlan_addr_eq(bcast_addr, eth_dest)){
		if(queue_num_queued(0) < max_queue_size){
			wlan_mac_high_setup_tx_queue ( tx_queue, NULL, tx_length, 0, default_tx_gain_target, 0 );

			enqueue_after_end(0, tx_queue_list);
			check_tx_queue();
		} else {
			return 0;
		}
	} else {
		//Check associations
		//Is this packet meant for a station we are associated with?
		station = wlan_mac_high_find_station_info_ADDR(&association_table, eth_dest);
		if( station != NULL ) {
			if(queue_num_queued(station->AID) < max_queue_size){
				wlan_mac_high_setup_tx_queue ( tx_queue, (void*)station, tx_length, MAX_RETRY, default_tx_gain_target,
								 (TX_MPDU_FLAGS_FILL_DURATION | TX_MPDU_FLAGS_REQ_TO) );

				enqueue_after_end(station->AID, tx_queue_list);
				check_tx_queue();
			} else {
				return 0;
			}
		} else {
			//Checkin this packet_bd so that it can be checked out again
			return 0;
		}
	}

	return 1;

}

void send_frame_to_wifi(dl_list* checkout, u16 tx_length, transmit_element* transmit_info) {
	u16 type = transmit_info->type;
	u8* eth_dest = transmit_info->dst_mac;
	u8* eth_src = transmit_info->src_mac;

	//Receives the pre-encapsulated Ethernet frames
	if (type != 0xae08) {//Data packets
		if (queue_num_queued(0) < max_queue_size) {
			packet_bd* tx_queue = (packet_bd*) (checkout->first);

			station_info* station = wlan_mac_high_find_station_info_ADDR(&association_table, eth_dest);

			if (station == NULL) {
				wlan_mac_high_setup_tx_header(&tx_header_common, (u8*) (&(eth_dest[0])), eeprom_mac_addr , (u8*) (&(eth_src[0])));
			} else {
				wlan_mac_high_setup_tx_header(&tx_header_common, (u8*) (&(eth_dest[0])), station->bssid , (u8*) (&(eth_src[0])));
			}

			wlan_create_data_frame((void*) ((tx_packet_buffer*) (tx_queue->buf_ptr))->frame, &tx_header_common, MAC_FRAME_CTRL2_FLAG_FROM_DS);

			if (wlan_addr_eq(bcast_addr, eth_dest)) {//This needs to cover for the other ap as well?
				if (queue_num_queued(0) < max_queue_size) {
					wlan_mac_high_setup_tx_queue(tx_queue, NULL, tx_length, 0, default_tx_gain_target, 0);

					enqueue_after_end(0, checkout);
					check_tx_queue();
				} else {
					return;// 0;
				}
			} else {
				//Check associations
				//Is this packet meant for a station we are associated with?
				wlan_mac_high_setup_tx_queue(tx_queue, (void*) NULL, tx_length,	MAX_RETRY, default_tx_gain_target, (TX_MPDU_FLAGS_FILL_DURATION | TX_MPDU_FLAGS_REQ_TO));
				enqueue_after_end(0, checkout);

				check_tx_queue();
			}

			return; //1;
		} else {
			queue_checkin(checkout);
			return; //0;
		}
	} else {//Management packets
		if (queue_num_queued(0) < max_queue_size) {
			packet_bd* tx_queue = (packet_bd*) (checkout->first);
			wlan_mac_high_setup_tx_queue(tx_queue, NULL, tx_length, 0,	default_tx_gain_target, 0);
			enqueue_after_end(0, checkout);

			//print_packet(((tx_packet_buffer*)(tx_queue->buf_ptr))->frame, tx_length);

			check_tx_queue();
			//			xil_printf("pkt_transmitted\n");
			return;// 1;
		} else {
			queue_checkin(checkout);
			return;// 0;
		}
	}
}

//void send_data_to_wifi(dl_list* checkout, packet_bd*	tx_queue, u16 tx_length, transmit_element* transmit_info) {
//	if (queue_num_queued(0) < max_queue_size) {
//		wlan_mac_high_setup_tx_header( &tx_header_common, (u8*)(&(transmit_info->dst_mac[0])), (u8*)(&(transmit_info->bssid[0])), (u8*)(&(transmit_info->src_mac[0])) );
//		wlan_create_data_frame((void*)((tx_packet_buffer*)(tx_queue->buf_ptr))->frame, &tx_header_common, MAC_FRAME_CTRL2_FLAG_FROM_DS);
//
//		wlan_mac_high_setup_tx_queue ( tx_queue, NULL, tx_length, transmit_info->retry, default_tx_gain_target,	(TX_MPDU_FLAGS_FILL_DURATION | TX_MPDU_FLAGS_REQ_TO) );
//
//		enqueue_after_end(0, checkout);
//		check_tx_queue();
//		//xil_printf("pkt_transmitted\n");
//	} else {
//		xil_printf("Memory problem???\n");
//		queue_checkin(checkout);
//		memory_issue_cnt += 1;
//	}
//}

void print_packet(void* packet, u16 tx_length) {
	u16 i = 0;
	u8* tx_pkt = packet;
	xil_printf("packet length: %d\n", tx_length);
	while (i < tx_length) {
		xil_printf(" %02x ", (u8)tx_pkt[i]);
		i++;
	}
	xil_printf("\n");
}



void mpdu_rx_process(void* pkt_buf_addr, u8 rate, u16 length) {
	void * mpdu = pkt_buf_addr + PHY_RX_PKT_BUF_MPDU_OFFSET;
	u8* mpdu_ptr_u8 = (u8*)mpdu;
	u16 tx_length;
//	u8 send_response;
	mac_header_80211* rx_80211_header;
	rx_80211_header = (mac_header_80211*)((void *)mpdu_ptr_u8);
	u16 rx_seq;
	dl_list checkout;
	packet_bd*	tx_queue;
	station_info* associated_station = NULL;
	statistics* station_stats = NULL;
	u8 eth_send;

//	xil_printf("Length is %d\n", length);

//	void* rx_event_log_entry;

	rx_frame_info* mpdu_info = (rx_frame_info*)pkt_buf_addr;

	//*************
	// Event logging
	//*************
// Disable logging
//	if(rate != WLAN_MAC_RATE_1M){
//		rx_event_log_entry = (void*)get_next_empty_rx_ofdm_event();
//
//		if(rx_event_log_entry != NULL){
//			((rx_ofdm_event*)rx_event_log_entry)->state    = mpdu_info->state;
//			((rx_ofdm_event*)rx_event_log_entry)->AID      = 0;
//			((rx_ofdm_event*)rx_event_log_entry)->power    = mpdu_info->rx_power;
//			((rx_ofdm_event*)rx_event_log_entry)->rf_gain  = mpdu_info->rf_gain;
//			((rx_ofdm_event*)rx_event_log_entry)->bb_gain  = mpdu_info->bb_gain;
//			((rx_ofdm_event*)rx_event_log_entry)->length   = mpdu_info->length;
//			((rx_ofdm_event*)rx_event_log_entry)->rate     = mpdu_info->rate;
//			((rx_ofdm_event*)rx_event_log_entry)->mac_type = rx_80211_header->frame_control_1;
//			((rx_ofdm_event*)rx_event_log_entry)->seq      = ((rx_80211_header->sequence_control)>>4)&0xFFF;
//			((rx_ofdm_event*)rx_event_log_entry)->flags    = mpdu_info->flags;
//
//	#ifdef WLAN_MAC_EVENTS_LOG_CHAN_EST
//			if(rate != WLAN_MAC_RATE_1M) wlan_mac_high_cdma_start_transfer(((rx_ofdm_event*)rx_event_log_entry)->channel_est, mpdu_info->channel_est, sizeof(mpdu_info->channel_est));
//	#endif
//
//		}
//	} else {
//		rx_event_log_entry = (void*)get_next_empty_rx_dsss_event();
//
//		if(rx_event_log_entry != NULL){
//			((rx_dsss_event*)rx_event_log_entry)->state    = mpdu_info->state;
//			((rx_dsss_event*)rx_event_log_entry)->AID      = 0;
//			((rx_dsss_event*)rx_event_log_entry)->power    = mpdu_info->rx_power;
//			((rx_ofdm_event*)rx_event_log_entry)->rf_gain  = mpdu_info->rf_gain;
//			((rx_ofdm_event*)rx_event_log_entry)->bb_gain  = mpdu_info->bb_gain;
//			((rx_dsss_event*)rx_event_log_entry)->length   = mpdu_info->length;
//			((rx_dsss_event*)rx_event_log_entry)->rate     = mpdu_info->rate;
//			((rx_dsss_event*)rx_event_log_entry)->mac_type = rx_80211_header->frame_control_1;
//			((rx_dsss_event*)rx_event_log_entry)->seq      = ((rx_80211_header->sequence_control)>>4)&0xFFF;
//			((rx_dsss_event*)rx_event_log_entry)->flags    = mpdu_info->flags;
//		}
//	}


	associated_station = wlan_mac_high_find_station_info_ADDR(&association_table, (rx_80211_header->address_2));

	if( associated_station != NULL ){
		station_stats = associated_station->stats;
		rx_seq = ((rx_80211_header->sequence_control)>>4)&0xFFF;

		//Disable logging
//		if(rate != WLAN_MAC_RATE_1M){
//			if(rx_event_log_entry != NULL) ((rx_ofdm_event*)rx_event_log_entry)->AID = associated_station->AID;
//		} else {
//			if(rx_event_log_entry != NULL) ((rx_dsss_event*)rx_event_log_entry)->AID = associated_station->AID;
//		}

		associated_station->rx.last_timestamp = get_usec_timestamp();
		associated_station->rx.last_power = mpdu_info->rx_power;

		//Check if duplicate
		if( (associated_station->rx.last_seq != 0)  && (associated_station->rx.last_seq == rx_seq) ) {
			//Received seq num matched previously received seq num for this STA; ignore the MPDU and return
#ifdef WLAN_MAC_EVENTS_LOG_CHAN_EST
			if(rate != WLAN_MAC_RATE_1M) wlan_mac_high_cdma_finish_transfer();
#endif
			return;

		} else {
			associated_station->rx.last_seq = rx_seq;
		}
	} else {
		station_stats = add_statistics(&statistics_table, NULL, rx_80211_header->address_2);
	}

	if(station_stats != NULL){
		station_stats->last_timestamp = get_usec_timestamp();
		(station_stats->num_rx_success)++;
		(station_stats->num_rx_bytes) += mpdu_info->length;
	}

	switch(rx_80211_header->frame_control_1) {
		case (MAC_FRAME_CTRL1_SUBTYPE_DATA): //Data Packet
			if((rx_80211_header->frame_control_2) & MAC_FRAME_CTRL2_FLAG_TO_DS) {
				//MPDU is flagged as destined to the DS
				eth_send = 1;

				if(wlan_addr_eq(rx_80211_header->address_3,bcast_addr)){
					//No broadcast of received data packets from AP fow now
//					queue_checkout(&checkout,1);
//
//					if(checkout.length == 1){ //There was at least 1 free queue element
//						tx_queue = (packet_bd*)(checkout.first);
//						wlan_mac_high_setup_tx_header( &tx_header_common, bcast_addr, rx_80211_header->address_2);
//						mpdu_ptr_u8 = (u8*)((tx_packet_buffer*)(tx_queue->buf_ptr))->frame;
//						tx_length = wlan_create_data_frame((void*)((tx_packet_buffer*)(tx_queue->buf_ptr))->frame, &tx_header_common, MAC_FRAME_CTRL2_FLAG_FROM_DS);
//						mpdu_ptr_u8 += sizeof(mac_header_80211);
//						memcpy(mpdu_ptr_u8, (void*)rx_80211_header + sizeof(mac_header_80211), mpdu_info->length - sizeof(mac_header_80211));
//						wlan_mac_high_setup_tx_queue ( tx_queue, NULL, mpdu_info->length, 0, default_tx_gain_target, 0 );
//						enqueue_after_end(0, &checkout);
//						check_tx_queue();
//					}
				} else {
					associated_station = wlan_mac_high_find_station_info_ADDR(&association_table, rx_80211_header->address_3);
					if(associated_station != NULL){
						//This is to solve hidden node problem? For now assume that stations can always see each other
						queue_checkout(&checkout,1);

						if(checkout.length == 1){ //There was at least 1 free queue element
							tx_queue = (packet_bd*)(checkout.first);
							wlan_mac_high_setup_tx_header( &tx_header_common, rx_80211_header->address_3, associated_station->bssid, rx_80211_header->address_2);
							mpdu_ptr_u8 = (u8*)((tx_packet_buffer*)(tx_queue->buf_ptr))->frame;
							tx_length = wlan_create_data_frame((void*)((tx_packet_buffer*)(tx_queue->buf_ptr))->frame, &tx_header_common, MAC_FRAME_CTRL2_FLAG_FROM_DS);
							mpdu_ptr_u8 += sizeof(mac_header_80211);
							memcpy(mpdu_ptr_u8, (void*)rx_80211_header + sizeof(mac_header_80211), mpdu_info->length - sizeof(mac_header_80211));
							wlan_mac_high_setup_tx_queue ( tx_queue, (void*)associated_station, mpdu_info->length, MAX_RETRY, default_tx_gain_target,
														 (TX_MPDU_FLAGS_FILL_DURATION | TX_MPDU_FLAGS_REQ_TO) );

							enqueue_after_end(associated_station->AID,  &checkout);

							check_tx_queue();
							#ifndef ALLOW_ETH_TX_OF_WIRELESS_TX
							eth_send = 0;
							#endif
						}
					}
				}

				if(eth_send){
					wlan_mpdu_eth_send(mpdu,length);
				}
			}

		break;

		default:
			//This should be left as a verbose print. It occurs often when communicating with mobile devices since they tend to send
			//null data frames (type: DATA, subtype: 0x4) for power management reasons.
//			warp_printf(PL_VERBOSE, "Received unknown frame control type/subtype %x\n",rx_80211_header->frame_control_1);
			;
			//static u8 eth_dst[6]		= {0x00, 0x0a, 0xcd, 0x21, 0x0b, 0x64};//The PC Engine ethernet MAC --REMEMBER to sync this with fragment_sender.c

//			ethernet_header* eth_hdr = (ethernet_header*)(mpdu - sizeof(ethernet_header));
//
//			memcpy(eth_hdr->address_destination, eth_dst, 6);
//			memcpy(eth_hdr->address_source, eth_mac_addr, 6);
//			eth_hdr->type = 0xae08;

//			wlan_eth_dma_send((u8*) eth_hdr, length + sizeof(ethernet_header));
			static u8 warp_layer[] = {0x01, 0x00, 0x00, 0x00}; //Transmit header
			warp_layer[2] = (length >> 8) & 0xff;
			warp_layer[3] = length & 0xff;

			eth_pkt_send(mpdu, length, warp_layer, 4);
		break;
	}
//#ifdef WLAN_MAC_EVENTS_LOG_CHAN_EST
//	if(rate != WLAN_MAC_RATE_1M) wlan_mac_high_cdma_finish_transfer();
//#endif
	return;
}

void bad_fcs_rx_process(void* pkt_buf_addr, u8 rate, u16 length) {
	bad_fcs_event* bad_fcs_event_log_entry = get_next_empty_bad_fcs_event();
	bad_fcs_event_log_entry->length = length;
	bad_fcs_event_log_entry->rate = rate;
}

void print_associations(dl_list* assoc_tbl){
	u64 timestamp = get_usec_timestamp();
	station_info* curr_station_info;
	u32 i;
	xil_printf("\n   Current Associations\n (MAC time = %d usec)\n",timestamp);
				xil_printf("|-ID-|----- MAC ADDR ----|\n");

	curr_station_info = (station_info*)(assoc_tbl->first);
	for(i=0; i<(assoc_tbl->length); i++){
		xil_printf("| %02x | %02x:%02x:%02x:%02x:%02x:%02x |\n", curr_station_info->AID,
				curr_station_info->addr[0],curr_station_info->addr[1],curr_station_info->addr[2],curr_station_info->addr[3],curr_station_info->addr[4],curr_station_info->addr[5]);
		curr_station_info = station_info_next(curr_station_info);
	}
	xil_printf("|------------------------|\n");

	return;
}

u32  get_associations_status() {
	// Get the status of associations for the AP
	//   - 00 -> Associations not allowed
	//   - 01 -> Associations allowed for a window
	//   - 11 -> Associations allowed permanently

	return ( perma_assoc_mode * 2 ) + allow_assoc;
}

void enable_associations( u32 permanent_association ){
	// Send a message to other processor to tell it to enable associations
#ifdef _DEBUG_
	xil_printf("Allowing new associations\n");
#endif

	// Set the DSSS value in CPU Low
	wlan_mac_high_set_dsss( 1 );

    // Set the global variable
	allow_assoc = 1;

	// Set the global variable for permanently allowing associations
	switch ( permanent_association ) {

        case ASSOCIATION_ALLOW_PERMANENT:
        	perma_assoc_mode = 1;
        break;

        case ASSOCIATION_ALLOW_TEMPORARY:
        	perma_assoc_mode = 0;
        break;
	}
}

void disable_associations(){
	// Send a message to other processor to tell it to disable associations
	if(perma_assoc_mode == 0){

#ifdef _DEBUG_
		xil_printf("Not allowing new associations\n");
#endif

		// Set the DSSS value in CPU Low
		wlan_mac_high_set_dsss( 0 );

        // Set the global variables
		allow_assoc      = 0;

		// Stop the animation on the hex displays from continuing
		wlan_mac_remove_schedule(SCHEDULE_COARSE, animation_schedule_id);

		// Set the hex display
//		wlan_mac_high_write_hex_display(association_table.length);
//		wlan_mac_high_write_hex_display_dots(0);
	}
}

//void animate_hex(){
//	static u8 i = 0;
//	//wlan_mac_high_write_hex_display(next_free_assoc_index,i%2);
//	wlan_mac_high_write_hex_display_dots(i%2);
//	i++;
//}


/*****************************************************************************/
/**
* Reset Station Statistics
*
* Reset all statistics being kept for all stations
*
* @param    None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void reset_station_statistics(){
	u32 i;
	station_info* curr_station_info;
	curr_station_info = (station_info*)(association_table.first);
	for(i=0; i < association_table.length; i++){
		curr_station_info->stats->num_tx_total = 0;
		curr_station_info->stats->num_tx_success = 0;
		curr_station_info->stats->num_retry = 0;
		curr_station_info->stats->num_rx_success = 0;
		curr_station_info->stats->num_rx_bytes = 0;
		curr_station_info = station_info_next(curr_station_info);
	}
}


u32  deauthenticate_station( station_info* station ) {
	dl_list checkout;
	packet_bd*     tx_queue;
	u32            tx_length;
	u32            aid;

	if(station == NULL){
		return 0;
	}

	// Get the AID
	aid = station->AID;

	// Checkout 1 element from the queue
	queue_checkout(&checkout,1);

	if(checkout.length == 1){ //There was at least 1 free queue element
		tx_queue = (packet_bd*)(checkout.first);

		purge_queue(aid); //TODO: generalize

		// Create deauthentication packet
		wlan_mac_high_setup_tx_header( &tx_header_common, station->addr, &(eeprom_mac_addr[0]), eeprom_mac_addr );

		tx_length = wlan_create_deauth_frame((void*)((tx_packet_buffer*)(tx_queue->buf_ptr))->frame, &tx_header_common, DEAUTH_REASON_INACTIVITY);

		wlan_mac_high_setup_tx_queue ( tx_queue, NULL, tx_length, MAX_RETRY, default_tx_gain_target,
						 (TX_MPDU_FLAGS_FILL_DURATION | TX_MPDU_FLAGS_REQ_TO) );

		//
		enqueue_after_end(aid, &checkout);
		check_tx_queue();

		// Remove this STA from association list
		remove_association( &association_table, &statistics_table, station->addr );
	}

//	wlan_mac_high_write_hex_display(association_table.length);

	return aid;
}



/*****************************************************************************/
/**
* Deauthenticate all stations in the Association Table
*
* Loop through all associations in the table and deauthenticate the stations
*
* @param    None.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void deauthenticate_stations(){
//	u32 i;
//	station_info* curr_station_info;
//	station_info* next_station_info;
//
//	next_station_info = (station_info*)(association_table.first);
//	for (i = 0; i < association_table.length ; i++){
//		curr_station_info = next_station_info;
//		next_station_info = station_info_next(curr_station_info);
//		deauthenticate_station(curr_station_info);
//	}
}

station_info* add_association(dl_list* assoc_tbl, dl_list* stat_tbl, u8* addr){
	station_info* station;
	statistics*   station_stats;
	station_info* curr_station_info;
	u32 i;
	u16 curr_AID = 0;

	station = wlan_mac_high_find_station_info_ADDR(assoc_tbl, addr);
	if(station != NULL){
		//This addr is already tied to an association table entry. We'll just pass
		//this the pointer to that entry back to the calling function without creating
		//a new entry
		return station;
	} else {

		if(assoc_tbl->length < MAX_NUM_ASSOC){
			//This addr is new, so we'll have to add an entry into the association table

			station = wlan_mac_high_malloc(sizeof(station_info));
			station_stats = wlan_mac_high_find_statistics_ADDR(stat_tbl, addr);
			if(station_stats == NULL){
				station_stats = wlan_mac_high_calloc(sizeof(statistics));
				if(station_stats == NULL){
					//malloc failed. Passing that failure on to calling function
					return NULL;
				}
				memcpy(station_stats->addr, addr, 6);
				dl_node_insertEnd(stat_tbl, &(station_stats->node));
			}

			if(station == NULL){
				//malloc failed. Passing that failure on to calling function.
				return NULL;
			}

			station->stats = station_stats;
			station->stats->is_associated = 1;
			memcpy(station->addr, addr, 6);
			station->tx.rate = default_unicast_rate; //Default tx_rate for this station. Rate adaptation may change this value.
			station->AID = 0;

			//Find the minimum AID that can be issued to this station.
			curr_station_info = (station_info*)(assoc_tbl->first);
			for( i = 0 ; i < assoc_tbl->length ; i++ ){
				if( (curr_station_info->AID - curr_AID) > 1 ){
					//There is a hole in the association table and we can re-issue
					//a previously issued AID.
					station->AID = curr_station_info->AID - 1;

					//Add this station into the association table just before the curr_station_info
					dl_node_insertBefore(assoc_tbl, &(curr_station_info->node), &(station->node));

					break;
				} else {
					curr_AID = curr_station_info->AID;
				}

				curr_station_info = station_info_next(curr_station_info);
			}

			if(station->AID == 0){
				//There was no hole in the association table, so we just issue a new
				//AID larger than the last AID in the table.

				if(assoc_tbl->length == 0){
					//This is the first entry in the association table;
					station->AID = 1;
				} else {
					curr_station_info = (station_info*)(assoc_tbl->last);
					station->AID = (curr_station_info->AID)+1;
				}


				//Add this station into the association table at the end
				dl_node_insertEnd(assoc_tbl, &(station->node));
			}

			print_associations(assoc_tbl);
//			wlan_mac_high_write_hex_display(assoc_tbl->length);

			return station;
		} else {
			return NULL;
		}


	}
}

statistics* add_statistics(dl_list* stat_tbl, station_info* station, u8* addr){
	u32 i;
	statistics* station_stats = NULL;
	statistics* curr_statistics = NULL;
	statistics* oldest_statistics = NULL;

	if(station == NULL){
#ifndef ALLOW_PROMISC_STATISTICS
		//This statistics struct isn't being added to an associated station. Furthermore,
		//Promiscuous statistics are now allowed, so we will return NULL to the calling function.
		return NULL;
#endif
	}

	station_stats = wlan_mac_high_find_statistics_ADDR(stat_tbl, addr);

	if(station_stats == NULL){
		//Note: This memory allocation has no corresponding free. It is by definition a memory leak.
		//The reason for this is that it allows the node to monitor statistics on surrounding devices.
		//In a busy environment, this promiscuous statistics gathering can be disabled by commenting
		//out the ALLOW_PROMISC_STATISTICS.

		if(stat_tbl->length >= MAX_NUM_PROMISC_STATS){
			//There are too many statistics being tracked. We'll get rid of the oldest that isn't currently associated.
			curr_statistics = (statistics*)(stat_tbl->first);
			for(i=0; i<stat_tbl->length; i++){

				if( (oldest_statistics == NULL) ){
					if(curr_statistics->is_associated == 0){
						oldest_statistics = curr_statistics;
					}
				} else if(( (curr_statistics->last_timestamp) < (oldest_statistics->last_timestamp)) ){
					if(curr_statistics->is_associated == 0){
						oldest_statistics = curr_statistics;
					}
				}
				curr_statistics = statistics_next(curr_statistics);
			}

			if(oldest_statistics == NULL){
				xil_printf("Error: could not find deletable oldest statistics. Ensure that MAX_NUM_PROMISC_STATS > MAX_NUM_ASSOC\n");
				xil_printf("if using ALLOW_PROMISC_STATISTICS\n");
			} else {
				dl_node_remove(stat_tbl, &(oldest_statistics->node));
				wlan_mac_high_free(oldest_statistics);
			}
		}

		station_stats = wlan_mac_high_calloc(sizeof(statistics));
		memcpy(station_stats->addr, addr, 6);
		dl_node_insertEnd(&statistics_table, &(station_stats->node));

	}
	if(station != NULL){
		station->stats = station_stats;
	}

	return station_stats;

}

int remove_association(dl_list* assoc_tbl, dl_list* stat_tbl, u8* addr){
	station_info* station;

	station = wlan_mac_high_find_station_info_ADDR(assoc_tbl, addr);
	if(station == NULL){
		//This addr doesn't refer to any station currently in the association table,
		//so there is nothing to remove. We'll return an error to let the calling
		//function know that something is wrong.
		return -1;
	} else {
		//Remove station from the association table;
		dl_node_remove(assoc_tbl, &(station->node));

#ifndef ALLOW_PROMISC_STATISTICS
		//Remove station's statistics from statististics table
		dl_node_remove(stat_tbl, &(station->stats->node));
		wlan_mac_high_free(station->stats);
#else
		station->stats->is_associated = 0;
#endif
		wlan_mac_high_free(station);
		print_associations(assoc_tbl);
//		wlan_mac_high_write_hex_display(assoc_tbl->length);
		return 0;
	}
}

u8 is_valid_association(dl_list* assoc_tbl, station_info* station){
	u32 i;
	station_info* curr_station_info;
	curr_station_info = (station_info*)(assoc_tbl->first);
	for(i=0; i < assoc_tbl->length; i++){
		if(station == curr_station_info){
			return 1;
		}
		curr_station_info = station_info_next(curr_station_info);
	}
	return 0;
}

#ifdef WARP_PC_INTERFACE_TEST
void print_stats() {
	xil_printf("Packets received on ethernet: %d\n", eth_rx);
	xil_printf("Packets queued for wireless tx: %d\n", wlan_tx);
	xil_printf("Total number of memory issues: %d\n", memory_issue_cnt);
}

void reset_stats() {
	eth_rx = 0;
	memory_issue_cnt = 0;
	wlan_tx = 0;
}

void send_test_packet_6() {
//	xil_printf("Adding bssid 0x40, 0xd8, 0x055, 0x04, 0x22, 0x84");
//	static u8 warp_data[] = {0x02, 0x01, 0x01, 0x40, 0xd8, 0x55, 0x04, 0x22, 0x84};
//	read_mac_control_header(warp_data, NULL);

	xil_printf("Adding bssid 0x40, 0xd8, 0x055, 0x04, 0x22, 0x86\n");
	static u8 warp_data1[] = {0x02, 0x04, 0x01, 0x40, 0xd8, 0x55, 0x04, 0x22, 0x86};
	u16 warp_length = sizeof(warp_data1);
	read_mac_control_header(warp_data1, &warp_length);
}

void send_test_packet_7() {
	xil_printf("Associating station 0x00, 0x21, 0x5d, 0x22, 0x97, 0x8c to slice 0\n");
	//                              1 station              BSSID                   ASSOCIATE              Station mac
	u8 warp_data[] = {0x02, 12,      1,     0x40, 0xd8, 0x55, 0x04, 0x22, 0x84,    1,       0x00, 0x21, 0x5d, 0x22, 0x97, 0x8c};
	read_bssid_control_header(warp_data, NULL);
}

void send_test_packet_8() {
	xil_printf("Associating station 0x00, 0x21, 0x5d, 0x22, 0x97, 0x8c to slice 1\n");
	//                              1 station              BSSID                   ASSOCIATE              Station mac
	u8 warp_data[] = {0x02, 12,      1,     0x40, 0xd8, 0x55, 0x04, 0x22, 0x86,    1,       0x00, 0x21, 0x5d, 0x22, 0x97, 0x8c};
	read_bssid_control_header(warp_data, NULL);
}

void send_test_packet_9() {
	xil_printf("Disassociating station 0x00, 0x21, 0x5d, 0x22, 0x97, 0x8c\n");
	//                              1 station              BSSID                   DISASSOCIATE              Station mac
	u8 warp_data[] = {0x02, 12,      1,     0x40, 0xd8, 0x55, 0x04, 0x22, 0x84,    32,       0x00, 0x21, 0x5d, 0x22, 0x97, 0x8c};
	xil_printf("Address main is %x\n", &association_table);
	read_bssid_control_header(warp_data, NULL);
//	u8 mac[] = {0x00, 0x21, 0x5d, 0x22, 0x97, 0x8c};
//	remove_association(&association_table, &statistics_table, mac);
}

void send_test_packet_0() {
	xil_printf("Disassociating station 0x00, 0x21, 0x5d, 0x22, 0x97, 0x8c\n");
	//                              1 station              BSSID                   DISASSOCIATE              Station mac
	u8 warp_data[] = {0x02, 12,      1,     0x40, 0xd8, 0x55, 0x04, 0x22, 0x86,    32,       0x00, 0x21, 0x5d, 0x22, 0x97, 0x8c};
	read_bssid_control_header(warp_data, NULL);
}
#endif
