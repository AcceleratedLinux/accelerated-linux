###################################################################
"""
Copyright 2015-2016 Freescale Semiconductor, Inc.
Copyright 2017-2018 NXP

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of Freescale Semiconductor nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

This software is provided by Freescale Semiconductor "as is" and any
express or implied warranties, including, but not limited to, the implied
warranties of merchantability and fitness for a particular purpose are
disclaimed. In no event shall Freescale Semiconductor be liable for any
direct, indirect, incidental, special, exemplary, or consequential damages
(including, but not limited to, procurement of substitute goods or services;
loss of use, data, or profits; or business interruption) however caused and
on any theory of liability, whether in contract, strict liability, or tort
(including negligence or otherwise) arising in any way out of the use of
this software, even if advised of the possibility of such damage.
"""

###################################################################
######### Parameters
###################################################################

# Board type (Supported values: "QDS", "RDB").
BOARD_TYPE = "TX40"

# Flash types (Supported values: "nor", "nand", "qspi", "xspi", "sd", "mmc").
FLASH_TYPE = "xspi"

# Connection probe. Possible values:
# PROBE_CONNECTION = "" - use local CWTAP through USB.
# PROBE_CONNECTION = "<serial_number>" - use local CWTAP USB with specified serial number. Ex: "12:34:56:78:9a:bc"
# PROBE_CONNECTION = "<IP>|<host_name>" - use remote CWTAP
# PROBE_CONNECTION = "cmsisdap[:<serial_number>]" - use CMSIS-DAP with or without serial number
PROBE_CONNECTION = "192.168.22.1"

# Current SoC name.
SOC_NAME = "LS1027A"

# JTAG speed.
# For CMSIS-DAP please lower the JTAG speed to 6000.
JTAG_SPEED = 16000

# Address:port (IP/host name:port) where CCS is located.
# If empty, it will use local connection.
# Ex: 127.0.0.1:41475
CCS_CONNECTION = ""

# Address:port that will be used by GTA.
# If empty, it will use local connection.
# Ex: 127.0.0.1:45000
GTA_CONNECTION = ""

# Remote target responses timeout (seconds).
GDB_TIMEOUT = 7200

###################################################################
######### Start Flash Programming Services
###################################################################
import sys
import os

#compute gdb_extensions absolute path
FLASH_EXT_INSTALL_DIR = ''.join([os.path.dirname(os.path.realpath(__file__)), '/'])
GDB_EXT_INSTALL_DIR =''.join([FLASH_EXT_INSTALL_DIR, '../'])

#add gdb_extensions to system path
sys.path.insert(0, GDB_EXT_INSTALL_DIR)

from flash.scripts.services import start_fp_services

def fp_initialization():
    """
        CodeWarrior services initialization script
    """
    gtaip = GTA_CONNECTION
    ccsip = CCS_CONNECTION

    dev_arg = '-d %s' % FLASH_TYPE
    soc_arg = '-s %s' % SOC_NAME
    board_arg = '-b %s' % BOARD_TYPE
    conn_arg = ''
    if PROBE_CONNECTION:
        conn_arg = '--probe %s' % PROBE_CONNECTION

    # for CCS server to be started on host leave empty
    ccsip_arg = ''
    if ccsip:
        ccsip_arg = '--ccsip %s' % ccsip

    gtaip_arg = ''
    if gtaip:
        gtaip_arg = '--gtaip %s' % gtaip

    jtag_speed_arg = '--jtag_speed %d' % JTAG_SPEED
    timeout_arg = '--timeout %d' % GDB_TIMEOUT

    argument = ' '.join([dev_arg, soc_arg, board_arg, conn_arg, ccsip_arg, \
        gtaip_arg, jtag_speed_arg, timeout_arg])
    start_fp_services(argument)

fp_initialization()
