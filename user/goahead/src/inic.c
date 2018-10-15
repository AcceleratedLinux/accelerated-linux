/* vi: set sw=4 ts=4 sts=4: */
/*
 *	inic.c -- Intelligent NIC Settings 
 *
 *	Copyright (c) Ralink Technology Corporation All Rights Reserved.
 *
 *	$Id: inic.c,v 1.34.2.1 2010-03-02 13:21:12 chhung Exp $
 */

#include	<stdlib.h>
#include	"internet.h"
#include	"nvram.h"
#include	"utils.h"
#include	"webs.h"
#include	"inic.h"
#include	"wireless.h"		// inic.c inherits many security functions from it(wireless.c).

extern int g_Raix_wsc_configured;

static int	get2AutoProvisionBuilt(int eid, webs_t wp, int argc, char_t **argv);
static int  get2DLSBuilt(int eid, webs_t wp, int argc, char_t **argv);
static int  getInic11aChannels(int eid, webs_t wp, int argc, char_t **argv);
static int  getInic11bChannels(int eid, webs_t wp, int argc, char_t **argv);
static int  getInic11gChannels(int eid, webs_t wp, int argc, char_t **argv);
static int  getInicChannel(int eid, webs_t wp, int argc, char_t **argv);
static int  getInicCurrentMac(int eid, webs_t wp, int argc, char_t **argv);
static int  getInicWdsEncType(int eid, webs_t wp, int argc, char_t **argv);
static void inicBasic(webs_t wp, char_t *path, char_t *query);
static void inicAdvanced(webs_t wp, char_t *path, char_t *query);
static void inicWmm(webs_t wp, char_t *path, char_t *query);
static void inicSecurity(webs_t wp, char_t *path, char_t *query);
static void inicwGetSecurity(webs_t wp, char_t *path, char_t *query);
static void INICSecurity(webs_t wp, char_t *path, char_t *query);
static void INICDeleteAccessPolicyList(webs_t wp, char_t *path, char_t *query);

void formDefineInic(void)
{
	websAspDefine(T("get2AutoProvisionBuilt"), get2AutoProvisionBuilt);
	websAspDefine(T("get2DLSBuilt"), get2DLSBuilt);
	websAspDefine(T("getInic11aChannels"), getInic11aChannels);
	websAspDefine(T("getInic11bChannels"), getInic11bChannels);
	websAspDefine(T("getInic11gChannels"), getInic11gChannels);
	websAspDefine(T("getInicChannel"), getInicChannel);
	websAspDefine(T("getInicCurrentMac"), getInicCurrentMac);
	websAspDefine(T("getInicWdsEncType"), getInicWdsEncType);
	websFormDefine(T("inicBasic"), inicBasic);
	websFormDefine(T("inicAdvanced"), inicAdvanced);
	websFormDefine(T("inicWmm"), inicWmm);
	websFormDefine(T("inicwGetSecurity"), inicwGetSecurity);
	websFormDefine(T("INICSecurity"), INICSecurity);
	websFormDefine(T("INICDeleteAccessPolicyList"), INICDeleteAccessPolicyList);
}

/*
 * description: write 802.11a channels in <select> tag
 */
static int getInic11aChannels(int eid, webs_t wp, int argc, char_t **argv)
{
	int  idx = 0, channel;
	const char *value = nvram_bufget(RTDEV_NVRAM, "CountryCode");
	const char *channel_s = nvram_bufget(RTDEV_NVRAM, "Channel");

	channel = (channel_s == NULL)? 0 : atoi(channel_s);
	if ((value == NULL) || (strcmp(value, "") == 0) ||
			(strcmp(value, "US") == 0) || (strcmp(value, "FR") == 0) ||
			(strcmp(value, "IE") == 0) || (strcmp(value, "JP") == 0) ||
			(strcmp(value, "HK") == 0)) {
		for (idx = 0; idx < 4; idx++)
			websWrite(wp, T("%s%d %s>%d%s%d%s"), "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
	}
	if ((value == NULL) || (strcmp(value, "") == 0) ||
			(strcmp(value, "US") == 0) || (strcmp(value, "FR") == 0) ||
			(strcmp(value, "IE") == 0) || (strcmp(value, "TW") == 0) ||
			(strcmp(value, "HK") == 0)) {
		for (idx = 4; idx < 8; idx++)
			websWrite(wp, T("%s%d %s>%d%s%d%s"), "<option value=", 36+4*idx,
					(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
	}
	if ((value == NULL) || (strcmp(value, "") == 0)) {
		for (idx = 16; idx < 27; idx++)
			websWrite(wp, T("%s%d %s>%d%s%d%s"), "<option value=", 36+4*idx,
				   	(36+4*idx == channel)? "selected" : "", 5180+20*idx,
					"MHz (Channel ", 36+4*idx, ")</option>");
	}
	if ((value == NULL) || (strcmp(value, "") == 0) ||
			(strcmp(value, "US") == 0) || (strcmp(value, "TW") == 0) ||
			(strcmp(value, "CN") == 0) || (strcmp(value, "HK") == 0)) {
		for (idx = 28; idx < 32; idx++)
			websWrite(wp, T("%s%d %s>%d%s%d%s"), "<option value=",
					36+4*idx+1, (36+4*idx+1 == channel)? "selected" : "",
					5180+20*idx+5, "MHz (Channel ", 36+4*idx+1, ")</option>");
	}
	if ((value == NULL) || (strcmp(value, "") == 0) ||
			(strcmp(value, "US") == 0) || (strcmp(value, "CN") == 0) ||
			(strcmp(value, "HK") == 0)) {
		return websWrite(wp,
				T("<option value=165 %s>5825MHz (Channel 165)</option>\n"),
				(165 == channel)? "selected" : "");
	}
	return 0;
}

/*
 * description: write 802.11b channels in <select> tag
 */
static int getInic11bChannels(int eid, webs_t wp, int argc, char_t **argv)
{
	int idx = 0, channel;
	const char *value = nvram_bufget(RTDEV_NVRAM, "CountryCode");
	const char *channel_s = nvram_bufget(RTDEV_NVRAM, "Channel");

	channel = (channel_s == NULL)? 0 : atoi(channel_s);
	if ((value == NULL) || (strcmp(value, "") == 0) ||
			(strcmp(value, "US") == 0) || (strcmp(value, "JP") == 0) ||
			(strcmp(value, "FR") == 0) || (strcmp(value, "IE") == 0) ||
			(strcmp(value, "TW") == 0) || (strcmp(value, "CN") == 0) ||
			(strcmp(value, "HK") == 0)) {
		for (idx = 0; idx < 11; idx++)
			websWrite(wp, T("%s%d %s>%d%s%d%s"), "<option value=", idx+1,
					(idx+1 == channel)? "selected" : "", 2412+5*idx,
					"MHz (Channel ", idx+1, ")</option>");
	}

	if ((value == NULL) || (strcmp(value, "") == 0) ||
			(strcmp(value, "JP") == 0) || (strcmp(value, "TW") == 0) ||
			(strcmp(value, "FR") == 0) ||
			(strcmp(value, "IE") == 0) || (strcmp(value, "CN") == 0) ||
			(strcmp(value, "HK") == 0)) {
		for (idx = 11; idx < 13; idx++)
			websWrite(wp, T("%s%d %s>%d%s%d%s"), "<option value=", idx+1,
					(idx+1 == channel)? "selected" : "", 2412+5*idx,
					"MHz (Channel ", idx+1, ")</option>");
	}

	if ((value == NULL) || (strcmp(value, "") == 0) ||
			(strcmp(value, "JP") == 0)) {
		return websWrite(wp,
				T("<option value=14 %s>2484MHz (Channel 14)</option>\n"),
				(14 == channel)? "selected" : "");
	}
	return 0;
}

/*
 * description: write 802.11g channels in <select> tag
 */
static int getInic11gChannels(int eid, webs_t wp, int argc, char_t **argv)
{
	int idx = 0, channel;
	const char *value = nvram_bufget(RTDEV_NVRAM, "CountryCode");
	const char *channel_s = nvram_bufget(RTDEV_NVRAM, "Channel");

	channel = (channel_s == NULL)? 0 : atoi(channel_s);
	if ((value == NULL) || (strcmp(value, "") == 0) ||
			(strcmp(value, "US") == 0) || (strcmp(value, "JP") == 0) ||
			(strcmp(value, "FR") == 0) || (strcmp(value, "IE") == 0) ||
			(strcmp(value, "TW") == 0) || (strcmp(value, "CN") == 0) ||
			(strcmp(value, "HK") == 0)) {
		for (idx = 0; idx < 11; idx++)
			websWrite(wp, T("%s%d %s>%d%s%d%s"), "<option value=", idx+1,
					(idx+1 == channel)? "selected" : "", 2412+5*idx,
					"MHz (Channel ", idx+1, ")</option>");
	}                                                                           
	if ((value == NULL) || (strcmp(value, "") == 0) ||
			(strcmp(value, "JP") == 0) || (strcmp(value, "TW") == 0) ||
			(strcmp(value, "FR") == 0) || (strcmp(value, "IE") == 0) ||
			(strcmp(value, "CN") == 0) || (strcmp(value, "HK") == 0)) {
		for (idx = 11; idx < 13; idx++)
			websWrite(wp, T("%s%d %s>%d%s%d%s"), "<option value=", idx+1,
					(idx+1 == channel)? "selected" : "", 2412+5*idx,
					"MHz (Channel ", idx+1, ")</option>");
	}

	if ((value == NULL) || (strcmp(value, "") == 0)) {
		return websWrite(wp,
				T("<option value=14 %s>2484MHz (Channel 14)</option>\n"),
				(14 == channel)? "selected" : "");
	}
	return 0;
}

/*
 * description: write channel number or 0 if auto-select
 */
static int getInicChannel(int eid, webs_t wp, int argc, char_t **argv)
{
	const char *value = nvram_bufget(RTDEV_NVRAM, "AutoChannelSelect");

	if (NULL == value)
		return websWrite(wp, T("9"));
	if (!strncmp(value, "1", 2))
		return websWrite(wp, T("0"));

	value = nvram_bufget(RTDEV_NVRAM, "Channel");
	if (NULL == value)
		return websWrite(wp, T("9"));
	else
		return websWrite(wp, T("%s"), value);
}

/*
 * description: write MAC address from interface 'rai0'
 */
static int getInicCurrentMac(int eid, webs_t wp, int argc, char_t **argv)
{
	char if_hw[18] = {0};

	if (-1 == getIfMac("rai0", if_hw))
		return websWrite(wp, T(" "));
	return websWrite(wp, T("%s"), if_hw);
}

/*
 * description: write the WDS Encryp Type
 */
static int getInicWdsEncType(int eid, webs_t wp, int argc, char_t **argv)
{
	const char *value = nvram_bufget(RTDEV_NVRAM, "WdsEncrypType");

	if (NULL == value)
		return websWrite(wp, T("0"));
	else if (strcmp(value, "NONE;NONE;NONE;NONE") == 0)
		return websWrite(wp, T("0"));
	else if (strcmp(value, "WEP;WEP;WEP;WEP") == 0)
		return websWrite(wp, T("1"));
	else if (strcmp(value, "TKIP;TKIP;TKIP;TKIP") == 0)
		return websWrite(wp, T("2"));
	else if (strcmp(value, "AES;AES;AES;AES") == 0)
		return websWrite(wp, T("3"));
	else
		return websWrite(wp, T("0"));
}

static void revise_mbss_value(int old_num, int new_num)
{
	/* {{{ The parameters that support multiple BSSID is listed as followed,
	   1.) SSID,                 char SSID[33];
	   2.) AuthMode,             char AuthMode[14];
	   3.) EncrypType,           char EncrypType[8];
	   4.) WPAPSK,               char WPAPSK[65];
	   5.) DefaultKeyID,         int  DefaultKeyID;
	   6.) Key1Type,             int  Key1Type;
	   7.) Key1Str,              char Key1Str[27];
	   8.) Key2Type,             int  Key2Type;
	   9.) Key2Str,              char Key2Str[27];
	   10.) Key3Type,            int  Key3Type;
	   11.) Key3Str,             char Key3Str[27];
	   12.) Key4Type,            int  Key4Type;
	   13.) Key4Str,             char Key4Str[27];
	   14.) AccessPolicy,
	   15.) AccessControlList,
	   16.) NoForwarding,
	   17.) IEEE8021X,           int  IEEE8021X;
	   18.) TxRate,              int  TxRate;
	   19.) HideSSID,            int  HideSSID;
	   20.) PreAuth,             int  PreAuth;
	   21.) WmmCapable
	                             int  SecurityMode;
                             	 char VlanName[20];
	                             int  VlanId;
	                             int  VlanPriority;
	}}} */
	char new_value[264], *p;
	const char *old_value;
	int i;

#define MBSS_INIT(field, default_value) \
	do { \
		old_value = nvram_bufget(RTDEV_NVRAM, #field); \
		snprintf(new_value, 264, "%s", old_value); \
		p = new_value + strlen(old_value); \
		for (i = old_num; i < new_num; i++) { \
			snprintf(p, 264 - (p - new_value), ";%s", default_value); \
			p += 1 + strlen(default_value); \
		} \
		nvram_bufset(RTDEV_NVRAM, #field, new_value); \
	} while (0)

#define MBSS_REMOVE(field) \
	do { \
		old_value = nvram_bufget(RTDEV_NVRAM, #field); \
		snprintf(new_value, 264, "%s", old_value); \
		p = new_value; \
		for (i = 0; i < new_num; i++) { \
			if (0 == i) \
				p = strchr(p, ';'); \
			else \
				p = strchr(p+1, ';'); \
			if (NULL == p) \
				break; \
		} \
		if (p) \
			*p = '\0'; \
		nvram_bufset(RTDEV_NVRAM, #field, new_value); \
	} while (0)

	if (new_num > old_num) {
		//MBSS_INIT(SSID, "ssid");
		MBSS_INIT(AuthMode, "OPEN");
		MBSS_INIT(EncrypType, "NONE");
		//MBSS_INIT(WPAPSK, "12345678");
		MBSS_INIT(DefaultKeyID, "1");
		MBSS_INIT(Key1Type, "0");
		//MBSS_INIT(Key1Str, "");
		MBSS_INIT(Key2Type, "0");
		//MBSS_INIT(Key2Str, "");
		MBSS_INIT(Key3Type, "0");
		//MBSS_INIT(Key3Str, "");
		MBSS_INIT(Key4Type, "0");
		//MBSS_INIT(Key4Str, "");
/*		MBSS_INIT(AccessPolicy0, "0");
		MBSS_INIT(AccessControlList0, "");
		MBSS_INIT(AccessPolicy1, "0");
		MBSS_INIT(AccessControlList1, "");
		MBSS_INIT(AccessPolicy2, "0");
		MBSS_INIT(AccessControlList2, "");
		MBSS_INIT(AccessPolicy3, "0");
		MBSS_INIT(AccessControlList3, ""); */
		MBSS_INIT(NoForwarding, "0");
		MBSS_INIT(IEEE8021X, "0");
		MBSS_INIT(TxRate, "0");
		//MBSS_INIT(HideSSID, "0");
		MBSS_INIT(PreAuth, "0");
		MBSS_INIT(WmmCapable, "1");
		for (i = old_num + 1; i <= new_num; i++) {
			nvram_bufset(RTDEV_NVRAM, racat("WPAPSK", i), "12345678");
			nvram_bufset(RTDEV_NVRAM, racat("Key1Str", i), "");
			nvram_bufset(RTDEV_NVRAM, racat("Key2Str", i), "");
			nvram_bufset(RTDEV_NVRAM, racat("Key3Str", i), "");
			nvram_bufset(RTDEV_NVRAM, racat("Key4Str", i), "");
			// The index of AccessPolicy & AccessControlList starts at 0.
			nvram_bufset(RTDEV_NVRAM, racat("AccessPolicy", i-1), "0");
			nvram_bufset(RTDEV_NVRAM, racat("AccessControlList", i-1), "");
		}
	}
	else if (new_num < old_num) {
		//MBSS_REMOVE(SSID);
		MBSS_REMOVE(AuthMode);
		MBSS_REMOVE(EncrypType);
		//MBSS_REMOVE(WPAPSK);
		MBSS_REMOVE(DefaultKeyID);
		MBSS_REMOVE(Key1Type);
		//MBSS_REMOVE(Key1Str);
		MBSS_REMOVE(Key2Type);
		//MBSS_REMOVE(Key2Str);
		MBSS_REMOVE(Key3Type);
		//MBSS_REMOVE(Key3Str);
		MBSS_REMOVE(Key4Type);
		//MBSS_REMOVE(Key4Str);
/*		MBSS_REMOVE(AccessPolicy0);
		MBSS_REMOVE(AccessControlList0);
		MBSS_REMOVE(AccessPolicy1);
		MBSS_REMOVE(AccessControlList1);
		MBSS_REMOVE(AccessPolicy2);
		MBSS_REMOVE(AccessControlList2);
		MBSS_REMOVE(AccessPolicy3);
		MBSS_REMOVE(AccessControlList3); */
		MBSS_REMOVE(NoForwarding);
		MBSS_REMOVE(IEEE8021X);
		MBSS_REMOVE(TxRate);
		MBSS_REMOVE(HideSSID);
		MBSS_REMOVE(PreAuth);
		MBSS_REMOVE(WmmCapable);
		for (i = new_num + 1; i <= old_num; i++) {
			nvram_bufset(RTDEV_NVRAM, racat("SSID", i), "");
			nvram_bufset(RTDEV_NVRAM, racat("WPAPSK", i), "");
			nvram_bufset(RTDEV_NVRAM, racat("Key1Str", i), "");
			nvram_bufset(RTDEV_NVRAM, racat("Key2Str", i), "");
			nvram_bufset(RTDEV_NVRAM, racat("Key3Str", i), "");
			nvram_bufset(RTDEV_NVRAM, racat("Key4Str", i), "");
			// The index of AccessPolicy & AccessControlList starts at 0.
			nvram_bufset(RTDEV_NVRAM, racat("AccessPolicy", i-1), "0");
			nvram_bufset(RTDEV_NVRAM, racat("AccessControlList", i-1), "");
		}
	}
}

/* goform/inicBasic */
static void inicBasic(webs_t wp, char_t *path, char_t *query)
{
	char_t	*wirelessmode;
	char_t	*ssid, *mssid_1, *mssid_2, *mssid_3, *mssid_4, *mssid_5, *mssid_6,
			*mssid_7, *bssid_num, *broadcastssid;
	char_t	*sz11aChannel, *sz11bChannel, *sz11gChannel, *abg_rate;
	char_t	*wds_list, *wds_mode, *wds_phy_mode, *wds_encryp_type, *wds_encryp_key;
	char_t	*n_mode, *n_bandwidth, *n_gi, *n_mcs, *n_rdg, *n_extcha, *n_amsdu;
	char_t	*n_autoba, *n_badecline;
	char_t	*tx_stream, *rx_stream;
	int i = 0, is_n = 0, new_bssid_num, old_bssid_num = 1;

	//fetch from web input
	wirelessmode = websGetVar(wp, T("wirelessmode"), T("9")); //9: bgn mode
	ssid = websGetVar(wp, T("ssid"), T("")); 
	mssid_1 = websGetVar(wp, T("mssid_1"), T("")); 
	mssid_2 = websGetVar(wp, T("mssid_2"), T("")); 
	mssid_3 = websGetVar(wp, T("mssid_3"), T("")); 
	mssid_4 = websGetVar(wp, T("mssid_4"), T("")); 
	mssid_5 = websGetVar(wp, T("mssid_5"), T("")); 
	mssid_6 = websGetVar(wp, T("mssid_6"), T("")); 
	mssid_7 = websGetVar(wp, T("mssid_7"), T("")); 
	bssid_num = websGetVar(wp, T("bssid_num"), T("1"));
	broadcastssid = websGetVar(wp, T("broadcastssid"), T("1")); 
	sz11aChannel = websGetVar(wp, T("sz11aChannel"), T("")); 
	sz11bChannel = websGetVar(wp, T("sz11bChannel"), T("")); 
	sz11gChannel = websGetVar(wp, T("sz11gChannel"), T("")); 
	abg_rate = websGetVar(wp, T("abg_rate"), T("")); 
	wds_mode = websGetVar(wp, T("wds_mode"), T("0")); 
	wds_phy_mode = websGetVar(wp, T("wds_phy_mode"), T("0")); 
	wds_encryp_type = websGetVar(wp, T("wds_encryp_type"), T("0")); 
	wds_encryp_key = websGetVar(wp, T("wds_encryp_key"), T("0")); 
	wds_list = websGetVar(wp, T("wds_list"), T("")); 
	if (strlen(wds_list) > 0)
		wds_list[strlen(wds_list) - 1] = '\0';
	n_mode = websGetVar(wp, T("n_mode"), T("0"));
	n_bandwidth = websGetVar(wp, T("n_bandwidth"), T("0"));
	n_gi = websGetVar(wp, T("n_gi"), T("0"));
	n_mcs = websGetVar(wp, T("n_mcs"), T("33"));
	n_rdg = websGetVar(wp, T("n_rdg"), T("0"));
	n_extcha = websGetVar(wp, T("n_extcha"), T("0"));
	n_amsdu = websGetVar(wp, T("n_amsdu"), T("0"));
	n_autoba = websGetVar(wp, T("n_autoba"), T("0"));
	n_badecline = websGetVar(wp, T("n_badecline"), T("0"));
	tx_stream = websGetVar(wp, T("tx_stream"), T("0"));
	rx_stream = websGetVar(wp, T("rx_stream"), T("0"));

	old_bssid_num = atoi(nvram_bufget(RTDEV_NVRAM, "BssidNum"));
	new_bssid_num = atoi(bssid_num);

	nvram_bufset(RTDEV_NVRAM, "WirelessMode", wirelessmode);
	if (!strncmp(wirelessmode, "4", 2))
		nvram_bufset(RTDEV_NVRAM, "BasicRate", "351");
	else if (!strncmp(wirelessmode, "8", 2) || !strncmp(wirelessmode, "9", 2))
		is_n = 1;

#if 0
	i = atoi(mode);
	if (i == 1)
		nvram_bufset(RTDEV_NVRAM, "BasicRate", "3");
	else if (i == 0) {
		nvram_bufset(RTDEV_NVRAM, "TxPreamble","0");
		nvram_bufset(RTDEV_NVRAM, "TxBurst", "0");
		nvram_bufset(RTDEV_NVRAM, "PktAggregate", "0");
	}
	else if (i == 5) {
		nvram_bufset(RTDEV_NVRAM, "TxPreamble","1");
		nvram_bufset(RTDEV_NVRAM, "TxBurst", "1");
		nvram_bufset(RTDEV_NVRAM, "PktAggregate", "1");
		nvram_bufset(RTDEV_NVRAM, "DisableOLBC", "1");
	}
	if (i != 5) {
		nvram_bufset(RTDEV_NVRAM, "DisableOLBC", "0");
	}
#endif

	//SSID, Multiple SSID
	if (0 == strlen(ssid)) {
		nvram_commit(RTDEV_NVRAM);
		websError(wp, 403, T("'SSID' should not be empty!"));
		return;
	}
	nvram_bufset(RTDEV_NVRAM, "SSID1", ssid);

//#WPS
	{
		const char *wordlist= nvram_bufget(RTDEV_NVRAM, "WscModeOption");
		if(wordlist){
			if (strcmp(wordlist, "0"))
				doSystem("iwpriv rai0 set WscConfStatus=1");
			nvram_bufset(RTDEV_NVRAM, "WscConfigured", "1");
			g_Raix_wsc_configured = 1;
		}
	}

//#WPS

	i = 2;
	if (0 != strlen(mssid_1)) {
		nvram_bufset(RTDEV_NVRAM, racat("SSID", i), mssid_1);
		i++;
	}
	if (0 != strlen(mssid_2)) {
		nvram_bufset(RTDEV_NVRAM, racat("SSID", i), mssid_2);
		i++;
	}
	if (0 != strlen(mssid_3)) {
		nvram_bufset(RTDEV_NVRAM, racat("SSID", i), mssid_3);
		i++;
	}
	if (0 != strlen(mssid_4)) {
		nvram_bufset(RTDEV_NVRAM, racat("SSID", i), mssid_4);
		i++;
	}
	if (0 != strlen(mssid_5)) {
		nvram_bufset(RTDEV_NVRAM, racat("SSID", i), mssid_5);
		i++;
	}
	if (0 != strlen(mssid_6)) {
		nvram_bufset(RTDEV_NVRAM, racat("SSID", i), mssid_6);
		i++;
	}
	if (0 != strlen(mssid_7)) {
		nvram_bufset(RTDEV_NVRAM, racat("SSID", i), mssid_7);
		i++;
	}

	nvram_bufset(RTDEV_NVRAM, "BssidNum", bssid_num);
	if (new_bssid_num < 1 || new_bssid_num > 8) {
		nvram_commit(RTDEV_NVRAM);
		websError(wp, 403, T("'bssid_num' %s is out of range!"), bssid_num);
		return;
	}
	revise_mbss_value(old_bssid_num, new_bssid_num);

	//Broadcast SSID
	if (new_bssid_num == 1) {
		if (!strncmp(broadcastssid, "1", 2))
			nvram_bufset(RTDEV_NVRAM, "HideSSID", "0");
		else
			nvram_bufset(RTDEV_NVRAM, "HideSSID", "1");
	}
	else if (new_bssid_num == 2) {
		if (!strncmp(broadcastssid, "1", 2))
			nvram_bufset(RTDEV_NVRAM, "HideSSID", "0;0");
		else
			nvram_bufset(RTDEV_NVRAM, "HideSSID", "1;1");
	}
	else if (new_bssid_num == 3) {
		if (!strncmp(broadcastssid, "1", 2))
			nvram_bufset(RTDEV_NVRAM, "HideSSID", "0;0;0");
		else
			nvram_bufset(RTDEV_NVRAM, "HideSSID", "1;1;1");
	}
	else if (new_bssid_num == 4) {
		if (!strncmp(broadcastssid, "1", 2))
			nvram_bufset(RTDEV_NVRAM, "HideSSID", "0;0;0;0");
		else
			nvram_bufset(RTDEV_NVRAM, "HideSSID", "1;1;1;1");
	}
	else if (new_bssid_num == 5) {
		if (!strncmp(broadcastssid, "1", 2))
			nvram_bufset(RTDEV_NVRAM, "HideSSID", "0;0;0;0;0");
		else
			nvram_bufset(RTDEV_NVRAM, "HideSSID", "1;1;1;1;1");
	}
	else if (new_bssid_num == 6) {
		if (!strncmp(broadcastssid, "1", 2))
			nvram_bufset(RTDEV_NVRAM, "HideSSID", "0;0;0;0;0;0");
		else
			nvram_bufset(RTDEV_NVRAM, "HideSSID", "1;1;1;1;1;1");
	}
	else if (new_bssid_num == 7) {
		if (!strncmp(broadcastssid, "1", 2))
			nvram_bufset(RTDEV_NVRAM, "HideSSID", "0;0;0;0;0;0;0");
		else
			nvram_bufset(RTDEV_NVRAM, "HideSSID", "1;1;1;1;1;1;1");
	}
	else if (new_bssid_num == 8) {
		if (!strncmp(broadcastssid, "1", 2))
			nvram_bufset(RTDEV_NVRAM, "HideSSID", "0;0;0;0;0;0;0;0");
		else
			nvram_bufset(RTDEV_NVRAM, "HideSSID", "1;1;1;1;1;1;1;1");
	}

	//11abg Channel or AutoSelect
	if ((0 == strlen(sz11aChannel)) && (0 == strlen(sz11bChannel)) &&
			(0 == strlen(sz11gChannel))) {
		nvram_commit(RTDEV_NVRAM);
		websError(wp, 403, T("'Channel' should not be empty!"));
		return;
	}
	if (!strncmp(sz11aChannel, "0", 2) || !strncmp(sz11bChannel, "0", 2) ||
			!strncmp(sz11gChannel, "0", 2))
		nvram_bufset(RTDEV_NVRAM, "AutoChannelSelect", "1");
	else
		nvram_bufset(RTDEV_NVRAM, "AutoChannelSelect", "0");
	if (0 != strlen(sz11aChannel)) {
		nvram_bufset(RTDEV_NVRAM, "Channel", sz11aChannel);
		doSystem("iwpriv rai0 set Channel=%s", sz11aChannel);
	}
	if (0 != strlen(sz11bChannel)) {
		nvram_bufset(RTDEV_NVRAM, "Channel", sz11bChannel);
		doSystem("iwpriv rai0 set Channel=%s", sz11bChannel);
	}
	if (0 != strlen(sz11gChannel)) {
		nvram_bufset(RTDEV_NVRAM, "Channel", sz11gChannel);
		doSystem("iwpriv rai0 set Channel=%s", sz11gChannel);
	}

	//Rate for a, b, g
	if (strncmp(abg_rate, "", 1)) {
		int rate = atoi(abg_rate);
		if (!strncmp(wirelessmode, "0", 2) || !strncmp(wirelessmode, "2", 2) || !strncmp(wirelessmode, "4", 2)) {
			if (rate == 1 || rate == 2 || rate == 5 || rate == 11)
				nvram_bufset(RTDEV_NVRAM, "FixedTxMode", "1"); //CCK
			else
				nvram_bufset(RTDEV_NVRAM, "FixedTxMode", "2"); //OFDM

			if (rate == 1)
				nvram_bufset(RTDEV_NVRAM, "HT_MCS", "0");
			else if (rate == 2)
				nvram_bufset(RTDEV_NVRAM, "HT_MCS", "1");
			else if (rate == 5)
				nvram_bufset(RTDEV_NVRAM, "HT_MCS", "2");
			else if (rate == 6)
				nvram_bufset(RTDEV_NVRAM, "HT_MCS", "0");
			else if (rate == 9)
				nvram_bufset(RTDEV_NVRAM, "HT_MCS", "1");
			else if (rate == 11)
				nvram_bufset(RTDEV_NVRAM, "HT_MCS", "3");
			else if (rate == 12)
				nvram_bufset(RTDEV_NVRAM, "HT_MCS", "2");
			else if (rate == 18)
				nvram_bufset(RTDEV_NVRAM, "HT_MCS", "3");
			else if (rate == 24)
				nvram_bufset(RTDEV_NVRAM, "HT_MCS", "4");
			else if (rate == 36)
				nvram_bufset(RTDEV_NVRAM, "HT_MCS", "5");
			else if (rate == 48)
				nvram_bufset(RTDEV_NVRAM, "HT_MCS", "6");
			else if (rate == 54)
				nvram_bufset(RTDEV_NVRAM, "HT_MCS", "7");
			else
				nvram_bufset(RTDEV_NVRAM, "HT_MCS", "33");
		}
		else if (!strncmp(wirelessmode, "1", 2)) {
			nvram_bufset(RTDEV_NVRAM, "FixedTxMode", "1"); //CCK
			if (rate == 1)
				nvram_bufset(RTDEV_NVRAM, "HT_MCS", "0");
			else if (rate == 2)
				nvram_bufset(RTDEV_NVRAM, "HT_MCS", "1");
			else if (rate == 5)
				nvram_bufset(RTDEV_NVRAM, "HT_MCS", "2");
			else if (rate == 11)
				nvram_bufset(RTDEV_NVRAM, "HT_MCS", "3");
		}
		else //shall not happen
			error(E_L, E_LOG, T("wrong configurations from web UI"));
	}
	else
		nvram_bufset(RTDEV_NVRAM, "FixedTxMode", "0"); //None

	//WdsEnable, WdsPhyMode, WdsEncrypType, WdsKey, WdsList
	//where WdsPhyMode - 0:CCK, 1:OFDM, 2:HTMIX, 3:GREENFIELD
	//      WdsEncryptType - NONE, WEP, TKIP, AES
	nvram_bufset(RTDEV_NVRAM, "WdsEnable", wds_mode);
	if (strncmp(wds_mode, "0", 2)) {
		nvram_bufset(RTDEV_NVRAM, "WdsPhyMode", wds_phy_mode);
		nvram_bufset(RTDEV_NVRAM, "WdsEncrypType", wds_encryp_type);
		nvram_bufset(RTDEV_NVRAM, "WdsKey", wds_encryp_key);
		if (!strncmp(wds_mode, "2", 2) || !strncmp(wds_mode, "3", 2)) {
			if (0 != strlen(wds_list))
				nvram_bufset(RTDEV_NVRAM, "WdsList", wds_list);
		}
	}

	//HT_OpMode, HT_BW, HT_GI, HT_MCS, HT_RDG, HT_EXTCHA, HT_AMSDU, HT_TxStream, HT_RxStream
	if (is_n) {
		nvram_bufset(RTDEV_NVRAM, "HT_OpMode", n_mode);
		nvram_bufset(RTDEV_NVRAM, "HT_BW", n_bandwidth);
		nvram_bufset(RTDEV_NVRAM, "HT_GI", n_gi);
		nvram_bufset(RTDEV_NVRAM, "HT_MCS", n_mcs);
		nvram_bufset(RTDEV_NVRAM, "HT_RDG", n_rdg);
		nvram_bufset(RTDEV_NVRAM, "HT_EXTCHA", n_extcha);
		nvram_bufset(RTDEV_NVRAM, "HT_AMSDU", n_amsdu);
		nvram_bufset(RTDEV_NVRAM, "HT_AutoBA", n_autoba);
		nvram_bufset(RTDEV_NVRAM, "HT_BADecline", n_badecline);
	}
	nvram_bufset(RTDEV_NVRAM, "HT_TxStream", tx_stream);
	nvram_bufset(RTDEV_NVRAM, "HT_RxStream", rx_stream);

	nvram_commit(RTDEV_NVRAM);
	initInternet();

	//debug print
	websHeader(wp);
	websWrite(wp, T("<h2>mode: %s</h2><br>\n"), wirelessmode);
	websWrite(wp, T("ssid: %s, bssid_num: %s<br>\n"), ssid, bssid_num);
	websWrite(wp, T("mssid_1: %s, mssid_2: %s, mssid_3: %s<br>\n"),
			mssid_1, mssid_2, mssid_3);
	websWrite(wp, T("mssid_4: %s, mssid_5: %s, mssid_6: %s, mssid_7: %s<br>\n"),
			mssid_4, mssid_5, mssid_6, mssid_7);
	websWrite(wp, T("broadcastssid: %s<br>\n"), broadcastssid);
	websWrite(wp, T("sz11aChannel: %s<br>\n"), sz11aChannel);
	websWrite(wp, T("sz11bChannel: %s<br>\n"), sz11bChannel);
	websWrite(wp, T("sz11gChannel: %s<br>\n"), sz11gChannel);
	if (strncmp(abg_rate, "", 1)) {
		websWrite(wp, T("abg_rate: %s<br>\n"), abg_rate);
	}
	websWrite(wp, T("wds_mode: %s<br>\n"), wds_mode);
	if (strncmp(wds_mode, "0", 2)) {
		websWrite(wp, T("wds_phy_mode: %s<br>\n"), wds_phy_mode);
		websWrite(wp, T("wds_encryp_type: %s<br>\n"), wds_encryp_type);
		websWrite(wp, T("wds_encryp_key: %s<br>\n"), wds_encryp_key);
		if (!strncmp(wds_mode, "2", 2) || !strncmp(wds_mode, "3", 2))
			websWrite(wp, T("wds_list: %s<br>\n"), wds_list);
	}
	if (is_n) {
		websWrite(wp, T("n_mode: %s<br>\n"), n_mode);
		websWrite(wp, T("n_bandwidth: %s<br>\n"), n_bandwidth);
		websWrite(wp, T("n_gi: %s<br>\n"), n_gi);
		websWrite(wp, T("n_mcs: %s<br>\n"), n_mcs);
		websWrite(wp, T("n_rdg: %s<br>\n"), n_rdg);
		websWrite(wp, T("n_extcha: %s<br>\n"), n_extcha);
		websWrite(wp, T("n_amsdu: %s<br>\n"), n_amsdu);
		websWrite(wp, T("n_autoba: %s<br>\n"), n_autoba);
		websWrite(wp, T("n_badecline: %s<br>\n"), n_badecline);
	}
	websWrite(wp, T("tx_stream: %s<br>\n"), tx_stream);
	websWrite(wp, T("rx_stream: %s<br>\n"), rx_stream);
	websFooter(wp);
	websDone(wp, 200);
}

/* goform/inicAdvanced */
static void inicAdvanced(webs_t wp, char_t *path, char_t *query)
{
	char_t	*bg_protection, *basic_rate, *beacon, *dtim, *fragment, *rts,
			*tx_power, *short_preamble, *short_slot, *tx_burst, *pkt_aggregate,
			*ieee_80211h, *wmm_capable, *apsd_capable, *dls_capable, *countrycode;
	int		i, ssid_num, wlan_mode;
	char	wmm_enable[8];
#ifdef CONFIG_RT3090_AUTO_PROVISION
	char_t  *auto_provision;
#endif

	//fetch from web input
	bg_protection = websGetVar(wp, T("bg_protection"), T("0"));
	basic_rate = websGetVar(wp, T("basic_rate"), T("15"));
	beacon = websGetVar(wp, T("beacon"), T("100"));
	dtim = websGetVar(wp, T("dtim"), T("1"));
	fragment = websGetVar(wp, T("fragment"), T("2346"));
	rts = websGetVar(wp, T("rts"), T("2347"));
	tx_power = websGetVar(wp, T("tx_power"), T("100"));
	short_preamble = websGetVar(wp, T("short_preamble"), T("0"));
	short_slot = websGetVar(wp, T("short_slot"), T("0"));
	tx_burst = websGetVar(wp, T("tx_burst"), T("0"));
	pkt_aggregate = websGetVar(wp, T("pkt_aggregate"), T("0"));
#ifdef CONFIG_RT3090_AUTO_PROVISION
	auto_provision = websGetVar(wp, T("auto_provision"), T("0"));
#endif
	ieee_80211h = websGetVar(wp, T("ieee_80211h"), T("0"));
	wmm_capable = websGetVar(wp, T("wmm_capable"), T("0"));
	apsd_capable = websGetVar(wp, T("apsd_capable"), T("0"));
	dls_capable = websGetVar(wp, T("dls_capable"), T("0"));
	countrycode = websGetVar(wp, T("country_code"), T("NONE"));

	if (NULL != nvram_bufget(RTDEV_NVRAM, "BssidNum"))
		ssid_num = atoi(nvram_bufget(RTDEV_NVRAM, "BssidNum"));
	else
		ssid_num = 1;
	wlan_mode = atoi(nvram_bufget(RTDEV_NVRAM, "WirelessMode"));

	//set to nvram
	nvram_bufset(RTDEV_NVRAM, "BGProtection", bg_protection);
	nvram_bufset(RTDEV_NVRAM, "BasicRate", basic_rate);
	nvram_bufset(RTDEV_NVRAM, "BeaconPeriod", beacon);
	nvram_bufset(RTDEV_NVRAM, "DtimPeriod", dtim);
	nvram_bufset(RTDEV_NVRAM, "FragThreshold", fragment);
	nvram_bufset(RTDEV_NVRAM, "RTSThreshold", rts);
	nvram_bufset(RTDEV_NVRAM, "TxPower", tx_power);
	nvram_bufset(RTDEV_NVRAM, "TxPreamble", short_preamble);
	nvram_bufset(RTDEV_NVRAM, "ShortSlot", short_slot);
	nvram_bufset(RTDEV_NVRAM, "TxBurst", tx_burst);
	nvram_bufset(RTDEV_NVRAM, "PktAggregate", pkt_aggregate);
#ifdef CONFIG_RT3090_AUTO_PROVISION
	nvram_bufset(RTDEV_NVRAM, "AutoProvisionEn", auto_provision);
#endif
	nvram_bufset(RTDEV_NVRAM, "IEEE80211H", ieee_80211h);
	nvram_bufset(RTDEV_NVRAM, "WmmCapable", wmm_capable);
	nvram_bufset(RTDEV_NVRAM, "APSDCapable", apsd_capable);
	nvram_bufset(RTDEV_NVRAM, "DLSCapable", dls_capable);

	bzero(wmm_enable, sizeof(char)*8);
	for (i = 0; i < ssid_num; i++)
	{
		sprintf(wmm_enable+strlen(wmm_enable), "%d", atoi(wmm_capable));
		sprintf(wmm_enable+strlen(wmm_enable), "%c", ';');
	}
	wmm_enable[strlen(wmm_enable) - 1] = '\0';

	if (!strncmp(wmm_capable, "1", 2)) {
		if (wlan_mode < 5)
			nvram_bufset(RTDEV_NVRAM, "TxBurst", "0");
	}

	nvram_bufset(RTDEV_NVRAM, "CountryCode", countrycode);
	if (!strncmp(countrycode, "US", 3)) {
		nvram_bufset(RTDEV_NVRAM, "CountryRegion", "0");
		nvram_bufset(RTDEV_NVRAM, "CountryRegionABand", "0");
	}
	else if (!strncmp(countrycode, "JP", 3)) {
		nvram_bufset(RTDEV_NVRAM, "CountryRegion", "5");
		nvram_bufset(RTDEV_NVRAM, "CountryRegionABand", "6");
	}
	else if (!strncmp(countrycode, "FR", 3)) {
		nvram_bufset(RTDEV_NVRAM, "CountryRegion", "1");
		nvram_bufset(RTDEV_NVRAM, "CountryRegionABand", "2");
	}
	else if (!strncmp(countrycode, "TW", 3)) {
		nvram_bufset(RTDEV_NVRAM, "CountryRegion", "1");
		nvram_bufset(RTDEV_NVRAM, "CountryRegionABand", "3");
	}
	else if (!strncmp(countrycode, "IE", 3)) {
		nvram_bufset(RTDEV_NVRAM, "CountryRegion", "1");
		nvram_bufset(RTDEV_NVRAM, "CountryRegionABand", "2");
	}
	else if (!strncmp(countrycode, "HK", 3)) {
		nvram_bufset(RTDEV_NVRAM, "CountryRegion", "1");
		nvram_bufset(RTDEV_NVRAM, "CountryRegionABand", "0");
	}
	else {
		nvram_bufset(RTDEV_NVRAM, "CountryCode", "");
	}

	nvram_commit(RTDEV_NVRAM);
	initInternet();

	//debug print
	websHeader(wp);
	websWrite(wp, T("bg_protection: %s<br>\n"), bg_protection);
    websWrite(wp, T("basic_rate: %s<br>\n"), basic_rate);
    websWrite(wp, T("beacon: %s<br>\n"), beacon);
    websWrite(wp, T("dtim: %s<br>\n"), dtim);
    websWrite(wp, T("fragment: %s<br>\n"), fragment);
    websWrite(wp, T("rts: %s<br>\n"), rts);
    websWrite(wp, T("tx_power: %s<br>\n"), tx_power);
    websWrite(wp, T("short_preamble: %s<br>\n"), short_preamble);
    websWrite(wp, T("short_slot: %s<br>\n"), short_slot);
    websWrite(wp, T("tx_burst: %s<br>\n"), tx_burst);
    websWrite(wp, T("pkt_aggregate: %s<br>\n"), pkt_aggregate);
#ifdef CONFIG_RT3090_AUTO_PROVISION
    websWrite(wp, T("auto_provision: %s<br>\n"), auto_provision);
#endif
    websWrite(wp, T("ieee_80211h: %s<br>\n"), ieee_80211h);
    websWrite(wp, T("wmm_capable: %s<br>\n"), wmm_capable);
    websWrite(wp, T("apsd_capable: %s<br>\n"), apsd_capable);
    websWrite(wp, T("dls_capable: %s<br>\n"), dls_capable);
    websWrite(wp, T("countrycode: %s<br>\n"), countrycode);
	websFooter(wp);
	websDone(wp, 200);
}

void RaixWPSRestart(void);

/* goform/inicWmm */
static void inicWmm(webs_t wp, char_t *path, char_t *query)
{
	char_t	*ap_aifsn_all, *ap_cwmin_all, *ap_cwmax_all, *ap_txop_all,
			*ap_acm_all, *ap_ackpolicy_all,
			*sta_aifsn_all, *sta_cwmin_all, *sta_cwmax_all, *sta_txop_all,
			*sta_acm_all;

	ap_aifsn_all = websGetVar(wp, T("ap_aifsn_all"), T(""));
	ap_cwmin_all = websGetVar(wp, T("ap_cwmin_all"), T(""));
	ap_cwmax_all = websGetVar(wp, T("ap_cwmax_all"), T(""));
	ap_txop_all = websGetVar(wp, T("ap_txop_all"), T(""));
	ap_acm_all = websGetVar(wp, T("ap_acm_all"), T(""));
	ap_ackpolicy_all = websGetVar(wp, T("ap_ackpolicy_all"), T(""));
	sta_aifsn_all = websGetVar(wp, T("sta_aifsn_all"), T(""));
	sta_cwmin_all = websGetVar(wp, T("sta_cwmin_all"), T(""));
	sta_cwmax_all = websGetVar(wp, T("sta_cwmax_all"), T(""));
	sta_txop_all = websGetVar(wp, T("sta_txop_all"), T(""));
	sta_acm_all = websGetVar(wp, T("sta_acm_all"), T(""));

	if (0 != strlen(ap_aifsn_all))
		nvram_bufset(RTDEV_NVRAM, "APAifsn", ap_aifsn_all);
	if (0 != strlen(ap_cwmin_all))
		nvram_bufset(RTDEV_NVRAM, "APCwmin", ap_cwmin_all);
	if (0 != strlen(ap_cwmax_all))
		nvram_bufset(RTDEV_NVRAM, "APCwmax", ap_cwmax_all);
	if (0 != strlen(ap_txop_all))
		nvram_bufset(RTDEV_NVRAM, "APTxop", ap_txop_all);
	if (0 != strlen(ap_acm_all))
		nvram_bufset(RTDEV_NVRAM, "APACM", ap_acm_all);
	if (0 != strlen(ap_ackpolicy_all))
		nvram_bufset(RTDEV_NVRAM, "AckPolicy", ap_ackpolicy_all);
	if (0 != strlen(sta_aifsn_all))
		nvram_bufset(RTDEV_NVRAM, "BSSAifsn", sta_aifsn_all);
	if (0 != strlen(sta_cwmin_all))
		nvram_bufset(RTDEV_NVRAM, "BSSCwmin", sta_cwmin_all);
	if (0 != strlen(sta_cwmax_all))
		nvram_bufset(RTDEV_NVRAM, "BSSCwmax", sta_cwmax_all);
	if (0 != strlen(sta_txop_all))
		nvram_bufset(RTDEV_NVRAM, "BSSTxop", sta_txop_all);
	if (0 != strlen(sta_acm_all))
		nvram_bufset(RTDEV_NVRAM, "BSSACM", sta_acm_all);

	nvram_commit(RTDEV_NVRAM);

	doSystem("ralink_init make_wireless_config rtdev");
	if (0 == getIfLive("rai0")) {
		doKillPid("/var/run/RaCfg.pid");
		doSystem("brctl delif br0 rai0");
		doSystem("ifconfig rai0 down");
		Sleep(1);
		doSystem("ifconfig rai0 up");
/*
#ifndef INICv2_SUPPORT
		doSystem("RaAP&");
#endif
*/
		Sleep(3);
		doSystem("brctl addif br0 rai0");
	}
	//after rai0 down&up we must restore WPS status
	RaixWPSRestart();

	websHeader(wp);
	websWrite(wp, T("ap_aifsn_all: %s<br>\n"), ap_aifsn_all);
	websWrite(wp, T("ap_cwmin_all: %s<br>\n"), ap_cwmin_all);
	websWrite(wp, T("ap_cwmax_all: %s<br>\n"), ap_cwmax_all);
	websWrite(wp, T("ap_txop_all: %s<br>\n"), ap_txop_all);
	websWrite(wp, T("ap_acm_all: %s<br>\n"), ap_acm_all);
	websWrite(wp, T("ap_ackpolicy_all: %s<br>\n"), ap_ackpolicy_all);
	websWrite(wp, T("sta_aifsn_all: %s<br>\n"), sta_aifsn_all);
	websWrite(wp, T("sta_cwmin_all: %s<br>\n"), sta_cwmin_all);
	websWrite(wp, T("sta_cwmax_all: %s<br>\n"), sta_cwmax_all);
	websWrite(wp, T("sta_txop_all: %s<br>\n"), sta_txop_all);
	websWrite(wp, T("sta_acm_all: %s<br>\n"), sta_acm_all);
	websFooter(wp);
	websDone(wp, 200);
}

static void inicwGetSecurity(webs_t wp, char_t *path, char_t *query)
{
	getSecurity(RTDEV_NVRAM, wp, path, query);		//in wireless.c
}

static void INICSecurity(webs_t wp, char_t *path, char_t *query)
{
	Security(RTDEV_NVRAM, wp, path, query);		// in wireless.c
}

static void INICDeleteAccessPolicyList(webs_t wp, char_t *path, char_t *query)
{
	DeleteAccessPolicyList(RTDEV_NVRAM, wp, path, query);
}

static int get2DLSBuilt(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef CONFIG_RT3090_AP_DLS
	return websWrite(wp, T("1"));
#else
	return websWrite(wp, T("0"));
#endif
}

static int get2AutoProvisionBuilt(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef CONFIG_RT3090_AUTO_PROVISION
	return websWrite(wp, T("1"));
#else
	return websWrite(wp, T("0"));
#endif
}
