#ifndef __WPS__H__
#define __WPS__H__

#ifdef CONFIG_RT2860V2_STA_WSC                      // if RT2880 support Wifi - STA
#include "stapriv.h"
#endif

#ifdef CONFIG_RALINK_RT2880
#define WPS_AP_PBC_LED_GPIO     13   // 0 ~ 24( or disable this feature by undefine it)
#elif defined CONFIG_RALINK_RT3883
#define WPS_AP_PBC_LED_GPIO     0   // 0 ~ 24( or disable this feature by undefine it)
#else
#define WPS_AP_PBC_LED_GPIO     14   // 0 ~ 24( or disable this feature by undefine it)
#endif

#ifdef WPS_AP_PBC_LED_GPIO
#include "utils.h"
#define LedReset()                  {ledWps(WPS_AP_PBC_LED_GPIO, WPS_LED_RESET);}
#define LedInProgress()             {ledWps(WPS_AP_PBC_LED_GPIO, WPS_LED_PROGRESS);}
#define LedError()                  {ledWps(WPS_AP_PBC_LED_GPIO, WPS_LED_ERROR);}
#define LedSessionOverlapDetected() {ledWps(WPS_AP_PBC_LED_GPIO, WPS_LED_SESSION_OVERLAP);}
#define LedSuccess()                {ledWps(WPS_AP_PBC_LED_GPIO, WPS_LED_SUCCESS);}
#else
#define LedReset()
#define LedInProgress()
#define LedError()
#define LedSessionOverlapDetected()
#define LedSuccess()
#endif


void WPSRestart(void);
void formDefineWPS(void);

#define WSC_CONF_STATUS_STR "WscConfStatus"
#define WSC_CONF_STATUS_UNCONFIGURED    1   /* these value are taken from 2860 driver Release Note document. */
#define WSC_CONF_STATUS_CONFIGURED      2


/*
 * ripped from driver wsc.h,....ugly
  */
#define PACKED  __attribute__ ((packed))
#define USHORT  unsigned short
#define UCHAR   unsigned char
typedef struct PACKED _WSC_CONFIGURED_VALUE {
    USHORT WscConfigured; // 1 un-configured; 2 configured
    UCHAR   WscSsid[32 + 1];
    USHORT WscAuthMode; // mandatory, 0x01: open, 0x02: wpa-psk, 0x04: shared, 0x08:wpa, 0x10: wpa2, 0x
    USHORT  WscEncrypType;  // 0x01: none, 0x02: wep, 0x04: tkip, 0x08: aes
    UCHAR   DefaultKeyIdx;
    UCHAR   WscWPAKey[64 + 1];
} WSC_CONFIGURED_VALUE;


#ifdef CONFIG_RT2860V2_STA_WSC
// WSC configured credential
typedef struct  _WSC_CREDENTIAL
{
    NDIS_802_11_SSID    SSID;               // mandatory
    USHORT              AuthType;           // mandatory, 1: open, 2: wpa-psk, 4: shared, 8:wpa, 0x10: wpa2, 0x20: wpa-psk2
    USHORT              EncrType;           // mandatory, 1: none, 2: wep, 4: tkip, 8: aes
    UCHAR               Key[64];            // mandatory, Maximum 64 byte
    USHORT              KeyLength;
    UCHAR               MacAddr[6];         // mandatory, AP MAC address
    UCHAR               KeyIndex;           // optional, default is 1
    UCHAR               Rsvd[3];            // Make alignment
}   WSC_CREDENTIAL, *PWSC_CREDENTIAL;


// WSC configured profiles
typedef struct  _WSC_PROFILE
{
#ifndef UINT
#define UINT	unsigned long
#endif
    UINT           	ProfileCnt;
    UINT		ApplyProfileIdx;  // add by johnli, fix WPS test plan 5.1.1
    WSC_CREDENTIAL  	Profile[8];             // Support up to 8 profiles
}   WSC_PROFILE, *PWSC_PROFILE;
#endif

#define WSC_ID_VERSION					0x104A
#define WSC_ID_VERSION_LEN				1
#define WSC_ID_VERSION_BEACON			0x00000001

#define WSC_ID_SC_STATE					0x1044
#define WSC_ID_SC_STATE_LEN				1
#define WSC_ID_SC_STATE_BEACON			0x00000002

#define WSC_ID_AP_SETUP_LOCKED			0x1057
#define WSC_ID_AP_SETUP_LOCKED_LEN		1
#define WSC_ID_AP_SETUP_LOCKED_BEACON	0x00000004

#define WSC_ID_SEL_REGISTRAR			0x1041
#define WSC_ID_SEL_REGISTRAR_LEN		1
#define WSC_ID_SEL_REGISTRAR_BEACON		0x00000008

#define WSC_ID_DEVICE_PWD_ID			0x1012
#define WSC_ID_DEVICE_PWD_ID_LEN		2
#define WSC_ID_DEVICE_PWD_ID_BEACON		0x00000010


#define WSC_ID_SEL_REG_CFG_METHODS		0x1053
#define WSC_ID_SEL_REG_CFG_METHODS_LEN	2
#define WSC_ID_SEL_REG_CFG_METHODS_BEACON	0x00000020

#define WSC_ID_UUID_E					0x1047
#define WSC_ID_UUID_E_LEN				16
#define WSC_ID_UUID_E_BEACON			0x00000040

#define WSC_ID_RF_BAND					0x103C
#define WSC_ID_RF_BAND_LEN				1
#define WSC_ID_RF_BAND_BEACON			0x00000080

#define WSC_ID_PRIMARY_DEVICE_TYPE		0x1054
#define WSC_ID_PRIMARY_DEVICE_TYPE_LEN	8
#define WSC_ID_PRIMARY_DEVICE_TYPE_BEACON	0x00000100

#if defined (CONFIG_RTDEV_MII) || defined (CONFIG_RTDEV_USB) || defined (CONFIG_RTDEV_PCI)
void formDefineRaixWPS(void);
unsigned int getAPPIN(char *interface);
int isSafeForShell(char *str);
int getWscProfile(char *interface, WSC_CONFIGURED_VALUE *data, int len);
void getWPSAuthMode(WSC_CONFIGURED_VALUE *result, char *ret_str);
void getWPSEncrypType(WSC_CONFIGURED_VALUE *result, char *ret_str);
int getWscStatus(char *interface);
char *getWscStatusStr(int status);
int getAPMac(char *ifname, char *if_hw);

void RaixWPSRestart();
#endif

#endif /* __WPS__H_ */

