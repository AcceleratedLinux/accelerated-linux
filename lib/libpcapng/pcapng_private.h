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

#ifndef _PCAPNG_PRIVATE_H_
#define _PCAPNG_PRIVATE_H_

#define PCAPNG_SECTION_BLOCK 0x0a0d0d0a
#define PCAPNG_INTERFACE_BLOCK 0x00000001
#define PCAPNG_SIMPLE_PACKET_BLOCK 0x00000003
#define PCAPNG_NAME_RESOLUTION_BLOCK 0x00000004
#define PCAPNG_INTERFACE_STATS_BLOCK 0x00000005
#define PCAPNG_ENHANCED_PACKET_BLOCK 0x00000006

#define PCAPNG_MAGIC_NUMBER 0x1a2b3c4d

#define PCAPNG_MAJOR_NUMBER 0x0001
#define PCAPNG_MINOR_NUMBER 0x0000


/* Some pre-canned lengths */
#define TIME_RES_OPTION_LENGTH (sizeof(block_option_t) + sizeof(uint32_t))
#define EPB_FLAGS_LENGTH (sizeof(block_option_t) + sizeof(uint32_t))
#define END_OF_OPTIONS_LENGTH sizeof(block_option_t)

#include <stdint.h>

#define END_OF_OPTIONS  0

/* Interface Options */
enum {
	IF_NAME = 2,
	IF_DESCRIPTION = 3,
	IF_IPv4_ADDR = 4,
	IF_IPv6_ADDR = 5,
	IF_MAC_ADDR = 6,
	IF_EUI_ADDR = 7,
	IF_SPEED = 8,
	IF_TSRESOL = 9,
	IF_TZONE = 10,
	IF_FILTER = 11,
	IF_OS = 12,
	IF_FCS_LEN = 13,
	IF_TSOFFET = 14
};

/* Enhanced Packet Block Options */
enum {
	EPB_FLAGS = 2,
	EPB_HASH  = 3,
	EPB_DROPCOUNT = 4
};


#define EPB_DIRECTION_MASK 0x000000FF


enum epb_reception_type {
	EPB_RECEPTION_NOT_SPECIFIED = 0x00,
	EPB_RECEPTION_UNICAST = 0x01,
	EPB_RECEPTION_MULTICAST = 0x02,
	EPB_RECEPTION_BROADCAST = 0x03
};


#define USECS_PER_SEC 1000000L

#define ROUND_UP_TO_32_BIT_BOUNDARY(length)  ((((length) % 4) == 0) ? (length) : ((length) + (4 - ((length) % 4))))


#endif /* _PCAPNG_PRIVATE_H_ */
