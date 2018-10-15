/*
 * arch/arm/plat-orion/include/plat/mv_cesa.h
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef __PLAT_MV_CESA_H
#define __PLAT_MV_CESA_H

#include <linux/mbus.h>

struct mv_cesa_platform_data {
	struct mbus_dram_target_info	*dram;
};


#endif
