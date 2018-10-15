/* ---------------------------------------------------------------------------
                        Copyright (c) 2003 Micrel, Inc.
   ---------------------------------------------------------------------------
    Author      Date        Descriptions
    THa         03/11/03    Created file.
    THa         03/12/03    Load arrays from driver.
    THa         05/23/03    Add MAC table and VLAN functions.
    THa         07/16/03    Initialize MAC table.
   ---------------------------------------------------------------------------
*/


#ifndef _KS_IOCTL_H
#define _KS_IOCTL_H


#if 0
typedef struct {
    DWORD dwPort;
    DWORD dwLimit;
    DWORD fEnable;
} TBandwidth, *PTBandwidth;


typedef struct {
    DWORD dwVID;
    DWORD dwAddrLo;
    DWORD dwAddrHi;
    DWORD dwIndex;
    DWORD dwData[ 6 ];
} TMacTable, *PTMacTable;


typedef struct {
    int   nLen;
    PBYTE pBuf;
    DWORD dwIndex;
    DWORD dwData[ 4 ];
} TMem, *PTMem;


typedef struct {
    DWORD dwAddr;
    DWORD dwMask;
    DWORD dwPattern;
    DWORD dwTime;
} TPoll, *PTPoll;


typedef struct {
    int   cnPort;
    int   cnTotal;
    void* pCfg;
} TPort, *PTPort;


typedef struct {
    DWORD dwAddr;
    DWORD dwData;
    DWORD dwDev;
} TRegister, *PTRegister;


typedef struct {
    DWORD dwVID;
    DWORD dwFID;
    DWORD dwData[ 4 ];
} TVlan, *PTVlan;
#endif


typedef struct {
    DWORD dwLinkSpeed;
    DWORD dwCapabilities;
    DWORD dwAdvertised;
    DWORD dwLinkPartner;
    BYTE  bLinkUp;
    BYTE  bStatus;
} LINK_STATUS, *PLINK_STATUS;


typedef struct {
    DWORD dwLength;
    DWORD dwStatus;
} CABLE_STATUS, *PCABLE_STATUS;


enum {
    DEV_IOC_OK,
    DEV_IOC_INVALID_SIZE,
    DEV_IOC_INVALID_CMD,
    DEV_IOC_INVALID_LEN
};


enum {
    DEV_CMD_INIT,
    DEV_CMD_GET,
    DEV_CMD_PUT
};


enum {
    DEV_READ_REG,
    DEV_LINK_STATUS
};


enum {
    DEV_CAPABILITIES,
    DEV_CABLE_STATUS
};


#define MAX_DATA_SIZE  2000

typedef struct {
    int   nSize;
    int   nCmd;
    int   nSubCmd;
    int   nResult;
    union {
#if 0
        TBandwidth Bandwidth;
        TMacTable  MacTable;
        TMem       Mem;
        TPoll      Poll;
        TPort      Port;
        TRegister  Reg;
        TVlan      Vlan;
#endif
        unsigned char bData[ MAX_DATA_SIZE ];
        int           nData[ MAX_DATA_SIZE / sizeof( int )];
        LINK_STATUS   LinkStatus;
        CABLE_STATUS  CableStatus[ 5 ];
    } param;
} TRequest, *PTRequest;


#if 0
#define DEV_IOC_MAGIC  0x58

#define DEV_IOC_QMEM  _IOR( DEV_IOC_MAGIC, 1, TRequest )
#define DEV_IOC_SMEM  _IOW( DEV_IOC_MAGIC, 2, TRequest )
#define DEV_IOC_QREG  _IOR( DEV_IOC_MAGIC, 3, TRequest )
#define DEV_IOC_SREG  _IOW( DEV_IOC_MAGIC, 4, TRequest )
#define DEV_IOC_POLL  _IOW( DEV_IOC_MAGIC, 5, TRequest )
#define DEV_IOC_QCHK  _IOW( DEV_IOC_MAGIC, 6, TRequest )
#define DEV_IOC_QTAB  _IOR( DEV_IOC_MAGIC, 7, TRequest )
#define DEV_IOC_STAB  _IOW( DEV_IOC_MAGIC, 8, TRequest )
#define DEV_IOC_QMAC  _IOR( DEV_IOC_MAGIC, 9, TRequest )
#define DEV_IOC_SMAC  _IOW( DEV_IOC_MAGIC, 10, TRequest )
#define DEV_IOC_BAND  _IOW( DEV_IOC_MAGIC, 11, TRequest )
#define DEV_IOC_QPOR  _IOR( DEV_IOC_MAGIC, 12, TRequest )
#define DEV_IOC_SPOR  _IOW( DEV_IOC_MAGIC, 13, TRequest )

#define DEV_IOC_MAX  13


enum {
    CHK_BASE,
    CHK_MAC_TABLE,
    CHK_LAST
};

enum {
    MAC_TABLE,
    MAC_TABLE_DEL,
    MAC_TABLE_FIND,
    MAC_TABLE_CLEAR,
    MAC_TABLE_INIT,
    MAC_TABLE_LAST
};

enum {
    MEM_DEV,
    MEM_TABLE,
    MEM_REG_ADDR_SORTED,
    MEM_REG_OFF_SORTED,
    MEM_LAST
};

enum {
    REG_SWITCH,
    REG_PHY1G,
    REG_PHY10G,
    REG_PCI,
    REG_RESET,
    REG_LAST
};

enum {
    REG_VLAN,
    REG_VLAN_FID,
    REG_VLAN_VID,
    REG_VLAN_LAST
};
#endif


#endif
