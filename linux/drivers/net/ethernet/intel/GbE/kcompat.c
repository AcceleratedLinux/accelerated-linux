/************************************************************

GPL LICENSE SUMMARY

  Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.

  This program is free software; you can redistribute it and/or modify 
  it under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
  General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program; if not, write to the Free Software 
  Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
  The full GNU General Public License is included in this distribution 
  in the file called LICENSE.GPL.

  Contact Information:
  Intel Corporation

 version: Embedded.L.1.0.3-144
  
  Contact Information:
  
  Intel Corporation, 5000 W Chandler Blvd, Chandler, AZ 85226 

**************************************************************/
/**************************************************************************
 * @ingroup KCOMPAT_GENERAL
 *
 * @file kcompat.c
 *
 * @description
 *   
 *
 **************************************************************************/
#include "kcompat.h"

/*************************************************************/
/* 2.4.13 => 2.4.3 */
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(0x2,0x4,0xd) )

/**************************************/
/* PCI DMA MAPPING */

#if defined(CONFIG_HIGHMEM)

#ifndef PCI_DRAM_OFFSET
#define PCI_DRAM_OFFSET 0
#endif

u64 _kc_pci_map_page(struct pci_dev *dev, 
                     struct page *page, 
                     unsigned long offset, 
                     size_t size, 
                     int direction)
{
    u64 ret_val;
    ret_val = (((u64)(page - mem_map) << PAGE_SHIFT) + offset + 
                 PCI_DRAM_OFFSET);
    return ret_val;
}

#else /* CONFIG_HIGHMEM */

u64 _kc_pci_map_page(struct pci_dev *dev, 
                     struct page *page, 
                     unsigned long offset, 
                     size_t size, 
                     int direction)
{
    return pci_map_single(dev, (void *)page_address(page) + offset,
        size, direction);
}

#endif /* CONFIG_HIGHMEM */

void _kc_pci_unmap_page(struct pci_dev *dev, 
                        u64 dma_addr, 
                        size_t size, 
                        int direction)
{
    return pci_unmap_single(dev, dma_addr, size, direction);
}

#endif /* 2.4.13 => 2.4.3 */


/*****************************************************************************/
/* 2.4.3 => 2.4.0 */
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(0x2,0x4,0x3) )

/**************************************/
/* PCI DRIVER API */

int _kc_pci_set_dma_mask(struct pci_dev *dev, dma_addr_t mask)
{
    if(!pci_dma_supported(dev, mask)) {
        return -EIO;
    }          
    dev->dma_mask = mask;
    return 0;
}

int _kc_pci_request_regions(struct pci_dev *dev, char *res_name)
{
    int i;

    for (i = 0; i < 0x6; i++) {
        if (pci_resource_len(dev, i) == 0) {
            continue;
        }
        if (pci_resource_flags(dev, i) & IORESOURCE_IO) {
            if (!request_region(pci_resource_start(dev, i),
                pci_resource_len(dev, i), res_name)) {
                pci_release_regions(dev);
                return -EBUSY;
            }
        } else if (pci_resource_flags(dev, i) & IORESOURCE_MEM) {
            if (!request_mem_region(pci_resource_start(dev, i), 
                pci_resource_len(dev, i),
                res_name)) {
                pci_release_regions(dev);
                return -EBUSY;
            }
        }
    }
    return 0;
}

void _kc_pci_release_regions(struct pci_dev *dev)
{
    int i;

    for (i = 0; i < 0x6; i++) {
        if (pci_resource_len(dev, i) == 0) {
            continue;
        }
        if (pci_resource_flags(dev, i) & IORESOURCE_IO){
            release_region(pci_resource_start(dev, i),
                       pci_resource_len(dev, i));
        } else if (pci_resource_flags(dev, i) & IORESOURCE_MEM) {
            release_mem_region(pci_resource_start(dev, i),
                           pci_resource_len(dev, i));
          }        
    }
}

/**************************************/
/* NETWORK DRIVER API */

struct net_device * _kc_alloc_etherdev(int sizeof_priv)
{
    struct net_device *dev;
    int alloc_size;

    alloc_size = sizeof(*dev) + sizeof_priv + IFNAMSIZ + 0x1f;

    dev = kmalloc(alloc_size, GFP_KERNEL);

    if (!dev) { return NULL; }

    memset(dev, 0, alloc_size);

    if (sizeof_priv) {
        dev->priv = (void *) (((unsigned long)(dev + 1) + 0x1f) & ~0x1f);
    }
    dev->name[0] = '\0';

    ether_setup(dev);

    return dev;
}

int _kc_is_valid_ether_addr(u8 *addr)
{
    const char zaddr[0x6] = {0,};

    return !(addr[0]&1) && memcmp( addr, zaddr, 0x6);
}

#endif /* 2.4.3 => 2.4.0 */


/*****************************************************************/
/* 2.4.6 => 2.4.3 */
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(0x2,0x4,0x6) )

int _kc_pci_set_power_state(struct pci_dev *dev, int state)
{ return 0; }
int _kc_pci_save_state(struct pci_dev *dev, u32 *buffer)
{ return 0; }
int _kc_pci_restore_state(struct pci_dev *pdev, u32 *buffer)
{ return 0; }
int _kc_pci_enable_wake(struct pci_dev *pdev, u32 state, int enable)
{ return 0; }

#endif /* 2.4.6 => 2.4.3 */

 
