/*****************************************************************************
 * @ingroup WDT
 *
 * @file iwdt.h
 *
 * @description
 *   This module contains the watchdog timer driver code.
 *
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Embedded.L.1.0.3-144
 ****************************************************************************/

#ifndef WDT_H
#define WDT_H

#include <linux/ioctl.h>
#include <linux/watchdog.h>

/* Debug print macro */
#ifdef DEBUG
#define WDT_PRINTK(arg...) printk(KERN_INFO arg)
#else
#define WDT_PRINTK(arg...)
#endif

#define  PCI_DEVICE_ID_INTEL_E3100    0x2670
#define  PCI_DEVICE_ID_INTEL_EP80579  0x5031

/* WDT I/O Registers */
#define WDT_PRELD1_REG0   0x00   /* Pre load1 register */
#define WDT_PRELD1_REG1   0x01
#define WDT_PRELD1_REG2   0x02
#define WDT_PRELD2_REG0   0x04   /* Pre load 2 register */
#define WDT_PRELD2_REG1   0x05
#define WDT_PRELD2_REG2   0x06
#define WDT_INT_STS_REG   0x08   /* Interrupt status register */
#define WDT_RLD_REG0    0x0C   /* Reload register */
#define WDT_RLD_REG1    0x0D
#define WDT_CONFIG_REG    0x10   /* Configuration register */
#define WDT_DOWN_CNT_REG0 0x14   /* Down count register */
#define WDT_DOWN_CNT_REG1 0x15
#define WDT_DOWN_CNT_REG2 0x16
#define WDT_LOCK_REG    0x18   /* Lock register */

/* Masks and values for registers */
#define WDT_LOCK        0x01
#define WDT_ENABLE      0x02
#define WDT_MASK        0xfd
#define MODE_MASK       0xfb
#define MODE_TEST       0x04
#define OPIN_MASK       0x20

#define WDIOF_SETMODE     0x0100
#define WDIOF_SETTIMEOUT2 0x0200

#define TIMER_MARGIN    60000

/* Watchdog timer ioctls from 0-7 are defined in watchdog.h */
#define WDIOC_SETTIMEOUT2   _IOWR(WATCHDOG_IOCTL_BASE, 20,  unsigned long)
#define WDIOC_SETMODE     _IOWR(WATCHDOG_IOCTL_BASE, 21,  unsigned long)
#define WDIOC_GETTIMEOUT2   _IOR(WATCHDOG_IOCTL_BASE,  22, unsigned long)
#define WDIOC_SETSCALE    _IOWR(WATCHDOG_IOCTL_BASE, 23, unsigned long)
#define WDIOC_LOCK        _IOWR(WATCHDOG_IOCTL_BASE, 24, unsigned long)
#define WDIOC_NOTIFY      _IOWR(WATCHDOG_IOCTL_BASE, 25, unsigned long)
#define WDIOC_ENABLE      _IOWR(WATCHDOG_IOCTL_BASE, 26, unsigned long)
#define WDIOC_DISABLE     _IOWR(WATCHDOG_IOCTL_BASE, 27, unsigned long)
#define WDIOC_GETSCALE    _IOR(WATCHDOG_IOCTL_BASE,  28, unsigned long)
#define WDIOC_GETMODE     _IOR(WATCHDOG_IOCTL_BASE,  29, unsigned long)
#define WDIOC_GETIRQSTAT    _IOR(WATCHDOG_IOCTL_BASE,  30, unsigned long)
#define WDIOC_CLRIRQSTAT    _IOWR(WATCHDOG_IOCTL_BASE, 31, unsigned long)
#define WDIOC_SETOPIN     _IOWR(WATCHDOG_IOCTL_BASE, 32, unsigned long)
#define WDIOC_GETOPIN     _IOR(WATCHDOG_IOCTL_BASE,  33, unsigned long)
#define WDIOC_GETDC       _IOR(WATCHDOG_IOCTL_BASE,  34, unsigned long)
#define WDIOC_GETIRQVEC     _IOR(WATCHDOG_IOCTL_BASE,  35, unsigned long)
#define WDIOC_CLRSTATUS     _IOR(WATCHDOG_IOCTL_BASE,  36, unsigned long)
#define WDIOC_CLRNOTIFY     _IOWR(WATCHDOG_IOCTL_BASE, 37, unsigned long)
#define WDIOC_GETWDTENABLE  _IOR(WATCHDOG_IOCTL_BASE,  38, unsigned long)
#define WDIOC_SET_INT_TYPE  _IOWR(WATCHDOG_IOCTL_BASE, 39, unsigned long)
#define WDIOC_GET_INT_TYPE  _IOR(WATCHDOG_IOCTL_BASE,  40, unsigned long)


#define WQ_ACTIVE     1
#define WQ_NOT_ACTIVE 0

#define INT_8 8
#define INT_16 16
#define VERSION_NUM 0.1
#define MODULE_VERION 08000
#define MAX_INTEGER_1 1048575
#define INT_0600 0600
#define INT_12 12


/* Enumerated values */
/* WDT device modes of operation */
enum {
    WDT_MODE,  /* 0 */
    FREE_MODE, /* 1 */
};

/* Timeout value granulaties */
enum {
    HIGH_SCALE, /* 0 */
    LOW_SCALE,  /* 1 */
};

/* Out PIN enable or disable */
enum {
    OPIN0_EN,   /* 0 */
    OPIN1_DIS,  /* 1 */
};

/* Available interrupt types after 1st stage timeout */
enum {
    WDT_INT_TYPE_IRQ, /* 0 - SERIRQ default */
    WDT_INT_TYPE_NMI, /* 1 - NMI        */
    WDT_INT_TYPE_SMI, /* 2 - SMI        */
    WDT_INT_TYPE_DIS, /* 3 - Disabled     */
};
#endif
/*----------------------------------------------------------------------------
 * File Revision History
 * $Id: iwdt.h,v 1.1 2009-11-26 07:37:29 gerg Exp $
 * $Source: /cvs/sw/linux-3.x/drivers/watchdog/iwdt.h,v $
 *----------------------------------------------------------------------------
 */
