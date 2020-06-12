// SPDX-License-Identifier: GPL-2.0
/*
 * mt7621_nand.c - Mediatek MT7621 NAND Flash Controller device driver
 *
 * Copyright 2009-2012 MediaTek Co.,Ltd.
 * Copyright 2018-2019 Greg Ungerer <gerg@kernel.org>
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/rawnand.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include "mt7621_nand.h"

static const char * const probe_types[] = {
	"cmdlinepart",
	"ofpart",
	NULL
};

struct mt7621_nand_host {
	struct nand_chip	nand_chip;
	struct mtd_info		*mtd;

	void __iomem	*regs;
	void __iomem	*ecc;

	u32	addr_cycles;
	u32	access_timing;

	u32	column;
	u32	row;
	u32	OOBrow;
	u8	OOB[288];
	u8	*data_buf;
	bool	cmdstatus;
	bool	legacybbt;
	u32	legacybbt_block_num;

	u32	last_failed;
};

static struct mt7621_nand_host *host;

/*
 * This constant was taken from linux/nand/nand.h v 3.14
 * in later versions it seems it was removed in order to save a bit of space
 */
#define NAND_MAX_OOBSIZE 774
static u8 local_oob_buf[NAND_MAX_OOBSIZE];

struct nand_ecclayout {
	u32 eccbytes;
	u32 eccpos[MTD_MAX_ECCPOS_ENTRIES_LARGE];
	u32 oobavail;
	struct nand_oobfree oobfree[MTD_MAX_OOBFREE_ENTRIES_LARGE];
};

static struct nand_ecclayout *layout;

static struct nand_ecclayout nand_oob_16 = {
	.eccbytes = 8,
	.eccpos = {8, 9, 10, 11, 12, 13, 14, 15},
	.oobfree = {{1, 6}, {0, 0}}
};

static struct nand_ecclayout nand_oob_64 = {
	.eccbytes = 32,
	.eccpos = {32, 33, 34, 35, 36, 37, 38, 39,
		40, 41, 42, 43, 44, 45, 46, 47,
		48, 49, 50, 51, 52, 53, 54, 55,
		56, 57, 58, 59, 60, 61, 62, 63},
	.oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 6}, {0, 0}}
};

static struct nand_ecclayout nand_oob_128 = {
	.eccbytes = 64,
	.eccpos = {
		64, 65, 66, 67, 68, 69, 70, 71,
		72, 73, 74, 75, 76, 77, 78, 79,
		80, 81, 82, 83, 84, 85, 86, 86,
		88, 89, 90, 91, 92, 93, 94, 95,
		96, 97, 98, 99, 100, 101, 102, 103,
		104, 105, 106, 107, 108, 109, 110, 111,
		112, 113, 114, 115, 116, 117, 118, 119,
		120, 121, 122, 123, 124, 125, 126, 127},
	.oobfree = {{1, 7}, {9, 7}, {17, 7}, {25, 7}, {33, 7}, {41, 7}, {49, 7}, {57, 6}}
};

/*
 * Local read/write/set/clear register operations.
 */
static inline u8 regread8(u32 addr)
{
	return readb(host->regs + addr);
}

static inline u16 regread16(u32 addr)
{
	return readw(host->regs + addr);
}

static inline void regwrite16(u32 addr, u16 data)
{
	writew(data, host->regs + addr);
}

static inline u32 regread32(u32 addr)
{
	return readl(host->regs + addr);
}

static inline void regwrite32(u32 addr, u32 data)
{
	writel(data, host->regs + addr);
}

static inline void regset32(u32 addr, u32 data)
{
	u32 value;
	value = regread32(addr) | data;
	regwrite32(addr, value);
}

static inline void regset16(u32 addr, u16 data)
{
	u16 value;
	value = regread16(addr) | data;
	regwrite16(addr, value);
}

static inline void regclr16(u32 addr, u16 data)
{
	u16 value;
	value = regread16(addr) & (~data);
	regwrite16(addr, value);
}

/*
 * Local ECC register read/write operations. These are implemented as a
 * separate set of methods because the ECC register bank is a physically
 * seprate window within the address space.
 */
static inline u16 eccread16(u32 addr)
{
	return readw(host->ecc + addr);
}

static inline void eccwrite16(u32 addr, u16 data)
{
	writew(data, host->ecc + addr);
}

static inline u32 eccread32(u32 addr)
{
	return readl(host->ecc + addr);
}

static inline void eccwrite32(u32 addr, u32 data)
{
	writel(data, host->ecc + addr);
}

static void ecc_config(u32 ecc_bit)
{
	u32 ENCODESize;
	u32 DECODESize;
	u32 ecc_bit_cfg = ECC_CNFG_ECC4;
	u32 val;

	switch(ecc_bit){
	case 4:
		ecc_bit_cfg = ECC_CNFG_ECC4;
		break;
	case 8:
		ecc_bit_cfg = ECC_CNFG_ECC8;
		break;
	case 10:
		ecc_bit_cfg = ECC_CNFG_ECC10;
		break;
	case 12:
		ecc_bit_cfg = ECC_CNFG_ECC12;
		break;
	default:
		break;
	}

	eccwrite16(ECC_DECCON_REG16, DEC_DE);
	do {
	} while (!eccread16(ECC_DECIDLE_REG16));

	eccwrite16(ECC_ENCCON_REG16, ENC_DE);
	do {
	} while (!eccread32(ECC_ENCIDLE_REG32));

	/* setup FDM register base */
	eccwrite32(ECC_FDMADDR_REG32, (u32)NFI_FDM0L_REG32);

	/* Sector + FDM */
	ENCODESize = (NAND_SECTOR_SIZE + 8) << 3;
	/* Sector + FDM + YAFFS2 meta data bits */
	DECODESize = ((NAND_SECTOR_SIZE + 8) << 3) + ecc_bit * 13;

	/* configure ECC decoder && encoder */
	eccwrite32(ECC_DECCNFG_REG32, ecc_bit_cfg | DEC_CNFG_NFI | DEC_CNFG_EMPTY_EN | (DECODESize << DEC_CNFG_CODE_SHIFT));

	eccwrite32(ECC_ENCCNFG_REG32, ecc_bit_cfg | ENC_CNFG_NFI | (ENCODESize << ENC_CNFG_MSG_SHIFT));

	val = eccread32(ECC_DECCNFG_REG32) | DEC_CNFG_EL;
	eccwrite32(ECC_DECCNFG_REG32, val);
}

static void ecc_decode_start(void)
{
	while (!(eccread16(ECC_DECIDLE_REG16) & DEC_IDLE))
		;
	eccwrite16(ECC_DECCON_REG16, DEC_EN);
}

static void ecc_decode_end(void)
{
	while (!(eccread16(ECC_DECIDLE_REG16) & DEC_IDLE))
		;
	eccwrite16(ECC_DECCON_REG16, DEC_DE);
}

static void ecc_encode_start(void)
{
	while (!(eccread32(ECC_ENCIDLE_REG32) & ENC_IDLE))
		;
	mb();
	eccwrite16(ECC_ENCCON_REG16, ENC_EN);
}

static void ecc_encode_end(void)
{
	/* wait for device returning idle */
	while (!(eccread32(ECC_ENCIDLE_REG32) & ENC_IDLE))
		;
	mb();
	eccwrite16(ECC_ENCCON_REG16, ENC_DE);
}

static int mt7621_nand_check_bch_error(struct mtd_info *mtd, u8 *buf,
					u32 sector, u32 page)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct mt7621_nand_host *host = nand_get_controller_data(chip);
	u16 SectorDoneMask = 1 << sector;
	u32 ErrorNumDebug, i, ErrNum;
	u32 timeout = 0xFFFF;
	u32 ErrBitLoc[6];
	u32 ErrByteLoc, BitOffset;
	u32 ErrBitLoc1th, ErrBitLoc2nd;
	int bits_corrected = 0;

	/* Wait for Decode Done */
	while (0 == (SectorDoneMask & eccread16(ECC_DECDONE_REG16))) {
		timeout--;
		if (0 == timeout)
			return -EIO;
	}

	/*
 	 * We will manually correct the error bits in the last sector,
 	 * not all the sectors of the page!
 	 */
	memset(ErrBitLoc, 0x0, sizeof(ErrBitLoc));
	ErrorNumDebug = eccread32(ECC_DECENUM_REG32);
	ErrNum = eccread32(ECC_DECENUM_REG32) >> (sector << 2);
	ErrNum &= 0xF;

	if (ErrNum) {
		if (0xF == ErrNum) {
			/*
			 * Increment the last read's failed counter only. The
			 * caller supposed to check if it is a blank page with
			 * bit-flips, or a real ECC error. If the latter, it
			 * should increment the failed counter with this last
			 * read's failed counter
			 */
			host->last_failed++;
			bits_corrected = -EBADMSG;
		} else {
			bits_corrected = ErrNum;

			for (i = 0; i < ((ErrNum + 1) >> 1); ++i) {
				ErrBitLoc[i] = eccread32(ECC_DECEL0_REG32 + i);
				ErrBitLoc1th = ErrBitLoc[i] & 0x1FFF;
				/*
				 * Check if bitflip is in data block (< 0x1000)
				 * or OOB. Fix it only in data block.
				 */
				if (ErrBitLoc1th < 0x1000) {
					ErrByteLoc = ErrBitLoc1th / 8;
					BitOffset = ErrBitLoc1th % 8;
					buf[ErrByteLoc] = buf[ErrByteLoc] ^ (1 << BitOffset);
				}

				mtd->ecc_stats.corrected++;

				ErrBitLoc2nd = (ErrBitLoc[i] >> 16) & 0x1FFF;
				if (0 != ErrBitLoc2nd) {
					/*
					 * Check if bitflip is in data block
					 * (< 0x1000) or OOB. Fix it only in
					 * data block.
					 */
					if (ErrBitLoc2nd < 0x1000) {
						ErrByteLoc = ErrBitLoc2nd / 8;
						BitOffset = ErrBitLoc2nd % 8;
						buf[ErrByteLoc] = buf[ErrByteLoc] ^ (1 << BitOffset);
					}

					mtd->ecc_stats.corrected++;
				}
			}
		}
		if (0 == (eccread16(ECC_DECFER_REG16) & (1 << sector)))
			bits_corrected = -EIO;
	}
	return bits_corrected;
}

static bool mt7621_nand_RFIFOValidSize(u16 size)
{
	u32 timeout = 0xFFFF;

	while (FIFO_RD_REMAIN(regread16(NFI_FIFOSTA_REG16)) < size) {
		timeout--;
		if (0 == timeout)
			return false;
	}
	return true;
}

static bool mt7621_nand_WFIFOValidSize(u16 size)
{
	u32 timeout = 0xFFFF;

	while (FIFO_WR_REMAIN(regread16(NFI_FIFOSTA_REG16)) > size) {
		timeout--;
		if (0 == timeout)
			return false;
	}
	return true;
}

static bool mt7621_nand_status_ready(u32 status)
{
	u32 timeout = 0xFFFF;

	while ((regread32(NFI_STA_REG32) & status) != 0) {
		timeout--;
		if (0 == timeout)
			return false;
	}
	return true;
}

static bool mt7621_nand_reset(void)
{
	int timeout = 0xFFFF;

	if (regread16(NFI_MASTERSTA_REG16)) {
		mb();
		regwrite16(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);
		while (regread16(NFI_MASTERSTA_REG16)) {
			timeout--;
			if (!timeout)
				pr_err("mt7621-nand: Wait for NFI_MASTERSTA timeout\n");
		}
	}

	/* issue reset operation */
	mb();
	regwrite16(NFI_CON_REG16, CON_FIFO_FLUSH | CON_NFI_RST);

	return mt7621_nand_status_ready(STA_NFI_FSM_MASK | STA_NAND_BUSY) &&
	       mt7621_nand_RFIFOValidSize(0) &&
	       mt7621_nand_WFIFOValidSize(0);
}

static void mt7621_nand_set_mode(u16 mode)
{
	u16 v = regread16(NFI_CNFG_REG16);

	v &= ~CNFG_OP_MODE_MASK;
	v |= mode;
	regwrite16(NFI_CNFG_REG16, v);
}

static void mt7621_nand_set_autoformat(bool enable)
{
	if (enable)
		regset16(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
	else
		regclr16(NFI_CNFG_REG16, CNFG_AUTO_FMT_EN);
}

static void mt7621_nand_configure_fdm(u16 FDMsize)
{
	regclr16(NFI_PAGEFMT_REG16, PAGEFMT_FDM_MASK | PAGEFMT_FDM_ECC_MASK);
	regset16(NFI_PAGEFMT_REG16, FDMsize << PAGEFMT_FDM_SHIFT);
	regset16(NFI_PAGEFMT_REG16, FDMsize << PAGEFMT_FDM_ECC_SHIFT);
}

static void mt7621_nand_configure_lock(void)
{
	u32 WriteColNOB = 2;
	u32 WriteRowNOB = 3;
	u32 EraseColNOB = 0;
	u32 EraseRowNOB = 3;

	regwrite16(NFI_LOCKANOB_REG16,
		(WriteColNOB << PROG_CADD_NOB_SHIFT) |
		(WriteRowNOB << PROG_RADD_NOB_SHIFT) |
		(EraseColNOB << ERASE_CADD_NOB_SHIFT) |
		(EraseRowNOB << ERASE_RADD_NOB_SHIFT));
}

static bool mt7621_nand_pio_ready(void)
{
	int count = 0;

	while (!(regread16(NFI_PIO_DIRDY_REG16) & 1)) {
		count++;
		if (count > 0xffff) {
			pr_err("mt7621-nand: PIO_DIRDY timeout\n");
			return false;
		}
	}

	return true;
}

static bool mt7621_nand_set_command(u16 command)
{
	mb();
	regwrite16(NFI_CMD_REG16, command);
	return mt7621_nand_status_ready(STA_CMD_STATE);
}

static bool mt7621_nand_set_address(u32 column, u32 row, u16 colNOB, u16 rowNOB)
{
	mb();
	regwrite32(NFI_COLADDR_REG32, column);
	regwrite32(NFI_ROWADDR_REG32, row);
	regwrite16(NFI_ADDRNOB_REG16, colNOB | (rowNOB << ADDR_ROW_NOB_SHIFT));
	return mt7621_nand_status_ready(STA_ADDR_STATE);
}

static void mt7621_cmd_ctrl(struct nand_chip *chip, int dat,
				unsigned int ctrl)
{
	if (ctrl & NAND_ALE) {
		mt7621_nand_set_address(dat, 0, 1, 0);
	} else if (ctrl & NAND_CLE) {
		mt7621_nand_reset();
		mt7621_nand_set_mode(0x6000);
		mt7621_nand_set_command(dat);
	}
}

static bool mt7621_nand_check_RW_count(u16 writesize)
{
	u32 timeout = 0xFFFF;
	u16 sec = writesize >> 9;

	while (ADDRCNTR_CNTR(regread16(NFI_ADDRCNTR_REG16)) < sec) {
		timeout--;
		if (0 == timeout) {
			pr_warn("mt7621-nand: [%s] timeout\n", __FUNCTION__);
			return false;
		}
	}
	return true;
}

/*
 * Reset NFI HW internal state machine and flush NFI in/out FIFO
 */
static bool mt7621_nand_ready_for_read(struct nand_chip *chip, u32 row,
				       u32 column, bool full, u8 *buf)
{
	u16 sec = 1 << (chip->page_shift - 9);
	u32 colnob = 2, rownob = host->addr_cycles - 2;
	bool ret = false;

	if (chip->options & NAND_BUSWIDTH_16)
		column /= 2;

	if (!mt7621_nand_reset())
		goto cleanup;

	regset16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
	mt7621_nand_set_mode(CNFG_OP_READ);
	regset16(NFI_CNFG_REG16, CNFG_READ_EN);
	regwrite16(NFI_CON_REG16, sec << CON_NFI_SEC_SHIFT);

	if (full) {
		regclr16(NFI_CNFG_REG16, CNFG_AHB);
		regset16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
	} else {
		regclr16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		regclr16(NFI_CNFG_REG16, CNFG_AHB);
	}

	mt7621_nand_set_autoformat(full);
	if (full)
		ecc_decode_start();
	if (!mt7621_nand_set_command(NAND_CMD_READ0))
		goto cleanup;
	if (!mt7621_nand_set_address(column, row, colnob, rownob))
		goto cleanup;
	if (!mt7621_nand_set_command(NAND_CMD_READSTART))
		goto cleanup;
	if (!mt7621_nand_status_ready(STA_NAND_BUSY))
		goto cleanup;

	ret = true;

cleanup:
	return ret;
}

static bool mt7621_nand_ready_for_write(struct nand_chip *chip, u32 row,
					u32 column, bool full, u8 *buf)
{
	u32 sec = 1 << (chip->page_shift - 9);
	u32 colnob = 2, rownob = host->addr_cycles - 2;
	bool ret = false;

	if (chip->options & NAND_BUSWIDTH_16)
		column /= 2;

	/* Reset NFI HW internal state machine and flush NFI in/out FIFO */
	if (!mt7621_nand_reset())
		return false;

	mt7621_nand_set_mode(CNFG_OP_PRGM);

	regclr16(NFI_CNFG_REG16, CNFG_READ_EN);

	regwrite16(NFI_CON_REG16, sec << CON_NFI_SEC_SHIFT);

	if (full) {
		regclr16(NFI_CNFG_REG16, CNFG_AHB);
		regset16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
	} else {
		regclr16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		regclr16(NFI_CNFG_REG16, CNFG_AHB);
	}

	mt7621_nand_set_autoformat(full);

	if (full)
		ecc_encode_start();

	if (!mt7621_nand_set_command(NAND_CMD_SEQIN))
		goto cleanup;
	/* FIXED ME: For Any Kind of AddrCycle */
	if (!mt7621_nand_set_address(column, row, colnob, rownob))
		goto cleanup;

	if (!mt7621_nand_status_ready(STA_NAND_BUSY))
		goto cleanup;

	ret = true;

cleanup:
	return ret;
}

static bool mt7621_nand_check_dececc_done(u32 sec)
{
	u32 timeout, dec_mask;

	timeout = 0xffff;
	dec_mask = (1 << sec) - 1;
	while ((dec_mask != eccread16(ECC_DECDONE_REG16)) && timeout > 0)
		timeout--;
	if (timeout == 0) {
		pr_err("mt7621-nand: ECC_DECDONE: timeout\n");
		return false;
	}
	return true;
}

static bool mt7621_nand_mcu_read_data(u8 *buf, u32 length)
{
	int timeout = 0xffff;
	u32 *buf32 = (u32 *) buf;
	u32 i;

	if ((u32) buf % 4 || length % 4)
		regset16(NFI_CNFG_REG16, CNFG_BYTE_RW);
	else
		regclr16(NFI_CNFG_REG16, CNFG_BYTE_RW);

	/* regwrite32(NFI_STRADDR_REG32, 0); */
	mb();
	regset16(NFI_CON_REG16, CON_NFI_BRD);

	if ((u32) buf % 4 || length % 4) {
		for (i = 0; (i < (length)) && (timeout > 0);) {
			if (regread16(NFI_PIO_DIRDY_REG16) & 1) {
				*buf++ = (u8) regread32(NFI_DATAR_REG32);
				i++;
			} else {
				timeout--;
			}
			if (0 == timeout) {
				pr_warn("mt7621-nand: [%s] timeout\n", __FUNCTION__);
				return false;
			}
		}
	} else {
		for (i = 0; (i < (length >> 2)) && (timeout > 0);) {
			if (regread16(NFI_PIO_DIRDY_REG16) & 1) {
				*buf32++ = regread32(NFI_DATAR_REG32);
				i++;
			} else {
				timeout--;
			}
			if (0 == timeout) {
				pr_warn("mt7621-nand: [%s] timeout\n", __FUNCTION__);
				return false;
			}
		}
	}
	return true;
}

static bool mt7621_nand_read_page_data(struct mtd_info *mtd, u8 *buf, u32 size)
{
	return mt7621_nand_mcu_read_data(buf, size);
}

static bool mt7621_nand_mcu_write_data(struct mtd_info *mtd, const u8 *buf,
				       u32 length)
{
	u32 timeout = 0xFFFF;
	u32 *buf32;
	u32 i;

	regclr16(NFI_CNFG_REG16, CNFG_BYTE_RW);
	mb();
	regset16(NFI_CON_REG16, CON_NFI_BWR);
	buf32 = (u32 *) buf;

	if ((u32) buf % 4 || length % 4)
		regset16(NFI_CNFG_REG16, CNFG_BYTE_RW);
	else
		regclr16(NFI_CNFG_REG16, CNFG_BYTE_RW);

	if ((u32) buf % 4 || length % 4) {
		for (i = 0; (i < (length)) && (timeout > 0);) {
			if (regread16(NFI_PIO_DIRDY_REG16) & 1) {
				regwrite32(NFI_DATAW_REG32, *buf++);
				i++;
			} else {
				timeout--;
			}
			if (0 == timeout) {
				pr_warn("mt7621-nand: [%s] timeout\n", __FUNCTION__);
				return false;
			}
		}
	} else {
		for (i = 0; (i < (length >> 2)) && (timeout > 0);) {
			if (regread16(NFI_PIO_DIRDY_REG16) & 1) {
				regwrite32(NFI_DATAW_REG32, *buf32++);
				i++;
			} else {
				timeout--;
			}
			if (0 == timeout) {
				pr_warn("mt7621-nand: [%s] timeout\n", __FUNCTION__);
				return false;
			}
		}
	}

	return true;
}

static bool mt7621_nand_write_page_data(struct mtd_info *mtd, u8 *buf, u32 size)
{
	return mt7621_nand_mcu_write_data(mtd, buf, size);
}

static void mt7621_nand_read_fdm_data(u8 *buf, u32 sec)
{
	u32 *buf32 = (u32 *) buf;
	u32 i;

	if (buf32) {
		for (i = 0; i < sec; ++i) {
			*buf32++ = regread32(NFI_FDM0L_REG32 + (i << 3));
			*buf32++ = regread32(NFI_FDM0M_REG32 + (i << 3));
		}
	}
}

static u8 fdm_buf[64];

static void mt7621_nand_write_fdm_data(struct nand_chip *chip, u8 *buf, u32 sec)
{
	struct nand_oobfree *free_entry;
	bool empty = true;
	u8 checksum = 0;
	u32 *buf32;
	u32 i, j;

	memcpy(fdm_buf, buf, sec * 8);

	free_entry = layout->oobfree;
	for (i = 0; i < MTD_MAX_OOBFREE_ENTRIES && free_entry[i].length; i++) {
		for (j = 0; j < free_entry[i].length; j++) {
			if (buf[free_entry[i].offset + j] != 0xFF)
				empty = false;
			checksum ^= buf[free_entry[i].offset + j];
		}
	}

	if (!empty) {
		fdm_buf[free_entry[i - 1].offset + free_entry[i - 1].length] = checksum;
	}

	buf32 = (u32 *) fdm_buf;
	for (i = 0; i < sec; ++i) {
		regwrite32(NFI_FDM0L_REG32 + (i << 3), *buf32++);
		regwrite32(NFI_FDM0M_REG32 + (i << 3), *buf32++);
	}
}

static void mt7621_nand_stop_read(void)
{
	regclr16(NFI_CON_REG16, CON_NFI_BRD);
	mt7621_nand_reset();
	ecc_decode_end();
	regwrite16(NFI_INTR_EN_REG16, 0);
}

static void mt7621_nand_stop_write(void)
{
	regclr16(NFI_CON_REG16, CON_NFI_BWR);
	ecc_encode_end();
	regwrite16(NFI_INTR_EN_REG16, 0);
}

/*
 * This is a copy of the nand_check_erased_buf() function from nand_base.c, to
 * keep the nand_base.c clean
 */
static int mt7621_nand_check_erased_buf(void *buf, int len,
					int bitflips_threshold)
{
	const unsigned char *bitmap = buf;
	int bitflips = 0;
	int weight;

	for (; len && ((uintptr_t)bitmap) % sizeof(long);
	     len--, bitmap++) {
		weight = hweight8(*bitmap);
		bitflips += BITS_PER_BYTE - weight;
		if (unlikely(bitflips > bitflips_threshold))
			return -EBADMSG;
	}

	for (; len >= sizeof(long);
	     len -= sizeof(long), bitmap += sizeof(long)) {
		unsigned long d = *((unsigned long *)bitmap);

		if (d == ~0UL)
			continue;
		weight = hweight_long(d);
		bitflips += BITS_PER_LONG - weight;
		if (unlikely(bitflips > bitflips_threshold))
			return -EBADMSG;
	}

	for (; len > 0; len--, bitmap++) {
		weight = hweight8(*bitmap);
		bitflips += BITS_PER_BYTE - weight;
		if (unlikely(bitflips > bitflips_threshold))
			return -EBADMSG;
	}

	return bitflips;
}

/*
 * This is the modified version of the nand_check_erased_ecc_chunk() in
 * nand_base.c. This driver cannot use the generic function, as the ECC layout
 * is slightly different (the 2k(data)+64(OOB) page is splitted up to 4
 * (512-byte data + 16-byte OOB) pages. Each of these sectors have 4-bit ECC, so
 * we check them separately, and allow 4 bitflips / sector.
 * We'll fix up the page to all-0xFF only if all sectors in the page appears to
 * be blank
 */
static int mt7621_nand_check_erased_ecc_chunk(void *data, int datalen,
					      void *oob, int ooblen,
					      int bitflips_threshold)
{
	int data_bitflips = 0, oob_bitflips = 0;

	data_bitflips = mt7621_nand_check_erased_buf(data, datalen,
						     bitflips_threshold);
	if (data_bitflips < 0)
		return data_bitflips;

	bitflips_threshold -= data_bitflips;

	oob_bitflips = mt7621_nand_check_erased_buf(oob, ooblen,
						    bitflips_threshold);
	if (oob_bitflips < 0)
		return oob_bitflips;

	bitflips_threshold -= oob_bitflips;

	return data_bitflips + oob_bitflips;
}

static int mt7621_nand_read_oob_raw(struct mtd_info *mtd, u8 *buf,
				    int page, int len)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	u32 column = 0;
	u32 sector = 0;
	int res = 0;
	int read_len = 0;
	int sec_num = 1 << (chip->page_shift - 9);
	int spare_per_sector = mtd->oobsize / sec_num;

	if (len > NAND_MAX_OOBSIZE || len % OOB_AVAI_PER_SECTOR || !buf) {
		pr_warn("mt7621-nand: invalid parameter, len: %d, buf: %p\n", len, buf);
		return -EINVAL;
	}

	while (len > 0) {
		read_len = min(len, spare_per_sector);
		column = NAND_SECTOR_SIZE + sector * (NAND_SECTOR_SIZE + spare_per_sector); /* TODO: Fix this hard-code 16 */
		if (!mt7621_nand_ready_for_read(chip, page, column, false, NULL)) {
			pr_warn("mt7621-nand: mt7621_nand_ready_for_read() return failed\n");
			res = -EIO;
			goto error;
		}
		if (!mt7621_nand_mcu_read_data(buf + spare_per_sector * sector, read_len)) {
			pr_warn("mt7621-nand: mt7621_nand_mcu_read_data() return failed\n");
			res = -EIO;
			goto error;
		}
		mt7621_nand_check_RW_count(read_len);
		mt7621_nand_stop_read();
		sector++;
		len -= read_len;
	}

error:
	regclr16(NFI_CON_REG16, CON_NFI_BRD);
	return res;
}

static int mt7621_nand_write_oob_raw(struct mtd_info *mtd, const u8 *buf,
				     int page, int len)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	u32 column = 0;
	u32 sector = 0;
	int write_len = 0;
	int status;
	int sec_num = 1 << (chip->page_shift - 9);
	int spare_per_sector = mtd->oobsize / sec_num;

	if (len > NAND_MAX_OOBSIZE || len % OOB_AVAI_PER_SECTOR || !buf) {
		pr_warn("mt7621-nand: invalid parameter, len: %d, buf: %p\n", len, buf);
		return -EINVAL;
	}

	while (len > 0) {
		write_len = min(len, spare_per_sector);
		column = sector * (NAND_SECTOR_SIZE + spare_per_sector) + NAND_SECTOR_SIZE;
		if (!mt7621_nand_ready_for_write(chip, page, column, false, NULL))
			return -EIO;
		if (!mt7621_nand_mcu_write_data(mtd, buf + sector * spare_per_sector, write_len))
			return -EIO;
		mt7621_nand_check_RW_count(write_len);
		regclr16(NFI_CON_REG16, CON_NFI_BWR);
		mt7621_nand_set_command(NAND_CMD_PAGEPROG);
		while (regread32(NFI_STA_REG32) & STA_NAND_BUSY)
			;
		status = chip->legacy.waitfunc(chip);
		if (status & NAND_STATUS_FAIL) {
			pr_warn("mt7621-nand: failed status: %d\n", status);
			return -EIO;
		}
		len -= write_len;
		sector++;
	}

	return 0;
}

static int mt7621_nand_exec_read_page(struct mtd_info *mtd, u32 row,
				       u32 PageSize, u8 *buf, u8 *FDMbuf)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct mt7621_nand_host *host = nand_get_controller_data(chip);
	u32 sec = PageSize >> 9;
	int bits_corrected = 0;

	host->last_failed = 0;

	if (mt7621_nand_ready_for_read(chip, row, 0, true, buf)) {
		int j;
		for (j = 0; j < sec; j++) {
			int ret;

			if (!mt7621_nand_read_page_data(mtd, buf + j * 512, 512)) {
				bits_corrected = -EIO;
				break;
			}
			if (!mt7621_nand_check_dececc_done(j + 1)) {
				bits_corrected = -EIO;
				break;
			}
			ret = mt7621_nand_check_bch_error(mtd, buf + j * 512,
							  j, row);
			if (ret < 0) {
				bits_corrected = ret;
				if (ret != -EBADMSG)
					break;
			} else if (bits_corrected >= 0) {
				bits_corrected = max_t(int, bits_corrected,
						       ret);
			}
		}
		if (!mt7621_nand_status_ready(STA_NAND_BUSY))
			bits_corrected = -EIO;

		mt7621_nand_read_fdm_data(FDMbuf, sec);
		mt7621_nand_stop_read();
	} else {
		bits_corrected = -EIO;
	}

	if (bits_corrected == -EBADMSG) {
		u32 local_oob_aligned[NAND_MAX_OOBSIZE / sizeof(u32)];
		u8 *local_oob = (u8 *)local_oob_aligned;
		int ret;

		/*
		 * If there was an uncorrectable ECC error, check if it is a
		 * blank page with bit-flips. For this, we check the number of
		 * 0s in each sector (including the OOB), which should not
		 * exceed the ECC strength. Until the number of 0s is lower or
		 * equal, we consider it as a blank page
		 */

		ret = mt7621_nand_read_oob_raw(mtd, local_oob, row,
					       mtd->oobsize);
		if (ret == 0) {
			int spare_per_sector = mtd->oobsize / sec;
			int sector_size = PageSize / sec;
			int i;
			u32 corrected = 0;
			int max_bitflips = 0;

			for (i = 0; i < sec; i++) {
				ret = mt7621_nand_check_erased_ecc_chunk(
					&buf[i * sector_size], sector_size,
					&local_oob[i * spare_per_sector],
					spare_per_sector, chip->ecc.strength);
				if (ret < 0)
					break;

				max_bitflips = max_t(int, max_bitflips, ret);
				corrected += ret;
			}

			if (ret >= 0) {
				mtd->ecc_stats.corrected += corrected;

				memset(buf, 0xff, PageSize);
				memset(FDMbuf, 0xff, sizeof(u32) * 2 * sec);

				bits_corrected = max_bitflips;
			} else {
				mtd->ecc_stats.failed += host->last_failed;
				pr_err("mt7621-nand: uncorrectable ECC errors at page=%d\n",
				       row);
			}
		} else {
			mtd->ecc_stats.failed += host->last_failed;
			pr_warn("mt7621-nand: [%s] failed to read oob after ECC error\n",
				__func__);
			/* Keep return value as -EBADMSG */
		}
	}

	return bits_corrected;
}

static int mt7621_nand_exec_write_page(struct mtd_info *mtd, u32 row,
				       u32 PageSize, u8 *buf, u8 *FDMbuf)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	u32 sec = PageSize >> 9;
	u8 status;

	if (mt7621_nand_ready_for_write(chip, row, 0, true, buf)) {
		mt7621_nand_write_fdm_data(chip, FDMbuf, sec);
		mt7621_nand_write_page_data(mtd, buf, PageSize);
		mt7621_nand_check_RW_count(PageSize);
		mt7621_nand_stop_write();
		mt7621_nand_set_command(NAND_CMD_PAGEPROG);
		while (regread32(NFI_STA_REG32) & STA_NAND_BUSY)
			;
	}

	status = chip->legacy.waitfunc(chip);
	if (status & NAND_STATUS_FAIL)
		return -EIO;
	return 0;
}

static void mt7621_nand_command_bp(struct nand_chip *chip, unsigned int command,
				   int column, int page)
{
	struct mtd_info *mtd = nand_to_mtd(chip);

	switch (command) {
	case NAND_CMD_SEQIN:
		memset(host->OOB, 0xFF, sizeof(host->OOB));
		host->data_buf = NULL;
		host->row = page;
		host->column = column;
		break;

	case NAND_CMD_PAGEPROG:
		if (host->data_buf || (0xFF != host->OOB[0])) {
			u8 *buf = host->data_buf ? host->data_buf : chip->data_buf;
			mt7621_nand_exec_write_page(mtd, host->row, mtd->writesize, buf, host->OOB);
			host->row = (u32) - 1;
			host->OOBrow = (u32) - 1;
		}
		break;

	case NAND_CMD_READOOB:
		host->row = page;
		host->column = column + mtd->writesize;
		break;

	case NAND_CMD_READ0:
		host->row = page;
		host->column = column;
		break;

	case NAND_CMD_ERASE1:
		mt7621_nand_reset();
		mt7621_nand_set_mode(CNFG_OP_ERASE);
		mt7621_nand_set_command(NAND_CMD_ERASE1);
		mt7621_nand_set_address(0, page, 0, host->addr_cycles - 2);
		break;

	case NAND_CMD_ERASE2:
		mt7621_nand_set_command(NAND_CMD_ERASE2);
		while (regread32(NFI_STA_REG32) & STA_NAND_BUSY)
			;
		break;

	case NAND_CMD_STATUS:
		mt7621_nand_reset();
		regclr16(NFI_CNFG_REG16, CNFG_BYTE_RW);
		mt7621_nand_set_mode(CNFG_OP_SRD);
		mt7621_nand_set_mode(CNFG_READ_EN);
		regclr16(NFI_CNFG_REG16, CNFG_AHB);
		regclr16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		mt7621_nand_set_command(NAND_CMD_STATUS);
		regclr16(NFI_CON_REG16, CON_NFI_NOB_MASK);
		mb();
		regwrite16(NFI_CON_REG16, CON_NFI_SRD | (1 << CON_NFI_NOB_SHIFT));
		host->cmdstatus = true;
		break;

	case NAND_CMD_RESET:
		mt7621_nand_reset();
		regwrite16(NFI_INTR_EN_REG16, INTR_RST_DONE_EN);
		mt7621_nand_set_command(NAND_CMD_RESET);
		regwrite16(NFI_CNRNB_REG16, 0xF1);
		while (!(regread16(NFI_INTR_REG16) & INTR_RST_DONE_EN))
			;
		break;

	case NAND_CMD_READID:
		mt7621_nand_reset();
		/* Disable HW ECC */
		regclr16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		regclr16(NFI_CNFG_REG16, CNFG_AHB);
		regset16(NFI_CNFG_REG16, CNFG_READ_EN | CNFG_BYTE_RW);
		mt7621_nand_reset();
		mb();
		mt7621_nand_set_mode(CNFG_OP_SRD);
		mt7621_nand_set_command(NAND_CMD_READID);
		mt7621_nand_set_address(0, 0, 1, 0);
		regwrite16(NFI_CON_REG16, CON_NFI_SRD);
		while (regread32(NFI_STA_REG32) & STA_DATAR_STATE)
			;
		break;

	default:
		BUG();
		break;
	}
}

static void mt7621_nand_set_ecc_mode(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	//struct mt7621_nand_host *host = nand_get_controller_data(chip);
	u32 spare_per_sector = mtd->oobsize / (mtd->writesize / 512);
	u32 spare_bit = PAGEFMT_SPARE_16;
	u32 ecc_bit = 4;

	if (spare_per_sector >= 28) {
		spare_bit = PAGEFMT_SPARE_28;
		ecc_bit = 12;
		spare_per_sector = 28;
	} else if (spare_per_sector >= 27) {
		spare_bit = PAGEFMT_SPARE_27;
		ecc_bit = 8;
		spare_per_sector = 27;
	} else if (spare_per_sector >= 26) {
		spare_bit = PAGEFMT_SPARE_26;
		ecc_bit = 8;
		spare_per_sector = 26;
	} else if (spare_per_sector >= 16) {
		spare_bit = PAGEFMT_SPARE_16;
		ecc_bit = 4;
		spare_per_sector = 16;
	} else {
		pr_warn("nand: NFI not support oobsize: %x\n", spare_per_sector);
	}
	chip->ecc.strength = ecc_bit;
	mtd->oobsize = spare_per_sector * (mtd->writesize / 512);
	pr_info("nand: ecc bit: %d, spare_per_sector: %d\n", ecc_bit, spare_per_sector);
	/* Setup PageFormat */
	if (4096 == mtd->writesize) {
		regset16(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_4K);
		chip->legacy.cmdfunc = mt7621_nand_command_bp;
	} else if (2048 == mtd->writesize) {
		regset16(NFI_PAGEFMT_REG16, (spare_bit << PAGEFMT_SPARE_SHIFT) | PAGEFMT_2K);
		chip->legacy.cmdfunc = mt7621_nand_command_bp;
	}
	ecc_config(ecc_bit);
}

static void mt7621_nand_select_chip(struct nand_chip *chip, int nr)
{
	switch (nr) {
	case -1:
		break;
	case 0:
	case 1:
		/* Jun Shen, 2011.04.13 */
		/* Note: MT6577 EVB NAND is mounted on CS0, but FPGA is CS1 */
		regwrite16(NFI_CSEL_REG16, nr);
		/* Jun Shen, 2011.04.13 */
		break;
	}
}

static u8 mt7621_nand_read_byte(struct nand_chip *chip)
{
	u8 retval = 0;

	if (!mt7621_nand_pio_ready()) {
		pr_err("mt7621-nand: pio ready timeout\n");
		retval = false;
	}

	if (host->cmdstatus) {
		retval = regread8(NFI_DATAR_REG32);
		regclr16(NFI_CON_REG16, CON_NFI_NOB_MASK);
		mt7621_nand_reset();
		regset16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		host->cmdstatus = false;
	} else {
		retval = regread8(NFI_DATAR_REG32);
	}

	return retval;
}

static void mt7621_nand_read_buf(struct nand_chip *chip, u8 *buf, int len)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	u32 PageSize = mtd->writesize;
	u32 column = host->column;

	if (column < PageSize) {
		if ((column == 0) && (len >= PageSize)) {
			mt7621_nand_exec_read_page(mtd, host->row, PageSize, buf, host->OOB);
			if (len > PageSize) {
				u32 size = min(len - PageSize, sizeof(host->OOB));
				memcpy(buf + PageSize, host->OOB, size);
			}
		} else {
			mt7621_nand_exec_read_page(mtd, host->row, PageSize, chip->data_buf, host->OOB);
			memcpy(buf, chip->data_buf + column, len);
		}
		host->OOBrow = host->row;
	} else {
		u32 offset = column - PageSize;
		u32 size = min(len - offset, sizeof(host->OOB));
		if (host->OOBrow != host->row) {
			mt7621_nand_exec_read_page(mtd, host->row, PageSize, chip->data_buf, host->OOB);
			host->OOBrow = host->row;
		}
		memcpy(buf, host->OOB + offset, size);
	}
	host->column += len;
}

static void mt7621_nand_write_buf(struct nand_chip *chip, const u8 *buf,
				  int len)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	u32 PageSize = mtd->writesize;
	u32 column = host->column;
	int size, i;

	if (column >= PageSize) {
		u32 offset = column - PageSize;
		u8 *OOB = host->OOB + offset;
		size = min(len, (int)(sizeof(host->OOB) - offset));
		for (i = 0; i < size; i++)
			OOB[i] &= buf[i];
	} else {
		host->data_buf = (u8 *) buf;
	}

	host->column += len;
}

static int mt7621_nand_write_page_hwecc(struct nand_chip *chip, const u8 *buf,
					int oob_required, int page)
{
	struct mtd_info *mtd = nand_to_mtd(chip);

	nand_prog_page_begin_op(chip, page, 0, buf, mtd->writesize);
	mt7621_nand_write_buf(chip, chip->oob_poi, mtd->oobsize);
	return nand_prog_page_end_op(chip);
}

static int mt7621_nand_read_page_hwecc(struct nand_chip *chip, u8 *buf,
				       int oob_required, int page)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	int bits_corrected;

	bits_corrected = mt7621_nand_exec_read_page(mtd, page, mtd->writesize,
						    buf, chip->oob_poi);

	return (bits_corrected == -EBADMSG) ? 0 : bits_corrected;
}

static int mt7621_nand_write_oob_hw(struct nand_chip *chip, int page)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	int sec_num = 1 << (chip->page_shift - 9);
	int spare_per_sector = mtd->oobsize / sec_num;
	int i, iter;

	memcpy(local_oob_buf, chip->oob_poi, mtd->oobsize);

	/* copy ecc data */
	for (i = 0; i < layout->eccbytes; i++) {
		iter = (i / (spare_per_sector-OOB_AVAI_PER_SECTOR)) * spare_per_sector + OOB_AVAI_PER_SECTOR + i % (spare_per_sector-OOB_AVAI_PER_SECTOR);
		local_oob_buf[iter] = chip->oob_poi[layout->eccpos[i]];
	}

	/* copy FDM data */
	for (i = 0; i < sec_num; i++)
		memcpy(&local_oob_buf[i * spare_per_sector], &chip->oob_poi[i * OOB_AVAI_PER_SECTOR], OOB_AVAI_PER_SECTOR);

	return mt7621_nand_write_oob_raw(mtd, local_oob_buf, page, mtd->oobsize);
}

static int mt7621_nand_write_oob(struct nand_chip *chip, int page)
{
	int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
	int block = page / page_per_block;
	int page_in_block = page % page_per_block;

	do {
		if (mt7621_nand_write_oob_hw(chip, page_in_block + block * page_per_block)) {
			pr_warn("mt7621-nand: write oob fail at block: 0x%x, page: 0x%x\n", block, page_in_block);
			return NAND_STATUS_FAIL;
		} else {
			break;
		}
	} while (1);

	return 0;
}

static int mt7621_nand_read_oob_hw(struct nand_chip *chip, int page)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	int sec_num = 1 << (chip->page_shift - 9);
	int spare_per_sector = mtd->oobsize / sec_num;
	int i;
	u8 iter = 0;

	if (mt7621_nand_read_oob_raw(mtd, chip->oob_poi, page, mtd->oobsize)) {
		pr_err("mt7621-nand: mt7621_nand_read_oob_raw() return failed\n");
		return -EIO;
	}

	/* adjust to ecc physical layout to memory layout */
	/*********************************************************/
	/* FDM0 | ECC0 | FDM1 | ECC1 | FDM2 | ECC2 | FDM3 | ECC3 */
	/*  8B  |  8B  |  8B  |  8B  |  8B  |  8B  |  8B  |  8B  */
	/*********************************************************/

	memcpy(local_oob_buf, chip->oob_poi, mtd->oobsize);

	/* copy ecc data */
	for (i = 0; i < layout->eccbytes; i++) {
		iter = (i / (spare_per_sector-OOB_AVAI_PER_SECTOR)) * spare_per_sector + OOB_AVAI_PER_SECTOR + i % (spare_per_sector-OOB_AVAI_PER_SECTOR);
		chip->oob_poi[layout->eccpos[i]] = local_oob_buf[iter];
	}

	/* copy FDM data */
	for (i = 0; i < sec_num; i++) {
		memcpy(&chip->oob_poi[i * OOB_AVAI_PER_SECTOR], &local_oob_buf[i * spare_per_sector], OOB_AVAI_PER_SECTOR);
	}

	return 0;
}

static int mt7621_nand_read_oob(struct nand_chip *chip, int page)
{
	int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
	int block = page / page_per_block;
	int page_in_block = page % page_per_block;

	if (mt7621_nand_read_oob_hw(chip, page_in_block + block * page_per_block) != 0)
		return -1;
	return 0;
}

static int mt7621_nand_block_bad_hw(struct nand_chip *chip, loff_t ofs)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	int page = (int)(ofs >> chip->page_shift);
	unsigned int page_per_block = 1 << (chip->phys_erase_shift - chip->page_shift);
	u8 oob_buf[8];

	page &= ~(page_per_block - 1);
	if (mt7621_nand_read_oob_raw(mtd, oob_buf, page, sizeof(oob_buf))) {
		pr_warn("mt7621-nand: mt7621_nand_read_oob_raw() return error\n");
		return 1;
	}

	if (oob_buf[0] != 0xff) {
		pr_warn("mt7621-nand: bad block detected at page=%d\n", page);
		return 1;
	}

	return 0;
}

static int mt7621_nand_block_bad(struct nand_chip *chip, loff_t ofs)
{
	int block = (int)ofs >> chip->phys_erase_shift;

	return mt7621_nand_block_bad_hw(chip, block << chip->phys_erase_shift);
}

static void mt7621_nand_init_hw(struct mt7621_nand_host *host)
{
	host->OOBrow = (u32) - 1;

	/* Set default NFI access timing control */
	regwrite32(NFI_ACCCON_REG32, host->access_timing);
	regwrite16(NFI_CNFG_REG16, 0);
	regwrite16(NFI_PAGEFMT_REG16, 0);

	/* Reset the state machine and data FIFO, because flushing FIFO */
	mt7621_nand_reset();

	/* Set the ECC engine */
	if (host->nand_chip.ecc.mode == NAND_ECC_HW) {
		regset16(NFI_CNFG_REG16, CNFG_HW_ECC_EN);
		ecc_config(4);
		mt7621_nand_configure_fdm(8);
		mt7621_nand_configure_lock();
	}

	regset16(NFI_IOCON_REG16, 0x47);
}

static int mt7621_nand_dev_ready(struct nand_chip *chip)
{
	return !(regread32(NFI_STA_REG32) & STA_NAND_BUSY);
}

static int oob_mt7621_ooblayout_ecc(struct mtd_info *mtd, int section,
				    struct mtd_oob_region *oobregion)
{
	oobregion->length = 8;
	oobregion->offset = layout->eccpos[section * 8];

	return 0;
}

static int oob_mt7621_ooblayout_free(struct mtd_info *mtd, int section,
				     struct mtd_oob_region *oobregion)
{
	if (section >= (layout->eccbytes / 8))
		return -ERANGE;
	oobregion->offset = layout->oobfree[section].offset;
	oobregion->length = layout->oobfree[section].length;

	return 0;
}

/*
 * Code to support the legacy mediatek nand flash bad block table.
 * The default for this driver is to use the standard Linux bad block
 * table format. However you need a new boot loader that supports that.
 * The old (and most often used) medaitek boot loaders use their own
 * BBT format, and this code implements that. There is a devicetree
 * binding that enables use of this.
 */
#define BBT_BLOCK_NUM_DEFAULT	32
#define BBT_OOB_SIGNATURE	1
#define BBT_SIGNATURE_LEN	7

const u8 oob_signature[] = "mtknand";
static u32 bbt_size;

static int mt7621_read_legacy_bbt_page(struct mtd_info *mtd, unsigned int page)
{
	struct nand_chip *chip = mtd_to_nand(mtd);

	if (mt7621_nand_read_oob_hw(chip, page) == 0) {
		int bits_corrected;

		if (chip->oob_poi[0] != 0xff) {
			pr_info("mt7621-nand: Bad Block on page=%d\n", page);
			return -ENODEV;
		}
		if (memcmp(&chip->oob_poi[BBT_OOB_SIGNATURE], oob_signature, BBT_SIGNATURE_LEN) != 0) {
			pr_info("mt7621-nand: no BBT signature, page=%d\n", page);
			return -EINVAL;
		}
		bits_corrected = mt7621_nand_exec_read_page(
			mtd, page, mtd->writesize, chip->data_buf,
			chip->oob_poi);
		if (bits_corrected >= 0) {
			int bbt_bytes = (bbt_size <= mtd->writesize) ? bbt_size : mtd->writesize;
			pr_info("mt7621-nand: BBT signature match, page=%d\n", page);
			memcpy(chip->bbt, chip->data_buf, bbt_bytes);
			return 0;
		}
	}

	pr_err("mt7621-nand: legacy BBT read failed at page %d\n", page);
	return -EIO;
}

static int mt7621_load_legacy_bbt(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct mt7621_nand_host *host = nand_get_controller_data(chip);
	u32 blocks;
	int i;

	blocks = 0x1 << (chip->chip_shift - chip->phys_erase_shift);
	bbt_size = blocks >> 2;

	if (chip->bbt == NULL) {
		chip->bbt = kzalloc(bbt_size, GFP_KERNEL);
		if (chip->bbt == NULL)
			return -ENOMEM;
	}

	for (i = blocks - 1; i >= (blocks - host->legacybbt_block_num); i--) {
		int page = i << (chip->phys_erase_shift - chip->page_shift);
		if (mt7621_read_legacy_bbt_page(mtd, page) == 0) {
			pr_info("mt7621-nand: loading BBT success (%d)\n", page);
			return 0;
		}
	}

	pr_err("mt7621-nand: loading Bad Block Table failed\n");
	return -ENODEV;
}

static int mt7621_nand_attach(struct nand_chip *chip)
{
	struct mtd_info *mtd = nand_to_mtd(chip);
	int i;

	mt7621_nand_set_ecc_mode(mtd);

	if (nanddev_target_size(&chip->base) < (256*1024*1024))
		host->addr_cycles = 4;

	/* allocate buffers or call select_chip here or a bit earlier*/
	chip->data_buf = kzalloc(mtd->writesize + mtd->oobsize, GFP_KERNEL);
	chip->ecc.calc_buf = kzalloc(mtd->oobsize, GFP_KERNEL);
	chip->ecc.code_buf = kzalloc(mtd->oobsize, GFP_KERNEL);
	if (!chip->data_buf || !chip->ecc.calc_buf || !chip->ecc.code_buf)
		return -ENOMEM;

	chip->oob_poi = chip->data_buf + mtd->writesize;
	chip->badblockpos = 0;
	chip->ecc.size = mtd->writesize;
	chip->ecc.bytes = (mtd->writesize / NAND_SECTOR_SIZE) * 8;

	if (mtd->writesize == 4096)
		layout = &nand_oob_128;
	else if (mtd->writesize == 2048)
		layout = &nand_oob_64;
	else if (mtd->writesize == 512)
		layout = &nand_oob_16;

	layout->eccbytes = mtd->oobsize - OOB_AVAI_PER_SECTOR * (mtd->writesize / NAND_SECTOR_SIZE);
	for (i = 0; i < layout->eccbytes; i++)
		layout->eccpos[i] = OOB_AVAI_PER_SECTOR * (mtd->writesize / NAND_SECTOR_SIZE) + i;

	regwrite32(NFI_ACCCON_REG32, host->access_timing);

	if (chip->options & NAND_BUSWIDTH_16)
		regset16(NFI_PAGEFMT_REG16, PAGEFMT_DBYTE_EN);

	chip->legacy.select_chip(chip, 0);

	return 0;
}

static const struct mtd_ooblayout_ops oob_mt7621_ops = {
	.ecc = oob_mt7621_ooblayout_ecc,
	.free = oob_mt7621_ooblayout_free,
};

static const struct nand_controller_ops mt7621_controller_ops = {
	.attach_chip = mt7621_nand_attach,
};

static int mt7621_nand_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mtd_part_parser_data ppdata;
	struct nand_chip *chip;
	struct mtd_info *mtd;
	struct resource *r;
	int err = 0;

	/* Allocate memory for the device structure (and zero it) */
	host = devm_kzalloc(dev, sizeof(struct mt7621_nand_host), GFP_KERNEL);
	if (!host)
		return -ENOMEM;

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	host->regs = devm_ioremap_resource(dev, r);
	pr_info("mt7621-nand: NAND register bank at 0x%x\n", (int) host->regs);
	if (IS_ERR(host->regs))
		return PTR_ERR(host->regs);

	r = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	host->ecc = devm_ioremap_resource(dev, r);
	pr_info("mt7621-nand: NAND ECC register bank at 0x%x\n", (int) host->ecc);
	if (IS_ERR(host->ecc))
		return PTR_ERR(host->ecc);

	host->access_timing = NFI_DEFAULT_ACCESS_TIMING;
	host->addr_cycles = 5;

	if (of_property_read_bool(dev->of_node, "mediatek,nand-bbt-compat")) {
		pr_info("mt7621-nand: using legacy BBT format\n");
		host->legacybbt = true;
	}
	if (host->legacybbt) {
		err = of_property_read_u32(dev->of_node,
					   "mediatek,nand-bbt-block-num",
					   &host->legacybbt_block_num);
		if (err)
			host->legacybbt_block_num = BBT_BLOCK_NUM_DEFAULT;
	}

	/* init mtd data structure */
	chip = &host->nand_chip;
	chip->priv = host;

	mtd = host->mtd = nand_to_mtd(chip);
	mtd->priv = chip;
	mtd->owner = THIS_MODULE;
	mtd->name = "MT7621-NAND";
	mtd->dev.parent = &pdev->dev;
	nand_set_flash_node(chip, pdev->dev.of_node);

	chip->legacy.select_chip = mt7621_nand_select_chip;
	chip->legacy.chip_delay = 20; /* 20us command delay time */
	chip->legacy.read_byte = mt7621_nand_read_byte;
	chip->legacy.read_buf = mt7621_nand_read_buf;
	chip->legacy.write_buf = mt7621_nand_write_buf;
	chip->legacy.dev_ready = mt7621_nand_dev_ready;
	chip->legacy.cmdfunc = mt7621_nand_command_bp;
	chip->legacy.block_bad = mt7621_nand_block_bad;
	chip->legacy.cmd_ctrl = mt7621_cmd_ctrl;
	chip->legacy.dummy_controller.ops = &mt7621_controller_ops;

	chip->ecc.mode = NAND_ECC_HW;
	chip->ecc.strength = 1;
	chip->ecc.read_page = mt7621_nand_read_page_hwecc;
	chip->ecc.write_page = mt7621_nand_write_page_hwecc;
	chip->ecc.write_oob = mt7621_nand_write_oob;
	chip->ecc.read_oob = mt7621_nand_read_oob;

	chip->options |= NAND_USE_BOUNCE_BUFFER;
	chip->bbt_options |= NAND_BBT_USE_FLASH;
	chip->buf_align = 16;

	if (host->legacybbt)
		chip->options |= NAND_SKIP_BBTSCAN;

	mtd_set_ooblayout(mtd, &oob_mt7621_ops);

	mt7621_nand_init_hw(host);

	err = nand_scan(chip, 1);
	if (err) {
		pr_err("mt7621-nand: failed to identify NAND device\n");
		goto out;
	}

	/* Don't use OOB for JFFS2, as that's not compatible with the ECC */
	mtd->flags &= ~MTD_OOB_WRITEABLE;

	platform_set_drvdata(pdev, host);

	err = mtd_device_parse_register(mtd, probe_types, &ppdata, NULL, 0);
	if (err)
		goto out;

	if (host->legacybbt) {
		err = mt7621_load_legacy_bbt(mtd);
		if (err)
			goto out;
	}

	return 0;

out:
	pr_err("mt7621-nand: probe failed, err=%d\n", err);
	nand_release(chip);
	platform_set_drvdata(pdev, NULL);

	return err;
}

static int mt7621_nand_remove(struct platform_device *pdev)
{
	struct mt7621_nand_host *host = platform_get_drvdata(pdev);
	struct nand_chip *chip = &host->nand_chip;

	nand_release(chip);
	return 0;
}

static const struct of_device_id mt7621_nand_match[] = {
	{ .compatible = "mtk,mt7621-nand" },
	{},
};
MODULE_DEVICE_TABLE(of, mt7621_nand_match);

static struct platform_driver mt7621_nand_driver = {
	.probe = mt7621_nand_probe,
	.remove = mt7621_nand_remove,
	.driver = {
		.name = "MT7621-NAND",
		.owner = THIS_MODULE,
		.of_match_table = mt7621_nand_match,
	},
};
module_platform_driver(mt7621_nand_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MediaTek MT7621 NAND controller driver");
