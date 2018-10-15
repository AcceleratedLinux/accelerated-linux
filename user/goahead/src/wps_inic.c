#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <asm/types.h>
#include <linux/if.h>
#include <linux/wireless.h>

#include <ctype.h>

#include "nvram.h"
#include "utils.h"
#include "webs.h"
#include "internet.h"
#include "wireless.h"

#include "wps.h"

#define DD printf("%s(),  %d\n", __FUNCTION__, __LINE__);	fflush(stdout);
#define AP_MODE
#include "oid.h"

#define WPS_AP_TIMEOUT_SECS				120000				// 120 seconds
#define WPS_AP_TIMEOUT_SECS_SEND_M7		120000				// 120 seconds
#define WPS_AP_CATCH_CONFIGURED_TIMER	100					// 0.1 sec 

static int g_wps_timer_state = 0;

int g_Raix_wsc_configured = 0;							// export for wireless.c
													// We can't know if WSC process success or not by the variable 
													// g_wsc_configured when AP being a WSC proxy.
static int g_WscResult = 0;							// for AP only ( STA WPS don't need this)
static int g_isEnrollee = 0;						// for AP only

static void resetTimerAll(void)
{
	stopTimer();
	g_wps_timer_state = 0;
}

/*
 *  Browsers will poll WPS info. from this funsction.
 */
void RaixUpdateWPS( webs_t wp, char_t *path, char_t *query)
{
	int i;
	char tmp_str[128];

	WSC_CONFIGURED_VALUE result;
	getWscProfile("rai0", &result, sizeof(WSC_CONFIGURED_VALUE));

	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain\nPragma: no-cache\nCache-Control: no-cache\n\n"));

	//1. WPSConfigured
	websWrite(wp, T("%d\n"), result.WscConfigured);
	
	//2. WPSSSID
	if (strchr(result.WscSsid, '\n')) {
		websWrite(wp, T("Invalid SSID character: new line"));
	} else {
		websWrite(wp, T("%s\n"), result.WscSsid);
	}

	//3. WPSAuthMode
	tmp_str[0] = '\0';
	getWPSAuthMode(&result, tmp_str);
	websWrite(wp, T("%s\n"), tmp_str);

	//4. EncrypType
	tmp_str[0] = '\0';
	getWPSEncrypType(&result, tmp_str);
	websWrite(wp, T("%s\n"), tmp_str);
	
	//5. DefaultKeyIdx
	websWrite(wp, T("%d\n"), result.DefaultKeyIdx);
	
	//6. Key
	for (i=0; i<64; i++) {		// WPA key default length is 64 (defined & hardcode in driver) 
		if(i!=0 && !(i % 32))
			websWrite(wp, T("<br>"));
		websWrite(wp, T("%c"), result.WscWPAKey[i]);
	}
	websWrite(wp, T("\n"));

	//7. WSC Status
	websWrite(wp, T("%s\n"), getWscStatusStr(getWscStatus("rai0")));

	//8. WSC Result
	websWrite(wp, T("%d"), g_WscResult);

    websDone(wp, 200);	
	return;
}

void RaixWPSRestart(void)
{
	const char *wordlist;
	const char *mode = nvram_bufget(RT2860_NVRAM, "OperationMode");

	doSystem("kill -9 `cat /var/run/wscd.pid.rai0`");
	doSystem("iwpriv rai0 set WscConfMode=0 1>/dev/null 2>&1");	// WPS disable

	if (!strcmp(mode, "0" ) || !strcmp(mode, "1")) {	// bridge || gateway
		wordlist = nvram_bufget(RTDEV_NVRAM, "WscModeOption");
		if (wordlist && strcmp(wordlist, "0") > 0) {				// WPS Enable
			char lan_if_addr[16];
			if ((getIfIp(getLanIfName(), lan_if_addr)) == -1) {
				printf("WPSRestart error, can't get lan ip.\n");
				return;
			}
			doSystem("iwpriv rai0 set WscConfMode=%d", 7);
			wordlist = nvram_bufget(RTDEV_NVRAM, "WscConfigured");
			if (strcmp(wordlist, "0") == 0)
				doSystem("iwpriv rai0 set WscConfStatus=1");
			doSystem("route add -host 239.255.255.250 dev br0 1>/dev/null 2>&1");
			doSystem("wscd -m 1 -a %s -i rai0 &", lan_if_addr);
		} else {
			const char *rax_wsc_enable = nvram_bufget(RT2860_NVRAM, "WscModeOption");
			if (strcmp(rax_wsc_enable, "0") == 0)
				doSystem("route delete 239.255.255.250 1>/dev/null 2>&1");
		}
	} else						// wireless isp || ap-clinet || others
		return;

	wordlist = nvram_get(RTDEV_NVRAM, "WscConfigured");
	if (wordlist)
		g_Raix_wsc_configured = atoi(wordlist);
	else
		g_Raix_wsc_configured = 0;

	g_WscResult = 0;
	g_isEnrollee = 0;
}

/*
 * used
 */
static int getRaixPINASP(int eid, webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, T("%08d"), getAPPIN("rai0"));
	return 0;
}

/* Load from Web */
#define LFW(x, y)	do{							\
				if(!(x = websGetVar(wp, T(#y), T(""))))		\
					return;					\
			} while (0)

#define LFWi(x, y)	do{							\
				char_t *x##_str;				\
				if (!(x##_str = websGetVar(wp, T(#y), T(""))))	\
					return;					\
				x = atoi(x##_str);				\
			} while (0)

static void STF(int index, char *flash_key, char *value)
{
	char *tmp = (char *) nvram_bufget(RTDEV_NVRAM, flash_key);
	nvram_bufset(RTDEV_NVRAM, flash_key, setNthValue(index, tmp, value));
	return;
}

static void RaixWPSSetup(webs_t wp, char_t *path, char_t *query)
{
	int     wsc_enable = 0;

	LFWi(wsc_enable, WPSEnable);

	resetTimerAll();
	g_WscResult = 0;
	//LedReset();

	if (wsc_enable == 0) {
		nvram_bufset(RTDEV_NVRAM, "WscModeOption", "0");
	} else {
		nvram_bufset(RTDEV_NVRAM, "WscModeOption", "7");
	}
	nvram_commit(RTDEV_NVRAM);

	doSystem("kill -9 `cat /var/run/wscd.pid.rai0`");
	if (wsc_enable == 0) {
		doSystem("iwpriv rai0 set WscConfMode=0 1>/dev/null 2>&1");
		const char *rax_wsc_enable = nvram_bufget(RT2860_NVRAM, "WscModeOption");
		if (strcmp(rax_wsc_enable, "0") == 0) 
			doSystem("route delete 239.255.255.250 1>/dev/null 2>&1");
	} else {
		char lan_if_addr[16];
		if ((getIfIp(getLanIfName(), lan_if_addr)) == -1) {
			printf("WPSRestart error, can't get lan ip.\n");
			return;
		}
		doSystem("iwpriv rai0 set WscConfMode=%d", 7);
		doSystem("route add -host 239.255.255.250 dev br0");
		doSystem("wscd -m 1 -a %s -i rai0 &", lan_if_addr);
	}
	
	websRedirect(wp, "wps/wps_inic.asp");
}

static void GenRaixPIN(webs_t wp, char_t *path, char_t *query)
{
	doSystem("iwpriv rai0 set WscGenPinCode");

	char new_pin[9];
	sprintf(new_pin, "%08d", getAPPIN("rai0"));

	nvram_bufset(RTDEV_NVRAM, "WscVendorPinCode", new_pin);
	nvram_commit(RTDEV_NVRAM);
	doSystem("ralink_init make_wireless_config rtdev");

	websRedirect(wp, "wps/wps_inic.asp");
}

/*
 *  AP: OOB
 */
static void RaixOOB(webs_t wp, char_t *path, char_t *query)
{
        char SSID[64], mac[32];

        // clear timer
        resetTimerAll();

        // clear WSC result indicator
        g_WscResult = 0;
        //LedReset();

        if (getAPMac("rai0", mac) != -1)
                sprintf(SSID, "RalinkInitAP_%s", mac);
        else
                sprintf(SSID, "RalinkInitAP_unknown");
        nvram_bufset(RTDEV_NVRAM, "SSID1", SSID);

        nvram_bufset(RTDEV_NVRAM, "WscConfigured", "0");

        STF(0, "AuthMode", "OPEN");
        STF(0, "EncrypType", "NONE");

        STF(0, "IEEE8021X", "0");

        /*
         *   IMPORTANT !!!!!
         *   5VT doesn't need it cause it will reboot after OOB reset, but RT2880 does.
         */
        g_Raix_wsc_configured = 0;

        nvram_commit(RTDEV_NVRAM);

        doSystem("iwpriv rai0 set AuthMode=OPEN");
        doSystem("iwpriv rai0 set EncrypType=NONE");
        doSystem("iwpriv rai0 set SSID=%s", nvram_bufget(RTDEV_NVRAM, "SSID1"));

        restart8021XDaemon(RTDEV_NVRAM);

        RaixWPSRestart();

        websRedirect(wp, "wps/wps_inic.asp");
}

static void WPSAPTimerHandler(int signo)
{
	int WscStatus = 0;
	static int wsc_timeout_counter = 0;
	struct _WSC_CONFIGURED_VALUE wsc_value;

	WscStatus = getWscStatus("rai0");
	printf("WscStatus == %d\n", WscStatus);

	if (WscStatus == 3 && g_wps_timer_state == 0) {	// 3 == "Start WSC Process"
		printf("goahead: Start to monitor WSC Status...\n");
		g_wps_timer_state = 1;
		wsc_timeout_counter = 0;
		//LedInProgress();
	}

	// check if timeout
	wsc_timeout_counter += WPS_AP_CATCH_CONFIGURED_TIMER;
	if (wsc_timeout_counter > WPS_AP_TIMEOUT_SECS) {
		// Timeout happened.
		// Set g_WscResult to indicate WSC process failed.
		g_WscResult = -1;
		wsc_timeout_counter = 0;
		resetTimerAll();
		//LedError();
		trace(0, T("-- WSC failed, Timeout\n"));
		return;
	}

	// deal with error
	if (WscStatus == 2) {								// 2 == "Wsc Process failed"
		if (g_isEnrollee) {
			return;		// go on monitoring
		} else {
			g_WscResult = -1;
			fprintf(stderr, "%s", "Error occured. Is the PIN correct?\n");
		}

		// set g_WscResult to indicate WSC process failed.
		wsc_timeout_counter = 0;
		resetTimerAll();

		return;
	}

	// Driver 1.9 supports AP PBC Session Overlapping Detection.
	if (WscStatus == 0x109 /* PBC_SESSION_OVERLAP */) {
		g_WscResult = -1;
		wsc_timeout_counter = 0;
		resetTimerAll();                
		//LedSessionOverlapDetected();
		return;
	}

	// then check idle status
	if (WscStatus == 1 /*Idle*/ && g_Raix_wsc_configured == 1) {
		// This means a proxy WPS AP (has got profile from other registrar)
		// transfer registrar's profile to enrollee successfully.
		wsc_timeout_counter = 0;
		resetTimerAll();
		return;
	}

	if (WscStatus != 34 /* Configured*/) {			// 34 == "Configured"
		// still in progress and keep monitoring.
		return;
	} else {
		wsc_timeout_counter = 0;
		resetTimerAll();
		g_WscResult = 1;
		//LedSuccess();

		getWscProfile("rai0", &wsc_value, sizeof(WSC_CONFIGURED_VALUE));
		if (g_Raix_wsc_configured == 0 && wsc_value.WscConfigured == 2) {

			nvram_bufset(RTDEV_NVRAM, "WscConfigured", "1");
			g_Raix_wsc_configured = 1;

			nvram_bufset(RTDEV_NVRAM, "SSID1", wsc_value.WscSsid);
			nvram_bufset(RTDEV_NVRAM, "WscSSID", wsc_value.WscSsid);

			if (wsc_value.WscAuthMode == 0x0001) {
				STF(0, "AuthMode", "OPEN");
			} else if (wsc_value.WscAuthMode == 0x0002) {
				STF(0, "AuthMode", "WPAPSK");
			} else if (wsc_value.WscAuthMode == 0x0004) {
				STF(0, "AuthMode", "SHARED");
			} else if (wsc_value.WscAuthMode == 0x0008) {
				STF(0, "AuthMode", "WPA");
			} else if (wsc_value.WscAuthMode == 0x0010) {
				STF(0, "AuthMode", "WPA2");
			} else if (wsc_value.WscAuthMode == 0x0020) {
				STF(0, "AuthMode", "WPA2PSK");
			} else if (wsc_value.WscAuthMode == 0x0022) {
				STF(0, "AuthMode", "WPAPSKWPA2PSK");
			} else {
				printf("goahead: Warning: can't get invalid authmode\n.");
				STF(0, "AuthMode", "OPEN");
			}
			if (wsc_value.WscEncrypType == 0x0001) {
				STF(0, "EncrypType", "NONE");
				STF(0, "DefaultKeyID", "1");
			} else if (wsc_value.WscEncrypType == 0x0002) {
				STF(0, "EncrypType", "WEP");
				if ((strlen(wsc_value.WscWPAKey) == 5) || (strlen(wsc_value.WscWPAKey) == 13)) {
					// Key Entry Method == ASCII 
					STF(0, "Key1Type", "1");
					STF(0, "Key2Type", "1");
					STF(0, "Key3Type", "1");
					STF(0, "Key4Type", "1");
				} else if ((strlen(wsc_value.WscWPAKey) == 10) || (strlen(wsc_value.WscWPAKey) == 26)) {
					// Key Entry Method == HEX 
					STF(0, "Key1Type", "0");
					STF(0, "Key2Type", "0");
					STF(0, "Key3Type", "0");
					STF(0, "Key4Type", "0");
				} else {
					// Key Entry Method == ASCII
					STF(0, "Key1Type", "1");
					STF(0, "Key2Type", "1");
					STF(0, "Key3Type", "1");
					STF(0, "Key4Type", "1");
				}

				if (wsc_value.DefaultKeyIdx == 1) {
					STF(0, "Key1Str1", wsc_value.WscWPAKey);
					STF(0, "DefaultKeyID", "1");
				} else if (wsc_value.DefaultKeyIdx == 2) {
					STF(0, "Key2Str1", wsc_value.WscWPAKey);
					STF(0, "DefaultKeyID", "2");
				} else if (wsc_value.DefaultKeyIdx == 3) {
					STF(0, "Key3Str1", wsc_value.WscWPAKey);
					STF(0, "DefaultKeyID", "3");
				} else if (wsc_value.DefaultKeyIdx == 4) {
					STF(0, "Key4Str1", wsc_value.WscWPAKey);
					STF(0, "DefaultKeyID", "4");
				}
			} else if (wsc_value.WscEncrypType == 0x0004) {
				STF(0, "EncrypType", "TKIP");
				STF(0, "DefaultKeyID", "2");
				nvram_bufset(RTDEV_NVRAM, "WPAPSK1", wsc_value.WscWPAKey);
			} else if (wsc_value.WscEncrypType == 0x0008) {
				STF(0, "EncrypType", "AES");
				STF(0, "DefaultKeyID", "2");
				nvram_bufset(RTDEV_NVRAM, "WPAPSK1", wsc_value.WscWPAKey);
			} else if (wsc_value.WscEncrypType == 0x000C) {
				STF(0, "EncrypType", "TKIPAES");
				STF(0, "DefaultKeyID", "2");
				nvram_bufset(RTDEV_NVRAM, "WPAPSK1", wsc_value.WscWPAKey);
			} else {
				printf("goahead: Warning: can't get invalid encryptype\n.");
				STF(0, "EncrypType", "NONE");
				STF(0, "DefaultKeyID", "1");
			}

			STF(0, "IEEE8021X", "0");
			nvram_commit(RTDEV_NVRAM);

			restart8021XDaemon(RTDEV_NVRAM);

			const char *wordlist = nvram_get(RTDEV_NVRAM, "WscConfigured");
			if (wordlist)
				g_Raix_wsc_configured = atoi(wordlist);
			else
				g_Raix_wsc_configured = 0;

			g_isEnrollee = 0;
		}
	}

	return;
}

/*
 * WPS Single Trigger Signal handler.
 */
void RaixWPSSingleTriggerHandler(int signo)
{
	// WPS single trigger is launch now and AP is as enrollee
	g_isEnrollee = 1;
	resetTimerAll();
	setTimer(WPS_AP_CATCH_CONFIGURED_TIMER * 1000, WPSAPTimerHandler);
}

static void RaixWPS(webs_t wp, char_t *path, char_t *query)
{
	int	pin_code = 0;

	char_t *wsc_config_option;
	char_t *wsc_pin_code_w;

	LFW(wsc_config_option, PINPBCRadio);

	// reset wsc result indicator
	g_WscResult = 0;
	//LedReset();
	if (!strcmp(wsc_config_option, "1")) {
		doSystem("iwpriv rai0 set WscMode=1");

		// get pin code
		wsc_pin_code_w = websGetVar(wp, T("PIN"), T(""));
		if (!wsc_config_option || strlen(wsc_config_option) == 0) {
			pin_code = 0;
		} else {
			pin_code = atoi(wsc_pin_code_w);
		}

		g_isEnrollee = pin_code ? 0 : 1;

		doSystem("iwpriv rai0 set WscPinCode=%d", atoi(wsc_pin_code_w));
		doSystem("iwpriv rai0 set WscGetConf=1");

		resetTimerAll();
		setTimer(WPS_AP_CATCH_CONFIGURED_TIMER * 1000, WPSAPTimerHandler);
	} else if (!strcmp(wsc_config_option, "2")) {
		g_isEnrollee = 1;
		doSystem("iwpriv rai0 set WscMode=2");
		doSystem("iwpriv rai0 set WscGetConf=1");
		resetTimerAll();
		setTimer(WPS_AP_CATCH_CONFIGURED_TIMER * 1000, WPSAPTimerHandler);
	} else {
		printf("ignore unknown WSC method: %s\n", wsc_config_option);
	}

	websRedirect(wp, "wps/wps_inic.asp"); 
}

void formDefineRaixWPS(void)
{
	websAspDefine(T("getRaixPINASP"), getRaixPINASP);

	websFormDefine(T("RaixWPSSetup"), RaixWPSSetup);
	websFormDefine(T("GenRaixPIN"), GenRaixPIN);
	websFormDefine(T("RaixOOB"), RaixOOB);
	websFormDefine(T("RaixUpdateWPS"), RaixUpdateWPS);
	websFormDefine(T("RaixWPS"), RaixWPS);
}
