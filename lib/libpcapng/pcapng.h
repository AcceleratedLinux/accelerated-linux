/*****************************************************************************
 * Copyright (c) 2019 Digi International Inc., All Rights Reserved
 *
 * This software contains proprietary and confidential information of Digi
 * International Inc.  By accepting transfer of this copy, Recipient agrees
 * to retain this software in confidence, to prevent disclosure to others,
 * and to make no use of this software other than that for which it was
 * delivered.  This is an unpublished copyrighted work of Digi International
 * Inc.  Except as permitted by federal law, 17 USC 117, copying is strictly
 * prohibited.
 *
 * Restricted Rights Legend
 *
 * Use, duplication, or disclosure by the Government is subject to
 * restrictions set forth in sub-paragraph (c)(1)(ii) of The Rights in
 * Technical Data and Computer Software clause at DFARS 252.227-7031 or
 * subparagraphs (c)(1) and (2) of the Commercial Computer Software -
 * Restricted Rights at 48 CFR 52.227-19, as applicable.
 *
 * Digi International 9350 Excelsior Blvd. Suite 700 Hopkins, MN 55343
 *
 *****************************************************************************/

#ifndef _PCAPNG_H_
#define _PCAPNG_H_

#include <pcap.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_PCAPNG_INTERFACES    10

/**
 * Enhanced Packet Block (EPB) Direction
 */
typedef enum epb_direction
{
	EPB_UNKNOWN  = 0x00,
	EPB_INBOUND  = 0x01,
	EPB_OUTBOUND = 0x02
} epb_direction_t;


typedef struct
{
	uint32_t datalink_type;
	uint32_t packet_length;
	uint32_t captured_length;
	uint8_t * packet_data;
	char * if_name;
	struct timeval timestamp;
	epb_direction_t direction;
} pcapng_packet_t;

typedef struct file_st file_st;

int pcapng_fp_read(FILE * const fp, void (* packet_cb)(pcapng_packet_t * packet, void * const user_context), void * const user_context);
int pcapng_file_read(char const * const filename, void (* packet_cb)(pcapng_packet_t * const packet, void * const user_context), void * const user_context);

/**
 * Opens a file either truncated or for appending.
 * Section, interface and then enhanced packet blocks can then be written to the file.
 *
 * Returns
 *   File descriptor if the file is opened successfully.
 *   NULL if an error occurs or the max file size limit is reached. errno
 *   is set accordingly. If the max file size is reached, errno is EFBIG.
 */
file_st * pcapng_file_open(char const * const filename, bool const append_to_file);

void pcapng_file_close(file_st * const file);

bool pcapng_file_size_exceeds_limit(file_st const * const file, long const limit);

int pcapng_file_truncate(file_st * const file, off_t length);

bool pcapng_write_section_block(file_st * const file);

int pcapng_write_interface_block(file_st * const file,
						  char const * const ifname,
						  uint16_t const link_type,
						  uint32_t const snap_length);

int pcapng_write_enhanced_packet_block(file_st * const file,
								uint8_t const * const data,
								struct pcap_pkthdr const * const phdr,
								int const direction,
								int const if_index);

#endif /* _PCAPNG_ */
