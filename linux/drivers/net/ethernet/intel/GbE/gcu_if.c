/*****************************************************************************

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

*****************************************************************************/

/**************************************************************************
 * @ingroup GCU_INTERFACE
 *
 * @file gcu_if.c
 *
 * @description
 *   This module contains shared functions for accessing and configuring 
 *   the GCU.
 *
 **************************************************************************/

#include "gcu.h"
#include "gcu_reg.h"
#include "gcu_if.h"


#if defined(CONFIG_UTM2000) || defined(CONFIG_UTM3000)
/*
 *	Extra code to support the Marvell 88E6161 switched on the
 *	McAfee UTM2000/3000 boards.
 */
#define MVL_PHYS_PHY_MAX    32

#define MVL_SMI_READ_OP     0x2
#define MVL_SMI_WRITE_OP    0x1

#define MVL_SMI_CMD_REG     0x0
#define MVL_SMI_DATA_REG    0x1
   
#define MVL_SMI_CMD_REG_ADDR_MASK   0x1F
#define MVL_SMI_CMD_PHY_ADDR_MASK   0x3E0
#define MVL_SMI_CMD_SMI_OP_MASK     0xC00
 
static uint16_t gen_mvl_smi_cmd_reg(uint32_t phy_num, uint32_t reg_addr, uint8_t op) {
    uint16_t reg = 0;
    /* Set the register address bits */
    reg |= (MVL_SMI_CMD_REG_ADDR_MASK & reg_addr);
    /* Set the phy address bits */
    reg |= (MVL_SMI_CMD_PHY_ADDR_MASK & (phy_num << 5));
    /* Set the SMI Operation */
    reg |= (MVL_SMI_CMD_SMI_OP_MASK & (op << 10));
    /* Set the SMI Frame type to IEEE 802.3 Clause 22 */
    reg |= 1 << 12;
    /* Set the SMI Busy bit */
    reg |= 1 << 15; 
    return reg;
} 

static int32_t mii_write(const struct gcu_adapter *adapter, uint32_t phy_num, uint32_t reg_addr, uint16_t phy_data)
{
    uint32_t data = 0;
    uint32_t timeoutCounter = 0;
    const uint32_t timeoutCounterMax = GCU_MAX_ATTEMPTS;
    uint32_t complete;

    /* format the data to be written to the MDIO_COMMAND_REG */
    data = phy_data;
    data |= (reg_addr << MDIO_COMMAND_PHY_REG_OFFSET);
    data |= (phy_num << MDIO_COMMAND_PHY_ADDR_OFFSET);
    data |= MDIO_COMMAND_OPER_MASK | MDIO_COMMAND_GO_MASK;

    iowrite32(data, adapter->hw_addr + MDIO_COMMAND_REG);     
    do {
        timeoutCounter++;
        udelay(0x32); /* 50 microsecond delay */
        data = ioread32(adapter->hw_addr + MDIO_COMMAND_REG);
        complete = (data & MDIO_COMMAND_GO_MASK) >> MDIO_COMMAND_GO_OFFSET;
    } while(complete && timeoutCounter < timeoutCounterMax);

    if(timeoutCounter == timeoutCounterMax && !complete)
      return -1;
    return 0;
}

static int32_t mii_read(const struct gcu_adapter *adapter, uint32_t phy_num, uint32_t reg_addr, uint16_t *phy_data)
{
    uint32_t data = 0;
    uint32_t timeoutCounter = 0;
    const uint32_t timeoutCounterMax = GCU_MAX_ATTEMPTS;
    uint32_t complete = 0;
    
    /* format the data to be written to MDIO_COMMAND_REG */
    data |= (reg_addr << MDIO_COMMAND_PHY_REG_OFFSET);
    data |= (phy_num << MDIO_COMMAND_PHY_ADDR_OFFSET);
    data |= MDIO_COMMAND_GO_MASK;

    iowrite32(data, adapter->hw_addr + MDIO_COMMAND_REG);     
    do {
        timeoutCounter++;
        udelay(0x32); /* 50 microsecond delay */
        data = ioread32(adapter->hw_addr + MDIO_COMMAND_REG);
        complete = (data & MDIO_COMMAND_GO_MASK) >> MDIO_COMMAND_GO_OFFSET;
    } while(complete && timeoutCounter < timeoutCounterMax);

    if(timeoutCounter == timeoutCounterMax && !complete)
        return -1;

    data = ioread32(adapter->hw_addr + MDIO_STATUS_REG);
    if((data & MDIO_STATUS_STATUS_MASK) != 0)
        return -1;

    *phy_data = (uint16_t) (data & MDIO_STATUS_READ_DATA_MASK);
    return 0;
}

static int32_t mvl_mii_write(const struct gcu_adapter *adapter, uint32_t mphy, uint32_t phy_num, uint32_t reg_addr, uint16_t phy_data)
{
        int32_t cnt, rc;
        uint16_t smi_cmd_reg;

        smi_cmd_reg = gen_mvl_smi_cmd_reg(phy_num, reg_addr, MVL_SMI_WRITE_OP); 
        rc = mii_write(adapter, mphy, MVL_SMI_DATA_REG, phy_data); 
	if (rc < 0)
		return -1;
        rc = mii_write(adapter, mphy, MVL_SMI_CMD_REG, smi_cmd_reg);
	if (rc < 0)
		return -1;
        
	cnt = 0;
        do {
            udelay(0x32);
            rc = mii_read(adapter, mphy, MVL_SMI_CMD_REG, &smi_cmd_reg);
	    if (rc < 0)
		return -1;
	    if (cnt++ > 10000)
		return -1;
        } while ((smi_cmd_reg & (1 << 15)));

	return 0;
}

static int32_t mvl_mii_read(const struct gcu_adapter *adapter, uint32_t mphy, uint32_t phy_num, uint32_t reg_addr, uint16_t *phy_data)
{
        int32_t cnt, rc;
        uint16_t smi_cmd_reg;

        smi_cmd_reg = gen_mvl_smi_cmd_reg(phy_num, reg_addr, MVL_SMI_READ_OP); 
        rc = mii_write(adapter, mphy, MVL_SMI_CMD_REG, smi_cmd_reg);
	if (rc < 0)
		return -1;

	cnt = 0;
        do {
            udelay(0x32);
            rc = mii_read(adapter, mphy, MVL_SMI_CMD_REG, &smi_cmd_reg);
	    if (rc < 0)
		return -1;
	    if (cnt++ > 10000)
		return -1;
        } while ((smi_cmd_reg & (1 << 15)));
        
        rc = mii_read(adapter, mphy, MVL_SMI_DATA_REG, phy_data);
	if (rc < 0)
		return -1;
        return 0; 
}

static int32_t mvl_mii_phy_read(const struct gcu_adapter *adapter, uint32_t mphy, uint32_t phy_num, uint32_t reg_addr, uint16_t *phy_data)
{
        int32_t cnt, rc;
        uint16_t smi_cmd_reg;

        smi_cmd_reg = gen_mvl_smi_cmd_reg(phy_num, reg_addr, MVL_SMI_READ_OP); 
        rc = mvl_mii_write(adapter, mphy, 0x1c, 24, smi_cmd_reg);
	if (rc < 0)
		return -1;

	cnt = 0;
        do {
            udelay(0x32);
            rc = mvl_mii_read(adapter, mphy, 0x1c, 24, &smi_cmd_reg);
	    if (rc < 0)
		return -1;
	    if (cnt++ > 10000)
		return -1;
        } while ((smi_cmd_reg & (1 << 15)));
        
        rc = mvl_mii_read(adapter, mphy, 0x1c, 25, phy_data);
	if (rc < 0)
		return -1;
        return 0; 
}

static int32_t mvl_mii_phy_write(const struct gcu_adapter *adapter, uint32_t mphy, uint32_t phy_num, uint32_t reg_addr, uint16_t phy_data)
{
        int32_t cnt, rc;
        uint16_t smi_cmd_reg;

        smi_cmd_reg = gen_mvl_smi_cmd_reg(phy_num, reg_addr, MVL_SMI_WRITE_OP); 
        rc = mvl_mii_write(adapter, mphy, 0x1c, 25, phy_data); 
	if (rc < 0)
		return -1;
        rc = mvl_mii_write(adapter, mphy, 0x1c, 24, smi_cmd_reg);
	if (rc < 0)
		return -1;
        
	cnt = 0;
        do {
            udelay(0x32);
            rc = mvl_mii_read(adapter, mphy, 0x1c, 24, &smi_cmd_reg);
	    if (rc < 0)
		return -1;
	    if (cnt++ > 10000)
		return -1;
        } while ((smi_cmd_reg & (1 << 15)));

	return 0;
}

#if 0
void dump_mvl(const struct gcu_adapter *adapter, int mphy)
{
	int phy, r, rc;
	uint16_t v;

	printk("DUMPING MARVELL PHY at ADDR=%d\n", mphy);

	for (phy = 0; (phy < 32); phy++) {
		printk("---------------------------------------------------\n");
		printk("MASTER PHY=%d  -->>  PHY=%d:\n", mphy, phy);

		for (r = 0; (r < 32); r++) {
			if ((r % 8) == 0) printk("%04x:  ", r);
			rc = mvl_mii_read(adapter, mphy, phy, r, &v);
			if (rc)
				printk("FFFF ");
			else
				printk("%04x ", v);
			if (((r + 1) % 8) == 0) printk("\n");
		}

	}

	printk("---------------------------------------------------\n");
}

void mii_dump(void)
{
	const struct gcu_adapter *adapter;
	int phy, r, rc;
	uint16_t v;

	adapter = gcu_get_adapter();
	if (!adapter) {
		printk("No adapter??\n");
		return;
	}

	for (phy = 0; (phy < 32); phy++) {
		printk("---------------------------------------------------\n");
		printk("PHY=%d:\n", phy);

		for (r = 0; (r < 32); r++) {
			if ((r % 8) == 0) printk("%04x:  ", r);
			rc = mii_read(adapter, phy, r, &v);
			if (rc)
				printk("FFFF ");
			else
				printk("%04x ", v);
			if (((r + 1) % 8) == 0) printk("\n");
		}

	}

	printk("---------------------------------------------------\n");

	dump_mvl(adapter, 2);
	dump_mvl(adapter, 4);

	for (phy = 0; (phy < 32); phy++) {
		printk("---------------------------------------------------\n");
		printk("SUB(2) PHY=%d:\n", phy);

		for (r = 0; (r < 32); r++) {
			if ((r % 8) == 0) printk("%04x:  ", r);
			rc = mvl_mii_phy_read(adapter, 2, phy, r, &v);
			if (rc)
				printk("FFFF ");
			else
				printk("%04x ", v);
			if (((r + 1) % 8) == 0) printk("\n");
		}

	}

	printk("---------------------------------------------------\n");

	gcu_release_adapter(&adapter);
}

#endif

static void mvl_enable_ports(int mphy)
{
	const struct gcu_adapter *adapter;
	int phy;
	uint16_t v;

	adapter = gcu_get_adapter();
	if (!adapter) {
		printk("No adapter??\n");
		return;
	}

	for (phy = 0; (phy < 4); phy++) {
		mvl_mii_read(adapter, mphy, 16+phy, 4, &v);
		v |= 0x3;
		mvl_mii_write(adapter, mphy, 16+phy, 4, v);
		mvl_mii_phy_write(adapter, mphy, phy, 0, 0x1140);

		/* Set LED actions */
		mvl_mii_phy_write(adapter, mphy, phy, 22, 3);
		mvl_mii_phy_write(adapter, mphy, phy, 16, 0x0030);
		mvl_mii_phy_write(adapter, mphy, phy, 22, 0);
	}

	/* Enable port5 - the RGMII port - as well */
	mvl_mii_read(adapter, mphy, 21, 4, &v);
	v |= 0x3;
	mvl_mii_write(adapter, mphy, 21, 4, v);

#if 0
	mvl_mii_write(adapter, mphy, 21, 0, 0x0e49);
	mvl_mii_write(adapter, mphy, 21, 1, 0x003e);
#endif

	/* Enable the RGMII delay bits */
	mvl_mii_write(adapter, mphy, 20, 0x1a, 0x81e7);
	mvl_mii_read(adapter, mphy, 21, 0x1a, &v);
	v |= 0x18;
	mvl_mii_write(adapter, mphy, 21, 0x1a, v);
	mvl_mii_write(adapter, mphy, 20, 0x1a, 0xc1e7);

	gcu_release_adapter(&adapter);
}

static void vt_enable_port(int phy)
{
	const struct gcu_adapter *adapter;
	adapter = gcu_get_adapter();
	if (!adapter) {
		printk("No adapter??\n");
		return;
	}

	mii_write(adapter, phy, 23, 0x1100);

	gcu_release_adapter(&adapter);
}

#endif /* CONFIG_UTM2000 || CONFIG_UTM3000 */


/* forward declaration for write verify used in gcu_write_eth_phy */
int32_t gcu_write_verify(uint32_t phy_num, 
                         uint32_t reg_addr, 
                         uint16_t written_data, 
                         const struct gcu_adapter *adapter);

/**
 * gcu_write_eth_phy
 * @phy_num: phy we want to write to, either 0, 1, or 2
 * @reg_addr: address in PHY's register space to write to
 * @phy_data: data to be written
 *
 * interface function for other modules to access the GCU
 **/
int32_t 
gcu_write_eth_phy(uint32_t phy_num, uint32_t reg_addr, uint16_t phy_data)
{
    const struct gcu_adapter *adapter;
    uint32_t data = 0;
    uint32_t timeoutCounter = 0;
    const uint32_t timeoutCounterMax = GCU_MAX_ATTEMPTS;
    uint32_t complete;

    GCU_DBG("%s\n", __func__);

#if defined(CONFIG_UTM2000) || defined(CONFIG_UTM3000)
    /*
     * For the Marvell 88E6161, we use an indirect addressing mode
     * to get at all the registers. We map PHY addresses over 32 to
     * these devices. ie. Device has phy address 2: to access, use a
     * virtual phy address range of 64 to 96, which will be mapped into
     * the switch. 
     */
    if (phy_num >= MVL_PHYS_PHY_MAX) {
        int32_t mphy, phyid, result;

    	adapter = gcu_get_adapter();
    	if(!adapter)
    	{
        	GCU_ERR("gcu_adapter not available, cannot access MMIO\n");
        	return -1;
    	}

	mphy = phy_num / MVL_PHYS_PHY_MAX;
	phyid = phy_num % MVL_PHYS_PHY_MAX;
	if (phyid < 6)
		result = mvl_mii_phy_write(adapter, mphy, phyid, reg_addr, phy_data);
	else
		result = mvl_mii_write(adapter, mphy, phyid, reg_addr, phy_data);

    	gcu_release_adapter(&adapter);

        if (result == -1) {
            GCU_ERR("Error writing to Marvell phy register\n");
            return result;
        }

        return 0; 
    }
#endif /* CONFIG_UTM2000 || CONFIG_UTM3000 */

    if(phy_num > MDIO_COMMAND_PHY_ADDR_MAX)
    {
        GCU_ERR("phy_num = %d, which is greater than "
                "MDIO_COMMAND_PHY_ADDR_MAX\n", phy_num);

        return -1;
    }

    if(reg_addr > MDIO_COMMAND_PHY_REG_MAX)
    {
        GCU_ERR("reg_addr = %d, which is greater than "
                "MDIO_COMMAND_PHY_REG_MAX\n", phy_num);

        return -1;
    }

    /* format the data to be written to the MDIO_COMMAND_REG */
    data = phy_data;
    data |= (reg_addr << MDIO_COMMAND_PHY_REG_OFFSET);
    data |= (phy_num << MDIO_COMMAND_PHY_ADDR_OFFSET);
    data |= MDIO_COMMAND_OPER_MASK | MDIO_COMMAND_GO_MASK;

   /*
     * get_gcu_adapter contains a spinlock, this may pause for a bit
     */
    adapter = gcu_get_adapter();
    if(!adapter)
    {
      GCU_ERR("gcu_adapter not available, cannot access MMIO\n");
      return -1;
    }

    /*
     * We write to MDIO_COMMAND_REG initially, then read that
     * same register until its MDIO_GO bit is cleared. When cleared,
     * the transaction is complete
     */
    iowrite32(data, adapter->hw_addr + MDIO_COMMAND_REG);     
    do {
        timeoutCounter++;
        udelay(0x32); /* 50 microsecond delay */
        data = ioread32(adapter->hw_addr + MDIO_COMMAND_REG);
        complete = (data & MDIO_COMMAND_GO_MASK) >> MDIO_COMMAND_GO_OFFSET;
    } while(complete && timeoutCounter < timeoutCounterMax);
    /* KAD !complete to complete */

    if(timeoutCounter == timeoutCounterMax && !complete)
    {
      GCU_ERR("Reached maximum number of retries"
                " accessing MDIO_COMMAND_REG\n");

      gcu_release_adapter(&adapter);

      return -1;
    }    

    /* validate the write during debug */
#ifdef DBG
    if(!gcu_write_verify(phy_num, reg_addr, phy_data, adapter))
    {
        GCU_ERR("Write verification failed for PHY=%d and addr=%d\n",
                phy_num, reg_addr);

        gcu_release_adapter(&adapter);

        return -1;
    }
#endif
    
    gcu_release_adapter(&adapter);

    return 0;
}
EXPORT_SYMBOL(gcu_write_eth_phy);


/**
 * gcu_read_eth_phy
 * @phy_num: phy we want to write to, either 0, 1, or 2
 * @reg_addr: address in PHY's register space to write to
 * @phy_data: data to be written
 *
 * interface function for other modules to access the GCU
 **/
int32_t 
gcu_read_eth_phy(uint32_t phy_num, uint32_t reg_addr, uint16_t *phy_data)
{
    const struct gcu_adapter *adapter;
    uint32_t data = 0;
    uint32_t timeoutCounter = 0;
    const uint32_t timeoutCounterMax = GCU_MAX_ATTEMPTS;
    uint32_t complete = 0;
    
#if defined(CONFIG_UTM2000) || defined(CONFIG_UTM3000)
    static int once = 0;
    if (once++ == 0) {
        vt_enable_port(1);
        mvl_enable_ports(2);
        mvl_enable_ports(4);
        //mii_dump();
    }
#endif /* CONFIG_UTM2000 || CONFIG_UTM3000 */

    GCU_DBG("%s\n", __func__);

#if defined(CONFIG_UTM2000) || defined(CONFIG_UTM3000)
    /*
     * For the Marvell 88E6161, we use an indirect addressing mode
     * to get at all the registers. We map PHY addresses over 32 to
     * these devices. ie. Device has phy address 2: to access, use a
     * virtual phy address range of 64 to 96, which will be mapped into
     * the switch. 
     */
    if (phy_num >= MVL_PHYS_PHY_MAX) {
        int32_t mphy, phyid, result;

    	adapter = gcu_get_adapter();
    	if(!adapter)
    	{
        	GCU_ERR("gcu_adapter not available, cannot access MMIO\n");
        	return -1;
    	}

	mphy = phy_num / MVL_PHYS_PHY_MAX;
	phyid = phy_num % MVL_PHYS_PHY_MAX;
	if (phyid < 6)
		result = mvl_mii_phy_read(adapter, mphy, phyid, reg_addr, phy_data);
	else
		result = mvl_mii_read(adapter, mphy, phyid, reg_addr, phy_data);

    	gcu_release_adapter(&adapter);

        if (result == -1) {
            GCU_ERR("Error reading from Marvell phy register\n");
            return result;
        }

        return 0; 
    }
#endif /* CONFIG_UTM2000 || CONFIG_UTM3000 */

    if(phy_num > MDIO_COMMAND_PHY_ADDR_MAX)
    {
        GCU_ERR("phy_num = %d, which is greater than "
                "MDIO_COMMAND_PHY_ADDR_MAX\n", phy_num);

        return -1;
    }

    if(reg_addr > MDIO_COMMAND_PHY_REG_MAX)
    {
        GCU_ERR("reg_addr = %d, which is greater than "
                "MDIO_COMMAND_PHY_REG_MAX\n", phy_num);

        return -1;
    }

    /* format the data to be written to MDIO_COMMAND_REG */
    data |= (reg_addr << MDIO_COMMAND_PHY_REG_OFFSET);
    data |= (phy_num << MDIO_COMMAND_PHY_ADDR_OFFSET);
    data |= MDIO_COMMAND_GO_MASK;

    /* 
     * this call contains a spinlock, so this may pause for a bit
     */
    adapter = gcu_get_adapter();
    if(!adapter)
    {
        GCU_ERR("gcu_adapter not available, cannot access MMIO\n");
        return -1;
    }

    /*
     * We write to MDIO_COMMAND_REG initially, then read that
     * same register until its MDIO_GO bit is cleared. When cleared,
     * the transaction is complete
     */
    iowrite32(data, adapter->hw_addr + MDIO_COMMAND_REG);     
    do {
        timeoutCounter++;
        udelay(0x32); /* 50 microsecond delay */
        data = ioread32(adapter->hw_addr + MDIO_COMMAND_REG);
        complete = (data & MDIO_COMMAND_GO_MASK) >> MDIO_COMMAND_GO_OFFSET;
    } while(complete && timeoutCounter < timeoutCounterMax);
    /* KAD !complete to complete */

    if(timeoutCounter == timeoutCounterMax && !complete)
    {
        GCU_ERR("Reached maximum number of retries"
                " accessing MDIO_COMMAND_REG\n");

         gcu_release_adapter(&adapter);

        return -1;
    }

    /* we retrieve the data from the MDIO_STATUS_REGISTER */
    data = ioread32(adapter->hw_addr + MDIO_STATUS_REG);
    if((data & MDIO_STATUS_STATUS_MASK) != 0)
    {
        GCU_ERR("Unable to retrieve data from MDIO_STATUS_REG\n");

        gcu_release_adapter(&adapter);

        return -1;
    }

    *phy_data = (uint16_t) (data & MDIO_STATUS_READ_DATA_MASK);

    gcu_release_adapter(&adapter);

    return 0;
}
EXPORT_SYMBOL(gcu_read_eth_phy);


/**
 * gcu_write_verify
 * @phy_num: phy we want to write to, either 0, 1, or 2
 * @reg_addr: address in PHY's register space to write to
 * @phy_data: data to be checked
 * @adapter: pointer to global adapter struct
 *
 * This f(n) assumes that the spinlock acquired for adapter is
 * still in force.
 **/
int32_t 
gcu_write_verify(uint32_t phy_num, uint32_t reg_addr, uint16_t written_data,
                 const struct gcu_adapter *adapter)
{
    uint32_t data = 0;
    uint32_t timeoutCounter = 0;
    const uint32_t timeoutCounterMax = GCU_MAX_ATTEMPTS;
    uint32_t complete = 0;

    GCU_DBG("%s\n", __func__);

    if(!adapter)
    {
        GCU_ERR("Invalid adapter pointer\n");
        return 0;
    }

    if(phy_num > MDIO_COMMAND_PHY_ADDR_MAX)
    {
        GCU_ERR("phy_num = %d, which is greater than "
                "MDIO_COMMAND_PHY_ADDR_MAX\n", phy_num);

        return 0;
    }

    if(reg_addr > MDIO_COMMAND_PHY_REG_MAX)
    {
        GCU_ERR("reg_addr = %d, which is greater than "
                "MDIO_COMMAND_PHY_REG_MAX\n", phy_num);

        return 0;
    }

    /* format the data to be written to MDIO_COMMAND_REG */
    data |= (reg_addr << MDIO_COMMAND_PHY_REG_OFFSET);
    data |= (phy_num << MDIO_COMMAND_PHY_ADDR_OFFSET);
    data |= MDIO_COMMAND_GO_MASK;

    /*
     * We write to MDIO_COMMAND_REG initially, then read that
     * same register until its MDIO_GO bit is cleared. When cleared,
     * the transaction is complete
     */
    iowrite32(data, adapter->hw_addr + MDIO_COMMAND_REG);     
    do {
        timeoutCounter++;
        udelay(0x32); /* 50 microsecond delay */
        data = ioread32(adapter->hw_addr + MDIO_COMMAND_REG);
        complete = (data & MDIO_COMMAND_GO_MASK) >> MDIO_COMMAND_GO_OFFSET;
    } while(complete && timeoutCounter < timeoutCounterMax);


    if(timeoutCounter == timeoutCounterMax && !complete)
    {
        GCU_ERR("Reached maximum number of retries"
                " accessing MDIO_COMMAND_REG\n");

        return 0;
    }

    /* we retrieve the data from the MDIO_STATUS_REGISTER */
    data = ioread32(adapter->hw_addr + MDIO_STATUS_REG);
    if((data & MDIO_STATUS_STATUS_MASK) != 0)
    {
        GCU_ERR("Unable to retrieve data from MDIO_STATUS_REG\n");

        return 0;
    }

    return written_data == (uint16_t) (data & MDIO_STATUS_READ_DATA_MASK);
}
 
/*
 * gcu_iegbe_resume
 * @pdev: gcu pci_dev  
 * purpose - exported PM resume function used by iegbe 
 *           driver to enable the GCU device.
 */
void gcu_iegbe_resume(struct pci_dev *pdev)
{
    GCU_DBG("%s\n", __func__);

    pci_restore_state(pdev);
    pci_enable_device(pdev);

    return;
}
EXPORT_SYMBOL(gcu_iegbe_resume);

/*
 * gcu_iegbe_suspend
 * @pdev: gcu pci_dev  
 * @state: PM state  
 * purpose - exported PM suspend function used by iegbe 
 *           driver to disable the GCU device.   
 */
int gcu_iegbe_suspend(struct pci_dev *pdev, uint32_t state)
{
    GCU_DBG("%s\n", __func__);

    pci_save_state(pdev);
    pci_disable_device(pdev);
    state = (state > 0) ? 0 : 0;

    return state;
}

EXPORT_SYMBOL(gcu_iegbe_suspend);
