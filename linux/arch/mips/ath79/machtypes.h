/*
 *  Atheros AR71XX/AR724X/AR913X machine type definitions
 *
 *  Copyright (C) 2008-2010 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef _ATH79_MACHTYPE_H
#define _ATH79_MACHTYPE_H

#include <asm/mips_machine.h>

enum ath79_mach_type {
	ATH79_MACH_GENERIC_OF = -1,	/* Device tree board */
	ATH79_MACH_GENERIC = 0,
	ATH79_MACH_ALFA_AP96,		/* ALFA Network AP96 board */
	ATH79_MACH_ALFA_NX,		/* ALFA Network N2/N5 board */
	ATH79_MACH_ALL0258N,		/* Allnet ALL0258N */
	ATH79_MACH_AP113,		/* Atheros AP113 reference board */
	ATH79_MACH_AP121,		/* Atheros AP121 reference board */
	ATH79_MACH_AP121_MINI,		/* Atheros AP121-MINI reference board */
	ATH79_MACH_AP136_010,		/* Atheros AP136-010 reference board */
	ATH79_MACH_AP81,		/* Atheros AP81 reference board */
	ATH79_MACH_AP83,		/* Atheros AP83 */
	ATH79_MACH_AP96,		/* Atheros AP96 */
	ATH79_MACH_AW_NR580,		/* AzureWave AW-NR580 */
	ATH79_MACH_DB120,		/* Atheros DB120 reference board */
	ATH79_MACH_PB44,		/* Atheros PB44 reference board */
	ATH79_MACH_DIR_600_A1,		/* D-Link DIR-600 rev. A1 */
	ATH79_MACH_DIR_615_C1,		/* D-Link DIR-615 rev. C1 */
	ATH79_MACH_DIR_615_E4,		/* D-Link DIR-615 rev. E4 */
	ATH79_MACH_DIR_825_B1,		/* D-Link DIR-825 rev. B1 */
	ATH79_MACH_EAP7660D,		/* Senao EAP7660D */
	ATH79_MACH_JA76PF,		/* jjPlus JA76PF */
	ATH79_MACH_JA76PF2,		/* jjPlus JA76PF2 */
	ATH79_MACH_JWAP003,		/* jjPlus JWAP003 */
	ATH79_MACH_HORNET_UB,		/* ALFA Networks Hornet-UB */
	ATH79_MACH_MZK_W04NU,		/* Planex MZK-W04NU */
	ATH79_MACH_MZK_W300NH,		/* Planex MZK-W300NH */
	ATH79_MACH_NBG460N,		/* Zyxel NBG460N/550N/550NH */
	ATH79_MACH_NETREACH,		/* Accelerated NetReach */
	ATH79_MACH_OM2P,		/* OpenMesh OM2P */
	ATH79_MACH_PB42,		/* Atheros PB42 */
	ATH79_MACH_PB92,		/* Atheros PB92 */
	ATH79_MACH_RB_411,		/* MikroTik RouterBOARD 411/411A/411AH */
	ATH79_MACH_RB_411U,		/* MikroTik RouterBOARD 411U */
	ATH79_MACH_RB_433,		/* MikroTik RouterBOARD 433/433AH */
	ATH79_MACH_RB_433U,		/* MikroTik RouterBOARD 433UAH */
	ATH79_MACH_RB_450G,		/* MikroTik RouterBOARD 450G */
	ATH79_MACH_RB_450,		/* MikroTik RouterBOARD 450 */
	ATH79_MACH_RB_493,		/* Mikrotik RouterBOARD 493/493AH */
	ATH79_MACH_RB_493G,		/* Mikrotik RouterBOARD 493G */
	ATH79_MACH_RB_750,		/* MikroTik RouterBOARD 750 */
	ATH79_MACH_RB_750G_R3,		/* MikroTik RouterBOARD 750GL */
	ATH79_MACH_RB_751,		/* MikroTik RouterBOARD 751 */
	ATH79_MACH_RB_751G,		/* Mikrotik RouterBOARD 751G */
	ATH79_MACH_RW2458N,		/* Redwave RW2458N */
	ATH79_MACH_TEW_632BRP,		/* TRENDnet TEW-632BRP */
	ATH79_MACH_TEW_673GRU,		/* TRENDnet TEW-673GRU */
	ATH79_MACH_TL_MR11U,		/* TP-LINK TL-MR11U */
	ATH79_MACH_TL_MR3020,		/* TP-LINK TL-MR3020 */
	ATH79_MACH_TL_MR3220,		/* TP-LINK TL-MR3220 */
	ATH79_MACH_TL_MR3420,		/* TP-LINK TL-MR3420 */
	ATH79_MACH_TL_WA901ND,		/* TP-LINK TL-WA901ND */
	ATH79_MACH_TL_WA901ND_V2,	/* TP-LINK TL-WA901ND v2 */
	ATH79_MACH_TL_WR1043ND,		/* TP-LINK TL-WR1041ND */
	ATH79_MACH_TL_WR2543N,		/* TP-LINK TL-WR2543N/ND */
	ATH79_MACH_TL_WR703N,		/* TP-LINK TL-WR703N */
	ATH79_MACH_TL_WR741ND,		/* TP-LINK TL-WR741ND */
	ATH79_MACH_TL_WR741ND_V4,	/* TP-LINK TL-WR741ND  v4*/
	ATH79_MACH_TL_WR841N_V1,	/* TP-LINK TL-WR841N v1 */
	ATH79_MACH_TL_WR841N_V7,	/* TP-LINK TL-WR841N/ND v7 */
	ATH79_MACH_TL_WR941ND,		/* TP-LINK TL-WR941ND */
	ATH79_MACH_UBNT_AIRROUTER,	/* Ubiquiti AirRouter */
	ATH79_MACH_UBNT_BULLET_M,	/* Ubiquiti Bullet M */
	ATH79_MACH_UBNT_LSSR71,		/* Ubiquiti LS-SR71 */
	ATH79_MACH_UBNT_LSX,		/* Ubiquiti LSX */
	ATH79_MACH_UBNT_NANO_M, 	/* Ubiquiti NanoStation M */
	ATH79_MACH_UBNT_ROCKET_M,	/* Ubiquiti Rocket M */
	ATH79_MACH_UBNT_RSPRO,		/* Ubiquiti RouterStation Pro */
	ATH79_MACH_UBNT_RS,		/* Ubiquiti RouterStation */
	ATH79_MACH_UBNT_UNIFI, 		/* Ubiquiti Unifi */
	ATH79_MACH_UBNT_XM,		/* Ubiquiti Networks XM board rev 1.0 */
	ATH79_MACH_WHR_G301N,		/* Buffalo WHR-G301N */
	ATH79_MACH_WHR_HP_G300N,	/* Buffalo WHR-HP-G300N */
	ATH79_MACH_WHR_HP_GN,		/* Buffalo WHR-HP-GN */
	ATH79_MACH_WNDR3700,		/* NETGEAR WNDR3700/WNDR3800/WNDRMAC */
	ATH79_MACH_WNR2000,		/* NETGEAR WNR2000 */
	ATH79_MACH_WP543,		/* Compex WP543 */
	ATH79_MACH_WPE72,		/* Compex WPE72 */
	ATH79_MACH_WRT160NL,		/* Linksys WRT160NL */
	ATH79_MACH_WRT400N,		/* Linksys WRT400N */
	ATH79_MACH_WZR_HP_AG300H,	/* Buffalo WZR-HP-AG300H */
	ATH79_MACH_WZR_HP_G300NH,	/* Buffalo WZR-HP-G300NH */
	ATH79_MACH_WZR_HP_G300NH2,	/* Buffalo WZR-HP-G300NH2 */
	ATH79_MACH_WZR_HP_G450H,	/* Buffalo WZR-HP-G450H */
	ATH79_MACH_ZCN_1523H_2,		/* Zcomax ZCN-1523H-2-xx */
};

#endif /* _ATH79_MACHTYPE_H */
