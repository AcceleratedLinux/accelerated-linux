/*****************************************************************************
 * @ingroup WDT
 *
 * @file iwdt.c
 *
 * @description
 *   This module contains the watchdog timer driver code.
 *
 * <DEFAULT_LICENSE_BEGIN>
 *
 * Copyright (c) 2004 - 2005 Intel Corporation.
 * All rights reserved.
 *
 * The license is provided to Recipient and Recipient's Licensees under the
 * following terms.
 *
 * Redistribution and use in source and binary forms of the Software, with or
 * without modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code of the Software may retain the above
 * copyright notice, this list of conditions and the following disclaimer
 * Redistributions in binary form of the Software may reproduce the above
 * copyright notice, this list of conditions and the following disclaimer in
 * the documentation and/or materials provided with the distribution.
 *
 * Neither the name of Intel Corporation nor the names of its contributors
 * shall be used to endorse or promote products derived from this Software
 * without specific prior written permission.
 *
 * Intel hereby grants Recipient and Licensees a non-exclusive, worldwide,
 * royalty-free patent license under Licensed Patents to make, use, sell, offer
 * to sell, import and otherwise transfer the Software, if any, in source code
 * and object code form. This license shall include changes to the Software
 * that are error corrections or other minor changes to the Software that do
 * not add functionality or features when the Software is incorporated in any
 * version of an operating system that has been distributed under the GNU
 * General Public License 2.0 or later. This patent license shall apply to the
 * combination of the Software and any operating system licensed under the GNU
 * General Public License 2.0 or later if, at the time Intel provides the
 * Software to Recipient, such addition of the Software to the then publicly
 * available versions of such operating systems available under the GNU General
 * Public License 2.0 or later (whether in gold, beta or alpha form) causes
 * such combination to be covered by the Licensed Patents. The patent license
 * shall not apply to any other combinations which include the Software. NO
 * hardware per se is licensed hereunder.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MECHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR IT CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * ANY LOSS OF USE; DATA, OR PROFITS; OR BUSINESS INTERUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * <DEFAULT_LICENSE_END>
 ****************************************************************************/
 /*
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
 */


#ifndef __KERNEL__
#define __KERNEL__
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/reboot.h>
#include <linux/ioport.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/watchdog.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include "iwdt.h"

MODULE_AUTHOR("Intel(R) Corporation");
MODULE_DESCRIPTION("Watchdog Timer Driver");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("1.0.1");


static u16 wdt_iobase;  /* IO Base address for WDT device */
static u32 wdt_margin1 = TIMER_MARGIN;
static u32 wdt_margin2 = TIMER_MARGIN;
static u8  wdt_mode = WDT_MODE;
static u8  wdt_scale = HIGH_SCALE;
static u8  wdt_intr_type = WDT_INT_TYPE_IRQ;
static u8  wdt_opin_mode = OPIN0_EN;
static u8  wdt_state = 0;
static u8  wdt_count = 0;
static u8  wdt_lk = 0;
static int nowayout = 0;
static struct pci_dev *lpc_pci;
static int wdt_irq;
static spinlock_t wdt_lock;
static DECLARE_WAIT_QUEUE_HEAD(wdt_wait_queue);
static int wdt_wq_active = WQ_NOT_ACTIVE;
static char wdt_expect_close = 0;
/*
 * SIW Configuration port addresses.
 * WDT timer is a logical device in SIW device.
 * SIW has two sets of configuration addresses based on the BIOS strap pin:
 *     1. 0x4E  and 0x4F or
 *     2. 0x20E and 0x20F
 */
static int wdt_index_port = 0x4E;
static int wdt_data_port = 0x4F;

module_param(wdt_mode, byte, 0);
MODULE_PARM_DESC(wdt_mode, "Intel Watchdog timer mode (default WDT mode)");

module_param(wdt_scale, byte, 0);
MODULE_PARM_DESC(wdt_scale, "Intel WDT scale (default in steps of 1 ms).");

module_param(wdt_intr_type, byte, 0);
MODULE_PARM_DESC(wdt_intr_type, "Intel WDT interrupt type (default SERIRQ).");

module_param(wdt_margin1, uint, 0);
MODULE_PARM_DESC(wdt_margin1, "First stage Intel WDT timeout in steps of 1 ms by default.");

module_param(wdt_margin2, uint, 0);
MODULE_PARM_DESC(wdt_margin2, "Second stage Intel WDT timeout in steps of 1 ms by default.");

module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout, "Intel WDT can't be stopped once started (default=0)");

module_param(wdt_index_port, int, 0);
MODULE_PARM_DESC(wdt_index_port, "WDT Index Port (default 0x4e)");

module_param(wdt_data_port, int, 0);
MODULE_PARM_DESC(wdt_data_port, "WDT Data Port (default 0x4f)");

static int wdt_get_iobase(struct pci_dev *dev, u16 *iobase, int *irq);
static u8 wdt_read_byte(int reg);
static void wdt_write_byte(int reg, u8 value);
static int wdt_get_mode(void);
static int wdt_set_mode(u8 mode);
static int wdt_get_scale(void);
static int wdt_set_scale(u8 scale);
static int wdt_enable(void);
static int wdt_disable(void);
static int wdt_getenable(void);
static int wdt_set_lk(void);
static int wdt_set_timeout1(u32 val);
static int wdt_set_timeout2(u32 val);
static void wdt_reload(void);
static int wdt_clear_toutsts(void);
static int wdt_sys_status(void);
static int wdt_set_hw_default(void);
static int wdt_open(struct inode *inode, struct file *file);
static int wdt_release(struct inode *inode, struct file *file);
static ssize_t wdt_write(struct file *file, const char *data,
                     size_t count, loff_t * pos);
static int wdt_ioctl(struct inode *inode, struct file *file,
                    unsigned int cmd, unsigned long arg);
static irqreturn_t wdt_isr(int irq, void *dev_id);
static void __exit wdt_cleanup(void);
static int __init wdt_init(void);
static int __init wdt_init_one(struct pci_dev *dev,
                          const struct pci_device_id *ent);
static void __devexit wdt_remove_one(struct pci_dev *pdev);
static int wdt_pci_suspend(struct pci_dev *dev, pm_message_t state);
static int wdt_pci_resume(struct pci_dev *dev);
static int wdt_dis_opin(void);
static int wdt_en_opin(void);
static int wdt_get_irqstat(void);
static int wdt_clr_irqstat(void);
static u32 wdt_get_dc(void);

static struct pci_device_id lpc_pci_tbl[] __initdata = {
    {PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_E3100, PCI_ANY_ID, PCI_ANY_ID,},
    {PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_INTEL_EP80579,
             PCI_ANY_ID, PCI_ANY_ID, },
    {0, },
};

static struct file_operations wdt_fops = {
     owner:        THIS_MODULE,
     write:        wdt_write,
     ioctl:        wdt_ioctl,
     open:         wdt_open,
     release:      wdt_release,
};
static struct miscdevice wdt_miscdev = {
     minor:        WATCHDOG_MINOR,
     name:        "watchdog",
     fops:        &wdt_fops,
};
static struct pci_driver wdt_driver = {
     name:        "iwdt",
     id_table:     lpc_pci_tbl,
     probe:        wdt_init_one,
     remove:       wdt_remove_one,
     suspend:      wdt_pci_suspend,
     resume:       wdt_pci_resume,
};

/*
 * Function Name:   wdt_get_iobase()
 * Parameter:    dev    - dev pointer to lpc pci device
 *           iobase - iobase for WDT lpc device
 *           irq    - irq for WDT device
 * Return:       0 - succesful, negative value - failed.
 * Description:     This function is to get watch dog timer base
 *           address and irq number.
 */
static int wdt_get_iobase(struct pci_dev *dev, u16 *iobase, int *wirq)
{
     u8 device_id = 0, device_rev = 0, prev_device_id = 0;
     int irq = 0;

     /* Getting iobase is a multi-step process:
    *   1. SIW needs to be in configuration state:
    *    To enter SIW into configuration state
    *       a) write 0x80 into wdt_index_port
    *       b) write 0x86 into wdt_index_port
    *   2. SIW can be in two modes after entering into Configuration state.
    *       a) Configuration Mode: When SIW is in this mode,
    *         logical device configuration registers are accessible.
    *       b) Run Mode: When SIW is in this mode, global configuration
    *         registers are accessable.
    *   3. SIW has different logical devices. Inorder to access WDT device
    *     specific configuration registers:
    *       a) Write the WDT logical device number 0x06 into register 0x07.
    *   4. Once writing WDT logical device number into 0x07, read WDT
    *     configuration registers 0x60(MSB) and 0x61(LSB) to get the iobase
    *     address for WDT.
    *   5. To exit SIW from configuration state
    *      a) write 0x68 into wdt_index_port
    *      b) write 0x08 into wdt_index_port
    */

     /* Enter into configuration state */
     outb(0x80, wdt_index_port);
     outb(0x86, wdt_index_port);

     /* Access global registers */
     outb(0x20, wdt_index_port);
     device_id = inb(wdt_data_port);
     outb(0x21, wdt_index_port);
     device_rev = inb(wdt_index_port);
     WDT_PRINTK("iwdt: SIW device_id = 0x%x device_rev = 0x%x\n",
        device_id, device_rev);

     /* Before entering into the configuration mode,
    * read who is using the SIW */
     outb(0x07, wdt_index_port);
     prev_device_id = inb(wdt_data_port);
     WDT_PRINTK("iwdt: Prev logical device id = %d\n", prev_device_id);

     /* Enter into configuration mode */
     outb(0x07, wdt_index_port);
     outb(0x06, wdt_data_port);

     /* Read iobase address */
     outb(0x60, wdt_index_port);
     *iobase = (inb(wdt_data_port) << INT_8);
     outb(0x61, wdt_index_port);
     *iobase += inb(wdt_data_port);

     /* Read primary interrupt select */
     outb(0x70, wdt_index_port);
     irq = inb(wdt_data_port);

     WDT_PRINTK("iwdt: iobase = 0x%x irq = %d\n", *iobase, irq);
     *wirq = irq;

     /* Exit from configuration state */
     outb(0x68, wdt_index_port);
     outb(0x08, wdt_index_port);
     return 0;
}

/*---------------------------------------------------------------
 * Functions to read, write WDT IO registers
 *---------------------------------------------------------------
 */
/*
 * Function Name:   wdt_read_byte()
 * Parameter:    reg   - wdt register offset to read.
 * Return:       value - value read from the register
 * Description:     This function is to read a byte from wdt register
 *           using port IO.
 */
static u8 wdt_read_byte(int reg)
{
     return inb(wdt_iobase+reg);
}

/*
 * Function Name:   wdt_write_byte()
 * Parameter:    reg   - wdt register offset to write.
 *           value - value to write into the register
 * Return:       None.
 * Description:     This function is to write a byte into wdt register
 *           using port IO.
 */
static void wdt_write_byte(int reg, u8 value)
{
     outb(value, wdt_iobase+reg);
     return;
}

/*
 * Function Name:   wdt_get_mode()
 * Parameter:    None.
 * Return:       Mode value - WDT_MODE or FREE_MODE (free running mode)
 * Description:     This function is used to get current mode.
 */
static int wdt_get_mode(void)
{
     u8 val;
     WDT_PRINTK("iwdt: Enter wdt_get_mode()\n");
     spin_lock(&wdt_lock);
     val = wdt_read_byte(WDT_LOCK_REG);
     spin_unlock(&wdt_lock);
     if ((val &= MODE_TEST) == 0) {
        return WDT_MODE;
     }
     return FREE_MODE;
}

/*
 * Function Name:   wdt_get_dc()
 * Parameter:    None.
 * Return:       ulong dcount
 * Description:     returns upper 20bits of 35bit downcounter.
 */
static u32 wdt_get_dc(void)
{
     u32 val;
     u8 val1;
     WDT_PRINTK("iwdt: Enter wdt_get_dc()\n");
     spin_lock(&wdt_lock);
     val1 = wdt_read_byte(WDT_DOWN_CNT_REG0);
     val = val1;
     val1 = wdt_read_byte(WDT_DOWN_CNT_REG1);
     val += (val1 << INT_8);
     val1 = wdt_read_byte(WDT_DOWN_CNT_REG2);
     val += (val1 << INT_16);
     spin_unlock(&wdt_lock);
     return val;
}

/*
 * Function Name:   wdt_get_opin()
 * Parameter:    None.
 * Return:       OPIN0_EN - Enabled OPIN1_DIS - Disabled
 * Description:     This function is used to get state if WDT_OUTPUT bit.
 */
static int wdt_get_opin(void)
{
     u8 val;
     WDT_PRINTK("iwdt: Enter wdt_get_opin()\n");
     spin_lock(&wdt_lock);
     val = wdt_read_byte(WDT_CONFIG_REG);
     spin_unlock(&wdt_lock);
     if ((val &= OPIN_MASK) == 0) {
        return OPIN0_EN;
     }
     return OPIN1_DIS;
}

/*
 * Function Name:   wdt_dis_opin()
 * Parameter:    None.
 * Return:       Return 0 as success
 * Description:     Set WDT_OUTPUT bit 5 in WDT_CONFIG register
 *           WDT_OUTPUT = 1 disables WDT_OUTPUT pin...
 *           prevents resets from occurring
 */
static int wdt_dis_opin(void)
{
     u8 val;
     WDT_PRINTK("iwdt: Enter wdt_dis_opin()\n");
     spin_lock(&wdt_lock);
     val = wdt_read_byte(WDT_CONFIG_REG);
     val |= 0x20;
     wdt_write_byte(WDT_CONFIG_REG, val);
     spin_unlock(&wdt_lock);
     return 0;
}

/*
 * Function Name:   wdt_en_opin()
 * Parameter:    None.
 * Return:       Return 0 as success
 * Description:     Clears WDT_OUTPUT bit 5 in WDT_CONFIG register
 *           WDT_OUTPUT = 0 enables WDT_OUTPUT pin...
 *           enables output pin to toggle
 */
static int wdt_en_opin(void)
{
     u8 val;

     WDT_PRINTK("iwdt: Enter wdt_en_opin()\n");
     spin_lock(&wdt_lock);
     val = wdt_read_byte(WDT_CONFIG_REG);
     val &= 0xdf;
     wdt_write_byte(WDT_CONFIG_REG, val);
     spin_unlock(&wdt_lock);
     return 0;
}

/*
 * Function Name:   wdt_set_mode()
 * Parameter:    The mode value to be set: 0 - WDT, 1 - free running.
 * Return:       0 - succesful, negative value - failed.
 * Description:     This function is used to set current mode.
 */
static int wdt_set_mode(u8 mode)
{
     u8 val;

     WDT_PRINTK("iwdt: Enter wdt_set_mode(): mode = %u\n", mode);

     switch (mode) {
     case WDT_MODE:
        spin_lock(&wdt_lock);
        val = wdt_read_byte(WDT_LOCK_REG);
        val &= MODE_MASK;
        wdt_write_byte(WDT_LOCK_REG, val);
        spin_unlock(&wdt_lock);
        if (wdt_get_mode() == WDT_MODE) {
           wdt_mode = WDT_MODE;
           return 0;
        } else {
           return -1;
        }
     case FREE_MODE:
        spin_lock(&wdt_lock);
        val = wdt_read_byte(WDT_LOCK_REG);
        val |= ~MODE_MASK;
        wdt_write_byte(WDT_LOCK_REG, val);
        spin_unlock(&wdt_lock);
        if (wdt_get_mode() == FREE_MODE) {
           wdt_mode = FREE_MODE;
           return 0;
        } else {
           return -1;
        }
     default:
        return -EINVAL;
     }
}

/*
 * Function Name:   wdt_set_opin()
 * Parameter:    The mode value to be set:
                    0 - enabled,
                    1 - disabled (backass backward in hw!)
 * Return:       0 - succesful, negative value - failed.
 * Description:     This function is used to en/dis WDT_OUTPUT pin
 */
static int wdt_set_opin(u8 mode)
{
     WDT_PRINTK("iwdt: Enter wdt_set_opin(): mode = %u\n", mode);
     switch (mode) {
     case OPIN0_EN:
        wdt_en_opin();
        wdt_opin_mode = OPIN0_EN;
        return 0;
     case OPIN1_DIS:
        wdt_dis_opin();
        wdt_opin_mode = OPIN1_DIS;
        return 0;
     default:
        return -EINVAL;
     }
}

/*
 * Function Name:   wdt_get_scale()
 * Parameter:    none.
 * Return:       Current scale.
 * Description:     This function is used to get scale.
 */
static int wdt_get_scale(void)
{
     u8 val;
     WDT_PRINTK("iwdt: Enter wdt_get_scale()\n");
     spin_lock(&wdt_lock);
     val = wdt_read_byte(WDT_CONFIG_REG);
     spin_unlock(&wdt_lock);
     if ((val &= MODE_TEST) == 0) {
        return HIGH_SCALE;
     }
     return LOW_SCALE;
}

/*
 * Function Name:   wdt_set_scale()
 * Parameter:    The scale value to be set.
 * Return:       0 - successful, negative value -  failed.
 * Description:     This function is used to set scale.
 */
static int wdt_set_scale(u8 scale)
{
     u8 val;

     WDT_PRINTK("iwdt: Enter wdt_set_scale(): scale = %u\n", scale);

     switch (scale) {
     case HIGH_SCALE:
        spin_lock(&wdt_lock);
        val = wdt_read_byte(WDT_CONFIG_REG);
        val &= MODE_MASK;
        wdt_write_byte(WDT_CONFIG_REG, val);
        spin_unlock(&wdt_lock);
        if (wdt_get_scale() == HIGH_SCALE) {
           wdt_scale = HIGH_SCALE;
           return 0;
        } else {
           return -1;
        }
     case  LOW_SCALE:
        spin_lock(&wdt_lock);
        val = wdt_read_byte(WDT_CONFIG_REG);
        val |= ~MODE_MASK;
        wdt_write_byte(WDT_CONFIG_REG, val);
        spin_unlock(&wdt_lock);
        if (wdt_get_scale() ==  LOW_SCALE) {
           wdt_scale =  LOW_SCALE;
           return 0;
        } else {
           return -1;
        }
     default:
        return -EINVAL;
     }
}

/*
 * Function Name:   wdt_enable()
 * Parameter:    none.
 * Return:       0 - enable successful, -1 - failed.
 * Description:     This function is used to enable WDT.
 */
static int wdt_enable(void)
{
     u8 val;
     WDT_PRINTK("iwdt: Enter wdt_enable()\n");
     spin_lock(&wdt_lock);
     val = wdt_read_byte(WDT_LOCK_REG);
     val |= WDT_ENABLE;
     wdt_write_byte(WDT_LOCK_REG, val);
     val = wdt_read_byte(WDT_LOCK_REG);
     spin_unlock(&wdt_lock);
     if ((val & WDT_ENABLE) == 0) {
        return -1;
     }
     wdt_state = val;
     return 0;
}

/*
 * Function Name:   wdt_disable()
 * Parameter:    none.
 * Return:       0 - enable successful, -1 - failed.
 * Description:     This function is used to disable WDT.
 */
static int wdt_disable(void)
{
     u8 val;
     WDT_PRINTK("iwdt: Enter wdt_disable()\n");
     spin_lock(&wdt_lock);
     val = wdt_read_byte(WDT_LOCK_REG);
     val &= WDT_MASK;
     wdt_write_byte(WDT_LOCK_REG, val);
     val = wdt_read_byte(WDT_LOCK_REG);
     spin_unlock(&wdt_lock);
     if ((val & WDT_ENABLE) == 0) {
        wdt_state = val;
        return 0;
     }
     return -1;
}

/*
 * Function Name:   wdt_getenable()
 * Parameter:    none.
 * Return:       state of WDT_ENABLE bit1 @ pci config offset 0x68h
 * Description:     This function is used to read state of WDT_ENABLE bit
 */
static int wdt_getenable(void)
{
     u8 val;
     WDT_PRINTK("iwdt: Enter wdt_getenable()\n");
     spin_lock(&wdt_lock);
     val = wdt_read_byte(WDT_LOCK_REG);
     spin_unlock(&wdt_lock);
     if ((val & WDT_ENABLE) == 0)    /* WDT_ENABLE =0x02 */ {
        return 0;
     }
     return 1;
}

/*
 * Function Name:   wdt_set_lk()
 * Parameter:    none.
 * Return:       0 - successful, -1 - failed.
 * Description:     This function is used to lock WDT.
 */
static int wdt_set_lk(void)
{
     u8 val;
     WDT_PRINTK("iwdt: Enter wdt_set_lk()\n");
     spin_lock(&wdt_lock);
     val = wdt_read_byte(WDT_LOCK_REG);
     val |= WDT_LOCK;
     wdt_write_byte(WDT_LOCK_REG, val);
     val = wdt_read_byte(WDT_LOCK_REG);
     spin_unlock(&wdt_lock);
     if ((val & WDT_LOCK) == 0) {
        return -1;
     }
     wdt_lk = WDT_LOCK;
     return 0;
}

/*
 * Function Name:   wdt_get_timeout1(void)
 * Parameter:    None.
 * Return:       Timeout value.
 * Description:     This function is used to get WDT first stage timeout.
 */
static u32 wdt_get_timeout1(void)
{
     u32 val;
     spin_lock(&wdt_lock);
     val = wdt_read_byte(WDT_PRELD1_REG2);
     val = (val << INT_8) | wdt_read_byte(WDT_PRELD1_REG1);
     val = (val << INT_8) | wdt_read_byte(WDT_PRELD1_REG0);
     spin_unlock(&wdt_lock);
     return val;
}

/*
 * Function Name:   wdt_set_timeout1()
 * Parameter:    Time out value to be set.
 * Return:       0 - successful, -1 - failed.
 * Description:     This function is used to set WDT first stage timeout.
 */
static int wdt_set_timeout1(u32 val)
{
     WDT_PRINTK("iwdt: Enter wdt_set_timeout1(): val = 0x%x\n", val);
     if (val < 1 || val > MAX_INTEGER_1) {
        return -EINVAL;
     }

     spin_lock(&wdt_lock);

     wdt_write_byte(WDT_RLD_REG0, 0x80);
     wdt_write_byte(WDT_RLD_REG0, 0x86);
     wdt_write_byte(WDT_PRELD1_REG0, (u8)val);

     wdt_write_byte(WDT_RLD_REG0, 0x80);
     wdt_write_byte(WDT_RLD_REG0, 0x86);
     wdt_write_byte(WDT_PRELD1_REG1, (u8)(val>>INT_8));

     wdt_write_byte(WDT_RLD_REG0, 0x80);
     wdt_write_byte(WDT_RLD_REG0, 0x86);
     wdt_write_byte(WDT_PRELD1_REG2, (u8)(val>>INT_16));

     spin_unlock(&wdt_lock);
     if ((wdt_get_timeout1() & 0x000fffff) == val) {
        return 0;
     }
     return -1;
}

/*
 * Function Name:   wdt_get_timeout2(void)
 * Parameter:    None.
 * Return:       Timeout value.
 * Description:     This function is used to get WDT second stage timeout.
 */
static u32 wdt_get_timeout2(void)
{
     u32 val;
     spin_lock(&wdt_lock);
     val = wdt_read_byte(WDT_PRELD2_REG2);
     val = (val << INT_8) | wdt_read_byte(WDT_PRELD2_REG1);
     val = (val << INT_8) | wdt_read_byte(WDT_PRELD2_REG0);
     spin_unlock(&wdt_lock);
     return val;
}

/*
 * Function Name:   wdt_set_timeout2()
 * Parameter:    Time out value to be set.
 * Return:       0 - successful, -1 - failed.
 * Description:     This function is used to set WDT second stage timeout.
 */
static int wdt_set_timeout2(u32 val)
{
     WDT_PRINTK("iwdt: Enter wdt_set_timeout2(): val = 0x%x\n", val);
     if (val < 1 || val > MAX_INTEGER_1) {
        return -EINVAL;
     }

     spin_lock(&wdt_lock);
     wdt_write_byte(WDT_RLD_REG0, 0x80);
     wdt_write_byte(WDT_RLD_REG0, 0x86);
     wdt_write_byte(WDT_PRELD2_REG0, (u8)(val));

     wdt_write_byte(WDT_RLD_REG0, 0x80);
     wdt_write_byte(WDT_RLD_REG0, 0x86);
     wdt_write_byte(WDT_PRELD2_REG1, (u8)(val>>INT_8));

     wdt_write_byte(WDT_RLD_REG0, 0x80);
     wdt_write_byte(WDT_RLD_REG0, 0x86);
     wdt_write_byte(WDT_PRELD2_REG2, (u8)(val>>INT_16));
     spin_unlock(&wdt_lock);
     if ((wdt_get_timeout2() & 0x000fffff) == val) {
        return 0;
     }
     return -1;
}

/*
 * Function Name:   wdt_reload()
 * Parameter:    None.
 * Return Value:    None.
 * Description:     This function is used to reset WDT.
 */
static void wdt_reload(void)
{
     WDT_PRINTK("iwdt: Enter wdt_reload()\n");
     spin_lock(&wdt_lock);
     wdt_write_byte(WDT_RLD_REG0, 0x80);
     wdt_write_byte(WDT_RLD_REG0, 0x86);

     wdt_write_byte(WDT_RLD_REG1, 0x01);
     spin_unlock(&wdt_lock);
     return;
}

/*
 * Function Name:   wdt_get_irqstat()
 * Parameter:    None.
 * Return:       IRQ status
 * Description:     This function is used to read IRQ status bit
 */
static int wdt_get_irqstat(void)
{
     u8 status;
     WDT_PRINTK("iwdt: Enter wdt_get_irqstat()\n");
     spin_lock(&wdt_lock);
     status = wdt_read_byte(WDT_INT_STS_REG);
     spin_unlock(&wdt_lock);
     if ((status & 0x07) == 0) {
        return 0;
     }
     return 1;
}

/*
 * Function Name:   wdt_clr_irqstat()
 * Parameter:    None.
 * Return:       IRQ status
 * Description:     This function is used to
 *           write 1 to clear IRQ Status bit in IRQ stat reg
 */
static int wdt_clr_irqstat(void)
{
     u8 val;
     WDT_PRINTK("iwdt: Enter wdt_clr_irqstat()\n");
     spin_lock(&wdt_lock);
     val = wdt_read_byte(WDT_INT_STS_REG);
     val |= 0x07;   /* clear interrupt status bits 0:1:2 */
     wdt_write_byte(WDT_INT_STS_REG, val);
     spin_unlock(&wdt_lock);
     return wdt_get_irqstat();
}

/*
 * Function Name:   wdt_clear_toutsts()
 * Parameter:    None.
 * Return Value:    state of WDT_TIMEOUT bit (0 noTO 1 TO)
 * Description:     This function is used to clear timeout bit.
 */
static int wdt_clear_toutsts(void)
{
     WDT_PRINTK("iwdt: Enter wdt_clear_toutsts()\n");
     spin_lock(&wdt_lock);
     wdt_write_byte(WDT_RLD_REG0, 0x80);
     wdt_write_byte(WDT_RLD_REG0, 0x86);
     wdt_write_byte(WDT_RLD_REG1, 0x02); /* clear TOUT (bit1) by writing 1 */
     spin_unlock(&wdt_lock);
     return wdt_sys_status();
}

/*
 * Function Name:   wdt_sys_status()
 * Parameter:    None.
 * Return:       0 if WDT_TIMEOUT=0 (sys is stable - no timeouts)
 *           1 if WDT_TIMEOUT=1 (sys is unstable - timeouts occured)
 * Description:     This function is used to get system status.
 */
static int wdt_sys_status(void)
{
     u8 status;
     WDT_PRINTK("iwdt: Enter wdt_sys_status()\n");
     spin_lock(&wdt_lock);
     status = wdt_read_byte(WDT_RLD_REG1);
     spin_unlock(&wdt_lock);
     if (status & 0x02) {
        return 1;
     }
     return 0;
}

/*
 * Function Name:   wdt_get_intr_type()
 * Parameter:    None.
 * Return:       Interrupt type
 * Description:     This function is used to get interrupt type.
 */
static int wdt_get_intr_type(void)
{
     u8 val;
     WDT_PRINTK("iwdt: Enter wdt_get_intr_type()\n");
     spin_lock(&wdt_lock);
     val = wdt_read_byte(WDT_CONFIG_REG);
     spin_unlock(&wdt_lock);
     val = val & 0x3;
     return val;
}

/*
 * Function Name:   wdt_set_intr_type()
 * Parameter:    interrupt type
 * Return:       0 - succesful, negative value - failed.
 * Description:     This function is used to set interrupt type.
 */
static int wdt_set_intr_type(u8 int_type)
{
     u8 val;

     WDT_PRINTK("iwdt: Enter wdt_set_intr_type(): int_type = %u\n", int_type);

     spin_lock(&wdt_lock);
     val = wdt_read_byte(WDT_CONFIG_REG);
     val = (val & 0xfc) | (int_type & 0x3);
     wdt_write_byte(WDT_CONFIG_REG, val);
     spin_unlock(&wdt_lock);
     if (wdt_get_intr_type() == int_type) {
        wdt_intr_type = int_type;
        return 0;
     } else {
        return -1;
     }
}

/*
 * Function Name:   wdt_set_hw_default()
 * Parameter:    None.
 * Return:       0 - successful, -1 - failed.
 * Description:     This function is used to set hw config at insmod time
 */

static int wdt_set_hw_default(void)
{
     int ret;
     WDT_PRINTK("iwdt: Enter wdt_set_hw_default()\n");

     /* STOP the WDT */
     ret = wdt_disable();
     if (ret) {
        WDT_PRINTK("iwdt: failed to disable wdt.\n");
        return -1;
     }

     /* clear all TOUT bits */
     wdt_clear_toutsts();

     /* Note: Hardware defaults to Interrupts Enabled (STUPID!!!)
    * ideally we should disable interrupts until after the ISR is installed
    * for now, assume hw is in power-on state and wdt is stopped
    */
     /* clear irq status */
     wdt_clr_irqstat();

     /* set default configurations in hardware */
     wdt_set_mode(wdt_mode);

     /* set the default scale value */
     wdt_set_scale(wdt_scale);

     /* set interrupt type */
     ret = wdt_set_intr_type(wdt_intr_type);
     if (ret) {
        WDT_PRINTK("iwdt: Failed to set %u interrupt type.\n",
           wdt_intr_type);
        return -1;
     }

     if (wdt_mode ==  FREE_MODE) {
        wdt_set_timeout2(wdt_margin2);
     } else {
        wdt_set_timeout1(wdt_margin1);
        wdt_set_timeout2(wdt_margin2);
     }

     /* enable wdt output pin */
     wdt_en_opin();

     /* Debug: Print WDT STATUS */
     WDT_PRINTK("iwdt: wdt_mode = %u\n", wdt_get_mode());
     WDT_PRINTK("iwdt: wdt_scale = %u\n", wdt_get_scale());
     WDT_PRINTK("iwdt: wdt_enable = %u\n", wdt_getenable());
     WDT_PRINTK("iwdt: wdt_irqstat = %u\n", wdt_get_irqstat());
     WDT_PRINTK("iwdt: wdt_dc = %u\n", wdt_get_dc());
     WDT_PRINTK("iwdt: wdt_opin = %u\n", wdt_get_opin());
     WDT_PRINTK("iwdt: wdt_timeout1 = %u\n", wdt_get_timeout1());
     WDT_PRINTK("iwdt: wdt_timeout2 = %u\n", wdt_get_timeout2());
     return 0;
}

/*
 * Function Name:   wdt_open()
 * Parameter:    struct inode *inode, struct file *file.
 * Return Value:    0 - successful, negative value - failed.
 * Description:     This function is used to open the device.
 */
static int wdt_open(struct inode *inode, struct file *file)
{
     WDT_PRINTK("iwdt: Enter wdt_open()\n");
     if (test_and_set_bit(0, (void *)&wdt_count)) {
        return -EBUSY;
     }

     if (wdt_getenable() == 0) { /* if STOPPED */
        wdt_enable();        /* start with insmod dflts */
     }                 /* or with last progd hw configs */

     /* if RUNNING, leave it alone...assume user/wdt app will
    * stop/config/restart as needed
    */
     return 0;
}

/*
 * Function Name:   wdt_release()
 * Parameter:    struct inode *inode, struct file *file.
 * Return Value:    0 - successful.
 * Description:     This function is used to release the device @ close
 */
static int wdt_release(struct inode *inode, struct file *file)
{
     WDT_PRINTK("iwdt: Enter wdt_release()\n");
     if (wdt_expect_close == 0x2A && !nowayout) {
        /* 'V' was written prior to close & nowayout=0
         * stop the wdt
         */
        wdt_disable();
     } else {
        /* 'V' wasn't written prior to close OR nowayout=1
         * leave wdt running
         */
        wdt_reload();
        printk("Intel wdt: unexpected close, not stopping watchdog!!\n");
     }
     /* stuff that gets done on all closes */
     clear_bit(0, (void *)&wdt_count);
     wdt_expect_close = 0;
     return 0;
}

/*
 * Function Name:   wdt_write()
 * Parameter:    struct file *file, const char *data, size_t count,
                 loff_t * pos.
 * Return Value:    count.
 * Description:     This function is used to open the device.
 */
static ssize_t wdt_write(struct file *file, const char *data, size_t count,
                         loff_t * pos)
{
     WDT_PRINTK("iwdt: Enter wdt_write()\n");
#if 0
     /* Can't seek (pwrite) on this device */
     if (pos != &file->f_pos) {
        return -ESPIPE;
     }
#endif

     /* reload on all writes
    * app writing magic char 'V' prior to closing /dev/watchdog stops wdt when
    * started with nowayout=1, otherwise wdt stays running after close
    */
     if (count) {
        size_t i;
        wdt_expect_close = 0;
        for (i = 0; i != count; i++) {
           u8 c='x';
           if (get_user(c, data+i)) {
              return -EFAULT;
           }
           if (c == 'V') {
              wdt_expect_close = 0x2A;
           }
        }
        wdt_reload();
        return 1;
     }
     return 0;
}

/*
 * Function Name:   wdt_ioctl_get_string()
 * Parameter:    cmd
 * Return Value:    ioctl string
 * Description:     This function is to get the ioctl string for the cmd
 */
char *wdt_get_ioctl_string(unsigned int cmd)
{
     char *ioctl_str = NULL;
     switch(cmd)
     {
     case WDIOC_GETSUPPORT:
        ioctl_str = "WDIOC_GET_SUPPORT";
        break;

     case WDIOC_GETSTATUS:
        ioctl_str = "WDIOC_GETSTATUS";
        break;

     case WDIOC_CLRSTATUS:
        ioctl_str = "WDIOC_CLRSTATUS";
        break;

     case WDIOC_GETBOOTSTATUS:
        ioctl_str = "WDIOC_GETBOOTSTATUS";
        break;

     case WDIOC_DISABLE:
        ioctl_str = "WDIOC_DISABLE";
        break;

     case WDIOC_ENABLE:
        ioctl_str = "WDIOC_ENABLE";
        break;

     case WDIOC_GETWDTENABLE:
        ioctl_str = "WDIOC_GETWDTENABLE";
        break;

     case WDIOC_SETMODE:
        ioctl_str = "WDIOC_SETMODE";
        break;

     case WDIOC_GETMODE:
        ioctl_str = "WDIOC_GETMODE";
        break;

     case WDIOC_SETOPIN:
        ioctl_str = "WDIOC_SETOPIN";
        break;

     case WDIOC_GETOPIN:
        ioctl_str = "WDIOC_GETOPIN";
        break;

     case WDIOC_GETIRQSTAT:
        ioctl_str = "WDIOC_GETIRQSTAT";
        break;

     case WDIOC_CLRIRQSTAT:
        ioctl_str = "WDIOC_CLRIRQSTAT";
        break;

     case WDIOC_SETSCALE:
        ioctl_str = "WDIOC_SETSCALE";
        break;

     case WDIOC_GETSCALE:
        ioctl_str = "WDIOC_GETSCALE";
        break;

     case WDIOC_LOCK:
        ioctl_str = "WDIOC_LOCK";
        break;

     case WDIOC_KEEPALIVE:
        ioctl_str = "WDIOC_KEEPALIVE";
        break;

     case WDIOC_SETTIMEOUT:
        ioctl_str = "WDIOC_SETTIMEOUT";
        break;

     case WDIOC_SETTIMEOUT2:
        ioctl_str = "WDIOC_SETTIMEOUT2";
        break;

     case WDIOC_GETTIMEOUT:
        ioctl_str = "WDIOC_GETTIMEOUT";
        break;

     case WDIOC_GETTIMEOUT2:
        ioctl_str = "WDIOC_GETTIMEOUT2";
        break;

     case WDIOC_NOTIFY:
        ioctl_str = "WDIOC_NOTIFY";
        break;

     case WDIOC_GETDC:
        ioctl_str = "WDIOC_GETDC";
        break;

     case WDIOC_GETIRQVEC:
        ioctl_str = "WDIOC_GETIRQVEC";
        break;

     case WDIOC_CLRNOTIFY:
        ioctl_str = "WDIOC_CLRNOTIFY";
        break;

     case WDIOC_GET_INT_TYPE:
        ioctl_str = "WDIOC_GET_INT_TYPE";
        break;

     case WDIOC_SET_INT_TYPE:
        ioctl_str = "WDIOC_SET_INT_TYPE";
        break;

     default:
        ioctl_str = "Unknown IOCTL";
        break;
     }
     return ioctl_str;
}

/*
 * Function Name:   wdt_ioctl()
 * Parameter:    struct inode *inode, struct file *file,
                 unsigned int cmd, u32 arg.
 * Return Value:    0 - successful, negative value - failed.
 * Description:     This function is used to provide IO interface.
 */
static int wdt_ioctl(struct inode *inode, struct file *file,
                    unsigned int cmd, unsigned long arg)
{
     u8  mode=0, scale=0, int_type=0;
     u32 u_margin=0, dcount=0;
     int ret = 0;

     static  struct watchdog_info ident = {
        options:     WDIOF_SETMODE | WDIOF_KEEPALIVEPING |
                 WDIOF_SETTIMEOUT | WDIOF_SETTIMEOUT2,
        firmware_version:     VERSION_NUM,
        identity:     "Intel Watchdog Timer",
     };

     switch (cmd) {
     case WDIOC_GETSUPPORT:
        if (copy_to_user((struct watchdog_info *) arg, &ident,
           sizeof(ident))) {
              ret = -EFAULT;
        }
        break;

     case WDIOC_GETSTATUS:
        ret = put_user(wdt_sys_status(),(int *)arg);
        break;

     case WDIOC_CLRSTATUS:
        ret = put_user(wdt_clear_toutsts(),(int *)arg);
        break;

     case WDIOC_GETBOOTSTATUS:
        ret = put_user(0, (int *)arg);
        break;

     case WDIOC_DISABLE:
        wdt_reload();
        ret = wdt_disable();
        break;

     case WDIOC_ENABLE:
        ret = wdt_enable();
        break;

     case WDIOC_GETWDTENABLE:
        ret = put_user(wdt_getenable(),(int *)arg);
        break;

     case WDIOC_SETMODE:
        if (get_user(mode, (u8 *) arg)) {
           ret = -EFAULT;
        } else {
           ret = wdt_set_mode(mode);
        }
        break;

     case WDIOC_GETMODE:
        ret = put_user(wdt_get_mode(),(int *)arg);
        break;

     case WDIOC_SETOPIN:
        if (get_user(mode, (u8 *) arg)) {
           ret = -EFAULT;
        } else {
           ret = wdt_set_opin(mode);
        }
        break;

     case WDIOC_GETOPIN:
        ret = put_user(wdt_get_opin(),(int *)arg);
        break;

     case WDIOC_GETIRQSTAT:
        ret = put_user(wdt_get_irqstat(),(int *)arg);
        break;

     case WDIOC_CLRIRQSTAT:
        wdt_clr_irqstat();
        break;

     case WDIOC_SETSCALE:
        if (get_user(scale, (u8 *) arg)) {
           ret = -EFAULT;
        } else {
           ret = wdt_set_scale(scale);
        }
        break;

     case WDIOC_GETSCALE:
        ret = put_user(wdt_get_scale(),(int *)arg);
        break;

     case WDIOC_LOCK:
        ret = wdt_set_lk();
        break;

     case WDIOC_KEEPALIVE:
        wdt_reload();
        break;

     case WDIOC_SETTIMEOUT:
        if (get_user(u_margin, (u32 *) arg)) {
           ret = -EFAULT;
        } else {
           if (wdt_set_timeout1(u_margin)) {
              ret = -EINVAL;
           } else {
              wdt_margin1 = u_margin;
              wdt_reload();
           }
        }
        break;

     case WDIOC_SETTIMEOUT2:
        if (get_user(u_margin, (u32 *) arg)) {
           ret = -EFAULT;
        } else {
           if (wdt_set_timeout2(u_margin)) {
              ret = -EINVAL;
           } else {
              wdt_margin2 = u_margin;
              wdt_reload();
           }
        }
        break;

     case WDIOC_GETTIMEOUT:
        ret = put_user(wdt_margin1, (u32 *) arg);
        break;

     case WDIOC_GETTIMEOUT2:
        ret = put_user(wdt_margin2, (u32 *) arg);
        break;

     case WDIOC_NOTIFY:
        /* Don't wait if watchdog timer interrupt is disabled */
        if (wdt_intr_type == WDT_INT_TYPE_DIS) {
           ret = -1;
        } else {
		   /* The process is put to sleep (TASK_INTERRUPTIBLE) until a
		    * wake_up signal and the wdt_wq_active == WQ_NOT_ACTIVE condition
		    * evaluates to true. */
           wdt_wq_active = WQ_ACTIVE;
           wait_event_interruptible(wdt_wait_queue, wdt_wq_active == WQ_NOT_ACTIVE);
        }
        break;

     case WDIOC_GETDC:
        dcount = wdt_get_dc();
        put_user(dcount, (u32 *) arg);
        break;

     case WDIOC_GETIRQVEC:
        put_user(wdt_irq, (u32 *) arg);
        break;

     case WDIOC_CLRNOTIFY:
           wdt_wq_active = WQ_NOT_ACTIVE;
           wake_up_interruptible(&wdt_wait_queue);
        break;

     case WDIOC_SET_INT_TYPE:
        if (get_user(int_type, (u8 *) arg)) {
           ret = -EFAULT;
        } else {
           ret = wdt_set_intr_type(int_type);
        }
        break;

     case WDIOC_GET_INT_TYPE:
        ret = put_user(wdt_get_intr_type(),(int *)arg);
        break;

     default:
        ret = -ENOTTY;
        break;
     }

     if (ret) {
        WDT_PRINTK("iwdt: wdt_ioctl() %s FAILED ret = %d\n",
           wdt_get_ioctl_string(cmd), ret);
     } else {
        WDT_PRINTK("iwdt: wdt_ioctl() %s SUCCESS\n",
           wdt_get_ioctl_string(cmd));
     }
     return ret;
}

/*
 * Function Name:   wdt_isr()
 * Parameter:    int irq - irq number, void *dev_id, struct pt_regs *regs
 * Return Value::   IRQ_NONE -  if the interrupt is not for wdt.
 *           IRQ_HANDLED - if it is for wdt.
 * Description:     This is the interrupt service routine of the WDT.
 */
static irqreturn_t wdt_isr(int irq, void *dev_id)
{
     u8 val;

     /* Check if this call is for wdt */
     if (dev_id != &wdt_miscdev) {
        return IRQ_NONE;
     }

     WDT_PRINTK("iwdt: Enter wdt_isr()\n");

     wdt_wq_active = WQ_NOT_ACTIVE;
     val = wdt_read_byte(WDT_INT_STS_REG);
     if ((val & 0x01) == 0) {
        wake_up_interruptible(&wdt_wait_queue);
        return IRQ_HANDLED;
     }
     wdt_write_byte(WDT_INT_STS_REG, val);
     wake_up_interruptible(&wdt_wait_queue);
     return IRQ_HANDLED;
}

MODULE_DEVICE_TABLE(pci, lpc_pci_tbl);

/*
 * Function Name:   wdt_init_one()
 * Parameter:    struct pci_dev *dev, const struct pci_device_id *id_ptr
 * Return Value:    0 - successful, negative value - failed.
 * Description:     This function is used to init the device and request
 *           resources.
 */
static int __init wdt_init_one(struct pci_dev *dev,
              const struct pci_device_id *id_ptr)
{
     int ret, irq;
     WDT_PRINTK("iwdt: Enter wdt_init_one called for device = 0x%x()\n",
        dev->device);
     spin_lock_init(&wdt_lock);

     /* save lpc pci device */
     lpc_pci = dev;

     /* Get wdt IO base */
     wdt_get_iobase(lpc_pci, &wdt_iobase, &irq);
     /* wdt_base = physical address in BAR0 */
     WDT_PRINTK("iwdt: Intel wdt: wdt_iobase = 0x%x, Interrupt %d\n",
        wdt_iobase, irq);

     /* Configure hardware with driver defaults,
    * but don't start the wdt timer */
     ret = wdt_set_hw_default();
     if (ret) {
        return -1;
     }

     /* Use user-specified irq number if bios setup irq is 0 */
     if (irq == 0 && wdt_irq != 0) {
        printk("iwdt: Using user provided irq = %d\n", wdt_irq);
     } else {
        wdt_irq = irq;
     }

     /* Request irq only if wdt_irq is other than 0 */
     if (wdt_irq) {
        if (request_irq(wdt_irq, wdt_isr, IRQF_SHARED, "iwdt", &wdt_miscdev)) {
           printk("IRQ %d is not free.\n", wdt_irq);
           return -EIO;
        }
     }

     if (misc_register(&wdt_miscdev) != 0) {
	if (wdt_irq)
        	free_irq(wdt_irq, &wdt_miscdev);
        printk("iwdt: cannot register miscdev\n");
        return -EIO;
     }
     printk("iwdt: Intel WDT init done.\n");
     return 0;
}

/*
 * Function Name:   wdt_remove_one()
 * Parameter:    struct pci_dev *pdev
 * Return Value:    None.
 * Description:     This function is used to release resources at rmmod time.
 */
static void __devexit wdt_remove_one(struct pci_dev *pdev)
{
     printk("iwdt: Enter wdt_remove_one()\n");
     /* stop the WDT */
     wdt_reload();
     wdt_disable();
     /* deallocate resources that were assigned @ insmod time */
     misc_deregister(&wdt_miscdev);
     free_irq(wdt_irq, &wdt_miscdev);
}

/*
 * Function Name:   wdt_init()
 * Parameter:    None.
 * Return Value:    0 - successful, negative value - failed.
 * Description:     This function is used to register the driver.
 */
static int __init wdt_init(void)
{
     WDT_PRINTK("iwdt: Enter wdt_init()\n");
     if (pci_register_driver(&wdt_driver) < 0) {
        printk("iwdt: Intel WDT device not found.\n");
        return -ENODEV;
     }
     return 0;
}

/*
 * Function Name:   wdt_cleanup()
 * Parameter:    None.  * Return Value:    None.
 * Description:     This function is used to deregister the driver.
 */
static void __exit wdt_cleanup(void)
{
     WDT_PRINTK("iwdt: Enter wdt_cleanup()\n");
     pci_unregister_driver(&wdt_driver);
}

/*
 * Function Name:   wdt_pci_suspend()
 * Parameter:       dev     pci device structure
 *                  state   suspend state
 * Return Value:    0 - successful, negative value - failed.
 * Description:     This functions is called when the device is suspended.
 */
int wdt_pci_suspend(struct pci_dev *dev, pm_message_t state)
{
   WDT_PRINTK("iwdt: Enter wdt_pci_suspend()\n");
   /* Simply return as configuration data has already been preserved in */
   /* global variables, so nothing to do here.                          */
   return 0;
}

/*
 * Function Name:   wdt_pci_resume()
 * Parameter:       dev     pci device structure
 * Return Value:    0 - successful, negative value - failed.
 * Description:     This functions is called when the device is resumed.
 */
int wdt_pci_resume(struct pci_dev *dev)
{
     WDT_PRINTK("iwdt: Enter wdt_pci_resume()\n");

     /* Restore the WDT configuration registers to the values they had before */
     /* going into a suspended state                                          */
     wdt_set_mode(wdt_mode);
     wdt_set_scale(wdt_scale);
     wdt_set_intr_type(wdt_intr_type);
     wdt_set_opin(wdt_opin_mode);

     if (wdt_mode ==  FREE_MODE) {
        wdt_set_timeout2(wdt_margin2);
     } else {
        wdt_set_timeout1(wdt_margin1);
        wdt_set_timeout2(wdt_margin2);
     }

     if ((wdt_state & WDT_ENABLE) == 0) {
         wdt_disable();
     } else {
         wdt_enable();
     }

     if ((wdt_lk & WDT_LOCK) == 1) {
		 wdt_set_lk();
	 }

     return 0;
}

module_init(wdt_init);
module_exit(wdt_cleanup);

/*----------------------------------------------------------------------------
 * File Revision History
 * $Id: iwdt.c,v 1.4 2010-02-01 03:09:08 gerg Exp $
 * $Source: /cvs/sw/linux-3.x/drivers/watchdog/iwdt.c,v $
 *----------------------------------------------------------------------------
 */
