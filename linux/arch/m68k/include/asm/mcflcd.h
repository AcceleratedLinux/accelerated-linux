/****************************************************************************/

/*
 *	mcflcd.h -- ColdFire 5329 LCD support definitions
 */

/****************************************************************************/
#ifndef	mcflcd_h
#define	mcflcd_h
/****************************************************************************/

#define MCF_REG32(x) (*(volatile unsigned long  *)(x))
#define MCF_REG16(x) (*(volatile unsigned short *)(x))
#define MCF_REG08(x) (*(volatile unsigned char  *)(x))

/*********************************************************************
*
* LCD Controller (LCDC)
*
*********************************************************************/

/* Register read/write macros */
#define MCF_LCDC_LSSAR                  MCF_REG32(0xFC0AC000)
#define MCF_LCDC_LSR                    MCF_REG32(0xFC0AC004)
#define MCF_LCDC_LVPWR                  MCF_REG32(0xFC0AC008)
#define MCF_LCDC_LCPR                   MCF_REG32(0xFC0AC00C)
#define MCF_LCDC_LCWHBR                 MCF_REG32(0xFC0AC010)
#define MCF_LCDC_LCCMR                  MCF_REG32(0xFC0AC014)
#define MCF_LCDC_LPCR                   MCF_REG32(0xFC0AC018)
#define MCF_LCDC_LHCR                   MCF_REG32(0xFC0AC01C)
#define MCF_LCDC_LVCR                   MCF_REG32(0xFC0AC020)
#define MCF_LCDC_LPOR                   MCF_REG32(0xFC0AC024)
#define MCF_LCDC_LSCR                   MCF_REG32(0xFC0AC028)
#define MCF_LCDC_LPCCR                  MCF_REG32(0xFC0AC02C)
#define MCF_LCDC_LDCR                   MCF_REG32(0xFC0AC030)
#define MCF_LCDC_LRMCR                  MCF_REG32(0xFC0AC034)
#define MCF_LCDC_LICR                   MCF_REG32(0xFC0AC038)
#define MCF_LCDC_LIER                   MCF_REG32(0xFC0AC03C)
#define MCF_LCDC_LISR                   MCF_REG32(0xFC0AC040)
#define MCF_LCDC_LGWSAR                 MCF_REG32(0xFC0AC050)
#define MCF_LCDC_LGWSR                  MCF_REG32(0xFC0AC054)
#define MCF_LCDC_LGWVPWR                MCF_REG32(0xFC0AC058)
#define MCF_LCDC_LGWPOR                 MCF_REG32(0xFC0AC05C)
#define MCF_LCDC_LGWPR                  MCF_REG32(0xFC0AC060)
#define MCF_LCDC_LGWCR                  MCF_REG32(0xFC0AC064)
#define MCF_LCDC_LGWDCR                 MCF_REG32(0xFC0AC068)
#define MCF_LCDC_BPLUT_BASE             MCF_REG32(0xFC0AC800)
#define MCF_LCDC_GWLUT_BASE             MCF_REG32(0xFC0ACC00)

/* Bit definitions and macros for MCF_LCDC_LSSAR */
#define MCF_LCDC_LSSAR_SSA(x)           (((x)&0x3FFFFFFF)<<2)

/* Bit definitions and macros for MCF_LCDC_LSR */
#define MCF_LCDC_LSR_YMAX(x)            (((x)&0x000003FF)<<0)
#define MCF_LCDC_LSR_XMAX(x)            (((x)&0x0000003F)<<20)

/* Bit definitions and macros for MCF_LCDC_LVPWR */
#define MCF_LCDC_LVPWR_VPW(x)           (((x)&0x000003FF)<<0)

/* Bit definitions and macros for MCF_LCDC_LCPR */
#define MCF_LCDC_LCPR_CYP(x)            (((x)&0x000003FF)<<0)
#define MCF_LCDC_LCPR_CXP(x)            (((x)&0x000003FF)<<16)
#define MCF_LCDC_LCPR_OP                (0x10000000)
#define MCF_LCDC_LCPR_CC(x)             (((x)&0x00000003)<<30)
#define MCF_LCDC_LCPR_CC_TRANSPARENT    (0x00000000)
#define MCF_LCDC_LCPR_CC_OR             (0x40000000)
#define MCF_LCDC_LCPR_CC_XOR            (0x80000000)
#define MCF_LCDC_LCPR_CC_AND            (0xC0000000)
#define MCF_LCDC_LCPR_OP_ON             (0x10000000)
#define MCF_LCDC_LCPR_OP_OFF            (0x00000000)

/* Bit definitions and macros for MCF_LCDC_LCWHBR */
#define MCF_LCDC_LCWHBR_BD(x)           (((x)&0x000000FF)<<0)
#define MCF_LCDC_LCWHBR_CH(x)           (((x)&0x0000001F)<<16)
#define MCF_LCDC_LCWHBR_CW(x)           (((x)&0x0000001F)<<24)
#define MCF_LCDC_LCWHBR_BK_EN           (0x80000000)
#define MCF_LCDC_LCWHBR_BK_EN_ON        (0x80000000)
#define MCF_LCDC_LCWHBR_BK_EN_OFF       (0x00000000)

/* Bit definitions and macros for MCF_LCDC_LCCMR */
#define MCF_LCDC_LCCMR_CUR_COL_B(x)     (((x)&0x0000003F)<<0)
#define MCF_LCDC_LCCMR_CUR_COL_G(x)     (((x)&0x0000003F)<<6)
#define MCF_LCDC_LCCMR_CUR_COL_R(x)     (((x)&0x0000003F)<<12)

/* Bit definitions and macros for MCF_LCDC_LPCR */
#define MCF_LCDC_LPCR_PCD(x)            (((x)&0x0000003F)<<0)
#define MCF_LCDC_LPCR_SHARP             (0x00000040)
#define MCF_LCDC_LPCR_SCLKSEL           (0x00000080)
#define MCF_LCDC_LPCR_ACD(x)            (((x)&0x0000007F)<<8)
#define MCF_LCDC_LPCR_ACDSEL            (0x00008000)
#define MCF_LCDC_LPCR_REV_VS            (0x00010000)
#define MCF_LCDC_LPCR_SWAP_SEL          (0x00020000)
#define MCF_LCDC_LPCR_ENDSEL            (0x00040000)
#define MCF_LCDC_LPCR_SCLKIDLE          (0x00080000)
#define MCF_LCDC_LPCR_OEPOL             (0x00100000)
#define MCF_LCDC_LPCR_CLKPOL            (0x00200000)
#define MCF_LCDC_LPCR_LPPOL             (0x00400000)
#define MCF_LCDC_LPCR_FLM               (0x00800000)
#define MCF_LCDC_LPCR_PIXPOL            (0x01000000)
#define MCF_LCDC_LPCR_BPIX(x)           (((x)&0x00000007)<<25)
#define MCF_LCDC_LPCR_PBSIZ(x)          (((x)&0x00000003)<<28)
#define MCF_LCDC_LPCR_COLOR             (0x40000000)
#define MCF_LCDC_LPCR_TFT               (0x80000000)
#define MCF_LCDC_LPCR_MODE_MONOCGROME   (0x00000000)
#define MCF_LCDC_LPCR_MODE_CSTN         (0x40000000)
#define MCF_LCDC_LPCR_MODE_TFT          (0xC0000000)
#define MCF_LCDC_LPCR_PBSIZ_1           (0x00000000)
#define MCF_LCDC_LPCR_PBSIZ_2           (0x10000000)
#define MCF_LCDC_LPCR_PBSIZ_4           (0x20000000)
#define MCF_LCDC_LPCR_PBSIZ_8           (0x30000000)
#define MCF_LCDC_LPCR_BPIX_1bpp         (0x00000000)
#define MCF_LCDC_LPCR_BPIX_2bpp         (0x02000000)
#define MCF_LCDC_LPCR_BPIX_4bpp         (0x04000000)
#define MCF_LCDC_LPCR_BPIX_8bpp         (0x06000000)
#define MCF_LCDC_LPCR_BPIX_12bpp        (0x08000000)
#define MCF_LCDC_LPCR_BPIX_16bpp        (0x0A000000)
#define MCF_LCDC_LPCR_BPIX_18bpp        (0x0C000000)

#define MCF_LCDC_LPCR_PANEL_TYPE(x)     (((x)&0x00000003)<<30) 

/* Bit definitions and macros for MCF_LCDC_LHCR */
#define MCF_LCDC_LHCR_H_WAIT_2(x)       (((x)&0x000000FF)<<0)
#define MCF_LCDC_LHCR_H_WAIT_1(x)       (((x)&0x000000FF)<<8)
#define MCF_LCDC_LHCR_H_WIDTH(x)        (((x)&0x0000003F)<<26)

/* Bit definitions and macros for MCF_LCDC_LVCR */
#define MCF_LCDC_LVCR_V_WAIT_2(x)       (((x)&0x000000FF)<<0)
#define MCF_LCDC_LVCR_V_WAIT_1(x)       (((x)&0x000000FF)<<8)
#define MCF_LCDC_LVCR_V_WIDTH(x)        (((x)&0x0000003F)<<26)

/* Bit definitions and macros for MCF_LCDC_LPOR */
#define MCF_LCDC_LPOR_POS(x)            (((x)&0x0000001F)<<0)

/* Bit definitions and macros for MCF_LCDC_LPCCR */
#define MCF_LCDC_LPCCR_PW(x)            (((x)&0x000000FF)<<0)
#define MCF_LCDC_LPCCR_CC_EN            (0x00000100)
#define MCF_LCDC_LPCCR_SCR(x)           (((x)&0x00000003)<<9)
#define MCF_LCDC_LPCCR_LDMSK            (0x00008000)
#define MCF_LCDC_LPCCR_CLS_HI_WIDTH(x)  (((x)&0x000001FF)<<16)
#define MCF_LCDC_LPCCR_SCR_LINEPULSE    (0x00000000)
#define MCF_LCDC_LPCCR_SCR_PIXELCLK     (0x00002000)
#define MCF_LCDC_LPCCR_SCR_LCDCLOCK     (0x00004000)

/* Bit definitions and macros for MCF_LCDC_LDCR */
#define MCF_LCDC_LDCR_TM(x)             (((x)&0x0000001F)<<0)
#define MCF_LCDC_LDCR_HM(x)             (((x)&0x0000001F)<<16)
#define MCF_LCDC_LDCR_BURST             (0x80000000)

/* Bit definitions and macros for MCF_LCDC_LRMCR */
#define MCF_LCDC_LRMCR_SEL_REF          (0x00000001)

/* Bit definitions and macros for MCF_LCDC_LICR */
#define MCF_LCDC_LICR_INTCON            (0x00000001)
#define MCF_LCDC_LICR_INTSYN            (0x00000004)
#define MCF_LCDC_LICR_GW_INT_CON        (0x00000010)

/* Bit definitions and macros for MCF_LCDC_LIER */
#define MCF_LCDC_LIER_BOF_EN            (0x00000001)
#define MCF_LCDC_LIER_EOF_EN            (0x00000002)
#define MCF_LCDC_LIER_ERR_RES_EN        (0x00000004)
#define MCF_LCDC_LIER_UDR_ERR_EN        (0x00000008)
#define MCF_LCDC_LIER_GW_BOF_EN         (0x00000010)
#define MCF_LCDC_LIER_GW_EOF_EN         (0x00000020)
#define MCF_LCDC_LIER_GW_ERR_RES_EN     (0x00000040)
#define MCF_LCDC_LIER_GW_UDR_ERR_EN     (0x00000080)

/* Bit definitions and macros for MCF_LCDC_LISR */
#define MCF_LCDC_LISR_BOF               (0x00000001)
#define MCF_LCDC_LISR_EOF               (0x00000002)
#define MCF_LCDC_LISR_ERR_RES           (0x00000004)
#define MCF_LCDC_LISR_UDR_ERR           (0x00000008)
#define MCF_LCDC_LISR_GW_BOF            (0x00000010)
#define MCF_LCDC_LISR_GW_EOF            (0x00000020)
#define MCF_LCDC_LISR_GW_ERR_RES        (0x00000040)
#define MCF_LCDC_LISR_GW_UDR_ERR        (0x00000080)

/* Bit definitions and macros for MCF_LCDC_LGWSAR */
#define MCF_LCDC_LGWSAR_GWSA(x)         (((x)&0x3FFFFFFF)<<2)

/* Bit definitions and macros for MCF_LCDC_LGWSR */
#define MCF_LCDC_LGWSR_GWH(x)           (((x)&0x000003FF)<<0)
#define MCF_LCDC_LGWSR_GWW(x)           (((x)&0x0000003F)<<20)

/* Bit definitions and macros for MCF_LCDC_LGWVPWR */
#define MCF_LCDC_LGWVPWR_GWVPW(x)       (((x)&0x000003FF)<<0)

/* Bit definitions and macros for MCF_LCDC_LGWPOR */
#define MCF_LCDC_LGWPOR_GWPO(x)         (((x)&0x0000001F)<<0)

/* Bit definitions and macros for MCF_LCDC_LGWPR */
#define MCF_LCDC_LGWPR_GWYP(x)          (((x)&0x000003FF)<<0)
#define MCF_LCDC_LGWPR_GWXP(x)          (((x)&0x000003FF)<<16)

/* Bit definitions and macros for MCF_LCDC_LGWCR */
#define MCF_LCDC_LGWCR_GWCKB(x)         (((x)&0x0000003F)<<0)
#define MCF_LCDC_LGWCR_GWCKG(x)         (((x)&0x0000003F)<<6)
#define MCF_LCDC_LGWCR_GWCKR(x)         (((x)&0x0000003F)<<12)
#define MCF_LCDC_LGWCR_GW_RVS           (0x00200000)
#define MCF_LCDC_LGWCR_GWE              (0x00400000)
#define MCF_LCDC_LGWCR_GWCKE            (0x00800000)
#define MCF_LCDC_LGWCR_GWAV(x)          (((x)&0x000000FF)<<24)

/* Bit definitions and macros for MCF_LCDC_LGWDCR */
#define MCF_LCDC_LGWDCR_GWTM(x)         (((x)&0x0000001F)<<0)
#define MCF_LCDC_LGWDCR_GWHM(x)         (((x)&0x0000001F)<<16)
#define MCF_LCDC_LGWDCR_GWBT            (0x80000000)

/* Bit definitions and macros for MCF_LCDC_LSCR */
#define MCF_LCDC_LSCR_PS_RISE_DELAY(x)  (((x)&0x0000003F)<<26)
#define MCF_LCDC_LSCR_CLS_RISE_DELAY(x) (((x)&0x000000FF)<<16)
#define MCF_LCDC_LSCR_REV_TOGGLE_DELAY(x) (((x)&0x0000000F)<<8)
#define MCF_LCDC_LSCR_GRAY_2(x)         (((x)&0x0000000F)<<4)
#define MCF_LCDC_LSCR_GRAY_1(x)         (((x)&0x0000000F)<<0)

/* Bit definitions and macros for MCF_LCDC_BPLUT_BASE */
#define MCF_LCDC_BPLUT_BASE_BASE(x)     (((x)&0xFFFFFFFF)<<0)

/* Bit definitions and macros for MCF_LCDC_GWLUT_BASE */
#define MCF_LCDC_GWLUT_BASE_BASE(x)     (((x)&0xFFFFFFFF)<<0)

/********************************************************************/
#endif	/* mcflcd_h */
