"""
Copyright 2018-2019 NXP.
All Rights Reserved

 This software is owned or controlled by NXP and may only be used strictly in accordance with the
 applicable license terms.  By expressly accepting such terms or by downloading, installing,
 activating and/or otherwise using the software, you are agreeing that you have read, and that you
 agree to comply with and are bound by, such license terms.  If you do not agree to be bound by the
 applicable license terms, then you may not retain, install, activate or otherwise use the software.
"""
import gdb
import time

from cw.dbg import ta
from cw.dbg import flash
from cw.dbg.rcw import SPRcwValidation
from cw.utils import cw_info, cw_warn, cw_error

from initialization import *

CORE_CONTEXT = ":ccs:LS1027A:CortexA72#0"
SAP_CORE_CONTEXT = ":ccs:LS1027A:SAP#0"

# In order to connect to a board with a broken RCW, set the following variable to True
# Override RCW using a safe hard-coded RCW option
USE_SAFE_RCW = True
# Base address for DCFG and Reset registers;
# they will be used to test if RCW and PBI phases were successful or not
DCFG_BASE_ADDRESS = 0x1E00000
RESET_BASE_ADDRESS = 0x1E60000

DCSR_BASE_ADDRESS = 0x700000000

FSPI_NAND = False

# Set to True in order to enable memory access to Ethernet controller
# Set to False if target initializes the Ethernet controller (ex. uboot, Linux, etc.)
# Note:
# ENETC is configurable through RCW, which implies that PCIe functions 5 and 6
# may or may not be present.
# If ENETC PCIe function is disabled, accesses to ENETC PCIe function registers
# might put the target in a bad state.
# It is advised to avoid reading the ENETC registers of a disabled ENETC PCIe function.
PCI_INIT = True

# Target access object
TA = ta.create()

###################################################################
# Prepare environment/convenience variables
###################################################################
def Prepare_Env():
    set_init_params(context=CORE_CONTEXT, dcsr_addr=DCSR_BASE_ADDRESS)
    gdb.execute("set $ddr_addr = 0x80000000")
    gdb.execute("set $ocram_addr = 0x18000000")

###################################################################
######### Reset
###################################################################
def Reset():
    if USE_SAFE_RCW:
        # Overridden RCW source
        TA.rcw.set_source(0x0)

        # Overridden RCW to get the cores out of boothold off
        TA.rcw.set_data({9: 0x00000000})
        # Overridden RCW to enable ENETC PF5 and PF6
        if PCI_INIT:
            TA.rcw.set_data({27:0x000e6000})

        TA.rcw.apply()
    else:
        # Perform a regular reset
        try:
            user_reset = int(gdb.parse_and_eval("$reset"))
            user_reset_delay = int(gdb.parse_and_eval("$reset_delay"))
        except gdb.error:
            user_reset = 1
            user_reset_delay = 0

        if user_reset:
            try:
                gdb.execute("cw_reset %d" % user_reset_delay)
            except gdb.error as exc:
                # Check if RCW or PBI phases were successful
                # Note: target accesses must be executed through SAP core
                target_access = ta.create()
                target_access.set_context(SAP_CORE_CONTEXT)
                SPRcwValidation(target_access, DCFG_BASE_ADDRESS, RESET_BASE_ADDRESS).check_for_rcw_or_pbi_error()
                # If check_for_rcw_or_pbi_error does not detect the error, forward the initial exception
                raise exc

    Init_BRR()

###################################################################
# Boot Release
###################################################################
def Init_BRR():

    # TODO: when we can detect the current context,
    # release all cores for SMP, current core for AMP

    # All cores must go to debug mode after release
    DCSR_LE_M(0x7002C, 0x00000003)
    # Write to BRR to release all cores
    CCSR_LE_M(0x01e60060, 0x00000003)

    # Make sure the gdb threads are refreshed just after the BRR initialization
    TA.rc.refresh_threads()

    # Make sure the cores are stopped
    TA.rc.halt()

###################################################################
# PCI
###################################################################
def Init_PCI():
    # Small initialization to permit reading of ENETC registers
    # Enable MEM_ACCESS bit in PFx_PCI_HDR_TYPE0 register
    for id in range(7):
        CCSR_LE_M16(0x1F0000004 + id * 0x1000, 0x0002)

###################################################################
# Adds Flash devices for this board
###################################################################
def Config_Flash_Devices():
    fl = flash.create(TA)

    # Add FlexSPI device
    if FSPI_NAND:
        fl.add_device({"alias": "xspi", "name": "MT29F4G01ABBF", "address": 0x0, "ws_address": 0x18000000, "ws_size": 0x1FFFF, "geometry": "8x1", "controller": "FSPI"})
    else:
        fl.add_device({"alias": "xspi", "name": "MX25U12832F", "address": 0x0, "ws_address": 0x18000000, "ws_size": 0x1FFFF, "geometry": "8x1", "controller": "FSPI"})

    #set xspi as current device
    fl.set_current_device("xspi")

###################################################################
# TrustZone Initialization
###################################################################
def Init_TZASC():
    # TZASC
    CCSR_LE_M(0x01100004, 0x00000001)
    CCSR_LE_M(0x01100120, 0x00000000)
    CCSR_LE_M(0x01100124, 0x00000000)
    CCSR_LE_M(0x01100128, 0xffffffff)
    CCSR_LE_M(0x0110012C, 0xffffffff)
    CCSR_LE_M(0x01100130, 0xc0000001)
    CCSR_LE_M(0x01100134, 0xffffffff)
    CCSR_LE_M(0x01100008, 0x00010001)

###################################################################
# FlexSPI Initialization
###################################################################
def Init_FSPI():
    pass

###################################################################
# Detect DDR frequency
###################################################################
def Detect_DDR_Freq():
    # Detect DDR frequency using RCWSR1

    # Hardcoded DDRCLK
    DDRCLK = 100.00

    # Read RCWSR1 and obtain MEM_PLL_RAT
    RCWSR1 = CCSR_LE_D(DCFG_BASE_ADDRESS + 0x100)
    MEM_PLL_RAT = (RCWSR1 >> 10) & 0x3F
    MEM_PLL_CFG_DIV = 1.0 / (((RCWSR1 >> 8) & 0x3) + 1)

    # Based on the reference clock for the DDR and PLLs config, compute DDR frequency
    DDR_FREQ = int(round(DDRCLK * MEM_PLL_RAT * MEM_PLL_CFG_DIV * 4)) # x4 is fixed

    return DDR_FREQ

###################################################################
# DDR Initialization
###################################################################
def Init_DDRC(ddr_freq):
    # DDR_CDR1
    CCSR_LE_M(0x01080b28, 0x80040000)
    # DDR_CDR2
    CCSR_LE_M(0x01080b2c, 0x0000A101)
    # DDR_SDRAM_CFG
    CCSR_LE_M(0x01080110, 0x470C0008)
    # CS0_BNDS
    CCSR_LE_M(0x01080000, 0x0000003F)
    # CS0_CONFIG
    CCSR_LE_M(0x01080080, 0x80044302)
    # TIMING_CFG_3
    CCSR_LE_M(0x01080100, 0x010E1000)
    # TIMING_CFG_0
    CCSR_LE_M(0x01080104, 0xA055000C)
    # TIMING_CFG_1
    CCSR_LE_M(0x01080108, 0xCEC48C46)
    # TIMING_CFG_2
    CCSR_LE_M(0x0108010c, 0x0040D128)
    # DDR_SDRAM_CFG_2
    CCSR_LE_M(0x01080114, 0x00401010)
    # DDR_SDRAM_MODE
    CCSR_LE_M(0x01080118, 0x00061C70)
    # DDR_SDRAM_MODE_2
    CCSR_LE_M(0x0108011c, 0x00180000)
    # DDR_SDRAM_MD_CNTL
    CCSR_LE_M(0x01080120, 0x00000000)
    # DDR_SDRAM_INTERVAL
    CCSR_LE_M(0x01080124, 0x18600618)
    # DDR_DATA_INIT
    CCSR_LE_M(0x01080128, 0x12345678)
    # DDR_SDRAM_CLK_CNTL
    CCSR_LE_M(0x01080130, 0x02800000)
    # TIMING_CFG_4
    CCSR_LE_M(0x01080160, 0x00000001)
    # TIMING_CFG_5
    CCSR_LE_M(0x01080164, 0x04401400)
    # TIMING_CFG_6
    CCSR_LE_M(0x01080168, 0x00000000)
    # TIMING_CFG_7
    CCSR_LE_M(0x0108016C, 0x13300000)
    # DDR_ZQ_CNTL
    CCSR_LE_M(0x01080170, 0x89080600)
    # DDR_WRLVL_CNTL
    CCSR_LE_M(0x01080174, 0x86550607)
    # DDR_WRLVL_CNTL_2
    CCSR_LE_M(0x01080190, 0x0708080A)
    # DDR_WRLVL_CNTL_3
    CCSR_LE_M(0x01080194, 0x0A0B0C09)
    # DDR_mode-9
    CCSR_LE_M(0x01080220, 0x00000000)
    # DDR_mode-10
    CCSR_LE_M(0x01080224, 0x00000000)
    # TIMING_CFG_8
    CCSR_LE_M(0x01080250, 0x00000000)
    # SDRAM_CFG_3
    CCSR_LE_M(0x01080260, 0x00000000)

    CCSR_LE_M(0x01080280, 0xeeeeeeee)
    CCSR_LE_M(0x01080284, 0x11111111)
    CCSR_LE_M(0x01080288, 0xffffffff)
    CCSR_LE_M(0x01080290, 0xe1100001)

    # DDR_ERR_SBE
    CCSR_LE_M(0x01080e58, 0x00010000)

    # DQ_MAPPING_0
    CCSR_LE_M(0x01080400, 0x06106104)

    # DQ_MAPPING_1
    CCSR_LE_M(0x01080404, 0x84184000)

    # DQ_MAPPING_2
    CCSR_LE_M(0x01080408, 0x00000000)

    # DQ_MAPPING_3
    CCSR_LE_M(0x0108040c, 0x00000000)

    # Errata
    if ddr_freq <= 1333:
        CCSR_LE_M(0x01080f70, 0x0080006a)
    elif ddr_freq <= 1600:
        CCSR_LE_M(0x01080f70, 0x0070006f)
    elif ddr_freq <= 1867:
        CCSR_LE_M(0x01080f70, 0x00700076)
    else:
        CCSR_LE_M(0x01080f70, 0x0060007b)

    # DDR_SDRAM_CFG
    CCSR_LE_M(0x01080110, 0xC70C0008)

    err_msg = ""
    # Poll for DDR to be initialized
    count = 0
    while True:
        time.sleep(0.2)
        SDRAM_CFG2 = CCSR_LE_D(0x01080114)
        D_INIT = SDRAM_CFG2 & 0x10
        count = count + 1
        if (D_INIT == 0):
            break
        if (count > 20):
            err_msg += "DDR: timeout while waiting for D_INIT.\n"
            break

    ERR_DETECT = CCSR_LE_D(0x01080E40)
    if (ERR_DETECT != 0):
        err_msg += "DDR: initialization failed (ERR_DETECT = 0x%08x)." % ERR_DETECT

    if err_msg:
        cw_warn(err_msg, USE_SAFE_RCW, True)

    time.sleep(1)

    # Clear Terminate all barrier transactions bit of CCI-400 Control Register
    CCSR_LE_M(0x04090000, 0x08000000)


###################################################################
# This is the actual function called by debugger, flash, etc.
###################################################################
def run_init_file():
    Prepare_Env()

    Reset()

    Init_TZASC()

    if PCI_INIT:
        Init_PCI()

    DDR_freq = Detect_DDR_Freq()
    Init_DDRC(DDR_freq)

    Config_Flash_Devices()
