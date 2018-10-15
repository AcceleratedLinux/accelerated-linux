/* vi: set sw=4 ts=4 sts=4: */
/*
 *  station.c -- Station Mode
 *
 *  Copyright (c) Ralink Technology Corporation All Rights Reserved.
 *
 *  $Id: station.c,v 1.67.2.2 2010-03-04 09:13:33 chhung Exp $
 */

#include	<sys/ioctl.h>
#include	<arpa/inet.h>
#include	<asm/types.h>
#include	<linux/if.h>
#include	<linux/wireless.h>
#include	<dirent.h>
#include	"webs.h"
#include	"oid.h"
#include	"stapriv.h"
#include	"nvram.h"
#include	"utils.h"
#include	"internet.h"

#define RA0 nvram_bufget(RT2860_NVRAM, "ra0")

#define Ndis802_11AuthMode8021x 20

static int	getWPASupplicantBuilt(int eid, webs_t wp, int argc, char_t **argv);
static int getCACLCertList(int eid, webs_t wp, int argc, char_t **argv);
static int getKeyCertList(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaAdhocChannel(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaAllProfileName(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaBSSIDList(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaConnectedBSSID(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaConnectionSSID(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaDbm(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaDLSList(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaDriverVer(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaExtraInfo(int eid, webs_t wp, int argc, char_t **argv);
static int	getLinkingMode(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaHT(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaLinkChannel(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaLinkQuality(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaLinkRxRate(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaLinkStatus(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaLinkTxRate(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaMacAddr(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaNewProfileName(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaNoiseLevel(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaProfile(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaProfileData(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaRadioStatus(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaRxThroughput(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaTxThroughput(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaSignalStrength(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaSignalStrength_1(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaSignalStrength_2(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaSNR(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaStatsRxCRCErr(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaStatsRxDup(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaStatsRxOk(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaStatsRxNoBuf(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaStatsTx(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaSuppAMode(int eid, webs_t wp, int argc, char_t **argv);
static int	getStaWirelessMode(int eid, webs_t wp, int argc, char_t **argv);

static void addStaProfile(webs_t wp, char_t *path, char_t *query);
static void editStaProfile(webs_t wp, char_t *path, char_t *query);
static void resetStaCounters(webs_t wp, char_t *path, char_t *query);
static void setSta11nCfg(webs_t wp, char_t *path, char_t *query);
static void setStaAdvance(webs_t wp, char_t *path, char_t *query);
static void setStaConnect(webs_t wp, char_t *path, char_t *query);
static void setStaDbm(webs_t wp, char_t *path, char_t *query);
static void setStaProfile(webs_t wp, char_t *path, char_t *query);
static void setStaOrgAdd(webs_t wp, char_t *path, char_t *query);
static void setStaOrgDel(webs_t wp, char_t *path, char_t *query);
static void setStaQoS(webs_t wp, char_t *path, char_t *query);

void formDefineStation(void)
{
	websAspDefine(T("getWPASupplicantBuilt"), getWPASupplicantBuilt);
	websAspDefine(T("getCACLCertList"), getCACLCertList);
	websAspDefine(T("getKeyCertList"), getKeyCertList);
	websAspDefine(T("getStaAdhocChannel"), getStaAdhocChannel);
	websAspDefine(T("getStaAllProfileName"), getStaAllProfileName);
	websAspDefine(T("getStaBSSIDList"), getStaBSSIDList);
	websAspDefine(T("getStaConnectedBSSID"), getStaConnectedBSSID);
	websAspDefine(T("getStaConnectionSSID"), getStaConnectionSSID);
	websAspDefine(T("getStaDbm"), getStaDbm);
	websAspDefine(T("getStaDLSList"), getStaDLSList);
	websAspDefine(T("getStaDriverVer"), getStaDriverVer);
	websAspDefine(T("getStaExtraInfo"), getStaExtraInfo);
	websAspDefine(T("getLinkingMode"), getLinkingMode);
	websAspDefine(T("getStaHT"), getStaHT);
	websAspDefine(T("getStaLinkChannel"), getStaLinkChannel);
	websAspDefine(T("getStaLinkQuality"), getStaLinkQuality);
	websAspDefine(T("getStaLinkRxRate"), getStaLinkRxRate);
	websAspDefine(T("getStaLinkStatus"), getStaLinkStatus);
	websAspDefine(T("getStaLinkTxRate"), getStaLinkTxRate);
	websAspDefine(T("getStaMacAddr"), getStaMacAddr);
	websAspDefine(T("getStaNewProfileName"), getStaNewProfileName);
	websAspDefine(T("getStaNoiseLevel"), getStaNoiseLevel);
	websAspDefine(T("getStaProfile"), getStaProfile);
	websAspDefine(T("getStaProfileData"), getStaProfileData);
	websAspDefine(T("getStaRadioStatus"), getStaRadioStatus);
	websAspDefine(T("getStaRxThroughput"), getStaRxThroughput);
	websAspDefine(T("getStaTxThroughput"), getStaTxThroughput);
	websAspDefine(T("getStaSignalStrength"), getStaSignalStrength);
	websAspDefine(T("getStaSignalStrength_1"), getStaSignalStrength_1);
	websAspDefine(T("getStaSignalStrength_2"), getStaSignalStrength_2);
	websAspDefine(T("getStaSNR"), getStaSNR);
	websAspDefine(T("getStaStatsRxCRCErr"), getStaStatsRxCRCErr);
	websAspDefine(T("getStaStatsRxDup"), getStaStatsRxDup);
	websAspDefine(T("getStaStatsRxOk"), getStaStatsRxOk);
	websAspDefine(T("getStaStatsRxNoBuf"), getStaStatsRxNoBuf);
	websAspDefine(T("getStaStatsTx"), getStaStatsTx);
	websAspDefine(T("getStaSuppAMode"), getStaSuppAMode);
	websAspDefine(T("getStaWirelessMode"), getStaWirelessMode);

	websFormDefine(T("addStaProfile"), addStaProfile);
	websFormDefine(T("editStaProfile"), editStaProfile);
	websFormDefine(T("resetStaCounters"), resetStaCounters);
	websFormDefine(T("setSta11nCfg"), setSta11nCfg);
	websFormDefine(T("setStaAdvance"), setStaAdvance);
	websFormDefine(T("setStaConnect"), setStaConnect);
	websFormDefine(T("setStaDbm"), setStaDbm);
	websFormDefine(T("setStaProfile"), setStaProfile);
	websFormDefine(T("setStaOrgAdd"), setStaOrgAdd);
	websFormDefine(T("setStaOrgDel"), setStaOrgDel);
	websFormDefine(T("setStaQoS"), setStaQoS);
}


PRT_PROFILE_SETTING selectedProfileSetting = NULL, headerProfileSetting = NULL, currentProfileSetting = NULL;

unsigned char   Active_flag=0, nConfig_flag=0;
unsigned int    m_nSigQua[3] = {0,0,0};
unsigned long   m_lTxCount = 0;
unsigned long   m_lRxCount = 0;
unsigned long   m_lChannelQuality = 0;
char    G_bRadio = 1; //TRUE
char    G_bdBm_ischeck = 0; //false
char    G_staProfileNum = 0;
NDIS_802_11_SSID        G_SSID;
unsigned char			G_Bssid[6];
int        G_ConnectStatus = NdisMediaStateDisconnected;
unsigned char WpaSupplicant_flag = FALSE;

PAIR_CHANNEL_FREQ_ENTRY ChannelFreqTable[] = {
	//channel Frequency
	{1,     2412000},
	{2,     2417000},
	{3,     2422000},
	{4,     2427000},
	{5,     2432000},
	{6,     2437000},
	{7,     2442000},
	{8,     2447000},
	{9,     2452000},
	{10,    2457000},
	{11,    2462000},
	{12,    2467000},
	{13,    2472000},
	{14,    2484000},
	{34,    5170000},
	{36,    5180000},
	{38,    5190000},
	{40,    5200000},
	{42,    5210000},
	{44,    5220000},
	{46,    5230000},
	{48,    5240000},
	{52,    5260000},
	{56,    5280000},
	{60,    5300000},
	{64,    5320000},
	{100,   5500000},
	{104,   5520000},
	{108,   5540000},
	{112,   5560000},
	{116,   5580000},
	{120,   5600000},
	{124,   5620000},
	{128,   5640000},
	{132,   5660000},
	{136,   5680000},
	{140,   5700000},
	{149,   5745000},
	{153,   5765000},
	{157,   5785000},
	{161,   5805000},
	{165,	5825000},
	{167,	5835000},
	{169,	5845000},
	{171,	5855000},
	{173,	5865000},
	{184,	4920000},
	{188,	4940000},
	{192,	4960000},
	{196,	4980000},
	{208,	5040000},	/* Japan, means J08 */
	{212,	5060000},	/* Japan, means J12 */
	{216,	5080000},	/* Japan, means J16 */
};
int G_nChanFreqCount = sizeof (ChannelFreqTable) / sizeof(PAIR_CHANNEL_FREQ_ENTRY);

void freeHeaderProfileSettings(void)
{
	PRT_PROFILE_SETTING list = headerProfileSetting;
	PRT_PROFILE_SETTING next;
	while(list){
		next = list->Next;
		free(list);
		list = next;
		next = list->Next;
	}
}

int OidQueryInformation(unsigned long OidQueryCode, int socket_id, char *DeviceName, void *ptr, unsigned long PtrLength)
{
	struct iwreq wrq;

	strcpy(wrq.ifr_name, DeviceName);
	wrq.u.data.length = PtrLength;
	wrq.u.data.pointer = (caddr_t) ptr;
	wrq.u.data.flags = OidQueryCode;

	return (ioctl(socket_id, RT_PRIV_IOCTL, &wrq));
}

int OidSetInformation(unsigned long OidQueryCode, int socket_id, char *DeviceName, void *ptr, unsigned long PtrLength)
{
	struct iwreq wrq;

	strcpy(wrq.ifr_name, DeviceName);
	wrq.u.data.length = PtrLength;
	wrq.u.data.pointer = (caddr_t) ptr;
	wrq.u.data.flags = OidQueryCode | OID_GET_SET_TOGGLE;

	return (ioctl(socket_id, RT_PRIV_IOCTL, &wrq));
}

void ConverterStringToDisplay(char *str)
{
    int  len, i;
    char buffer[193];
    char *pOut;

    memset(buffer,0,193);
    len = strlen(str);
    pOut = &buffer[0];

    for (i = 0; i < len; i++) {
		switch (str[i]) {
			case '&':
				strcpy (pOut, "&amp;");
				pOut += 5;
				break;

			case '<': 
				strcpy (pOut, "&lt;");
				pOut += 4;
				break;

			case '>': 
				strcpy (pOut, "&gt;");
				pOut += 4;
				break;

			case '"':
				strcpy (pOut, "&quot;");
				pOut += 6;
				break;

				//case ' ':
				//strcpy (pOut, "&nbsp;");
				//pOut += 6;
				//break;

			default:
				if ((str[i]>=0) && (str[i]<=31)) {
					//Device Control Characters
					sprintf(pOut, "&#%02d;", str[i]);
					pOut += 5;
				} else if ((str[i]==39) || (str[i]==47) || (str[i]==59) || (str[i]==92)) {
					// ' / ; (backslash)
					sprintf(pOut, "&#%02d;", str[i]);
					pOut += 5;
				} else if (str[i]>=127) {
					//Device Control Characters
					sprintf(pOut, "&#%03d;", str[i]);
					pOut += 6;
				} else {
					*pOut = str[i];
					pOut++;
				}
				break;
		}
    }
    *pOut = '\0';
    strcpy(str, buffer);
}

unsigned int ConvertRssiToSignalQuality(long RSSI)
{
    unsigned int signal_quality;
    if (RSSI >= -50)
        signal_quality = 100;
    else if (RSSI >= -80)    // between -50 ~ -80dbm
        signal_quality = (unsigned int)(24 + (RSSI + 80) * 2.6);
    else if (RSSI >= -90)   // between -80 ~ -90dbm
        signal_quality = (unsigned int)((RSSI + 90) * 2.6);
    else    // < -84 dbm
        signal_quality = 0;

    return signal_quality;
}

/*
 * description: write station Adhoc Channel (a << 8 | bg)
 */
static int getStaAdhocChannel(int eid, webs_t wp, int argc, char_t **argv)
{
	const char *p = NULL;
	unsigned int country_region_bg = 0, country_region_a = 0;
	long country_region = 0;

	p = nvram_bufget(RT2860_NVRAM, "CountryRegion");
	if (p)
		country_region_bg = atoi(p);
	p = nvram_bufget(RT2860_NVRAM, "CountryRegionABand");
	if (p)
		country_region_a = atoi(p);

	country_region = country_region_bg | ( country_region_a << 8);
	return websWrite(wp, "%ld", country_region);
}

/*
 * description: write station all profile names
 */
static int getStaAllProfileName(int eid, webs_t wp, int argc, char_t **argv)
{
	char tmp[1024];
	memset(tmp, 0x00, sizeof(tmp));
	if (headerProfileSetting != NULL) {
		currentProfileSetting = headerProfileSetting;
		snprintf(tmp, 1024, "%s", currentProfileSetting->Profile);
		do {
			currentProfileSetting = currentProfileSetting->Next;
			if (currentProfileSetting != NULL)
				snprintf(tmp, 1024, "%s;%s", tmp, currentProfileSetting->Profile);
		} while (currentProfileSetting != NULL );
		return websWrite(wp, tmp);
	}
	else
		return websWrite(wp, " ");
}

/*
 * description: write the BSSID list (site survey)
 */
static int getStaBSSIDList(int eid, webs_t wp, int argc, char_t **argv)
{
	int                         s, ret, retry=1;
	unsigned short                lBufLen = 4096, we_version=16; // 64K
	PNDIS_802_11_BSSID_LIST_EX	pBssidList;
	PNDIS_WLAN_BSSID_EX  		pBssid;
	unsigned int                ConnectStatus = NdisMediaStateDisconnected;
	unsigned char               BssidQuery[6];
	NDIS_802_11_SSID            SSIDQuery;
	int							QueryCount=0, EAGAIN_Count=0;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	pBssidList = (PNDIS_802_11_BSSID_LIST_EX) malloc(65536*2);  //64k
	memset(pBssidList, 0x00, sizeof(char)*65536*2);

	//step 1
	while(ConnectStatus != NdisMediaStateConnected && QueryCount < 3) {
		if (OidQueryInformation(OID_GEN_MEDIA_CONNECT_STATUS, s, RA0, &ConnectStatus, sizeof(ConnectStatus)) < 0) {
			websError(wp, 500, "Query OID_GEN_MEDIA_CONNECT_STATUS failed!");
			free(pBssidList); close(s);
			return -1;
		}
		sleep(2);
		QueryCount++;
	}

	//step 2
	if (OidQueryInformation(RT_OID_802_11_RADIO, s, RA0, &G_bRadio, sizeof(G_bRadio)) < 0) {
		websError(wp, 500, "Query RT_OID_802_11_RADIO failed!");
		free(pBssidList); close(s);
		return -1;
	}

	if (ConnectStatus == NdisMediaStateConnected && G_bRadio) {
		memset(&BssidQuery, 0x00, sizeof(BssidQuery));
		OidQueryInformation(OID_802_11_BSSID, s, RA0, &BssidQuery, sizeof(BssidQuery));
		memset(&SSIDQuery, 0x00, sizeof(SSIDQuery));
		OidQueryInformation(OID_802_11_SSID, s, RA0, &SSIDQuery, sizeof(SSIDQuery));
	}

	//step 3
	if (OidSetInformation(OID_802_11_BSSID_LIST_SCAN, s, RA0, 0, 0) < 0) {
		websError(wp, 500, "Set OID_802_11_BSSID_LIST_SCAN failed!");
		free(pBssidList); close(s);
		return -1;
	}
	// wait a few seconds to get all AP.
	Sleep(2);

	//step 4
	ret = OidQueryInformation(RT_OID_WE_VERSION_COMPILED, s, RA0, &we_version, sizeof(we_version));
	if (ret< 0)
	{
		websError(wp, 500, "Query RT_OID_WE_VERSION_COMPILED error! return=%d", ret);
		close(s);
		return -1;
	}
	if(we_version >= 17)
		lBufLen=8192;
	else
		lBufLen=4096;

	ret = -1;
	retry = 1;
	while (ret < 0) {
		memset(pBssidList, 0x00, sizeof(char)*65536*2);
		ret = OidQueryInformation(OID_802_11_BSSID_LIST, s, RA0, pBssidList, lBufLen);
#if 1
		if (errno == EAGAIN) {
			sleep(1);
			// fprintf(stderr, "errno == EAGAIN\n");
			EAGAIN_Count++;
			if(EAGAIN_Count>25) {
				websError(wp, 500, "Query OID_802_11_BSSID_LIST error! errno == EAGAIN");
				free(pBssidList);
				close(s);
				return -1;
			}
			else
				continue;
		} else if (errno == E2BIG) {
			//fprintf(stderr, "errno == E2BIG\n");
			lBufLen = lBufLen + 4096*retry;
			if (lBufLen < 65536) {
				retry++;
				// fprintf(stderr,"lBufLen=%d\n",lBufLen);
				continue;
			} else {
				websError(wp, 500, "Query OID_802_11_BSSID_LIST error! E2BIG");
				free(pBssidList);
				close(s);
				return -1;
			}
		}
		else if( ret != 0 ) {
			websError(wp, 500, "Query OID_802_11_BSSID_LIST error! return=%d", ret);
			free(pBssidList);
			close(s);
			return -1;
		}
#else
		switch(errno) {
			case EAGAIN:
					do {
						sleep(1);
						ret = OidQueryInformation(OID_802_11_BSSID_LIST, s, RA0, pBssidList, lBufLen);
						retry++;
					} while((errno == EAGAIN) && (ret < 0) && (retry < 5));
					break;

			case E2BIG:
					do {
						lBufLen = lBufLen + 4096*retry;
						free(pBssidList);
						pBssidList = (PNDIS_802_11_BSSID_LIST_EX) malloc(lBufLen);
						ret = OidQueryInformation(OID_802_11_BSSID_LIST, s, RA0, pBssidList, lBufLen);
						t++;
					} while((errno == E2BIG) && (ret <0) && (retry < 10));
			default:
					if (ret < 0) {
						websError(wp, 500, "Query OID_802_11_BSSID_LIST error! return=%d", ret);
						close(s);
						return;
					}
					else
						break;
		}
#endif
	}
	if ( pBssidList->NumberOfItems == 0) {
		websError(wp, 500, "Bssid List number is 0!\n");
		close(s);
		return -1;
	} else {
		unsigned char tmpRadio[188], tmpBSSIDII[16], tmpBSSID[28], tmpSSID[64+NDIS_802_11_LENGTH_SSID], tmpRSSI[16], tmpChannel[16], tmpAuth[32], tmpEncry[20], tmpNetworkType[24], tmpImg[40];
		unsigned char tmpSSIDII[NDIS_802_11_LENGTH_SSID+1];
		/*
		unsigned char tmpRadio[188], tmpBSSIDII[16], tmpBSSID[28], tmpSSID[64+NDIS_802_11_LENGTH_SSID*4], tmpRSSI[60], tmpChannel[16], tmpAuth[32], tmpEncry[52], tmpNetworkType[24], tmpImg[40];
		unsigned char tmpSSIDII[(NDIS_802_11_LENGTH_SSID+1)*4];
		*/
		int i=0, j=0;
		unsigned int nSigQua;
		int nChannel = 1;
		unsigned char radiocheck[8];

		pBssid = (PNDIS_WLAN_BSSID_EX) pBssidList->Bssid;
		G_ConnectStatus = NdisMediaStateDisconnected;
		for (i = 0; i < pBssidList->NumberOfItems; i++)
		{
			memset(radiocheck, 0x00, sizeof(radiocheck));
			memset(tmpRadio, 0x00, sizeof(tmpRadio));
			memset(tmpBSSID, 0x00, sizeof(tmpBSSID));
			memset(tmpRSSI, 0x00, sizeof(tmpRSSI));
			memset(tmpSSID, 0x00, sizeof(tmpSSID));
			memset(tmpChannel, 0x00, sizeof(tmpChannel));
			memset(tmpAuth, 0x00, sizeof(tmpAuth));
			memset(tmpEncry, 0x00, sizeof(tmpEncry));
			memset(tmpNetworkType, 0x00, sizeof(tmpNetworkType));
			memset(tmpBSSIDII, 0x00, sizeof(tmpBSSIDII));
			memset(tmpImg, 0x00, sizeof(tmpImg));
			memset(tmpSSIDII, 0x00, sizeof(tmpSSIDII));

			// compare BSSID with connected bssid
			if (memcmp(BssidQuery, pBssid->MacAddress, 6) == 0)
				sprintf((char *)tmpImg, "<img src=\"/graphics/handshake.gif\"> ");
			else
				sprintf((char *)tmpImg, " ");

			if (strcmp((char *)pBssid->Ssid.Ssid, "") == 0)
				sprintf((char *)tmpSSID, "<td>%s%s</td>", tmpImg, "&nbsp;");
			else {
				int i = 0;
				do {
					if (pBssid->Ssid.Ssid[i] < 32 || pBssid->Ssid.Ssid[i] > 126 || pBssid->Ssid.Ssid[i] == 13) // 13 is string end of Dos
					{
						strcpy((char *)pBssid->Ssid.Ssid, "&nbsp;");
						break;
					}
					i++;
				} while(i < pBssid->Ssid.SsidLength-1);
				sprintf((char *)tmpSSID, "<td>%s%s</td>", tmpImg, pBssid->Ssid.Ssid);
			}

			sprintf((char *)tmpBSSID, "<td>%02X-%02X-%02X-%02X-%02X-%02X</td>",
				pBssid->MacAddress[0], pBssid->MacAddress[1], pBssid->MacAddress[2],
				pBssid->MacAddress[3], pBssid->MacAddress[4], pBssid->MacAddress[5]);

			sprintf((char *)tmpBSSIDII, "%02X%02X%02X%02X%02X%02X",
				pBssid->MacAddress[0], pBssid->MacAddress[1], pBssid->MacAddress[2],
				pBssid->MacAddress[3], pBssid->MacAddress[4], pBssid->MacAddress[5]);

			nSigQua = ConvertRssiToSignalQuality(pBssid->Rssi);
       		sprintf((char *)tmpRSSI,"<td>%d%%</td>", nSigQua);

			nChannel = -1;	
			for(j = 0; j < G_nChanFreqCount; j++)
			{
				//fprintf(stderr, "pBssid->Configuration.DSConfig = %d, ChannelFreqTable[j].lFreq=%d\n ", pBssid->Configuration.DSConfig, ChannelFreqTable[j].lFreq);
				if (pBssid->Configuration.DSConfig == ChannelFreqTable[j].lFreq) {
					nChannel = ChannelFreqTable[j].lChannel;
					break;
				}
			}

			if (nChannel == -1)
				continue;

			sprintf((char *)tmpChannel, "<td>%u</td>", nChannel);
			if (pBssid->InfrastructureMode == Ndis802_11Infrastructure)
				sprintf((char *)tmpNetworkType, "<td>%s</td>", "Infrastructure");
			else
				sprintf((char *)tmpNetworkType, "<td>%s</td>", "Ad Hoc");
			
			// work with NDIS_WLAN_BSSID_EX
			unsigned char bTKIP = FALSE;
			unsigned char bAESWRAP = FALSE;
			unsigned char bAESCCMP = FALSE;
			unsigned char bWPA = FALSE;
			unsigned char bWPAPSK = FALSE;
			unsigned char bWPANONE = FALSE;
			unsigned char bWPA2 = FALSE;
			unsigned char bWPA2PSK = FALSE;
			unsigned char bWPA2NONE = FALSE;
			unsigned char bCCKM = FALSE; // CCKM for Cisco, add by candy 2006.11.24


			if ((pBssid->Length > sizeof(NDIS_WLAN_BSSID)) && (pBssid->IELength > sizeof(NDIS_802_11_FIXED_IEs)))
			{
				unsigned int lIELoc = 0;
				PNDIS_802_11_FIXED_IEs pFixIE = (PNDIS_802_11_FIXED_IEs)pBssid->IEs;
				PNDIS_802_11_VARIABLE_IEs pVarIE = (PNDIS_802_11_VARIABLE_IEs)((char*)pFixIE + sizeof(NDIS_802_11_FIXED_IEs));
				lIELoc += sizeof(NDIS_802_11_FIXED_IEs);

				while (pBssid->IELength > (lIELoc + sizeof(NDIS_802_11_VARIABLE_IEs)))
				{
					if ((pVarIE->ElementID == 221) && (pVarIE->Length >= 16))
					{
						unsigned int* pOUI = (unsigned int*)((char*)pVarIE->data);
						if (*pOUI != WPA_OUI_TYPE)
						{
							lIELoc += pVarIE->Length;
							lIELoc += 2;
							pVarIE = (PNDIS_802_11_VARIABLE_IEs)((char*)pVarIE + pVarIE->Length + 2);

							if(pVarIE->Length <= 0)
								break;

							continue;
						}

						unsigned int* plGroupKey; 
						unsigned short* pdPairKeyCount;
						unsigned int* plPairwiseKey=NULL;
						unsigned int* plAuthenKey=NULL;
						unsigned short* pdAuthenKeyCount;
						plGroupKey = (unsigned int*)((char*)pVarIE + 8);
				
						unsigned int lGroupKey = *plGroupKey & 0x00ffffff;
						//fprintf(stderr, "lGroupKey=%d\n", lGroupKey);
						if (lGroupKey == WPA_OUI) {
							lGroupKey = (*plGroupKey & 0xff000000) >> 0x18;
							if (lGroupKey == 2)
								bTKIP = TRUE;
							else if (lGroupKey == 3)
								bAESWRAP = TRUE;
							else if (lGroupKey == 4)
								bAESCCMP = TRUE;
						}
						else
						{
							lIELoc += pVarIE->Length;
							lIELoc += 2;
							pVarIE = (PNDIS_802_11_VARIABLE_IEs)((char*)pVarIE + pVarIE->Length + 2);

							if(pVarIE->Length <= 0)
								break;
							
							continue;
						}
				
						pdPairKeyCount = (unsigned short*)((char*)plGroupKey + 4);
						plPairwiseKey = (unsigned int*) ((char*)pdPairKeyCount + 2);
						unsigned short k = 0;
						for (k = 0; k < *pdPairKeyCount; k++) {
							unsigned int lPairKey = *plPairwiseKey & 0x00ffffff;
							if (lPairKey == WPA_OUI )//|| (lPairKey & 0xffffff00) == WPA_OUI_1)
							{
								lPairKey = (*plPairwiseKey & 0xff000000) >> 0x18;
								if (lPairKey == 2)
									bTKIP = TRUE;
								else if (lPairKey == 3)
									bAESWRAP = TRUE;
								else if (lPairKey == 4)
									bAESCCMP = TRUE;
							}
							else
								break;
							
							plPairwiseKey++;
						}
				
						pdAuthenKeyCount = (unsigned short*)((char*)pdPairKeyCount + 2 + 4 * (*pdPairKeyCount));
						plAuthenKey = (unsigned int*)((char*)pdAuthenKeyCount + 2);

						for(k = 0; k < *pdAuthenKeyCount; k++)
						{
							unsigned int lAuthenKey = *plAuthenKey & 0x00ffffff;
							if (lAuthenKey == CISCO_OUI) {
								bCCKM = TRUE; // CCKM for Cisco
							}
							else if (lAuthenKey == WPA_OUI) {
								lAuthenKey = (*plAuthenKey & 0xff000000) >> 0x18;

								if (lAuthenKey == 1)
									bWPA = TRUE;
								else if (lAuthenKey == 0 || lAuthenKey == 2) {
									if (pBssid->InfrastructureMode)
										bWPAPSK = TRUE;
									else
										bWPANONE = TRUE;
								}
							}					
							plAuthenKey++;
						}
					//break;
					}
					else if (pVarIE->ElementID == 48 && pVarIE->Length >= 12)
					{
						unsigned int* plGroupKey; 
						unsigned int* plPairwiseKey; 
						unsigned short* pdPairKeyCount;
						unsigned int* plAuthenKey; 
						unsigned short* pdAuthenKeyCount;
						plGroupKey = (unsigned int*)((char*)pVarIE + 4);

						unsigned int lGroupKey = *plGroupKey & 0x00ffffff;
						if (lGroupKey == WPA2_OUI) {
							lGroupKey = (*plGroupKey & 0xff000000) >> 0x18;
							if (lGroupKey == 2)
								bTKIP = TRUE;
							else if (lGroupKey == 3)
								bAESWRAP = TRUE;
							else if (lGroupKey == 4)
								bAESCCMP = TRUE;
						}
						else
						{
							lIELoc += pVarIE->Length;
							lIELoc += 2;
							pVarIE = (PNDIS_802_11_VARIABLE_IEs)((char*)pVarIE + pVarIE->Length + 2);

							if(pVarIE->Length <= 0)
								break;
							
							continue;
						}

						pdPairKeyCount = (unsigned short*)((char*)plGroupKey + 4);
						plPairwiseKey = (unsigned int*)((char*)pdPairKeyCount + 2);
						unsigned short k = 0;

						for (k = 0; k < *pdPairKeyCount; k++)
						{
							unsigned int lPairKey = *plPairwiseKey & 0x00ffffff;
							if (lPairKey == WPA2_OUI) {
								lPairKey = (*plPairwiseKey & 0xff000000) >> 0x18;
								if (lPairKey == 2)
									bTKIP = TRUE;
								else if (lPairKey == 3)
									bAESWRAP = TRUE;
								else if (lPairKey == 4)
									bAESCCMP = TRUE;
							}
							else
								break;
							plPairwiseKey++;
						}
			
						pdAuthenKeyCount = (unsigned short*)((char*)pdPairKeyCount + 2 + 4 * *pdPairKeyCount);
						plAuthenKey = (unsigned int*)((char*)pdAuthenKeyCount + 2);
						for (k = 0; k < *pdAuthenKeyCount; k++)
						{
							unsigned int lAuthenKey = *plAuthenKey & 0x00ffffff;
							if (lAuthenKey == CISCO_OUI) {
								bCCKM = TRUE; // CCKM for Cisco
							}
	                        else if (lAuthenKey == WPA2_OUI) {
								lAuthenKey = (*plAuthenKey & 0xff000000) >> 0x18;
								if (lAuthenKey == 1)
									bWPA2 = TRUE;
								else if (lAuthenKey == 0 || lAuthenKey == 2) {
									if (pBssid->InfrastructureMode)
										bWPA2PSK = TRUE;
									else
										bWPA2NONE = TRUE;
								}
							}					
							plAuthenKey++;
						}
					}
					lIELoc += pVarIE->Length;
					lIELoc += 2;
					pVarIE = (PNDIS_802_11_VARIABLE_IEs)((char*)pVarIE + pVarIE->Length + 2);

					if (pVarIE->Length <= 0)
						break;
				}
			}
			
			char strAuth[32], strEncry[32];
			memset( strAuth, 0x00, sizeof(strAuth) );
			memset( strEncry, 0x00, sizeof(strEncry) );
			if (bCCKM)
				strcpy(strAuth, "CCKM; ");
			if (bWPA)
				strcpy(strAuth, "WPA; ");
			if (bWPAPSK)
				strcat(strAuth, "WPA-PSK; ");
			if (bWPANONE)
				strcat(strAuth, "WPA-NONE; ");
			if (bWPA2)
				strcat(strAuth, "WPA2; ");
			if (bWPA2PSK)
				strcat(strAuth, "WPA2-PSK; ");
			if (bWPA2NONE)
				strcat(strAuth, "WPA2-NONE; ");
			
			if (strlen(strAuth) > 0) {
				strncpy((char *)tmpAuth, strAuth, strlen(strAuth) - 2);
				strcpy(strAuth, (char *)tmpAuth);
			}
			else {
				strcpy((char *)strAuth, "Unknown");
			}

			if (bTKIP)
				strcpy(strEncry, "TKIP; ");
			if (bAESWRAP || bAESCCMP)
				strcat(strEncry, "AES; ");

			if (strlen(strEncry) > 0) {
				strncpy((char *)tmpEncry, strEncry, strlen(strEncry) - 2);
				strcpy(strEncry, (char *)tmpEncry);
			}
			else {
				if (pBssid->Privacy)  // privacy value is on/of
					strcpy(strEncry, "WEP");
				else {
					strcpy(strEncry, "Not Use");
					strcpy(strAuth, "OPEN");
				}
			}

			sprintf((char *)tmpAuth, "<td>%s</td>", strAuth);
			sprintf((char *)tmpEncry, "<td>%s</td>", strEncry);

			strcpy((char *)tmpSSIDII, pBssid->Ssid.Ssid);
			if (strlen(G_SSID.Ssid)>0 && strcmp(pBssid->Ssid.Ssid, G_SSID.Ssid) == 0)
				strcpy(radiocheck, "checked");
			else
				strcpy(radiocheck, "");

			sprintf((char *)tmpRadio, "<td><input type=radio name=selectedSSID %s onClick=\"selectedSSIDChange('%s','%s',%d,%d,'%s','%s')\"></td>", radiocheck, tmpSSIDII, tmpBSSIDII, pBssid->InfrastructureMode, nChannel, strEncry, strAuth);
			websWrite(wp, "<tr> %s %s %s %s %s %s %s %s </tr>\n", tmpRadio, tmpSSID, tmpBSSID, tmpRSSI, tmpChannel, tmpEncry, tmpAuth, tmpNetworkType);
			pBssid = (PNDIS_WLAN_BSSID_EX)((char *)pBssid + pBssid->Length);
		}
	}
	free(pBssidList);
	close(s);
	return 0;
}

/*
 * description: write the BSSID that station connected to
 */
static int getStaConnectedBSSID(int eid, webs_t wp, int argc, char_t **argv)
{
	unsigned char BssidQuery[6];
	int s;

	s = socket(AF_INET, SOCK_DGRAM, 0);

	//fprintf(stderr, "-->ssi_getStaConnectedBSSID()\n");
	//step 1
	if (OidQueryInformation(OID_GEN_MEDIA_CONNECT_STATUS, s, RA0, &G_ConnectStatus, sizeof(G_ConnectStatus)) < 0) {
		websError(wp, 500, "Query OID_GEN_MEDIA_CONNECT_STATUS error!");
		close(s);
		return -1;
	}

	//step 2
	if (OidQueryInformation(RT_OID_802_11_RADIO, s, RA0, &G_bRadio, sizeof(G_bRadio)) < 0) {
		websError(wp, 500, "Query RT_OID_802_11_RADIO error!");
		close(s);
		return -1;
	}

	if (G_ConnectStatus == NdisMediaStateConnected && G_bRadio) {
		memset(&BssidQuery, 0x00, sizeof(BssidQuery));
		OidQueryInformation(OID_802_11_BSSID, s, RA0, &BssidQuery, sizeof(BssidQuery));
		websWrite(wp, "<tr><td><input type=checkbox name=mac onClick=selectedBSSID(\'%02X%02X%02X%02X%02X%02X\')> %02X:%02X:%02X:%02X:%02X:%02X</td></tr>",
				BssidQuery[0], BssidQuery[1], BssidQuery[2],BssidQuery[3], BssidQuery[4], BssidQuery[5],
				BssidQuery[0], BssidQuery[1], BssidQuery[2],BssidQuery[3], BssidQuery[4], BssidQuery[5]);
	}

	close(s);
	return 0;
}

/*
 * description: write the SSID that station connected to
 */
static int getStaConnectionSSID(int eid, webs_t wp, int argc, char_t **argv)
{
	int  ConnectStatus = NdisMediaStateDisconnected;
	NDIS_802_11_SSID  SsidQuery;
	unsigned char     BssidQuery[6];
	char              strSSID[NDIS_802_11_LENGTH_SSID + 1];
	int  s, ret;

	s = socket(AF_INET, SOCK_DGRAM, 0);

	//step 1
	if (OidQueryInformation(OID_GEN_MEDIA_CONNECT_STATUS, s, RA0, &ConnectStatus, sizeof(ConnectStatus)) < 0) {
		websError(wp, 500, "Query OID_GEN_MEDIA_CONNECT_STATUS error!");
		close(s);
		return -1;
	}

	//step 2
	if (OidQueryInformation(RT_OID_802_11_RADIO, s, RA0, &G_bRadio, sizeof(G_bRadio)) < 0) {
		websError(wp, 500, "Query RT_OID_802_11_RADIO error!");
		close(s);
		return -1;
	}

	if (ConnectStatus == NdisMediaStateConnected && G_bRadio) {
		memset(&SsidQuery, 0x00, sizeof(SsidQuery));
		OidQueryInformation(OID_802_11_SSID, s, RA0, &SsidQuery, sizeof(SsidQuery));

		if (SsidQuery.SsidLength == 0) {
			memset(&BssidQuery, 0x00, sizeof(BssidQuery));
			ret = OidQueryInformation(OID_802_11_BSSID, s, RA0, &BssidQuery, sizeof(BssidQuery));
			websWrite(wp, "Connected <--> [%02X:%02X:%02X:%02X:%02X:%02X]",
					BssidQuery[0], BssidQuery[1], BssidQuery[2],
					BssidQuery[3], BssidQuery[4], BssidQuery[5]);
		}
		else {
			memset(strSSID, 0x00, NDIS_802_11_LENGTH_SSID + 1);
			memcpy(strSSID, SsidQuery.Ssid, SsidQuery.SsidLength);
			websWrite(wp, "Connected <--> %s", strSSID);
		}
		G_ConnectStatus = NdisMediaStateConnected;
	}
	else if (G_bRadio) {
		websWrite(wp, "Disconnected");
		G_ConnectStatus = NdisMediaStateDisconnected;
	}
	else {
		G_ConnectStatus = NdisMediaStateDisconnected;
		websWrite(wp, "Radio Off");
	}
	close(s);
	return 0;
}

/*
 * description: return G_bdBm_ischeck (displaying dbm or % type)
 */
static int getStaDLSList(int eid, webs_t wp, int argc, char_t **argv)
{
	RT_802_11_DLS_INFO dls_info;
	int s, i;
	char tmpmac[20];

	memset(tmpmac, 0x00, sizeof(tmpmac));

	s = socket(AF_INET, SOCK_DGRAM, 0);

	OidQueryInformation(RT_OID_802_11_QUERY_DLS_PARAM, s, RA0, &dls_info, sizeof(dls_info));
	for (i=0; i<MAX_NUM_OF_DLS_ENTRY; i++) {
		if (dls_info.Entry[i].Valid == 1 && dls_info.Entry[i].Status == DLS_FINISH) {
			sprintf(tmpmac, "%02x-%02x-%02x-%02x-%02x-%02x",
					dls_info.Entry[i].MacAddr[0], dls_info.Entry[i].MacAddr[1], dls_info.Entry[i].MacAddr[2],
					dls_info.Entry[i].MacAddr[3], dls_info.Entry[i].MacAddr[4], dls_info.Entry[i].MacAddr[5]);
			websWrite(wp, "<tr><td><input type=radio name=selected_dls value=%d>%s</td><td>%d</td></tr>",
					  i+1, tmpmac, dls_info.Entry[i].TimeOut);
		}
	}
	close(s);
	return 0;
}

/*
 * description: return G_bdBm_ischeck (displaying dbm or % type)
 */
static int getStaDbm(int eid, webs_t wp, int argc, char_t **argv)
{
	if (G_bdBm_ischeck == 1)
		ejSetResult(eid, "1");
	else
		ejSetResult(eid, "0");
	return 0;
}

/*
 * description: write station driver version
 */
static int getStaDriverVer(int eid, webs_t wp, int argc, char_t **argv)
{
	//RT_VERSION_INFO DriverVersionInfo;
	unsigned char DriverVersionInfo[8];
	int s;

	s = socket(AF_INET, SOCK_DGRAM, 0);

	//Driver
	if (OidQueryInformation(RT_OID_VERSION_INFO, s, RA0, &DriverVersionInfo, sizeof(DriverVersionInfo)) >= 0) {
		//websWrite(wp, "%d.%d.%d.%d", DriverVersionInfo.DriverVersionW, DriverVersionInfo.DriverVersionX, DriverVersionInfo.DriverVersionY, DriverVersionInfo.DriverVersionZ);
		//sprintf(tmp, "%04d-%02d-%02d", DriverVersionInfo.DriverBuildYear, DriverVersionInfo.DriverBuildMonth, DriverVersionInfo.DriverBuildDay);
		websWrite(wp, "%s", DriverVersionInfo);
	}
	else
		websWrite(wp, "&nbsp;");

	close(s);
	return 0;
}

/*
 * description: write station extra info
 */
static int getStaExtraInfo(int eid, webs_t wp, int argc, char_t **argv)
{
	unsigned long lExtraInfo;
	int s, ret;
	s = socket(AF_INET, SOCK_DGRAM, 0);

	char *ExtraInfoTable[11] = {
		"Link is Up",
		"Link is Down",
		"Hardware radio off",
		"Software radio off",
		"Open authentication fail",
		"Shared authentication fail",
		"Association failed",
		"Deauthencation because MIC failure",
		"Deauthencation on 4-way handshake timeout",
		"Deauthencation on group key handshake timeout",
		"EAP successd"
	};

	ret = OidQueryInformation(RT_OID_802_11_EXTRA_INFO, s, RA0, &lExtraInfo, 4);
	if (ret < 0 )
		return websWrite(wp, "&nbsp;");
	else {
		if (lExtraInfo <= 0xa) {
			websWrite(wp, "%s", ExtraInfoTable[lExtraInfo]);
		}
		else
			websWrite(wp, "&nbsp;");
	}
	close(s);
	return 0;
}

static int getLinkingMode(int eid, webs_t wp, int argc, char_t **argv)
{
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	HTTRANSMIT_SETTING HTSetting;

	memset(&HTSetting, 0x00, sizeof(HTTRANSMIT_SETTING));
	OidQueryInformation(RT_OID_802_11_QUERY_LAST_TX_RATE, s, RA0, &HTSetting, sizeof(HTTRANSMIT_SETTING));
	if (HTSetting.field.MODE > 1) {		// 0: CCK, 1:OFDM, 2:Mixedmode, 3:GreenField
		return websWrite(wp, T("0"));
	} else {
		return websWrite(wp, T("1"));
	}
}

/*
 * description: write station HT transmit
 */
static int getStaHT(int eid, webs_t wp, int argc, char_t **argv)
{
	int s;
	HTTRANSMIT_SETTING HTSetting;
	char tmp[8], tmpBW[72], tmpGI[72], tmpSTBC[72], tmpMCS[72];

	if (G_ConnectStatus == NdisMediaStateDisconnected)
	{
		sprintf((char *)tmpBW, "<tr><td width=\"35%%\" bgcolor=\"#E8F8FF\" >BW</td><td >n/a</td></tr>");
		sprintf((char *)tmpGI, "<tr><td width=\"35%%\" bgcolor=\"#E8F8FF\" >GI</td><td >n/a</td></tr>");
		sprintf((char *)tmpSTBC,"<tr><td width=\"35%%\" bgcolor=\"#E8F8FF\" >STBC</td><td >n/a</td></tr>");
		sprintf((char *)tmpMCS, "<tr><td width=\"35%%\" bgcolor=\"#E8F8FF\" >MCS</td><td >n/a</td></tr>");
		return websWrite(wp,"%s %s %s %s", tmpBW, tmpGI, tmpSTBC, tmpMCS);
	}

	s = socket(AF_INET, SOCK_DGRAM, 0);
	memset(&HTSetting, 0x00, sizeof(HTTRANSMIT_SETTING));
	OidQueryInformation(RT_OID_802_11_QUERY_LAST_TX_RATE, s, RA0, &HTSetting, sizeof(HTTRANSMIT_SETTING));
	close(s);

	if(HTSetting.field.MODE > 1) {		// 0: CCK, 1:OFDM, 2:Mixedmode, 3:GreenField
		if (HTSetting.field.BW == 0)
			strcpy(tmp, "20");
		else
			strcpy(tmp, "40");
		snprintf(tmpBW, 72, "<tr><td width=\"35%%\" bgcolor=\"#E8F8FF\" >BW</td><td >%s</td></tr>", tmp);

		if (HTSetting.field.ShortGI == 0)
			strcpy(tmp, "long");
		else
			strcpy(tmp, "short");
		snprintf(tmpGI, 72, "<tr><td width=\"35%%\" bgcolor=\"#E8F8FF\" >GI</td><td >%s</td></tr>", tmp);

		if (HTSetting.field.STBC == 0)
			strcpy(tmp, "none");
		else
			strcpy(tmp, "used");
		snprintf(tmpSTBC, 72, "<tr><td width=\"35%%\" bgcolor=\"#E8F8FF\" >STBC</td><td >%s</td></tr>", tmp);

		snprintf(tmpMCS, 72, "<tr><td width=\"35%%\" bgcolor=\"#E8F8FF\" >MCS</td><td >%d</td></tr>", HTSetting.field.MCS);
	} else {
		sprintf((char *)tmpBW, "<tr><td width=\"35%%\" bgcolor=\"#E8F8FF\" >BW</td><td >n/a</td></tr>");
		sprintf((char *)tmpGI, "<tr><td width=\"35%%\" bgcolor=\"#E8F8FF\" >GI</td><td >n/a</td></tr>");
		sprintf((char *)tmpSTBC,"<tr><td width=\"35%%\" bgcolor=\"#E8F8FF\" >STBC</td><td >n/a</td></tr>");
		sprintf((char *)tmpMCS, "<tr><td width=\"35%%\" bgcolor=\"#E8F8FF\" >MCS</td><td >n/a</td></tr>");
	}

	return websWrite(wp,"%s %s %s %s", tmpBW, tmpGI, tmpSTBC, tmpMCS);
}

/*
 * description: write station extra info
 */
static int getStaLinkChannel(int eid, webs_t wp, int argc, char_t **argv)
{
	NDIS_802_11_CONFIGURATION Configuration;
	RT_802_11_LINK_STATUS     LinkStatus;
	HTTRANSMIT_SETTING HTSetting;
	int s, i;
	int nChannel = -1;
	int Japan_channel = 200;

	if (G_ConnectStatus == NdisMediaStateDisconnected) {
		return websWrite(wp, "&nbsp;");
	}

	s = socket(AF_INET, SOCK_DGRAM, 0);

	// Current Channel
	OidQueryInformation(OID_802_11_CONFIGURATION, s, RA0, &Configuration, sizeof(NDIS_802_11_CONFIGURATION));
	for (i = 0; i < G_nChanFreqCount; i++) {
		if (Configuration.DSConfig == ChannelFreqTable[i].lFreq) {
			nChannel = ChannelFreqTable[i].lChannel;
			break;
		}
	}

	OidQueryInformation(RT_OID_802_11_QUERY_LINK_STATUS, s, RA0, &LinkStatus, sizeof(&LinkStatus));

	memset(&HTSetting, 0x00, sizeof(HTTRANSMIT_SETTING));
	OidQueryInformation(RT_OID_802_11_QUERY_LAST_TX_RATE, s, RA0, &HTSetting, sizeof(HTTRANSMIT_SETTING));
	close(s);

	if (nChannel == -1) {
		websWrite(wp, "error!");
	} else if (HTSetting.field.MODE > 1) {		// 0: CCK, 1:OFDM, 2:Mixedmode, 3:GreenField
		if (nChannel == (Japan_channel + 8))
			websWrite(wp, "J8 <--> %ld KHz", Configuration.DSConfig);
		else if (nChannel == (Japan_channel + 12))
			websWrite(wp, "J12 <--> %ld KHz", Configuration.DSConfig);
		else if (nChannel == (Japan_channel + 16))
			websWrite(wp, "J16 <--> %ld KHz", Configuration.DSConfig);
		else
			websWrite(wp, "%u <--> %ld KHz", nChannel, Configuration.DSConfig);
	} else {
		if (nChannel == (Japan_channel + 8))
			websWrite(wp, "J8 <--> %ld KHz ; Central Channel: %ld", Configuration.DSConfig, LinkStatus.CentralChannel);
		else if (nChannel == (Japan_channel + 12))
			websWrite(wp, "J12 <--> %ld KHz ; Central Channel: %ld", Configuration.DSConfig, LinkStatus.CentralChannel);
		else if (nChannel == (Japan_channel + 16))
			websWrite(wp, "J16 <--> %ld KHz ; Central Channel: %ld", Configuration.DSConfig, LinkStatus.CentralChannel);
		else
			websWrite(wp, "%u <--> %ld KHz ; Central Channel: %ld", nChannel, Configuration.DSConfig, LinkStatus.CentralChannel);
	}

	return 0;
}

/*
 * description: write station link quality
 */
static int getStaLinkQuality(int eid, webs_t wp, int argc, char_t **argv)
{
	RT_802_11_LINK_STATUS LinkStatus;
	int s;

	if (G_ConnectStatus == NdisMediaStateDisconnected) {
		return websWrite(wp, "0%%");
	}

	s = socket(AF_INET, SOCK_DGRAM, 0);
	// Get Link Status Info from driver
	OidQueryInformation(RT_OID_802_11_QUERY_LINK_STATUS, s, RA0, &LinkStatus, sizeof(RT_802_11_LINK_STATUS));

	LinkStatus.ChannelQuality = (unsigned long)(LinkStatus.ChannelQuality * 1.2 + 10);
	if (LinkStatus.ChannelQuality > 100)
		LinkStatus.ChannelQuality = 100;
	if (m_lChannelQuality != 0)
		LinkStatus.ChannelQuality = (unsigned long)((m_lChannelQuality + LinkStatus.ChannelQuality) / 2.0 + 0.5);

	m_lChannelQuality = LinkStatus.ChannelQuality;
	close(s);

	if (LinkStatus.ChannelQuality > 70) {
		return websWrite(wp, "Good &nbsp;&nbsp;&nbsp;&nbsp; %d%%", LinkStatus.ChannelQuality);
	}
	else if (LinkStatus.ChannelQuality > 40) {
		return websWrite(wp, "Normal &nbsp;&nbsp;&nbsp;&nbsp; %d%%", LinkStatus.ChannelQuality);
	}
	else {
		return websWrite(wp, "Weak &nbsp;&nbsp;&nbsp;&nbsp; %d%%", LinkStatus.ChannelQuality);
	}
}

static char bGetHTTxRateByBW_GI_MCS(int nBW, int nGI, int nMCS, double* dRate)
{
	//fprintf(stderr, "bGetHTTxRateByBW_GI_MCS()\n");
	double HTTxRate20_800[24]={6.5, 13.0, 19.5, 26.0, 39.0, 52.0, 58.5, 65.0, 13.0, 26.0, 39.0, 52.0, 78.0, 104.0, 117.0, 130.0,
								19.5, 39.0, 58.5, 78.0, 117.0, 156.0, 175.5, 195.0};
	double HTTxRate20_400[24]={7.2, 14.4, 21.7, 28.9, 43.3, 57.8, 65.0, 72.2, 14.444, 28.889, 43.333, 57.778, 86.667, 115.556, 130.000, 144.444,
								21.7, 43.3, 65.0, 86.7, 130.0, 173.3, 195.0, 216.7};
	double HTTxRate40_800[25]={13.5, 27.0, 40.5, 54.0, 81.0, 108.0, 121.5, 135.0, 27.0, 54.0, 81.0, 108.0, 162.0, 216.0, 243.0, 270.0,
								40.5, 81.0, 121.5, 162.0, 243.0, 324.0, 364.5, 405.0, 6.0};
	double HTTxRate40_400[25]={15.0, 30.0, 45.0, 60.0, 90.0, 120.0, 135.0, 150.0, 30.0, 60.0, 90.0, 120.0, 180.0, 240.0, 270.0, 300.0,
								45.0, 90.0, 135.0, 180.0, 270.0, 360.0, 405.0, 450.0, 6.7};

	// no TxRate for (BW = 20, GI = 400, MCS = 32) & (BW = 20, GI = 400, MCS = 32)
	if (((nBW == BW_20) && (nGI == GI_400) && (nMCS == 32)) ||
			((nBW == BW_20) && (nGI == GI_800) && (nMCS == 32)))
	{
		return 0;
	}

	if (nMCS == 32)
		nMCS = 25;

	if (nBW == BW_20 && nGI == GI_800)
		*dRate = HTTxRate20_800[nMCS];
	else if (nBW == BW_20 && nGI == GI_400)
		*dRate = HTTxRate20_400[nMCS];
	else if (nBW == BW_40 && nGI == GI_800)
		*dRate = HTTxRate40_800[nMCS];
	else if (nBW == BW_40 && nGI == GI_400)
		*dRate = HTTxRate40_400[nMCS];
	else
		return 0; //false

	//fprintf(stderr, "dRate=%.1f\n", *dRate);
	return 1; //true
}

static void DisplayLastTxRxRateFor11n(int s, int nID, double* fLastTxRxRate)
{
	unsigned long lHTSetting;
	HTTRANSMIT_SETTING HTSetting;
	double b_mode[] ={1, 2, 5.5, 11};
	float g_Rate[] = { 6,9,12,18,24,36,48,54};

	OidQueryInformation(nID, s, RA0, &lHTSetting, sizeof(lHTSetting));

	memset(&HTSetting, 0x00, sizeof(HTSetting));
	memcpy(&HTSetting, &lHTSetting, sizeof(HTSetting));

	switch(HTSetting.field.MODE)
	{
		case 0:
			if (HTSetting.field.MCS >=0 && HTSetting.field.MCS<=3)
				*fLastTxRxRate = b_mode[HTSetting.field.MCS];
			else if (HTSetting.field.MCS >=8 && HTSetting.field.MCS<=11)
				*fLastTxRxRate = b_mode[HTSetting.field.MCS-8];
			else
				*fLastTxRxRate = 0;

			break;
		case 1:
			if ((HTSetting.field.MCS >= 0) && (HTSetting.field.MCS < 8))
				*fLastTxRxRate = g_Rate[HTSetting.field.MCS];
			else
				*fLastTxRxRate = 0;

			break;
		case 2:
		case 3:
			if (0 == bGetHTTxRateByBW_GI_MCS(HTSetting.field.BW,
						HTSetting.field.ShortGI,
						HTSetting.field.MCS,
						fLastTxRxRate))
			{
				*fLastTxRxRate = 0;
			}
			break;
		default:
			*fLastTxRxRate = 0;
			break;
	}
}

/*
 * description: write station link Rx rate
 */
static int getStaLinkRxRate(int eid, webs_t wp, int argc, char_t **argv)
{
	int s;
	char tmp[8];


	if (G_ConnectStatus == NdisMediaStateDisconnected) {
		return websWrite(wp, "0");
	}
	
	s = socket(AF_INET, SOCK_DGRAM, 0);
	double fLastRxRate = 1;
	DisplayLastTxRxRateFor11n(s, RT_OID_802_11_QUERY_LAST_RX_RATE, &fLastRxRate);
	snprintf(tmp, 8, "%.1f", fLastRxRate);
	websWrite(wp, "%s", tmp);

	close(s);
	return 0;
}

/*
 * description: write station link status
 */
static int getStaLinkStatus(int eid, webs_t wp, int argc, char_t **argv)
{
	int s, ret;
	s = socket(AF_INET, SOCK_DGRAM, 0);

	ret = OidQueryInformation(OID_GEN_MEDIA_CONNECT_STATUS, s, RA0, &G_ConnectStatus, sizeof(G_ConnectStatus));
	if (ret < 0 ) {
		return websWrite(wp, "Disconnected");
	}

	if (G_ConnectStatus == NdisMediaStateConnected) {
		NDIS_802_11_SSID SSID;
		unsigned char Bssid[6];

		memset(&SSID, 0x00, sizeof(NDIS_802_11_SSID));
		OidQueryInformation(OID_802_11_SSID, s, RA0, &SSID, sizeof(NDIS_802_11_SSID));

		memset(&Bssid, 0x00, sizeof(Bssid));
		OidQueryInformation(OID_802_11_BSSID, s, RA0, Bssid, 6);

		SSID.Ssid[SSID.SsidLength] = 0;
		websWrite(wp, "%s <--> %02X-%02X-%02X-%02X-%02X-%02X", SSID.Ssid,
				Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5]);
	} else {
		if (OidQueryInformation(RT_OID_802_11_RADIO, s, RA0, &G_bRadio, sizeof(G_bRadio)) < 0) {
			websWrite(wp, "error!");
			close(s);
			return 0;
		}
		if (G_bRadio) {
			RT_802_11_STA_CONFIG configSta;
			OidQueryInformation(RT_OID_802_11_STA_CONFIG, s, RA0, &configSta, sizeof(RT_802_11_STA_CONFIG));
			if (configSta.HwRadioStatus == 0) // Hardware radio off
				websWrite(wp, "RF Off");
			else
				websWrite(wp, "Disconnected");
		}
		else
			websWrite(wp, "RF Off");
	}
	close(s);
	return 0;
}

/*
 * description: write station link Rx rate
 */
static int getStaLinkTxRate(int eid, webs_t wp, int argc, char_t **argv)
{
	int s;
	char tmp[8];

	if (G_ConnectStatus == NdisMediaStateDisconnected) {
		return websWrite(wp, "0");
	}
	s = socket(AF_INET, SOCK_DGRAM, 0);
	double fLastTxRate = 1;
	DisplayLastTxRxRateFor11n(s, RT_OID_802_11_QUERY_LAST_TX_RATE, &fLastTxRate);
	snprintf(tmp, 8, "%.1f", fLastTxRate);
	websWrite(wp, "%s", tmp);

	close(s);
	return 0;
}

/*
 * description: write station mac address
 */
static int getStaMacAddr(int eid, webs_t wp, int argc, char_t **argv)
{
	unsigned char CurrentAddress[6];
	int s;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (OidQueryInformation(OID_802_3_CURRENT_ADDRESS, s, RA0, &CurrentAddress, sizeof(CurrentAddress)) >= 0) {
		websWrite(wp, "%02X-%02X-%02X-%02X-%02X-%02X", CurrentAddress[0], CurrentAddress[1],
				CurrentAddress[2], CurrentAddress[3], CurrentAddress[4], CurrentAddress[5]);
	}
	else
		websWrite(wp, "&nbsp;");

	close(s);
	return 0;
}

/*
 * description: write station new profile name
 */
static int getStaNewProfileName(int eid, webs_t wp, int argc, char_t **argv)
{
	char profilename[32+1]; //refer to _RT_PROFILE_SETTING.
	strcpy(profilename, "PROF00");

	if (headerProfileSetting != NULL)
	{
		int count = 1, len=0;
		char cnum;
		currentProfileSetting = headerProfileSetting;
		do {
			if (strncmp(currentProfileSetting->Profile, "PROF", 4) == 0) {
				len = strlen((char *)currentProfileSetting->Profile);
				cnum = currentProfileSetting->Profile[len-1];

				if (cnum >='0' && cnum <= '9') {
					count = cnum-48+1;  //ascii 0 is 48.
					memset(profilename, 0x00, 32);
					strncpy(profilename, (char *)currentProfileSetting->Profile, len-1);
				}
				currentProfileSetting = currentProfileSetting->Next;
			}
			else {
				currentProfileSetting = currentProfileSetting->Next;
			}
		} while (currentProfileSetting != NULL );
		return websWrite(wp, "%s%d", profilename, count);
	}
	else
		return websWrite(wp, "PROF001");
}

/*
 * description: write station noise level
 */
static int getStaNoiseLevel(int eid, webs_t wp, int argc, char_t **argv)
{
	unsigned char lNoise; // this value is (ULONG) in Ndis driver (NOTICE!!!)
	int nNoiseDbm;
	int nNoisePercent;
	int s;

	if (G_ConnectStatus == NdisMediaStateDisconnected) {
		return websWrite(wp, "0%%");
	}

	s = socket(AF_INET, SOCK_DGRAM, 0);
	// Noise Level
	// Get Noise Level From Driver
	OidQueryInformation(RT_OID_802_11_QUERY_NOISE_LEVEL, s, RA0, &lNoise, sizeof(lNoise));

	close(s);

	nNoiseDbm = lNoise;
	nNoiseDbm -= 143;
	nNoisePercent = (nNoiseDbm + 100) * 10 / 3;

	if (nNoisePercent > 100)
		nNoisePercent =100;
	else if (nNoisePercent < 0)
		nNoisePercent =0;

	// Show the NoiseLevel Strength Word & Percentage
	if (nNoisePercent > 90) {
		if (G_bdBm_ischeck)
			return websWrite(wp, "Strength &nbsp;&nbsp;&nbsp;&nbsp; %d dBm", nNoiseDbm);
		else
			return websWrite(wp, "Strength &nbsp;&nbsp;&nbsp;&nbsp; %d%%", nNoisePercent);
	}
	else if (nNoisePercent > 50) {
		if (G_bdBm_ischeck)
			return websWrite(wp, "Normal &nbsp;&nbsp;&nbsp;&nbsp; %d dBm", nNoiseDbm);
		else
			return websWrite(wp, "Normal &nbsp;&nbsp;&nbsp;&nbsp; %d%%", nNoisePercent);
	}
	else {
		if (G_bdBm_ischeck)
			return websWrite(wp, "Low &nbsp;&nbsp;&nbsp;&nbsp; %d dBm", nNoiseDbm);
		else
			return websWrite(wp, "Low &nbsp;&nbsp;&nbsp;&nbsp; %d%%", nNoisePercent);
	}
}

/*
 * description: station profile initialization
 */
int initStaProfile(void)
{
	PRT_PROFILE_SETTING nextProfileSetting;
	char tmp_buffer[512];
	const char *wordlist = NULL;
	char *tok = NULL;
	int i;

	// fprintf(stderr, "kathy -- init_StaProfile()\n");
	G_ConnectStatus = NdisMediaStateDisconnected;

	//staProfile
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staProfile");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("no previous profiles defined"));
		return 0;
	}

	if (headerProfileSetting == NULL ) {
		headerProfileSetting = malloc(sizeof(RT_PROFILE_SETTING));
		memset(headerProfileSetting, 0x00, sizeof(RT_PROFILE_SETTING));
		headerProfileSetting->Next = NULL;
	}
	currentProfileSetting = headerProfileSetting;

	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";") ; tok ;  i++) {
		//profile
		sprintf((char *)currentProfileSetting->Profile, "%s", tok);
		// fprintf(stderr, "i=%d, Profile=%s, tok=%s\n", i,currentProfileSetting->Profile, tok);
		tok = strtok(NULL,";");

		if (tok != NULL && currentProfileSetting->Next == NULL) {
			nextProfileSetting = malloc(sizeof(RT_PROFILE_SETTING));
			memset(nextProfileSetting, 0x00, sizeof(RT_PROFILE_SETTING));
			nextProfileSetting->Next = NULL;
			currentProfileSetting->Next = nextProfileSetting;
			currentProfileSetting = nextProfileSetting;
		}
		else
			currentProfileSetting = currentProfileSetting->Next;
	}
	G_staProfileNum = i;

	// SSID
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staSSID");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta SSID has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		sprintf((char *)currentProfileSetting->SSID, "%s", tok);
		currentProfileSetting->SsidLen = strlen(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// NetworkType
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staNetworkType");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta NetworkType has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->NetworkType = atoi(tok);
		// fprintf(stderr, "i=%d, NetworkType=%d\n", i,currentProfileSetting->NetworkType);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// PSMode
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staPSMode");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0)	{
		error(E_L, E_LOG, T("Sta PSMode has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->PSmode= atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// AdhocMode
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staAdhocMode");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0)	{
		error(E_L, E_LOG, T("Sta AdhocMode has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->AdhocMode= atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// Channel
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staChannel");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0)	{
		error(E_L, E_LOG, T("Sta Channel has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->Channel= atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// PreamType
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staPreamType");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0)	{
		error(E_L, E_LOG, T("Sta PreamType has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->PreamType= atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// RTSCheck
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staRTSCheck");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta RTSCheck has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->RTSCheck= atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// FragmentCheck
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staFragmentCheck");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0)	{
		error(E_L, E_LOG, T("Sta FragmentCheck has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->FragmentCheck= atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// RTS
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staRTS");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta RTS has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->RTS= atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// Fragment
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staFragment");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta Fragment has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->Fragment= atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// Auth
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staAuth");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta Auth has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->Authentication= atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// Encryption
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staEncrypt");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta Encryption has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->Encryption= atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// Key1	
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staKey1");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta Key1 has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		sprintf((char *)currentProfileSetting->Key1, "%s", tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// Key2
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staKey2");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta Key2 has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		sprintf((char *)currentProfileSetting->Key2, "%s", tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// Key3
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staKey3");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta Key3 has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		sprintf((char *)currentProfileSetting->Key3, "%s", tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// Key4
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staKey4");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta Key4 has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		sprintf((char *)currentProfileSetting->Key4, "%s", tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// Key1Type
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staKey1Type");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta Key1Type has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->Key1Type= atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// Key2Type
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staKey2Type");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta Key2Type has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->Key2Type= atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// Key3Type
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staKey3Type");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta Key3Type has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->Key3Type= atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// Key4Type
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staKey4Type");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta Key4Type has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->Key4Type= atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// Key1Length
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staKey1Length");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta Key1Lenght has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->Key1Length= atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// Key2Length
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staKey2Length");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta Key2Lenght has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->Key2Length= atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// Key3Length
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staKey3Length");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta Key3Lenght has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->Key3Length= atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// Key4Length
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staKey4Length");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta Key4Length has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->Key4Length= atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// DefaultKeyID
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staKeyDefaultId");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta DefaultKeyID has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->KeyDefaultId = atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// WPAPSK
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staWpaPsk");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta WPAPSK has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		sprintf((char *)currentProfileSetting->WpaPsk, "%s", tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

#ifdef WPA_SUPPLICANT_SUPPORT
	//keymgmt
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "sta8021xKeyMgmt");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta 802.1x Key Mgmt has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->KeyMgmt = atoi(tok);

		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	// EAP
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "sta8021xEAP");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta 802.1x EAP has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->EAP = atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	//Cert ID
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "sta8021xIdentity");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta 802.1x Identity has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		sprintf((char *)currentProfileSetting->Identity, "%s", tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	//Cert Password
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "sta8021xPassword");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta 802.1x Password has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		sprintf((char *)currentProfileSetting->Password, "%s", tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	//Cert Client Cert Path
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "sta8021xClientCert");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta 802.1x Client Cert has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		sprintf((char *)currentProfileSetting->ClientCert, "%s", tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	//Cert Private Key Path
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "sta8021xPrivateKey");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta 802.1x Private Key has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		sprintf((char *)currentProfileSetting->PrivateKey, "%s", tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	//Cert Private Key Password
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "sta8021xPrivateKeyPassword");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta 802.1x Private Key Password has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		sprintf((char *)currentProfileSetting->PrivateKeyPassword, "%s", tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	//Cert CA Cert
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "sta8021xCACert");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta 802.1x CA Cert has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		sprintf((char *)currentProfileSetting->CACert, "%s", tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}

	//Tunnel
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "sta8021xTunnel");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0) {
		error(E_L, E_LOG, T("Sta 802.1x Tunnel has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->Tunnel = atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}
#endif

	//Active
	bzero(tmp_buffer, sizeof(tmp_buffer));
	wordlist = nvram_bufget(RT2860_NVRAM, "staActive");
	if (wordlist == NULL || strcmp(wordlist, "" ) == 0)	{
		error(E_L, E_LOG, T("Sta Active has no data."));
		return -1;
	}

	currentProfileSetting = headerProfileSetting;
	sprintf(tmp_buffer, "%s", wordlist);
	for (i = 0, tok = strtok(tmp_buffer,";"); tok; tok = strtok(NULL,";"), i++) {
		currentProfileSetting->Active = atoi(tok);
		if (currentProfileSetting->Next != NULL)
			currentProfileSetting = currentProfileSetting->Next;
	}
	return 0;
}

/*
 * description: convert ascii byte to numeric
 */
unsigned char BtoH(char ch)
{
	if (ch >= '0' && ch <= '9') return (ch - '0');        // Handle numerals
	if (ch >= 'A' && ch <= 'F') return (ch - 'A' + 0xA);  // Handle capitol hex digits
	if (ch >= 'a' && ch <= 'f') return (ch - 'a' + 0xA);  // Handle small hex digits
	return(255);
}

/*
 * description: Converts ascii string to network order hex
 * parameters: src     - pointer to input ascii string
 *             dest    - pointer to output hex
 *             destlen - size of dest
 */
void AtoH(char * src, unsigned char * dest, int destlen)
{
	char * srcptr;

	srcptr = src;
	unsigned char* destTemp = dest;
	while (destlen--) {
		*destTemp = BtoH(*srcptr++) << 4;  // Put 1st ascii byte in upper nibble.
		*destTemp += BtoH(*srcptr++);      // Add 2nd ascii byte to above.
		destTemp++;
	}
}

static void shaHashBlock(A_SHA_CTX *ctx) 
{
	int t;
	unsigned long A,B,C,D,E,TEMP;

#define SHA_ROTL(X,n) ((((X) << (n)) | ((X) >> (32-(n)))) & 0xffffffffL)
	for (t = 16; t <= 79; t++)
		ctx->W[t] = SHA_ROTL(ctx->W[t-3] ^ ctx->W[t-8] ^ ctx->W[t-14] ^ ctx->W[t-16], 1);

	A = ctx->H[0];
	B = ctx->H[1];
	C = ctx->H[2];
	D = ctx->H[3];
	E = ctx->H[4];

	for (t = 0; t <= 19; t++) 
	{
		TEMP = (SHA_ROTL(A,5) + (((C^D)&B)^D)     + E + ctx->W[t] + 0x5a827999L) & 0xffffffffL;
		E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP;
	}
	for (t = 20; t <= 39; t++) 
	{
		TEMP = (SHA_ROTL(A,5) + (B^C^D)           + E + ctx->W[t] + 0x6ed9eba1L) & 0xffffffffL;
		E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP;
	}
	for (t = 40; t <= 59; t++) 
	{
		TEMP = (SHA_ROTL(A,5) + ((B&C)|(D&(B|C))) + E + ctx->W[t] + 0x8f1bbcdcL) & 0xffffffffL;
		E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP;
	}
	for (t = 60; t <= 79; t++) 
	{
		TEMP = (SHA_ROTL(A,5) + (B^C^D)           + E + ctx->W[t] + 0xca62c1d6L) & 0xffffffffL;
		E = D; D = C; C = SHA_ROTL(B, 30); B = A; A = TEMP;
	}

	ctx->H[0] += A;
	ctx->H[1] += B;
	ctx->H[2] += C;
	ctx->H[3] += D;
	ctx->H[4] += E;
}

void A_SHAInit(A_SHA_CTX *ctx) 
{
	int i;

	ctx->lenW = 0;
	ctx->sizeHi = ctx->sizeLo = 0;

	/* Initialize H with the magic constants (see FIPS180 for constants)
	*/
	ctx->H[0] = 0x67452301L;
	ctx->H[1] = 0xefcdab89L;
	ctx->H[2] = 0x98badcfeL;
	ctx->H[3] = 0x10325476L;
	ctx->H[4] = 0xc3d2e1f0L;

	for (i = 0; i < 80; i++)
		ctx->W[i] = 0;
}

void A_SHAUpdate(A_SHA_CTX *ctx, unsigned char *dataIn, int len) 
{
	int i;

	/* Read the data into W and process blocks as they get full */
	for (i = 0; i < len; i++) 
	{
		ctx->W[ctx->lenW / 4] <<= 8;
		ctx->W[ctx->lenW / 4] |= (unsigned long)dataIn[i];
		if ((++ctx->lenW) % 64 == 0) 
		{
			shaHashBlock(ctx);
			ctx->lenW = 0;
		}
		ctx->sizeLo += 8;
		ctx->sizeHi += (ctx->sizeLo < 8);
	}
}

void A_SHAFinal(A_SHA_CTX *ctx, unsigned char hashout[20]) 
{
	unsigned char pad0x80 = 0x80;
	unsigned char pad0x00 = 0x00;
	unsigned char padlen[8];
	int i;

	/* Pad with a binary 1 (e.g. 0x80), then zeroes, then length */
	padlen[0] = (unsigned char)((ctx->sizeHi >> 24) & 255);
	padlen[1] = (unsigned char)((ctx->sizeHi >> 16) & 255);
	padlen[2] = (unsigned char)((ctx->sizeHi >> 8) & 255);
	padlen[3] = (unsigned char)((ctx->sizeHi >> 0) & 255);
	padlen[4] = (unsigned char)((ctx->sizeLo >> 24) & 255);
	padlen[5] = (unsigned char)((ctx->sizeLo >> 16) & 255);
	padlen[6] = (unsigned char)((ctx->sizeLo >> 8) & 255);
	padlen[7] = (unsigned char)((ctx->sizeLo >> 0) & 255);
	A_SHAUpdate(ctx, &pad0x80, 1);
	while (ctx->lenW != 56)
		A_SHAUpdate(ctx, &pad0x00, 1);
	A_SHAUpdate(ctx, padlen, 8);

	/* Output hash
	*/
	for (i = 0; i < 20; i++) 
	{
		hashout[i] = (unsigned char)(ctx->H[i / 4] >> 24);
		ctx->H[i / 4] <<= 8;
	}

	/*
	 *  Re-initialize the context (also zeroizes contents)
	 */
	A_SHAInit(ctx); 
}

static void hmac_sha1(unsigned char *text, int text_len, unsigned char *key, int key_len, unsigned char *digest) 
{ 
	A_SHA_CTX context; 
	unsigned char k_ipad[65]; /* inner padding - key XORd with ipad */ 
	unsigned char k_opad[65]; /* outer padding - key XORd with opad */ 
	int i; 

	/* if key is longer than 64 bytes reset it to key=SHA1(key) */ 
	if (key_len > 64) 
	{ 
		A_SHA_CTX tctx; 

		A_SHAInit(&tctx); 
		A_SHAUpdate(&tctx, key, key_len); 
		A_SHAFinal(&tctx, key); 

		key_len = 20; 
	} 

	/* 
	 * the HMAC_SHA1 transform looks like: 
	 * 
	 * SHA1(K XOR opad, SHA1(K XOR ipad, text)) 
	 * 
	 * where K is an n byte key 
	 * ipad is the byte 0x36 repeated 64 times 
	 * opad is the byte 0x5c repeated 64 times 
	 * and text is the data being protected 
	 */ 

	/* start out by storing key in pads */ 
	memset(k_ipad, 0, sizeof k_ipad); 
	memset(k_opad, 0, sizeof k_opad); 
	memcpy(k_ipad, key, key_len); 
	memcpy(k_opad, key, key_len); 

	/* XOR key with ipad and opad values */ 
	for (i = 0; i < 64; i++) 
	{ 
		k_ipad[i] ^= 0x36; 
		k_opad[i] ^= 0x5c; 
	} 

	/* perform inner SHA1*/ 
	A_SHAInit(&context); /* init context for 1st pass */ 
	A_SHAUpdate(&context, k_ipad, 64); /* start with inner pad */ 
	A_SHAUpdate(&context, text, text_len); /* then text of datagram */ 
	A_SHAFinal(&context, digest); /* finish up 1st pass */ 

	/* perform outer SHA1 */ 
	A_SHAInit(&context); /* init context for 2nd pass */ 
	A_SHAUpdate(&context, k_opad, 64); /* start with outer pad */ 
	A_SHAUpdate(&context, digest, 20); /* then results of 1st hash */ 
	A_SHAFinal(&context, digest); /* finish up 2nd pass */ 
} 

/*
 * F(P, S, c, i) = U1 xor U2 xor ... Uc 
 * U1 = PRF(P, S || Int(i)) 
 * U2 = PRF(P, U1) 
 * Uc = PRF(P, Uc-1) 
 */ 

static void F(char *password, unsigned char *ssid, int ssidlength, int iterations, int count, unsigned char *output) 
{ 
	unsigned char digest[36], digest1[A_SHA_DIGEST_LEN]; 
	int i, j; 

	/* U1 = PRF(P, S || int(i)) */ 
	memcpy(digest, ssid, ssidlength); 
	digest[ssidlength] = (unsigned char)((count>>24) & 0xff); 
	digest[ssidlength+1] = (unsigned char)((count>>16) & 0xff); 
	digest[ssidlength+2] = (unsigned char)((count>>8) & 0xff); 
	digest[ssidlength+3] = (unsigned char)(count & 0xff); 
	hmac_sha1(digest, ssidlength+4, (unsigned char*) password, (int) strlen(password), digest1); // for WPA update

	/* output = U1 */ 
	memcpy(output, digest1, A_SHA_DIGEST_LEN); 

	for (i = 1; i < iterations; i++) 
	{ 
		/* Un = PRF(P, Un-1) */ 
		hmac_sha1(digest1, A_SHA_DIGEST_LEN, (unsigned char*) password, (int) strlen(password), digest); // for WPA update
		memcpy(digest1, digest, A_SHA_DIGEST_LEN); 

		/* output = output xor Un */ 
		for (j = 0; j < A_SHA_DIGEST_LEN; j++) 
		{ 
			output[j] ^= digest[j]; 
		} 
	} 
} 
/* 
 * password - ascii string up to 63 characters in length 
 * ssid - octet string up to 32 octets 
 * ssidlength - length of ssid in octets 
 * output must be 40 octets in length and outputs 256 bits of key 
 */ 
int PasswordHash(char *password, unsigned char *ssid, int ssidlength, unsigned char *output) 
{ 
	if ((strlen(password) > 63) || (ssidlength > 32)) 
		return 0; 

	F(password, ssid, ssidlength, 4096, 1, output); 
	F(password, ssid, ssidlength, 4096, 2, &output[A_SHA_DIGEST_LEN]); 
	return 1; 
}

static int getCACLCertList(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef WPA_SUPPLICANT_SUPPORT
	char *caclcert_file = (char *) nvram_get(CERT_NVRAM, "CACLCertFile");

	if (strlen(caclcert_file) > 0)
		websWrite(wp, T("<option value=\"%s\">%s</option>"), caclcert_file, caclcert_file);
	else
#endif
		websWrite(wp, T("<option value=\"\"></option>"));
	
	return 0;
}

static int getKeyCertList(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef WPA_SUPPLICANT_SUPPORT
	char *keycert_file = (char *) nvram_get(CERT_NVRAM, "KeyCertFile");

	if (strlen(keycert_file) > 0)
		websWrite(wp, T("<option value=\"%s\">%s</option>"), keycert_file, keycert_file);
	else
#endif
		websWrite(wp, T("<option value=\"\"></option>"));
	
	return 0;
}

static int getWPASupplicantBuilt(int eid, webs_t wp, int argc, char_t **argv)
{
#ifdef WPA_SUPPLICANT_SUPPORT
	return websWrite(wp, T("1"));
#else
	return websWrite(wp, T("0"));
#endif
}

#ifdef WPA_SUPPLICANT_SUPPORT
static void exec_WPASupplicant(char* ssid, NDIS_802_11_WEP_STATUS encryp, NDIS_802_11_AUTHENTICATION_MODE auth, RT_WPA_SUPPLICANT_KEY_MGMT keymgmt, int keyidx, char* wepkey)
{
	// auth mode
	int s, ret;
	unsigned char wpa_supplicant_support = 2, ieee8021x_support = 1;
	NDIS_802_11_SSID Ssid;

	system("killall wpa_supplicant");
	sleep(1);
	//fprintf(stderr, "exec_WPASupplicant()\n");
	memset(&Ssid, 0x00, sizeof(NDIS_802_11_SSID));
	strcpy((char *)Ssid.Ssid ,ssid);
	Ssid.SsidLength = strlen(ssid);

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (auth == Ndis802_11AuthMode8021x)
		auth = Ndis802_11AuthModeOpen;

	if (keymgmt == Rtwpa_supplicantKeyMgmtNONE)
	{
		wpa_supplicant_support = 0;
		ieee8021x_support = 0;

		ret = OidSetInformation(OID_802_11_SET_IEEE8021X, s, RA0, &ieee8021x_support, sizeof(ieee8021x_support));
		if (ret < 0) {
			fprintf(stderr, "Set OID_802_11_SET_IEEE8021X has error =%d, ieee8021x_support=%d\n", ret, ieee8021x_support);
			close(s);
			return;
		}

		ret = OidSetInformation(RT_OID_WPA_SUPPLICANT_SUPPORT, s, RA0, &wpa_supplicant_support, sizeof(wpa_supplicant_support));
		if (ret < 0) {
			fprintf(stderr, "Set RT_OID_WPA_SUPPLICANT_SUPPORT has error =%d, wpa_supplicant_support=%d\n", ret, wpa_supplicant_support);
			fprintf(stderr, "Please check the driver configuration whether support WAP_SUPPORT!!");
			close(s);
			return;
		}
	}
	else
	{
		ret = OidSetInformation(OID_802_11_SET_IEEE8021X, s, RA0, &ieee8021x_support, sizeof(ieee8021x_support));
		if (ret < 0) {
			fprintf(stderr, "Set OID_802_11_SET_IEEE8021X has error =%d, ieee8021x_support=%d\n", ret, ieee8021x_support);
			close(s);
			return;
		}

		ret = OidSetInformation(RT_OID_WPA_SUPPLICANT_SUPPORT, s, RA0, &wpa_supplicant_support, sizeof(wpa_supplicant_support));
		if (ret < 0) {
			fprintf(stderr, "Set RT_OID_WPA_SUPPLICANT_SUPPORT has error =%d, wpa_supplicant_support=%d\n", ret, wpa_supplicant_support);
			fprintf(stderr, "Please check the driver configuration whether support WAP_SUPPORT!!");
			close(s);
			return;
		}
	}

	ret = OidSetInformation(OID_802_11_AUTHENTICATION_MODE, s, RA0, &auth, sizeof(auth));
	if (ret < 0) {
		fprintf(stderr, "Set OID_802_11_AUTHENTICATION_MODE has error =%d, auth=%d\n", ret, auth);
		close(s);
		return;
	}

	// encryp mode
	ret = OidSetInformation(OID_802_11_ENCRYPTION_STATUS, s, RA0, &encryp, sizeof(encryp));
	if (ret < 0) {
		fprintf(stderr, "Set OID_802_11_ENCRYPTION_STATUS has error =%d, encry=%d\n", ret, encryp);
		close(s);
		return;
	}

	if (encryp == Ndis802_11WEPEnabled)
	{
		PNDIS_802_11_WEP	pWepKey = NULL;
		unsigned long		lBufLen;
		int 				nKeyLen;

		nKeyLen = strlen(wepkey);
		if (nKeyLen == 0)
		{
			NDIS_802_11_REMOVE_KEY	removeKey;
			int j=0;
			removeKey.Length = sizeof(NDIS_802_11_REMOVE_KEY);
			removeKey.KeyIndex = keyidx;
			for (j = 0; j < 6; j++)
				removeKey.BSSID[j] = 0xff;

			ret = OidSetInformation(OID_802_11_REMOVE_KEY, s, RA0, &removeKey, removeKey.Length);
			if (ret < 0)
				fprintf(stderr, "Set OID_802_11_REMOVE_KEY has error =%d, \n", ret);
		}
		else
		{
			if (nKeyLen == 10)
				nKeyLen = 5;
			else if (nKeyLen == 26)
				nKeyLen = 13;

			lBufLen = sizeof(NDIS_802_11_WEP) + nKeyLen - 1;
			// Allocate Resource
			pWepKey = (PNDIS_802_11_WEP)malloc(lBufLen);
			pWepKey->Length = lBufLen;
			pWepKey->KeyLength = nKeyLen;
			pWepKey->KeyIndex = keyidx;

			if (keyidx == 1)
				pWepKey->KeyIndex |= 0x80000000;

			if (strlen(wepkey) == 5)
				memcpy(pWepKey->KeyMaterial, wepkey, 5);
			else if (strlen(wepkey) == 10)
				AtoH(wepkey, pWepKey->KeyMaterial, 5);
			else if (strlen(wepkey) == 13)
				memcpy(pWepKey->KeyMaterial, wepkey, 13);
			else if (strlen(wepkey) == 26)
				AtoH(wepkey, pWepKey->KeyMaterial, 13);

			OidSetInformation(OID_802_11_ADD_WEP, s, RA0, pWepKey, pWepKey->Length);
			free(pWepKey);
		}
	}

	// set ssid for associate
	if (OidSetInformation(OID_802_11_SSID, s, RA0, &Ssid, sizeof(NDIS_802_11_SSID)) < 0) {
		fprintf(stderr, "Set OID_802_11_SSID has error =%d, pSsid->Ssid=%s\n", ret, Ssid.Ssid);
		close(s);
		return;
	}

	/*
	if (OidSetInformation(OID_802_11_BSSID, s, RA0, &bssid, 6) < 0) {
		error(E_L, E_LOG, T("Set OID_802_11_BSSID has error."));
		close(s);
		return;
	}
	*/

	close(s);

	doSystem("wpa_supplicant -B -i%s -bbr0 -c/etc/wpa_supplicant.conf -Dralink -d", RA0);
	doSystem("wan.sh");		// restart wan.sh if needed (renew dhcp, pppoe etc)
	WpaSupplicant_flag = TRUE;
}

static void conf_WPASupplicant(char* ssid, RT_WPA_SUPPLICANT_KEY_MGMT keymgmt, RT_WPA_SUPPLICANT_EAP eap, char* identity, char* password, char* cacert, char* clientcert, char* privatekey, char* privatekeypassword, char* wepkey, int keyidx, NDIS_802_11_WEP_STATUS encryp, RT_WPA_SUPPLICANT_TUNNEL tunnel, NDIS_802_11_AUTHENTICATION_MODE auth)
{
	FILE *wsconf;
	char wpaconf[] = "/etc/wpa_supplicant.conf";

	//fprintf(stderr, "conf_WPASupplicant()\n");

	fprintf(stderr, "wpaconf=%s\n", wpaconf);
	fprintf(stderr, "conf_WPASupplicant(), keymgmt=%d, Rtwpa_supplicantKeyMgmtNONE=%d\n", keymgmt, Rtwpa_supplicantKeyMgmtNONE);

	wsconf = fopen(wpaconf, "w+");

	fprintf(wsconf, "ctrl_interface=/var/run/wpa_supplicant\n");
	fprintf(wsconf, "eapol_version=1\n");
	fprintf(wsconf, "ap_scan=0\n");
	fprintf(wsconf, "network={\n");
	fprintf(wsconf, "ssid=\"%s\"\n", ssid);

	if (keymgmt == Rtwpa_supplicantKeyMgmtWPAEAP)
	{
		fprintf(wsconf, "key_mgmt=%s\n", "WPA-EAP");

		if (auth == Ndis802_11AuthModeWPA)
			fprintf(wsconf, "proto=WPA\n");
		else if (auth == Ndis802_11AuthModeWPA2)
			fprintf(wsconf, "proto=RSN\n");

		if (encryp == Ndis802_11Encryption2Enabled) //tkip
		{			
			fprintf(wsconf, "pairwise=TKIP\n");
			fprintf(wsconf, "group=TKIP\n");
		}
		else if (encryp == Ndis802_11Encryption3Enabled) //aes
		{
			fprintf(wsconf, "pairwise=CCMP TKIP\n");
			fprintf(wsconf, "group=CCMP TKIP\n");
		}

	}
	else if (keymgmt == Rtwpa_supplicantKeyMgmtIEEE8021X)
	{
		fprintf(wsconf, "key_mgmt=%s\n", "IEEE8021X");
		if (eap == Rtwpa_supplicantEAPTLS || eap == Rtwpa_supplicantEAPTTLS)
			fprintf(wsconf, "eapol_flags=3\n");
		else if (eap == Rtwpa_supplicantEAPMD5)
			fprintf(wsconf, "eapol_flags=0\n");
	}
	else if (keymgmt == Rtwpa_supplicantKeyMgmtNONE)
	{
		fprintf(wsconf, "key_mgmt=%s\n", "NONE");
		fprintf(wsconf, "}\n");
		fclose(wsconf);
		exec_WPASupplicant(ssid, encryp, auth, keymgmt, keyidx, wepkey);
		return;
	}

	//id
	fprintf(wsconf, "identity=\"%s\"\n",identity);

	//CA cert
	if (strcmp(cacert, "0" ) != 0 && strcmp(cacert, "") !=0) //option
		fprintf(wsconf, "ca_cert=\"%s\"\n", cacert);

	//eap
	switch(eap)
	{
		case Rtwpa_supplicantEAPTLS:
			fprintf(wsconf, "eap=TLS\n");
			fprintf(wsconf, "client_cert=\"%s\"\n", clientcert);
			fprintf(wsconf, "private_key=\"%s\"\n", privatekey);
			fprintf(wsconf, "private_key_passwd=\"%s\"\n", privatekeypassword);
			break;
		case Rtwpa_supplicantEAPTTLS:
			fprintf(wsconf, "eap=TTLS\n");
			if( strcmp(clientcert, "0" ) != 0 && strcmp(clientcert, "") !=0 ) //option
			{
				fprintf(wsconf, "client_cert=\"%s\"\n", clientcert);
				fprintf(wsconf, "private_key=\"%s\"\n", privatekey);
				fprintf(wsconf, "private_key_passwd=\"%s\"\n", privatekeypassword);
			}
			if (tunnel == Rtwpa_supplicantTUNNELMSCHAPV2)
				fprintf(wsconf, "phase2=\"auth=MSCHAPV2\"\n");
			else if (tunnel == Rtwpa_supplicantTUNNELMSCHAP)
				fprintf(wsconf, "phase2=\"auth=MSCHAP\"\n");
			else if (tunnel == Rtwpa_supplicantTUNNELPAP)
				fprintf(wsconf, "phase2=\"auth=PAP\"\n");
			break;

		case Rtwpa_supplicantEAPPEAP:
			fprintf(wsconf, "eap=PEAP\n");
			//fprintf(stderr, "clientcert=%s, strlen=%d\n", clientcert, strlen((char *)clientcert));
			if( strcmp(clientcert, "0" ) != 0 && strcmp(clientcert, "") !=0) //option
			{
				fprintf(wsconf, "client_cert=\"%s\"\n", clientcert);
				fprintf(wsconf, "private_key=\"%s\"\n", privatekey);
				fprintf(wsconf, "private_key_passwd=\"%s\"\n", privatekeypassword);
			}
			fprintf(wsconf, "password=\"%s\"\n", password);
			fprintf(wsconf, "phase1=\"peaplable=0\"\n");

			if (tunnel == Rtwpa_supplicantTUNNELMSCHAPV2)
				fprintf(wsconf, "phase2=\"auth=MSCHAPV2\"\n");

			break;
		case Rtwpa_supplicantEAPMD5:
			fprintf(wsconf, "eap=MD5\n");
			fprintf(wsconf, "password=\"%s\"\n", password);
			fprintf(wsconf, "wep_tx_keyidx=%d\n", keyidx);
			fprintf(wsconf, "wep_key%d=%s\n", keyidx, wepkey);
			break;
		default:
			break;
	}

	fprintf(wsconf, "}\n");
	fclose(wsconf);
	exec_WPASupplicant(ssid, encryp, auth, keymgmt, keyidx, wepkey);
}
#endif

/*
 * description: station connection
 */
static void sta_connection(int tmp_networktype, int tmp_auth, int tmp_encry, int tmp_defaultkeyid, PNDIS_802_11_SSID pSsid, unsigned char Bssid[6], char *tmp_wpapsk, char *tmp_key1, char *tmp_key2, char *tmp_key3, char *tmp_key4, RT_802_11_PREAMBLE tmp_preamtype, int tmp_rtscheck, NDIS_802_11_RTS_THRESHOLD tmp_rts, int tmp_fragmentcheck, NDIS_802_11_FRAGMENTATION_THRESHOLD tmp_fragment, NDIS_802_11_POWER_MODE tmp_psmode, int tmp_channel)
{
	int s, ret, nKeyLen=0, j, i;
	NDIS_802_11_REMOVE_KEY		removeKey;
	PNDIS_802_11_KEY			pKey = NULL;
	PNDIS_802_11_WEP			pWepKey = NULL;
	PNDIS_802_11_PASSPHRASE		pPassPhrase = NULL;
	unsigned long               PassphraseBufLen;
	unsigned long				lBufLen;
	unsigned char				keyMaterial[40];
	NDIS_802_11_CONFIGURATION	Configuration;
	unsigned long				CurrentWirelessMode;

	//fprintf(stderr, "sta_connection()\n");
	s = socket(AF_INET, SOCK_DGRAM, 0);

	if (OidQueryInformation(RT_OID_802_11_PHY_MODE, s, RA0, &CurrentWirelessMode, sizeof(unsigned char)) < 0 ) {
		error(E_L, E_LOG, T("Query OID_802_11_QUERY_WirelessMode error!"));
		close(s);
		return;
	}
	if(tmp_encry == Ndis802_11Encryption2Enabled || tmp_encry == Ndis802_11WEPEnabled) {
		if(CurrentWirelessMode > 4) {
			doSystem("iwpriv %s set WirelessMode=3", RA0);
			sleep(2);
		}
	} else {
		if(CurrentWirelessMode < 5) {
			doSystem("iwpriv %s set WirelessMode=5", RA0);
			sleep(2);
		}
	}

	if (WpaSupplicant_flag == TRUE)
	{
		int wpa_supplicant_support = 0 ,ieee8021x_support = 0;

		doSystem("killall wpa_supplicant");
		sleep(2);
		ret = OidSetInformation(OID_802_11_SET_IEEE8021X, s, RA0, &ieee8021x_support, sizeof(ieee8021x_support));
		if (ret < 0)
			fprintf(stderr, "Set OID_802_11_SET_IEEE8021X has error =%d, ieee8021x_support=%d\n", ret, ieee8021x_support);
		ret = OidSetInformation(RT_OID_WPA_SUPPLICANT_SUPPORT, s, RA0, &wpa_supplicant_support, sizeof(wpa_supplicant_support));                                           if (ret < 0)
			fprintf(stderr, "Set RT_OID_WPA_SUPPLICANT_SUPPORT has error =%d, wpa_supplicant_support=%d\n", ret, wpa_supplicant_support);
		WpaSupplicant_flag = FALSE;
	}

	//step 0: OID_802_11_INFRASTRUCTURE_MODE
	ret = OidSetInformation(OID_802_11_INFRASTRUCTURE_MODE, s, RA0, &tmp_networktype, sizeof(int));
	if (ret < 0)
		error(E_L, E_LOG, T("Set OID_802_11_INFRASTRUCTURE_MODE has error =%d, networktype=%d"), ret, tmp_networktype);

	//step 1:
	if (!tmp_rtscheck)
		tmp_rts = 2347;
	OidSetInformation(OID_802_11_RTS_THRESHOLD, s, RA0, &tmp_rts, sizeof(NDIS_802_11_RTS_THRESHOLD));

	if (!tmp_fragmentcheck)
		tmp_fragment = 2346;
	OidSetInformation(OID_802_11_FRAGMENTATION_THRESHOLD, s, RA0, &selectedProfileSetting->Fragment, sizeof(NDIS_802_11_FRAGMENTATION_THRESHOLD));

	if (tmp_networktype == Ndis802_11Infrastructure) {
		OidSetInformation(OID_802_11_POWER_MODE, s, RA0, &tmp_psmode, sizeof(NDIS_802_11_POWER_MODE));
		OidSetInformation(RT_OID_802_11_PREAMBLE, s, RA0, &tmp_preamtype, sizeof(RT_802_11_PREAMBLE));
	}
	else if (tmp_networktype == Ndis802_11IBSS) {
		unsigned long	lFreq = 0;
		OidQueryInformation(OID_802_11_CONFIGURATION, s, RA0, &Configuration, sizeof(Configuration));

		for (i = 0; i < G_nChanFreqCount; i++) {
			if (tmp_channel == ChannelFreqTable[i].lChannel) {
				lFreq = ChannelFreqTable[i].lFreq;
				break;
			}
		}
		if (lFreq != Configuration.DSConfig) {
			Configuration.DSConfig = lFreq/1000;
			ret = OidSetInformation(OID_802_11_CONFIGURATION, s, RA0, &Configuration, sizeof(Configuration));
			if (ret < 0)
				error(E_L, E_LOG, T("Set OID_802_11_CONFIGURATION has error=%d"),ret);
		}
	}

	//step 2: Security mode
    ret = OidSetInformation(OID_802_11_AUTHENTICATION_MODE, s, RA0, &tmp_auth, sizeof(tmp_auth));
	if (ret < 0)
		error(E_L, E_LOG, T("Set OID_802_11_AUTHENTICATION_MODE has error =%d, auth=%d"), ret, tmp_auth);

    ret = OidSetInformation(OID_802_11_ENCRYPTION_STATUS, s, RA0, &tmp_encry, sizeof(tmp_encry));
	if (ret < 0)
		error(E_L, E_LOG, T("Set OID_802_11_ENCRYPTION_STATUS has error =%d, encry=%d"), ret, tmp_encry);

	if (tmp_encry == Ndis802_11WEPEnabled) {
		//----------------------------------------------------------//
		//Key 1
		//----------------------------------------------------------//
	
		nKeyLen = strlen(tmp_key1);
		if (nKeyLen == 0) {
			removeKey.Length = sizeof(NDIS_802_11_REMOVE_KEY);
			removeKey.KeyIndex = 0;
			for (j = 0; j < 6; j++)
				removeKey.BSSID[j] = 0xff;
			ret = OidSetInformation(OID_802_11_REMOVE_KEY, s, RA0, &removeKey, removeKey.Length);
			if (ret < 0)
				error(E_L, E_LOG, T("Set OID_802_11_REMOVE_KEY has error =%d"), ret);
		} else if (strcmp(tmp_key1, "0")) {
			if (nKeyLen == 10)
				nKeyLen = 5;
			else if (nKeyLen == 26)
				nKeyLen = 13;

			lBufLen = sizeof(NDIS_802_11_WEP) + nKeyLen - 1;
			// Allocate Resource
			pWepKey = (PNDIS_802_11_WEP)malloc(lBufLen);
			pWepKey->Length = lBufLen;
			pWepKey->KeyLength = nKeyLen;
			pWepKey->KeyIndex = 0;

			if (tmp_defaultkeyid == 1)
				pWepKey->KeyIndex |= 0x80000000;

			if (strlen(tmp_key1) == 5)
				memcpy(pWepKey->KeyMaterial, tmp_key1, 5);
			else if (strlen(tmp_key1) == 10)
				AtoH(tmp_key1, pWepKey->KeyMaterial, 5);
			else if (strlen(tmp_key1) == 13)
				memcpy(pWepKey->KeyMaterial, tmp_key1, 13);
			else if (strlen(tmp_key1) == 26)
				AtoH(tmp_key1, pWepKey->KeyMaterial, 13);

			OidSetInformation(OID_802_11_ADD_WEP, s, RA0, pWepKey, pWepKey->Length);
			free(pWepKey);
		}

        //----------------------------------------------------------//
        //Key 2
        //----------------------------------------------------------//
        nKeyLen = strlen(tmp_key2);
        if (nKeyLen == 0) {
            removeKey.Length = sizeof(NDIS_802_11_REMOVE_KEY);
            removeKey.KeyIndex = 1;
            for (j = 0; j < 6; j++)
                removeKey.BSSID[j] = 0xff;
            OidSetInformation(OID_802_11_REMOVE_KEY, s, RA0, &removeKey, removeKey.Length);
        } else if (strcmp(tmp_key2, "0")) {
            if (nKeyLen == 10)
                nKeyLen = 5;
            else if (nKeyLen == 26)
                nKeyLen = 13;

            lBufLen = sizeof(NDIS_802_11_WEP) + nKeyLen - 1;
            // Allocate Resource
            pWepKey = (PNDIS_802_11_WEP)malloc(lBufLen);
            pWepKey->Length = lBufLen;
            pWepKey->KeyLength = nKeyLen;
            pWepKey->KeyIndex = 1;

            if (tmp_defaultkeyid == 2)
                pWepKey->KeyIndex |= 0x80000000;

            if (strlen(tmp_key2) == 5)
                memcpy(pWepKey->KeyMaterial, tmp_key2, 5);
            else if (strlen(tmp_key2) == 10)
                AtoH(tmp_key2, pWepKey->KeyMaterial, 5);
            else if (strlen(tmp_key2) == 13)
                memcpy(pWepKey->KeyMaterial, tmp_key2, 13);
            else if (strlen(tmp_key2) == 26)
                AtoH(tmp_key2, pWepKey->KeyMaterial, 13);

            OidSetInformation(OID_802_11_ADD_WEP, s, RA0, pWepKey, pWepKey->Length);
            free(pWepKey);
        }
        //----------------------------------------------------------//
        //Key 3
        //----------------------------------------------------------//
        nKeyLen = strlen(tmp_key3);
        if (nKeyLen == 0) {
            removeKey.Length = sizeof(NDIS_802_11_REMOVE_KEY);
            removeKey.KeyIndex = 2;
            for(j = 0; j < 6; j++)
                removeKey.BSSID[j] = 0xff;
            OidSetInformation(OID_802_11_REMOVE_KEY, s, RA0, &removeKey, removeKey.Length);
        } else if (strcmp(tmp_key3, "0")) {
            if (nKeyLen == 10)
                nKeyLen = 5;
            else if (nKeyLen == 26)
                nKeyLen = 13;

            lBufLen = sizeof(NDIS_802_11_WEP) + nKeyLen - 1;
            // Allocate Resource
            pWepKey = (PNDIS_802_11_WEP)malloc(lBufLen);
            pWepKey->Length = lBufLen;
            pWepKey->KeyLength = nKeyLen;
            pWepKey->KeyIndex = 2;
            if (tmp_defaultkeyid == 3)
                pWepKey->KeyIndex |= 0x80000000;
            
            if (strlen(tmp_key3) == 5)
                memcpy(pWepKey->KeyMaterial, tmp_key3, 5);
            else if (strlen(tmp_key3) == 10)
                AtoH(tmp_key3, pWepKey->KeyMaterial, 5);
            else if (strlen(tmp_key3) == 13)
                memcpy(pWepKey->KeyMaterial, tmp_key3, 13);
            else if (strlen(tmp_key3) == 26)
                AtoH(tmp_key3, pWepKey->KeyMaterial, 13);

			OidSetInformation(OID_802_11_ADD_WEP, s, RA0, pWepKey, pWepKey->Length);
            free(pWepKey);
        }
        //----------------------------------------------------------//
        //Key 4
        //----------------------------------------------------------//
        nKeyLen = strlen(tmp_key4);
        if (nKeyLen == 0) {
            removeKey.Length = sizeof(NDIS_802_11_REMOVE_KEY);
            removeKey.KeyIndex = 3;
            for(j = 0; j < 6; j++)
                removeKey.BSSID[j] = 0xff;
            OidSetInformation(OID_802_11_REMOVE_KEY, s, RA0, &removeKey, removeKey.Length);
        } else if (strcmp(tmp_key4, "0")) {
            if (nKeyLen == 10)
                nKeyLen = 5;
            else if (nKeyLen == 26)
                nKeyLen = 13;

            lBufLen = sizeof(NDIS_802_11_WEP) + nKeyLen - 1;
            // Allocate Resource
            pWepKey = (PNDIS_802_11_WEP)malloc(lBufLen);
            pWepKey->Length = lBufLen;
            pWepKey->KeyLength = nKeyLen;
            pWepKey->KeyIndex = 3;
            if (tmp_defaultkeyid == 4)
                pWepKey->KeyIndex |= 0x80000000;
           
            if (strlen(tmp_key4) == 5)
                memcpy(pWepKey->KeyMaterial, tmp_key4, 5);
            else if (strlen(tmp_key4) == 10)
                AtoH(tmp_key4, pWepKey->KeyMaterial, 5);
            else if (strlen(tmp_key4) == 13)
                memcpy(pWepKey->KeyMaterial, tmp_key4, 13);
            else if (strlen(tmp_key4) == 26)
                AtoH(tmp_key4, pWepKey->KeyMaterial, 13);

			OidSetInformation(OID_802_11_ADD_WEP, s, RA0, pWepKey, pWepKey->Length);
			free(pWepKey);
        }
	}
	else if (tmp_auth == Ndis802_11AuthModeWPAPSK || tmp_auth == Ndis802_11AuthModeWPA2PSK || tmp_auth == Ndis802_11AuthModeWPANone) {
        nKeyLen = 32;
		lBufLen = (sizeof(NDIS_802_11_KEY) + nKeyLen - 1);
		// Allocate Resouce
		pKey = (PNDIS_802_11_KEY)malloc(lBufLen); // Don't use GMEM_ZEROINIT to get random key
		pKey->Length = lBufLen;
		pKey->KeyLength = nKeyLen;
		pKey->KeyIndex = 0x80000000;

		if (strlen(tmp_wpapsk) == 64) {
			AtoH(tmp_wpapsk, keyMaterial, 32);
			memcpy(pKey->KeyMaterial, keyMaterial, 32);		
		}
		else {
			PasswordHash(tmp_wpapsk, pSsid->Ssid, pSsid->SsidLength, keyMaterial);
			memcpy(pKey->KeyMaterial, keyMaterial, 32);
		}
		PassphraseBufLen = sizeof(NDIS_802_11_PASSPHRASE) + strlen(tmp_wpapsk) - 1;
		pPassPhrase=(PNDIS_802_11_PASSPHRASE)malloc(PassphraseBufLen);
		pPassPhrase->KeyLength = strlen(tmp_wpapsk);
		memcpy(pPassPhrase->KeyMaterial, tmp_wpapsk, pPassPhrase->KeyLength);
		OidSetInformation(OID_802_11_SET_PASSPHRASE, s, RA0, pPassPhrase, PassphraseBufLen);
		OidSetInformation(RT_OID_802_11_ADD_WPA, s, RA0, pKey, pKey->Length);
		free(pKey);
	}

	//step 3: SSID
	if (tmp_networktype == Ndis802_11IBSS ) // Ad hoc use SSID
	{
		ret = OidSetInformation(OID_802_11_SSID, s, RA0, pSsid, sizeof(NDIS_802_11_SSID));
		if (ret < 0)
			error(E_L, E_LOG, T("Set OID_802_11_SSID has error =%d, pSsid->Ssid=%s"), ret, pSsid->Ssid);
		else
			memcpy(&G_SSID, pSsid, sizeof(NDIS_802_11_SSID));
	}
	else
	{
		ret = OidSetInformation(OID_802_11_SSID, s, RA0, pSsid, sizeof(NDIS_802_11_SSID));
		if (ret < 0)
			error(E_L, E_LOG, T("Set OID_802_11_SSID has error =%d, pSsid->Ssid=%s"), ret, pSsid->Ssid);
		else
			memcpy(&G_SSID, pSsid, sizeof(NDIS_802_11_SSID));

		/*
		ret = OidSetInformation(OID_802_11_BSSID, s, RA0, &Bssid, 6);
		if (ret < 0) {
			error(E_L, E_LOG, "Set OID_802_11_BSSID has error =%d, \n", ret);
		} else 
		*/
		{
			memcpy(G_Bssid, Bssid, 6);
		}
		Sleep(1);
	}

	close(s);

	doSystem("wan.sh");		// restart wan.sh if needed (renew dhcp, pppoe etc)
}

/*
 * description: connect to AP according to the active profile
 */
void initStaConnection(void)
{
	NDIS_802_11_SSID Ssid;
	PRT_PROFILE_SETTING p = headerProfileSetting;

	if (p == NULL)
		return;
	while (p->Active == 0) {
		if (p->Next == NULL) {
			p = headerProfileSetting;
			break;
		}
		p = p->Next;
	}

	strcpy((char *)Ssid.Ssid ,(char *)p->SSID);
	Ssid.SsidLength = p->SsidLen;
	unsigned char Bssid[6];
	sta_connection(p->NetworkType, p->Authentication, p->Encryption, p->KeyDefaultId, &Ssid, Bssid, (char *)p->WpaPsk, (char *)p->Key1, (char *)p->Key2, (char *)p->Key3, (char *)p->Key4, p->PreamType, p->RTSCheck, p->RTS, p->FragmentCheck, p->Fragment, p->PSmode, p->Channel);
}

/*
 * description: return station radio status
 */
static int getStaProfile(int eid, webs_t wp, int argc, char_t **argv)
{
	char tmpImg[40];
	int i = 0, s;

	NDIS_802_11_SSID                SsidQuery;
	unsigned int                    ConnectStatus = 0;
	NDIS_802_11_WEP_STATUS          Encryp = Ndis802_11WEPDisabled;
	NDIS_802_11_AUTHENTICATION_MODE AuthenType = Ndis802_11AuthModeOpen;
	NDIS_802_11_NETWORK_INFRASTRUCTURE      NetworkType = Ndis802_11Infrastructure;

	initStaProfile();
	if (G_staProfileNum == 0)
		return 0;
	if (headerProfileSetting == NULL)
		return 0;

	currentProfileSetting = headerProfileSetting;
	do {
		memset(tmpImg, 0x00, sizeof(tmpImg));
		// check activate function for the profile
		if (currentProfileSetting->Active)
		{
			// get connected SSID
			s = socket(AF_INET, SOCK_DGRAM, 0);

			//step 1
			if (OidQueryInformation(OID_GEN_MEDIA_CONNECT_STATUS, s, RA0, &ConnectStatus, sizeof(ConnectStatus)) < 0) {
				error(E_L, E_LOG, T("Query OID_GEN_MEDIA_CONNECT_STATUS error!"));
				close(s);
				return 0;
			}

			//step 2
			if (OidQueryInformation(RT_OID_802_11_RADIO, s, RA0, &G_bRadio, sizeof(G_bRadio)) < 0) {
				error(E_L, E_LOG, T("Query RT_OID_802_11_RADIO error!"));
				close(s);
				return 0;
			}
			//fprintf(stderr,"ConnectStatus=%d\n", ConnectStatus );
			if (ConnectStatus == 1 && G_bRadio)
			{
				OidQueryInformation(OID_802_11_WEP_STATUS, s, RA0, &Encryp, sizeof(Encryp) );
				OidQueryInformation(OID_802_11_AUTHENTICATION_MODE, s, RA0, &AuthenType, sizeof(AuthenType));
				OidQueryInformation(OID_802_11_INFRASTRUCTURE_MODE, s, RA0, &NetworkType, sizeof(NetworkType));

				memset(&SsidQuery, 0x00, sizeof(SsidQuery));
				OidQueryInformation(OID_802_11_SSID, s, RA0, &SsidQuery, sizeof(SsidQuery));

				if (strcmp((char *)SsidQuery.Ssid, (char *)currentProfileSetting->SSID) == 0 &&
						currentProfileSetting->Encryption == Encryp &&
						currentProfileSetting->Authentication == AuthenType &&
						currentProfileSetting->NetworkType == NetworkType)
				{
					memcpy(&G_SSID, &SsidQuery, sizeof(NDIS_802_11_SSID));
					sprintf(tmpImg, "<img src=\"/graphics/checkmrk.gif\">");
				}
				else
					sprintf(tmpImg, "<img src=\"/graphics/uncheckmrk.gif\">");
				close(s);
			}
			else if (G_bRadio)
			{
				int tmp_auth, tmp_encry, tmp_defaultkeyid, tmp_networktype, tmp_preamtype, tmp_channel; //tmp_adhocmode,
				char tmp_wpapsk[65], tmp_key1[27], tmp_key2[27], tmp_key3[27], tmp_key4[27], tmp_bssid[13];
				char tmp_rtscheck=0, tmp_fragmentcheck=0;
				NDIS_802_11_RTS_THRESHOLD	tmp_rts;
				NDIS_802_11_FRAGMENTATION_THRESHOLD	tmp_fragment;
				NDIS_802_11_SSID			SSID;
				NDIS_802_11_POWER_MODE		tmp_psmode;

				memset(&SSID, 0x00, sizeof(SSID));
				bzero(tmp_bssid, sizeof(tmp_bssid));
				bzero(tmp_wpapsk, sizeof(tmp_wpapsk));
				bzero(tmp_key1, sizeof(tmp_key1));
				bzero(tmp_key2, sizeof(tmp_key2));
				bzero(tmp_key3, sizeof(tmp_key3));
				bzero(tmp_key4, sizeof(tmp_key4));
				memset(tmp_wpapsk, 0x00, sizeof(tmp_wpapsk));

				SSID.SsidLength = currentProfileSetting->SsidLen;
				memcpy(SSID.Ssid, (const void *)currentProfileSetting->SSID, currentProfileSetting->SsidLen);

				tmp_networktype = currentProfileSetting->NetworkType;
				tmp_auth  = currentProfileSetting->Authentication;
				tmp_encry = currentProfileSetting->Encryption;
				tmp_preamtype = currentProfileSetting->PreamType;
				tmp_rts = currentProfileSetting->RTS;
				tmp_rtscheck = currentProfileSetting->RTSCheck;
				tmp_fragment = currentProfileSetting->Fragment;
				tmp_fragmentcheck = currentProfileSetting->FragmentCheck;
				tmp_psmode = currentProfileSetting->PSmode;
				tmp_channel = currentProfileSetting->Channel;
				tmp_defaultkeyid = currentProfileSetting->KeyDefaultId;

				//strncpy(tmp_wpapsk, selectedProfileSetting->WpaPsk, 63);
				sprintf((char *)tmp_wpapsk, "%s", currentProfileSetting->WpaPsk);
				strcpy(tmp_key1, (char *)currentProfileSetting->Key1);
				strcpy(tmp_key2, (char *)currentProfileSetting->Key2);
				strcpy(tmp_key3, (char *)currentProfileSetting->Key3);
				strcpy(tmp_key4, (char *)currentProfileSetting->Key4);

	unsigned char Bssid[6];
#ifdef WPA_SUPPLICANT_SUPPORT
				if (currentProfileSetting->Authentication == Ndis802_11AuthModeWPA ||
						currentProfileSetting->Authentication == Ndis802_11AuthModeWPA2 ||
						currentProfileSetting->Authentication == Ndis802_11AuthModeMax )//802.1x
				{
					char tmp_key[27];
					if (tmp_defaultkeyid == 1) // 1~4
						strcpy(tmp_key, tmp_key1);
					else if (tmp_defaultkeyid == 2)
						strcpy(tmp_key, tmp_key2);
					else if (tmp_defaultkeyid == 3)
						strcpy(tmp_key, tmp_key3);
					else if (tmp_defaultkeyid == 4)
						strcpy(tmp_key, tmp_key4);

					conf_WPASupplicant((char *)currentProfileSetting->SSID, currentProfileSetting->KeyMgmt, currentProfileSetting->EAP, (char *)currentProfileSetting->Identity, (char *)currentProfileSetting->Password, (char *)currentProfileSetting->CACert, (char *)currentProfileSetting->ClientCert, (char *)currentProfileSetting->PrivateKey, (char *)currentProfileSetting->PrivateKeyPassword, tmp_key, currentProfileSetting->KeyDefaultId-1, currentProfileSetting->Encryption, currentProfileSetting->Tunnel, currentProfileSetting->Authentication);
				}
				else
#endif
					sta_connection(tmp_networktype, tmp_auth, tmp_encry, tmp_defaultkeyid, &SSID, Bssid, tmp_wpapsk, tmp_key1, tmp_key2, tmp_key3, tmp_key4, tmp_preamtype, tmp_rtscheck, tmp_rts, tmp_fragmentcheck, tmp_fragment, tmp_psmode, tmp_channel);

				/*NDIS_802_11_SSID SSID;
				  memset(&SSID, 0x00, sizeof(SSID));
				  strcpy((char *)SSID.Ssid ,(char *)currentProfileSetting->SSID);
				  SSID.SsidLength = strlen((char *)currentProfileSetting->SSID);
				  OidSetInformation(OID_802_11_SSID, s, RA0, &SSID, sizeof(NDIS_802_11_SSID));*/
				sprintf(tmpImg, "<img src=\"/graphics/uncheckmrk.gif\">");
			}
			else
				sprintf(tmpImg, "<img src=\"/graphics/uncheckmrk.gif\">");
			close(s);
		}

		websWrite(wp, "<tr>");

		// Radio
		websWrite(wp, "<td><input type=radio name=selectedProfile value=%d onClick=\"selectedProfileChange()\">%s</td>",
				i+1, tmpImg);

		// Profile 
		websWrite(wp, "<td>%s</td>", currentProfileSetting->Profile);
		websWrite(wp, "<td>%s</td>", currentProfileSetting->SSID);

		// Channel
		if (currentProfileSetting->Channel <= 0)
			websWrite(wp, "<td>%s</td>", "Auto");
		else
			websWrite(wp, "<td>%d</td>", currentProfileSetting->Channel);

		// Auth
		if (currentProfileSetting->Authentication == Ndis802_11AuthModeOpen)
			websWrite(wp, "<td>%s</td>","OPEN");
		else if (currentProfileSetting->Authentication == Ndis802_11AuthModeShared)
			websWrite(wp, "<td>%s</td>", "SHARED");
		else if (currentProfileSetting->Authentication == Ndis802_11AuthModeWPAPSK)
			websWrite(wp, "<td>%s</td>", "WPA-PSK");
		else if (currentProfileSetting->Authentication == Ndis802_11AuthModeWPA2PSK)
			websWrite(wp, "<td>%s</td>", "WPA2-PSK");
		else if (currentProfileSetting->Authentication == Ndis802_11AuthModeWPANone)
			websWrite(wp, "<td>%s</td>", "WPA-NONE");
		else if (currentProfileSetting->Authentication == Ndis802_11AuthModeWPA)
			websWrite(wp, "<td>%s</td>", "WPA");
		else if (currentProfileSetting->Authentication == Ndis802_11AuthModeWPA2)
			websWrite(wp, "<td>%s</td>", "WPA2");
		else if (currentProfileSetting->Authentication == Ndis802_11AuthModeMax) //802.1x
			websWrite(wp, "<td>%s</td>", "OPEN");
		else
			websWrite(wp, "<td>%s</td>", "unknown");

		// Encryption
		if (currentProfileSetting->Encryption == Ndis802_11WEPEnabled)
			websWrite(wp, "<td>%s</td>", "WEP");
		else if (currentProfileSetting->Encryption == Ndis802_11WEPDisabled)
			websWrite(wp, "<td>%s</td>", "NONE");
		else if (currentProfileSetting->Encryption == Ndis802_11Encryption2Enabled)
			websWrite(wp, "<td>%s</td>", "TKIP");
		else if (currentProfileSetting->Encryption == Ndis802_11Encryption3Enabled)
			websWrite(wp, "<td>%s</td>", "AES");
		else
			websWrite(wp, "<td>%s</td>", "unknown");

		// NetworkType
		if (currentProfileSetting->NetworkType == Ndis802_11Infrastructure)
			websWrite(wp, "<td>%s</td>", "Infrastructure");
		else
			websWrite(wp, "<td>%s</td>", "Ad Hoc");

		websWrite(wp, "</tr>\n");
		currentProfileSetting = currentProfileSetting->Next;
		i++;
	} while (currentProfileSetting != NULL );
	return 0;
}

/*
 * arguments:   type - 1 ~ hmm
 * description: write selected profile data
 */
static int getStaProfileData(int eid, webs_t wp, int argc, char_t **argv)
{
	int type;

	if (selectedProfileSetting == NULL)
		return websWrite(wp, "0");
	if (ejArgs(argc, argv, T("%d"), &type) != 1) {
		return websWrite(wp, " ");
	}

	switch (type)
	{
		case 1: //profile name
			if (selectedProfileSetting->Profile == NULL)
				return websWrite(wp, "none");
			return websWrite(wp, "%s", selectedProfileSetting->Profile);
		case 2: //ssid
			if (selectedProfileSetting->SSID == NULL)
				return websWrite(wp, "none");
			return websWrite(wp, "%s", selectedProfileSetting->SSID);
		case 3: //network type
			return websWrite(wp, "%d", selectedProfileSetting->NetworkType);
		case 4: //power saving mode
			if (selectedProfileSetting->PSmode == Ndis802_11PowerModeCAM)
				return websWrite(wp, "0");
			return websWrite(wp, "1");
		case 5: //preamble type
			if (selectedProfileSetting->PreamType == Rt802_11PreambleAuto)
				return websWrite(wp, "0");
			return websWrite(wp, "1");
		case 6: //RTS check
			return websWrite(wp, "%d", selectedProfileSetting->RTSCheck);
		case 7: //RTS
			return websWrite(wp, "%d", selectedProfileSetting->RTS);
		case 8: //fragment check
			return websWrite(wp, "%d", selectedProfileSetting->FragmentCheck);
		case 9: //fragment
			return websWrite(wp, "%d", selectedProfileSetting->Fragment);
		case 10: //encryp
			return websWrite(wp, "%d", selectedProfileSetting->Encryption);
		case 11: //authentication
			return websWrite(wp, "%d", selectedProfileSetting->Authentication);
		case 12: //key1
			if (selectedProfileSetting->Key1 == NULL || strcmp(selectedProfileSetting->Key1, "0") == 0)
				return websWrite(wp, "");
			return websWrite(wp, "%s", selectedProfileSetting->Key1);
		case 13: //key2
			if (selectedProfileSetting->Key2 == NULL || strcmp(selectedProfileSetting->Key2, "0") == 0)
				return websWrite(wp, "");
			return websWrite(wp, "%s", selectedProfileSetting->Key2);
		case 14: //key3
			if (selectedProfileSetting->Key3 == NULL || strcmp(selectedProfileSetting->Key3, "0") == 0)
				return websWrite(wp, "");
			return websWrite(wp, "%s", selectedProfileSetting->Key3);
		case 15: //key4
			if (selectedProfileSetting->Key4 == NULL || strcmp(selectedProfileSetting->Key4, "0") == 0)
				return websWrite(wp, "");
			return websWrite(wp, "%s", selectedProfileSetting->Key4);
		case 16: //key1 type
			return websWrite(wp, "%d", selectedProfileSetting->Key1Type);
		case 17: //key2 type
			return websWrite(wp, "%d", selectedProfileSetting->Key2Type);
		case 18: //key3 type
			return websWrite(wp, "%d", selectedProfileSetting->Key3Type);
		case 19: //key4 type
			return websWrite(wp, "%d", selectedProfileSetting->Key4Type);
		case 20: //key1 length
			return websWrite(wp, "%d", selectedProfileSetting->Key1Length);
		case 21: //key2 length
			return websWrite(wp, "%d", selectedProfileSetting->Key2Length);
		case 22: //key3 length
			return websWrite(wp, "%d", selectedProfileSetting->Key3Length);
		case 23: //key4 length
			return websWrite(wp, "%d", selectedProfileSetting->Key4Length);
		case 24: //key default id
			return websWrite(wp, "%d", selectedProfileSetting->KeyDefaultId);
		case 25: //passphrase
			if (selectedProfileSetting->WpaPsk == NULL || strcmp(selectedProfileSetting->WpaPsk, "0") == 0)
				return websWrite(wp, "");
			return websWrite(wp, "%s", selectedProfileSetting->WpaPsk);
#ifdef WPA_SUPPLICANT_SUPPORT
		case 26: //key mgmt
			return websWrite(wp, "%d", selectedProfileSetting->KeyMgmt);
		case 27: //eap
			return websWrite(wp, "%d", selectedProfileSetting->EAP);
		case 28: //cert id
			if (selectedProfileSetting->Identity == NULL || strcmp(selectedProfileSetting->Identity, "0") == 0)
				return websWrite(wp, "");
			return websWrite(wp, "%s", selectedProfileSetting->Identity);
		case 29: //ca cert
			if (selectedProfileSetting->CACert == NULL || strcmp(selectedProfileSetting->CACert, "0") == 0)
				return websWrite(wp, "");
			return websWrite(wp, "%s", selectedProfileSetting->CACert);
		case 30: //client cert
			if (selectedProfileSetting->ClientCert == NULL || strcmp(selectedProfileSetting->ClientCert, "0") == 0)
				return websWrite(wp, "");
			return websWrite(wp, "%s", selectedProfileSetting->ClientCert);
		case 31: //private key path
			if (selectedProfileSetting->PrivateKey == NULL || strcmp(selectedProfileSetting->PrivateKey, "0") == 0)
				return websWrite(wp, "");
			return websWrite(wp, "%s", selectedProfileSetting->PrivateKey);
		case 32: //private key passwd
			if (selectedProfileSetting->PrivateKeyPassword == NULL || strcmp(selectedProfileSetting->PrivateKeyPassword, "0") == 0)
				return websWrite(wp, "");
			return websWrite(wp, "%s", selectedProfileSetting->PrivateKeyPassword);
		case 33: //passwd
			if (selectedProfileSetting->Password == NULL || strcmp(selectedProfileSetting->Password, "0") == 0)
				return websWrite(wp, "");
			return websWrite(wp, "%s", selectedProfileSetting->Password);
		case 34: //tunnel
			return websWrite(wp, "%d", selectedProfileSetting->Tunnel);
#else
		case 26:
		case 27:
		case 34:
			return websWrite(wp, "0");
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 33:
			return websWrite(wp, "");
#endif
		case 35: //channel
			return websWrite(wp, "%d", selectedProfileSetting->Channel);
	}
	return websWrite(wp, "unknown");
}

/*
 * description: return station radio status
 */
static int getStaRadioStatus(int eid, webs_t wp, int argc, char_t **argv)
{
	unsigned long RadioStatus=0;
	int s, ret;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	ret = OidQueryInformation(RT_OID_802_11_RADIO, s, RA0, &RadioStatus, sizeof(RadioStatus));
	if (ret < 0)
		error(E_L, E_LOG, T("getStaRadioStatus: Query RT_OID_802_11_RADIO failed!"));
	close(s);
	if (RadioStatus == 1)
		ejSetResult(eid, "1");
	else
		ejSetResult(eid, "0");
	return 0;
}

/*
 * description: write station link Rx throughput
 */
static int getStaRxThroughput(int eid, webs_t wp, int argc, char_t **argv)
{
	RT_802_11_LINK_STATUS LinkStatus;
	int s;
	char tmp[8];

	if (G_ConnectStatus == NdisMediaStateDisconnected) {
		return websWrite(wp, "0");
	}

	s = socket(AF_INET, SOCK_DGRAM, 0);
	// Get Link Status Info from driver
	OidQueryInformation(RT_OID_802_11_QUERY_LINK_STATUS, s, RA0, &LinkStatus, sizeof(RT_802_11_LINK_STATUS));

	// Rx Throughput (KBits/sec) (LinkStatus.RxByteCount - m_lRxCount) * 8(bits) /1000 / 2(secs)
	if (m_lRxCount != 0)
		snprintf(tmp, 8, "%.1f", (double)(LinkStatus.RxByteCount - m_lRxCount) / 250);
	else
		snprintf(tmp, 8, "%.1f", (double)0);

	websWrite(wp, "%s", tmp);
	m_lRxCount = LinkStatus.RxByteCount;
	close(s);
	return 0;
}

/*
 * description: write station link Tx throughput
 */
static int getStaTxThroughput(int eid, webs_t wp, int argc, char_t **argv)
{
	RT_802_11_LINK_STATUS LinkStatus;
	int s;
	char tmp[8];

	if (G_ConnectStatus == NdisMediaStateDisconnected) {
		return websWrite(wp, "0");
	}

	s = socket(AF_INET, SOCK_DGRAM, 0);
	// Get Link Status Info from driver
	OidQueryInformation(RT_OID_802_11_QUERY_LINK_STATUS, s, RA0, &LinkStatus, sizeof(RT_802_11_LINK_STATUS));

	// Tx Throughput (KBits/sec) (LinkStatus.TxByteCount - m_lTxCount) * 8(bits) /1000 / 2(secs)
	if (m_lTxCount != 0)
		snprintf(tmp, 8, "%.1f", (double)(LinkStatus.TxByteCount - m_lTxCount) / 250);
	else
		snprintf(tmp, 8, "%.1f", (double)0);

	websWrite(wp, "%s", tmp);
	m_lTxCount = LinkStatus.TxByteCount;
	close(s);
	return 0;
}

static int getRSSI(webs_t wp, int antenna)
{
	RT_802_11_LINK_STATUS LinkStatus;
	int s;
	unsigned int nSigQua;
	long RSSI;
	int oid[3] = {RT_OID_802_11_RSSI, RT_OID_802_11_RSSI_1, RT_OID_802_11_RSSI_2};

	if (G_ConnectStatus == NdisMediaStateDisconnected) {
		websWrite(wp, "0%%");
		return 0;
	}

	s = socket(AF_INET, SOCK_DGRAM, 0);
	// Get Link Status Info from driver
	OidQueryInformation(RT_OID_802_11_QUERY_LINK_STATUS, s, RA0, &LinkStatus, sizeof(RT_802_11_LINK_STATUS));

	// Signal Strength

	// Get Rssi Value from driver
	OidQueryInformation(oid[antenna], s, RA0, &RSSI, sizeof(RSSI));

	if (RSSI > 20 || RSSI < -200)
		return websWrite(wp, "None");

	// Use convert formula to getSignal Quality
	nSigQua = ConvertRssiToSignalQuality(RSSI);
	if (m_nSigQua[antenna] != 0)
		nSigQua = (unsigned int)((m_nSigQua[antenna] + nSigQua) / 2.0 + 0.5);

	close(s);

	m_nSigQua[antenna] = nSigQua;
	if (nSigQua > 70) {
		if (G_bdBm_ischeck == 1) { //checked
			return websWrite(wp, "Good &nbsp;&nbsp;&nbsp;&nbsp; %ld dBm", RSSI);
		}
		else {
			return websWrite(wp, "Good &nbsp;&nbsp;&nbsp;&nbsp; %d%%", nSigQua);
		}
	}
	else if (nSigQua > 40) {
		if (G_bdBm_ischeck == 1) { //checked
			return websWrite(wp, "Normal &nbsp;&nbsp;&nbsp;&nbsp; %ld dBm", RSSI);
		}
		else {
			return websWrite(wp, "Normal &nbsp;&nbsp;&nbsp;&nbsp; %d%%", nSigQua);
		}
	}
	else {
		if (G_bdBm_ischeck == 1) { //checked
			return websWrite(wp, "Weak &nbsp;&nbsp;&nbsp;&nbsp; %ld dBm", RSSI);
		}
		else {
			return websWrite(wp, "Weak &nbsp;&nbsp;&nbsp;&nbsp; %d%%", nSigQua);
		}
	}
}

/*
 * description: write station signal strength
 */
static int getStaSignalStrength(int eid, webs_t wp, int argc, char_t **argv)
{
	return getRSSI(wp, 0);
}

/*
 * description: write station signal strength
 */
static int getStaSignalStrength_1(int eid, webs_t wp, int argc, char_t **argv)
{
	return getRSSI(wp, 1);
}

/*
 * description: write station signal strength
 */
static int getStaSignalStrength_2(int eid, webs_t wp, int argc, char_t **argv)
{
	return getRSSI(wp, 2);
}

/*
 * description: write station SNR
 */
static int getStaSNR(int eid, webs_t wp, int argc, char_t **argv)
{
	int s, n, ret;
	unsigned long SNR;

	if (ejArgs(argc, argv, T("%d"), &n) < 1) {
		return websWrite(wp, T("Insufficient args\n"));
	}
	if (G_ConnectStatus == NdisMediaStateDisconnected) {
		return  websWrite(wp, "n/a");
	}

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (n == 0)
		ret = OidQueryInformation(RT_OID_802_11_SNR_0, s, RA0, &SNR, sizeof(SNR));
	else if (n == 1)
		ret = OidQueryInformation(RT_OID_802_11_SNR_1, s, RA0, &SNR, sizeof(SNR));
	else if (n == 2)
		ret = OidQueryInformation(RT_OID_802_11_SNR_2, s, RA0, &SNR, sizeof(SNR));
	else
		ret = -1;
	close(s);

	//fprintf(stderr, "SNR%d = %ld\n", n, SNR);
	if (ret < 0)
		return websWrite(wp, "n/a");
	else
		return websWrite(wp, "%ld", SNR);
}

/*
 * description: write station statistics Rx CRC error
 */
static int getStaStatsRxCRCErr(int eid, webs_t wp, int argc, char_t **argv)
{
	NDIS_802_11_STATISTICS  Statistics;

	int s;
	memset(&Statistics, 0x00, sizeof(Statistics));

	s = socket(AF_INET, SOCK_DGRAM, 0);
	// Frames Received With CRC Error
	if (OidQueryInformation(OID_802_11_STATISTICS, s, RA0, &Statistics, sizeof(Statistics)) >= 0)
		websWrite(wp, "%ld", Statistics.FCSErrorCount.QuadPart);
	else
		websWrite(wp, "0");

	close(s);
	return 0;
}

/*
 * description: write station statistics Rx duplicate
 */
static int getStaStatsRxDup(int eid, webs_t wp, int argc, char_t **argv)
{
	NDIS_802_11_STATISTICS  Statistics;

	int s;
	s = socket(AF_INET, SOCK_DGRAM, 0);

	memset(&Statistics, 0x00, sizeof(Statistics));
	// Duplicate Frames Received
	if (OidQueryInformation(OID_802_11_STATISTICS, s, RA0, &Statistics, sizeof(Statistics)) >= 0)
		websWrite(wp, "%ld", Statistics.FrameDuplicateCount.QuadPart);
	else
		websWrite(wp, "0");

	close(s);
	return 0;
}

/*
 * description: write station statistics Rx ok
 */
static int getStaStatsRxOk(int eid, webs_t wp, int argc, char_t **argv)
{
	unsigned long lRcvOk = 0;
	int s;

	s = socket(AF_INET, SOCK_DGRAM, 0);

	// Frames Received Successfully
	if (OidQueryInformation(OID_GEN_RCV_OK, s, RA0, &lRcvOk, sizeof(lRcvOk)) >= 0)
		websWrite(wp, "%ld", lRcvOk);
	else
		websWrite(wp, "0");

	close(s);
	return 0;
}

/*
 * description: write station statistics Rx no buffer
 */
static int getStaStatsRxNoBuf(int eid, webs_t wp, int argc, char_t **argv)
{
	unsigned long lRcvNoBuf = 0;
	int s;

	s = socket(AF_INET, SOCK_DGRAM, 0);

	// Frames Dropped Due To Out-of-Resource
	if (OidQueryInformation(OID_GEN_RCV_NO_BUFFER, s, RA0, &lRcvNoBuf, sizeof(lRcvNoBuf)) >= 0)
		websWrite(wp, "%ld", lRcvNoBuf);
	else
		websWrite(wp, "0");

	close(s);
	return 0;
}

/*
 * description: write station statistics Tx all
 */
static int getStaStatsTx(int eid, webs_t wp, int argc, char_t **argv)
{
	NDIS_802_11_STATISTICS  Statistics;
	char  tmpStatisics[16];
	int   s, ret=0;

	s = socket(AF_INET, SOCK_DGRAM, 0);

	memset(&tmpStatisics, 0x00, sizeof(tmpStatisics));

	// Transmit Section
	memset(&Statistics, 0x00, sizeof(Statistics));
	ret = OidQueryInformation(OID_802_11_STATISTICS, s, RA0, &Statistics, sizeof(Statistics));
	close(s);
	if (ret >= 0) {
		// Frames Transmitted Successfully
		sprintf(tmpStatisics, "%8lld", Statistics.TransmittedFragmentCount.QuadPart);
		websWrite(wp, "<tr><td class=\"head\">%s</td><td>%s</td></tr>", "Frames Transmitted Successfully", tmpStatisics);

		// Frames Transmitted Successfully  Without Retry(s)
		sprintf(tmpStatisics, "%8lld", Statistics.TransmittedFragmentCount.QuadPart - Statistics.RetryCount.QuadPart);
		websWrite(wp, "<tr><td class=\"head\">%s</td><td>%s</td></tr>", "Frames Transmitted Successfully Without Retry", tmpStatisics);

		// Frames Transmitted Successfully After Retry(s)
		sprintf(tmpStatisics, "%8lld", Statistics.RetryCount.QuadPart);
		websWrite(wp, "<tr><td class=\"head\">%s</td><td>%s</td></tr>", "Frames Transmitted Successfully After Retry(s)", tmpStatisics);

		// Frames Fail To Receive ACK After All Retries
		sprintf(tmpStatisics, "%8lld", Statistics.FailedCount.QuadPart);
		websWrite(wp, "<tr><td class=\"head\">%s</td><td>%s</td></tr>", "Frames Fail To Receive ACK After All Retries", tmpStatisics);

		// RTS Frames Successfully Receive CTS
		sprintf(tmpStatisics, "%8lld", Statistics.RTSSuccessCount.QuadPart);
		websWrite(wp, "<tr><td class=\"head\">%s</td><td>%s</td></tr>", "RTS Frames Sucessfully Receive CTS", tmpStatisics);

		// RTS Frames Fail To Receive CTS
		sprintf(tmpStatisics, "%8lld", Statistics.RTSFailureCount.QuadPart);
		websWrite(wp, "<tr><td class=\"head\">%s</td><td>%s</td></tr>", "RTS Frames Fail To Receive CTS", tmpStatisics);

		// Frames Received Successfully
		sprintf(tmpStatisics, "%8lld", Statistics.ReceivedFragmentCount.QuadPart);
		websWrite(wp, "<tr><td class=\"title\" colspan=2 id=statisticRx>Receive Statistics</td></tr>");
		websWrite(wp, "<tr><td class=\"head\">%s</td><td>%s</td></tr>", "Frames Received Successfully", tmpStatisics);
		return 0;
	}
	else
		return websWrite(wp, "<tr><td>no data</td></tr>");
}

static int myGetSuppAMode(void)
{
	unsigned long lBufLen = sizeof(NDIS_802_11_NETWORK_TYPE_LIST) + sizeof(NDIS_802_11_NETWORK_TYPE)*3 ;
	PNDIS_802_11_NETWORK_TYPE_LIST pNetworkTypeList = (PNDIS_802_11_NETWORK_TYPE_LIST) malloc(lBufLen);
	int i, s, G_bSupportAMode=0;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (OidQueryInformation(OID_802_11_NETWORK_TYPES_SUPPORTED, s, RA0, pNetworkTypeList, lBufLen) >= 0)
	{
		for (i = 0 ; i < pNetworkTypeList->NumberOfItems ; i++)
		{
			if ( pNetworkTypeList->NetworkType[i] == Ndis802_11OFDM5 )
			{
				G_bSupportAMode = 1;
				break;
			}
		}
	}
	free(pNetworkTypeList);
	close(s);
	return G_bSupportAMode;
}

/*
 * description: return station support A band
 */
static int getStaSuppAMode(int eid, webs_t wp, int argc, char_t **argv)
{
	if (myGetSuppAMode() == 1)
		ejSetResult(eid, "1");
	else
		ejSetResult(eid, "0");
	return 0;
}

/*
 * description: write station wireless mode
 */
static int getStaWirelessMode(int eid, webs_t wp, int argc, char_t **argv)
{
	const char *mode_s = nvram_bufget(RT2860_NVRAM, "WirelessMode");
	int mode;
	int bSuppA = myGetSuppAMode();

	mode = (NULL == mode_s)? 0 : atoi(mode_s);
	websWrite(wp, "<option value=0 %s>802.11 B/G mixed mode</option>", (mode == 0)? "selected" : "");
	websWrite(wp, "<option value=1 %s>802.11 B Only</option>", (mode == 1)? "selected" : "");
	if (bSuppA) {
		websWrite(wp, "<option value=2 %s>802.11 A Only</option>", (mode == 2)? "selected" : "");
		websWrite(wp, "<option value=3 %s>802.11 A/B/G mixed mode</option>", (mode == 3)? "selected" : "");
	}
	websWrite(wp, "<option value=4 %s>802.11 G Only</option>", (mode == 4)? "selected" : "");
	websWrite(wp, "<option value=6 %s>802.11 N Only</option>", (mode == 6)? "selected" : "");
	websWrite(wp, "<option value=7 %s>802.11 GN mixed mode</option>", (mode == 7)? "selected" : "");
	if (bSuppA) {
		websWrite(wp, "<option value=8 %s>802.11 AN mixed mode</option>", (mode == 8)? "selected" : "");
	}
	websWrite(wp, "<option value=9 %s>802.11 B/G/N mixed mode</option>", (mode == 9)? "selected" : "");
	if (bSuppA) {
		websWrite(wp, "<option value=10 %s>802.11 A/G/N mixed mode</option>", (mode == 10)? "selected" : "");
		websWrite(wp, "<option value=5 %s>802.11 A/B/G/N mixed mode</option>", (mode == 5)? "selected" : "");
	}

	return 0;
}

/*
 * description: goform - add station profile
 */
static void addStaProfile(webs_t wp, char_t *path, char_t *query)
{
	RT_PROFILE_SETTING  tmpProfileSetting;
	int  securitymode=-1;
	char tmp_buffer[512];
	const char *wordlist = NULL;
	char_t *value;
	
	memset(&tmpProfileSetting, 0x00, sizeof(RT_PROFILE_SETTING));
	tmpProfileSetting.Next = NULL;

	//profile name
	// TODO: to tell profile is duplication with other
	value = websGetVar(wp, T("profile_name"), T(""));
	if (strlen(value) <= 0) {
		websError(wp, 500, T("No profile name given!"));
		return;
	}
	strcpy((char *)tmpProfileSetting.Profile, value);
	wordlist = nvram_bufget(RT2860_NVRAM, "staProfile");
	if (wordlist && strcmp(wordlist,"") != 0)
		sprintf(tmp_buffer, "%s;%s", wordlist, value);
	else
		sprintf(tmp_buffer, "%s", value);
	nvram_bufset(RT2860_NVRAM, "staProfile", tmp_buffer);

	//ssid
	value = websGetVar(wp, T("Ssid"), T(""));
	strcpy((char *)tmpProfileSetting.SSID, value);
	tmpProfileSetting.SsidLen = strlen((char *)tmpProfileSetting.SSID);
	wordlist = nvram_bufget(RT2860_NVRAM, "staSSID");
	if (wordlist && strcmp(wordlist,"") != 0)
		sprintf(tmp_buffer, "%s;%s", wordlist, value);
	else
		sprintf(tmp_buffer, "%s", value);
	nvram_bufset(RT2860_NVRAM, "staSSID", tmp_buffer);

	//network type
	value = websGetVar(wp, T("network_type"), T("1"));
	tmpProfileSetting.NetworkType = atoi(value);
	wordlist = nvram_bufget(RT2860_NVRAM, "staNetworkType");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.NetworkType);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.NetworkType);
	nvram_bufset(RT2860_NVRAM, "staNetworkType", tmp_buffer);

	//Adhoc mode
	if (tmpProfileSetting.NetworkType == Ndis802_11Infrastructure)
		tmpProfileSetting.AdhocMode = 0;
	else
		tmpProfileSetting.AdhocMode = 1;
	wordlist = nvram_bufget(RT2860_NVRAM, "staAdhocMode");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.AdhocMode);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.AdhocMode);
	nvram_bufset(RT2860_NVRAM, "staAdhocMode", tmp_buffer);

	//power saving mode
	value = websGetVar(wp, T("power_saving_mode"), T("0"));
	if (wordlist && strcmp(value, "0") ==0) //CAM
		tmpProfileSetting.PSmode = Ndis802_11PowerModeCAM;
	else
		tmpProfileSetting.PSmode = Ndis802_11PowerModeMAX_PSP;
	wordlist = nvram_bufget(RT2860_NVRAM, "staPSMode");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.PSmode);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.PSmode);
	nvram_bufset(RT2860_NVRAM, "staPSMode", tmp_buffer);

	//channel
	value = websGetVar(wp, T("channel"), T(""));
	if (tmpProfileSetting.NetworkType == Ndis802_11IBSS)
		tmpProfileSetting.Channel = atoi(value);
	else
		tmpProfileSetting.Channel = 0;
	wordlist = nvram_bufget(RT2860_NVRAM, "staChannel");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.Channel);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.Channel);
	nvram_bufset(RT2860_NVRAM, "staChannel", tmp_buffer);

	//b preamble type
	value = websGetVar(wp, T("b_premable_type"), T("0"));
	if (wordlist && strcmp(value, "0") == 0)
		tmpProfileSetting.PreamType = Rt802_11PreambleAuto;
	else
		tmpProfileSetting.PreamType = Rt802_11PreambleLong;
	wordlist = nvram_bufget(RT2860_NVRAM, "staPreamType");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.PreamType);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.PreamType);
	nvram_bufset(RT2860_NVRAM, "staPreamType", tmp_buffer);

	//rts threshold value
	if (websCompareVar(wp, T("rts_threshold"), T("on"))) {
		tmpProfileSetting.RTSCheck = 1;
		value = websGetVar(wp, T("rts_thresholdvalue"), T("2347"));
		tmpProfileSetting.RTS = atoi(value);
	}
	else {
		tmpProfileSetting.RTSCheck = 0;
		tmpProfileSetting.RTS = 2347;
	}
	wordlist = nvram_bufget(RT2860_NVRAM, "staRTSCheck");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.RTSCheck);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.RTSCheck);
	nvram_bufset(RT2860_NVRAM, "staRTSCheck", tmp_buffer);

	wordlist = nvram_bufget(RT2860_NVRAM, "staRTS");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.RTS);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.RTS);
	nvram_bufset(RT2860_NVRAM, "staRTS", tmp_buffer);

	//fragment threshold value
	if (websCompareVar(wp, T("fragment_threshold"), T("on"))) {
		tmpProfileSetting.FragmentCheck = 1;
		value = websGetVar(wp, T("fragment_thresholdvalue"), T("2346"));
		tmpProfileSetting.Fragment = atoi(value);
	}
	else {
		tmpProfileSetting.FragmentCheck = 0;
		tmpProfileSetting.Fragment = 2346;
	}
	wordlist = nvram_bufget(RT2860_NVRAM, "staFragmentCheck");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.FragmentCheck);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.FragmentCheck);
	nvram_bufset(RT2860_NVRAM, "staFragmentCheck", tmp_buffer);

	wordlist = nvram_bufget(RT2860_NVRAM, "staFragment");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.Fragment);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.Fragment);
	nvram_bufset(RT2860_NVRAM, "staFragment", tmp_buffer);

	//security policy (security_infra_mode or security_adhoc_mode)
	value = websGetVar(wp, T("security_infra_mode"), T(""));
	if (strcmp(value, "") != 0)
		securitymode = atoi(value);
	value = websGetVar(wp, T("security_adhoc_mode"), T(""));
	if (strcmp(value, "") != 0)
		securitymode = atoi(value);

	tmpProfileSetting.Authentication = securitymode;
	wordlist = nvram_bufget(RT2860_NVRAM, "staAuth");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.Authentication);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.Authentication);
	nvram_bufset(RT2860_NVRAM, "staAuth", tmp_buffer);

#ifdef WPA_SUPPLICANT_SUPPORT
	if (tmpProfileSetting.Authentication == Ndis802_11AuthModeWPA
			|| tmpProfileSetting.Authentication == Ndis802_11AuthModeWPA2)
	{
		tmpProfileSetting.KeyMgmt = Rtwpa_supplicantKeyMgmtWPAEAP;
	}
	else if (tmpProfileSetting.Authentication == Ndis802_11AuthModeMax) //802.1x
		tmpProfileSetting.KeyMgmt = Rtwpa_supplicantKeyMgmtIEEE8021X;
	else 
		tmpProfileSetting.KeyMgmt = Rtwpa_supplicantKeyMgmtNONE;

	wordlist = nvram_bufget(RT2860_NVRAM, "sta8021xKeyMgmt");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.KeyMgmt);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.KeyMgmt);
	nvram_bufset(RT2860_NVRAM, "sta8021xKeyMgmt", tmp_buffer);
#endif

	//wep key 1
	value = websGetVar(wp, T("wep_key_1"), T("0"));
	if (strcmp(value, "") == 0)
		strcpy((char *)tmpProfileSetting.Key1, "0");
	else
		strcpy((char *)tmpProfileSetting.Key1, value);
	wordlist = nvram_bufget(RT2860_NVRAM, "staKey1");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%s", wordlist, tmpProfileSetting.Key1);
	else
		sprintf(tmp_buffer, "%s", tmpProfileSetting.Key1);
	nvram_bufset(RT2860_NVRAM, "staKey1", tmp_buffer);

	//wep key 2
	value = websGetVar(wp, T("wep_key_2"), T("0"));
	if (strcmp(value, "") == 0)
		strcpy((char *)tmpProfileSetting.Key2, "0");
	else
		strcpy((char *)tmpProfileSetting.Key2, value);
	wordlist = nvram_bufget(RT2860_NVRAM, "staKey2");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%s", wordlist, tmpProfileSetting.Key2);
	else
		sprintf(tmp_buffer, "%s", tmpProfileSetting.Key2);
	nvram_bufset(RT2860_NVRAM, "staKey2", tmp_buffer);

	//wep key 3
	value = websGetVar(wp, T("wep_key_3"), T("0"));
	if (strcmp(value, "") == 0)
		strcpy((char *)tmpProfileSetting.Key3, "0");
	else
		strcpy((char *)tmpProfileSetting.Key3, value);
	wordlist = nvram_bufget(RT2860_NVRAM, "staKey3");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%s", wordlist, tmpProfileSetting.Key3);
	else
		sprintf(tmp_buffer, "%s", tmpProfileSetting.Key3);
	nvram_bufset(RT2860_NVRAM, "staKey3", tmp_buffer);

	//wep key 4
	value = websGetVar(wp, T("wep_key_4"), T("0"));
	if (strcmp(value, "") == 0)
		strcpy((char *)tmpProfileSetting.Key4, "0");
	else
		strcpy((char *)tmpProfileSetting.Key4, value);
	wordlist = nvram_bufget(RT2860_NVRAM, "staKey4");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%s", wordlist, tmpProfileSetting.Key4);
	else
		sprintf(tmp_buffer, "%s", tmpProfileSetting.Key4);
	nvram_bufset(RT2860_NVRAM, "staKey4", tmp_buffer);

	//wep key entry method
	value = websGetVar(wp, T("wep_key_entry_method"), T("0"));
	tmpProfileSetting.Key1Type = tmpProfileSetting.Key2Type =
		tmpProfileSetting.Key3Type = tmpProfileSetting.Key4Type =
		atoi(value);

	wordlist = nvram_bufget(RT2860_NVRAM, "staKey1Type");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.Key1Type);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.Key1Type);
	nvram_bufset(RT2860_NVRAM, "staKey1Type", tmp_buffer);

	wordlist = nvram_bufget(RT2860_NVRAM, "staKey2Type");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.Key2Type);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.Key2Type);
	nvram_bufset(RT2860_NVRAM, "staKey2Type", tmp_buffer);

	wordlist = nvram_bufget(RT2860_NVRAM, "staKey3Type");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.Key3Type);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.Key3Type);
	nvram_bufset(RT2860_NVRAM, "staKey3Type", tmp_buffer);

	wordlist = nvram_bufget(RT2860_NVRAM, "staKey4Type");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.Key4Type);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.Key4Type);
	nvram_bufset(RT2860_NVRAM, "staKey4Type", tmp_buffer);

	//wep key length
	value = websGetVar(wp, T("wep_key_length"), T("0"));
	tmpProfileSetting.Key1Length = tmpProfileSetting.Key2Length = 
		tmpProfileSetting.Key3Length = tmpProfileSetting.Key4Length =
		atoi(value);

	wordlist = nvram_bufget(RT2860_NVRAM, "staKey1Length");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.Key1Length);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.Key1Length);
	nvram_bufset(RT2860_NVRAM, "staKey1Length", tmp_buffer);

	wordlist = nvram_bufget(RT2860_NVRAM, "staKey2Length");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.Key2Length);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.Key2Length);
	nvram_bufset(RT2860_NVRAM, "staKey2Length", tmp_buffer);

	wordlist = nvram_bufget(RT2860_NVRAM, "staKey3Length");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.Key3Length);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.Key3Length);
	nvram_bufset(RT2860_NVRAM, "staKey3Length", tmp_buffer);

	wordlist = nvram_bufget(RT2860_NVRAM, "staKey4Length");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.Key4Length);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.Key4Length);
	nvram_bufset(RT2860_NVRAM, "staKey4Length", tmp_buffer);

	//wep default key
	value = websGetVar(wp, T("wep_default_key"), T("1"));
	tmpProfileSetting.KeyDefaultId= atoi(value);
	wordlist = nvram_bufget(RT2860_NVRAM, "staKeyDefaultId");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.KeyDefaultId);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.KeyDefaultId);
	nvram_bufset(RT2860_NVRAM, "staKeyDefaultId", tmp_buffer);

	//cipher, "staEncrypt"
	value = websGetVar(wp, T("cipher"), T(""));
	if (strcmp(value, "0") == 0) //TKIP
		tmpProfileSetting.Encryption = Ndis802_11Encryption2Enabled;
	else if (strcmp(value, "1") == 0) //AES
		tmpProfileSetting.Encryption = Ndis802_11Encryption3Enabled;
	else { //empty
		if (tmpProfileSetting.Authentication <= Ndis802_11AuthModeShared) {
			if (strlen((char *)tmpProfileSetting.Key1) > 1 || strlen((char *)tmpProfileSetting.Key2) > 1 ||
					strlen((char *)tmpProfileSetting.Key4) > 1 || strlen((char *)tmpProfileSetting.Key3) > 1)
			{
				tmpProfileSetting.Encryption = Ndis802_11WEPEnabled;
			}
			else
				tmpProfileSetting.Encryption = Ndis802_11WEPDisabled;
		}
		else if (tmpProfileSetting.Authentication == Ndis802_11AuthModeMax) //802.1x
			tmpProfileSetting.Encryption = Ndis802_11WEPEnabled;
		else
			tmpProfileSetting.Encryption = Ndis802_11WEPDisabled;
	}
	wordlist = nvram_bufget(RT2860_NVRAM, "staEncrypt");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.Encryption);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.Encryption);
	nvram_bufset(RT2860_NVRAM, "staEncrypt", tmp_buffer);

	//passphrase
	value = websGetVar(wp, T("passphrase"), T("0"));
	strcpy((char *)tmpProfileSetting.WpaPsk, value);
	wordlist = nvram_bufget(RT2860_NVRAM, "staWpaPsk");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%s", wordlist, tmpProfileSetting.WpaPsk);
	else
		sprintf(tmp_buffer, "%s", tmpProfileSetting.WpaPsk);
	nvram_bufset(RT2860_NVRAM, "staWpaPsk", tmp_buffer);

#ifdef WPA_SUPPLICANT_SUPPORT
	//cert auth from 1x, wpa
	tmpProfileSetting.EAP = Rtwpa_supplicantEAPNONE;
	value = websGetVar(wp, T("cert_auth_type_from_1x"), T(""));
	if (strcmp(value, "") != 0)
		tmpProfileSetting.EAP = (RT_WPA_SUPPLICANT_EAP)atoi(value);
	value = websGetVar(wp, T("cert_auth_type_from_wpa"), T(""));
	if (strcmp(value, "") != 0)
		tmpProfileSetting.EAP = (RT_WPA_SUPPLICANT_EAP)atoi(value);

	wordlist = nvram_bufget(RT2860_NVRAM, "sta8021xEAP");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.EAP);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.EAP);
	nvram_bufset(RT2860_NVRAM, "sta8021xEAP", tmp_buffer);

	//cert tunnel auth peap, ttls
	tmpProfileSetting.Tunnel = Rtwpa_supplicantTUNNENONE;
	value = websGetVar(wp, T("cert_tunnel_auth_peap"), T(""));
	if (strcmp(value, "") != 0)
		tmpProfileSetting.Tunnel = (RT_WPA_SUPPLICANT_TUNNEL)atoi(value);
	value = websGetVar(wp, T("cert_tunnel_auth_ttls"), T(""));
	if (strcmp(value, "") != 0)
		tmpProfileSetting.Tunnel = (RT_WPA_SUPPLICANT_TUNNEL)atoi(value);

	wordlist = nvram_bufget(RT2860_NVRAM, "sta8021xTunnel");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.Tunnel);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.Tunnel);
		nvram_bufset(RT2860_NVRAM, "sta8021xTunnel", tmp_buffer);

	//certificate identity
	value = websGetVar(wp, T("cert_id"), T("0"));
	sprintf((char *)tmpProfileSetting.Identity, "%s", value);
	wordlist = nvram_bufget(RT2860_NVRAM, "sta8021xIdentity");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%s", wordlist, tmpProfileSetting.Identity);
	else
		sprintf(tmp_buffer, "%s", tmpProfileSetting.Identity);
	nvram_bufset(RT2860_NVRAM, "sta8021xIdentity", tmp_buffer);

	//certificate password
	value = websGetVar(wp, T("cert_password"), T("0"));
	sprintf((char *)tmpProfileSetting.Password, "%s", value);
	wordlist = nvram_bufget(RT2860_NVRAM, "sta8021xPassword");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%s", wordlist, tmpProfileSetting.Password);
	else
		sprintf(tmp_buffer, "%s", tmpProfileSetting.Password);
	nvram_bufset(RT2860_NVRAM, "sta8021xPassword", tmp_buffer);

	//client certificate path
	value = websGetVar(wp, T("cert_client_cert_path"), T("0"));
	sprintf((char *)tmpProfileSetting.ClientCert, "%s", value);
	wordlist = nvram_bufget(RT2860_NVRAM, "sta8021xClientCert");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%s", wordlist, tmpProfileSetting.ClientCert);
	else
		sprintf(tmp_buffer, "%s", tmpProfileSetting.ClientCert);
	nvram_bufset(RT2860_NVRAM, "sta8021xClientCert", tmp_buffer);

	//private key path
	value = websGetVar(wp, T("cert_private_key_path"), T("0"));
	sprintf((char *)tmpProfileSetting.PrivateKey, "%s", value);
	wordlist = nvram_bufget(RT2860_NVRAM, "sta8021xPrivateKey");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%s", wordlist, tmpProfileSetting.PrivateKey);
	else
		sprintf(tmp_buffer, "%s", tmpProfileSetting.PrivateKey);
	nvram_bufset(RT2860_NVRAM, "sta8021xPrivateKey", tmp_buffer);

	//private key password
	value = websGetVar(wp, T("cert_private_key_password"), T("0"));
	sprintf((char *)tmpProfileSetting.PrivateKeyPassword, "%s", value);
	wordlist = nvram_bufget(RT2860_NVRAM, "sta8021xPrivateKeyPassword");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%s", wordlist, tmpProfileSetting.PrivateKeyPassword);
	else
		sprintf(tmp_buffer, "%s", tmpProfileSetting.PrivateKeyPassword);
	nvram_bufset(RT2860_NVRAM, "sta8021xPrivateKeyPassword", tmp_buffer);

	//CA cert path
	value = websGetVar(wp, T("cert_ca_cert_path"), T("0"));
	sprintf((char *)tmpProfileSetting.CACert, "%s", value);
	wordlist = nvram_bufget(RT2860_NVRAM, "sta8021xCACert");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%s", wordlist, tmpProfileSetting.CACert);
	else
		sprintf(tmp_buffer, "%s", tmpProfileSetting.CACert);
	nvram_bufset(RT2860_NVRAM, "sta8021xCACert", tmp_buffer);
#else
#define BUFSET(column, value) \
	wordlist = nvram_bufget(RT2860_NVRAM, column); \
	if (wordlist && strcmp(wordlist, "") != 0) \
		sprintf(tmp_buffer, "%s;%s", wordlist, value); \
	else \
		sprintf(tmp_buffer, "%s", value); \
	nvram_bufset(RT2860_NVRAM, column, tmp_buffer);

	BUFSET("sta8021xEAP", "7");
	BUFSET("sta8021xTunnel", "3");
	BUFSET("sta8021xKeyMgmt", "3");
	BUFSET("sta8021xIdentity", "0");
	BUFSET("sta8021xPassword", "0");
	BUFSET("sta8021xClientCert", "0");
	BUFSET("sta8021xPrivateKey", "0");
	BUFSET("sta8021xPrivateKeyPassword", "0");
	BUFSET("sta8021xCACert", "0");
#endif		

	//write into /etc/rt61sta.ui
	//writeProfileToFile(&tmpProfileSetting);

	tmpProfileSetting.Active = 0;
	wordlist = nvram_bufget(RT2860_NVRAM, "staActive");
	if (wordlist && strcmp(wordlist, "") != 0)
		sprintf(tmp_buffer, "%s;%d", wordlist, tmpProfileSetting.Active);
	else
		sprintf(tmp_buffer, "%d", tmpProfileSetting.Active);
	nvram_bufset(RT2860_NVRAM, "staActive", tmp_buffer);

	nvram_commit(RT2860_NVRAM);

	freeHeaderProfileSettings();
	headerProfileSetting = NULL;
	initStaProfile();
}

static void writeProfileToNvram()
{
	char tmp_buffer[512], tmp_data[8];

	if (headerProfileSetting == NULL)
		return;

	//profile name
	bzero(tmp_buffer, 512);
	currentProfileSetting = headerProfileSetting;
	do {
		strcat(tmp_buffer, (char *)currentProfileSetting->Profile);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staProfile", tmp_buffer);

	//ssid
	bzero(tmp_buffer, 512);
	currentProfileSetting = headerProfileSetting;
	do {
		strcat(tmp_buffer, (char *)currentProfileSetting->SSID);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staSSID", tmp_buffer);

	//NetworkType
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->NetworkType);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staNetworkType", tmp_buffer);

	//PSMode
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->PSmode);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staPSMode", tmp_buffer);

	//AdhocMode
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->AdhocMode);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staAdhocMode", tmp_buffer);

	//Channel
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->Channel);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staChannel", tmp_buffer);

	//PreamType
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->PreamType);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staPreamType", tmp_buffer);

	//RTSCheck
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->RTSCheck);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staRTSCheck", tmp_buffer);

	//FragmentCheck
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->FragmentCheck);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staFragmentCheck", tmp_buffer);

	//AdhocMode
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->RTS);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staRTS", tmp_buffer);

	//Fragment
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->Fragment);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staFragment", tmp_buffer);
	/* Security Policy */

	//Authentication
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->Authentication);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staAuth", tmp_buffer);

	//Encryption
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->Encryption);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staEncrypt", tmp_buffer);

	//KeyDefaultId
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->KeyDefaultId);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staKeyDefaultId", tmp_buffer);

	//Key1Type
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->Key1Type);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staKey1Type", tmp_buffer);

	//Key2Type
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->Key2Type);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staKey2Type", tmp_buffer);

	//Key3Type
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->Key3Type);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staKey3Type", tmp_buffer);

	//Key4Type
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->Key4Type);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staKey4Type", tmp_buffer);

	//Key1Lenght
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->Key1Length);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staKey1Length", tmp_buffer);

	//Key2Length
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->Key2Length);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staKey2Type", tmp_buffer);

	//Key3Length
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->Key3Length);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staKey3Length", tmp_buffer);

	//Key4Length
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->Key4Length);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staKey4Length", tmp_buffer);

	//Key1
	bzero(tmp_buffer, 512);
	currentProfileSetting = headerProfileSetting;
	do {
		strcat(tmp_buffer, (char *)currentProfileSetting->Key1);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staKey1", tmp_buffer);

	//Key2
	bzero(tmp_buffer, 512);
	currentProfileSetting = headerProfileSetting;
	do {
		strcat(tmp_buffer, (char *)currentProfileSetting->Key2);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staKey2", tmp_buffer);

	//Key3
	bzero(tmp_buffer, 512);
	currentProfileSetting = headerProfileSetting;
	do {
		strcat(tmp_buffer, (char *)currentProfileSetting->Key3);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staKey3", tmp_buffer);

	//Key4
	bzero(tmp_buffer, 512);
	currentProfileSetting = headerProfileSetting;
	do {
		strcat(tmp_buffer, (char *)currentProfileSetting->Key4);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staKey4", tmp_buffer);

	//WpaPsk
	bzero(tmp_buffer, 512);
	currentProfileSetting = headerProfileSetting;
	do {
		strcat(tmp_buffer, (char *)currentProfileSetting->WpaPsk);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staWpaPsk", tmp_buffer);

#ifdef WPA_SUPPLICANT_SUPPORT
	//Key Mgmt
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->KeyMgmt);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "sta8021xKeyMgmt", tmp_buffer);

	//EAP
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->EAP);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "sta8021xEAP", tmp_buffer);

	//Tunnel
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->Tunnel);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "sta8021xTunnel", tmp_buffer);

	//Identity
	bzero(tmp_buffer, 512);
	currentProfileSetting = headerProfileSetting;
	do {
		strcat(tmp_buffer, (char *)currentProfileSetting->Identity);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "sta8021xIdentity", tmp_buffer);

	//Password
	bzero(tmp_buffer, 512);
	currentProfileSetting = headerProfileSetting;
	do {
		strcat(tmp_buffer, (char *)currentProfileSetting->Password);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "sta8021xPassword", tmp_buffer);

	//Client Cert Path
	bzero(tmp_buffer, 512);
	currentProfileSetting = headerProfileSetting;
	do {
		strcat(tmp_buffer, (char *)currentProfileSetting->ClientCert);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "sta8021xClientCert", tmp_buffer);

	//Private Key
	bzero(tmp_buffer, 512);
	currentProfileSetting = headerProfileSetting;
	do {
		strcat(tmp_buffer, (char *)currentProfileSetting->PrivateKey);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "sta8021xPrivateKey", tmp_buffer);

	//Private Key Password
	bzero(tmp_buffer, 512);
	currentProfileSetting = headerProfileSetting;
	do {
		strcat(tmp_buffer, (char *)currentProfileSetting->PrivateKeyPassword);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "sta8021xPrivateKeyPassword", tmp_buffer);

	//CA CertPath
	bzero(tmp_buffer, 512);
	currentProfileSetting = headerProfileSetting;
	do {
		strcat(tmp_buffer, (char *)currentProfileSetting->CACert);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "sta8021xCACert", tmp_buffer);
#else
	bzero(tmp_buffer, 512);
	currentProfileSetting = headerProfileSetting;
	do {
		strcat(tmp_buffer, "7");
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "sta8021xEAP", tmp_buffer);

	bzero(tmp_buffer, 512);
	currentProfileSetting = headerProfileSetting;
	do {
		strcat(tmp_buffer, "3");
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "sta8021xTunnel", tmp_buffer);
	nvram_bufset(RT2860_NVRAM, "sta8021xKeyMgmt", tmp_buffer);

	bzero(tmp_buffer, 512);
	currentProfileSetting = headerProfileSetting;
	do {
		strcat(tmp_buffer, "0");
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "sta8021xIdentity", tmp_buffer);
	nvram_bufset(RT2860_NVRAM, "sta8021xPassword", tmp_buffer);
	nvram_bufset(RT2860_NVRAM, "sta8021xClientCert", tmp_buffer);
	nvram_bufset(RT2860_NVRAM, "sta8021xPrivateKey", tmp_buffer);
	nvram_bufset(RT2860_NVRAM, "sta8021xPrivateKeyPassword", tmp_buffer);
	nvram_bufset(RT2860_NVRAM, "sta8021xCACert", tmp_buffer);
#endif

	//Active
	bzero(tmp_buffer, 512);
	bzero(tmp_data, 8);
	currentProfileSetting = headerProfileSetting;
	do {
		sprintf(tmp_data, "%d", currentProfileSetting->Active);
		strcat(tmp_buffer, tmp_data);
		currentProfileSetting = currentProfileSetting->Next;
		if (currentProfileSetting != NULL)
			strcat(tmp_buffer, ";");	
	} while (currentProfileSetting != NULL);
	nvram_bufset(RT2860_NVRAM, "staActive", tmp_buffer);

	nvram_commit(RT2860_NVRAM);
}

/*
 * description: goform - reset statistics counters
 */
static void editStaProfile(webs_t wp, char_t *path, char_t *query)
{
	char_t *value;

	// step 1, modify info on selectedProfileSetting

	value = websGetVar(wp, T("profile_name"), T(""));
	if (strcmp(value, "") != 0)
		strcpy((char *)selectedProfileSetting->Profile, value);

	value = websGetVar(wp, T("Ssid"), T(""));
	if (strcmp(value, "") != 0)
		strcpy((char *)selectedProfileSetting->SSID, value);

	value = websGetVar(wp, T("network_type"), T(""));
	if (strcmp(value, "") != 0)
		selectedProfileSetting->NetworkType = atoi(value);
	if (selectedProfileSetting->NetworkType == Ndis802_11Infrastructure) {
		selectedProfileSetting->AdhocMode = 0;
		selectedProfileSetting->Channel = 0;
		selectedProfileSetting->PreamType = Rt802_11PreambleLong;
	}

	value = websGetVar(wp, T("power_saving_mode"), T(""));
	if (strcmp(value, "") != 0) {
		if (strcmp(value, "0") == 0)
			selectedProfileSetting->PSmode = Ndis802_11PowerModeCAM;
		else
			selectedProfileSetting->PSmode = Ndis802_11PowerModeMAX_PSP;
	}

	value = websGetVar(wp, T("channel"), T(""));
	if (strcmp(value, "") != 0) {
		if (selectedProfileSetting->NetworkType == Ndis802_11IBSS)
			selectedProfileSetting->Channel = atoi(value);
		else
			selectedProfileSetting->Channel = 0;
	}

	value = websGetVar(wp, T("b_premable_type"), T(""));
	if (strcmp(value, "") != 0) {
		if (strcmp(value, "0") == 0)
			selectedProfileSetting->PreamType = Rt802_11PreambleAuto;
		else
			selectedProfileSetting->PreamType = Rt802_11PreambleLong;
	}

	if (websCompareVar(wp, T("rts_threshold"), T("on"))) {
		selectedProfileSetting->RTSCheck = 1;
		value = websGetVar(wp, T("rts_thresholdvalue"), T(""));
		if (strcmp(value, "") != 0)
			selectedProfileSetting->RTS = atoi(value);
	}
	else {
		selectedProfileSetting->RTSCheck = 0;
		selectedProfileSetting->RTS = 2347;
	}

	if (websCompareVar(wp, T("fragment_threshold"), T("on"))) {
		selectedProfileSetting->FragmentCheck = 1;
		value = websGetVar(wp, T("fragment_thresholdvalue"), T(""));
		if (strcmp(value, "") != 0)
			selectedProfileSetting->Fragment = atoi(value);
	}
	else {
		selectedProfileSetting->FragmentCheck = 0;
		selectedProfileSetting->Fragment = 2346;
	}

	value = websGetVar(wp, T("security_infra_mode"), T(""));
	if (strcmp(value, "") != 0)
		selectedProfileSetting->Authentication = atoi(value);
	value = websGetVar(wp, T("security_adhoc_mode"), T(""));
	if (strcmp(value, "") != 0)
		selectedProfileSetting->Authentication = atoi(value);

#ifdef WPA_SUPPLICANT_SUPPORT
	if ( selectedProfileSetting->Authentication == Ndis802_11AuthModeWPA
			|| selectedProfileSetting->Authentication == Ndis802_11AuthModeWPA2)
	{
		selectedProfileSetting->KeyMgmt = Rtwpa_supplicantKeyMgmtWPAEAP;
	}
	else if (selectedProfileSetting->Authentication == Ndis802_11AuthModeMax) //802.1x
		selectedProfileSetting->KeyMgmt = Rtwpa_supplicantKeyMgmtIEEE8021X;
	else 
		selectedProfileSetting->KeyMgmt = Rtwpa_supplicantKeyMgmtNONE;
#endif

	value = websGetVar(wp, T("wep_key_1"), T("0"));
	strcpy((char *)selectedProfileSetting->Key1, value);
	value = websGetVar(wp, T("wep_key_2"), T("0"));
	strcpy((char *)selectedProfileSetting->Key2, value);
	value = websGetVar(wp, T("wep_key_3"), T("0"));
	strcpy((char *)selectedProfileSetting->Key3, value);
	value = websGetVar(wp, T("wep_key_4"), T("0"));
	strcpy((char *)selectedProfileSetting->Key4, value);

	value = websGetVar(wp, T("wep_key_entry_method"), T(""));
	if (strcmp(value, "") != 0) {
		selectedProfileSetting->Key1Type =
			selectedProfileSetting->Key1Type =
			selectedProfileSetting->Key1Type =
			selectedProfileSetting->Key1Type =
			atoi(value);
	}

	value = websGetVar(wp, T("wep_key_length"), T(""));
	if (strcmp(value, "") != 0) {
		selectedProfileSetting->Key1Length =
			selectedProfileSetting->Key2Length =
			selectedProfileSetting->Key3Length =
			selectedProfileSetting->Key4Length =
			atoi(value);
	}

	value = websGetVar(wp, T("wep_default_key"), T(""));
	if (strcmp(value, "") != 0)
		selectedProfileSetting->KeyDefaultId = atoi(value);

	value = websGetVar(wp, T("cipher"), T(""));
	if (strcmp(value, "") != 0) {
		if (0 == atoi(value)) //TKIP
			selectedProfileSetting->Encryption = Ndis802_11Encryption2Enabled;
		else //AES
			selectedProfileSetting->Encryption = Ndis802_11Encryption3Enabled;
	}

	value = websGetVar(wp, T("passphrase"), T(""));
	if (strcmp(value, "") != 0)
		strcpy((char *)selectedProfileSetting->WpaPsk, value);
	else
		strcpy((char *)selectedProfileSetting->WpaPsk, "0");

#ifdef WPA_SUPPLICANT_SUPPORT
	value = websGetVar(wp, T("cert_auth_type_from_1x"), T(""));
	if (strcmp(value, "") != 0)
		selectedProfileSetting->EAP = atoi(value);
	value = websGetVar(wp, T("cert_auth_type_from_wpa"), T(""));
	if (strcmp(value, "") != 0)
		selectedProfileSetting->EAP = atoi(value);

	value = websGetVar(wp, T("cert_tunnel_auth_peap"), T(""));
	if (strcmp(value, "") != 0)
		selectedProfileSetting->Tunnel = atoi(value);
	value = websGetVar(wp, T("cert_tunnel_auth_ttls"), T(""));
	if (strcmp(value, "") != 0)
		selectedProfileSetting->Tunnel = atoi(value);

	value = websGetVar(wp, T("cert_id"), T(""));
	if (strcmp(value, "") != 0)
		strcpy((char *)selectedProfileSetting->Identity, value);
	else
		strcpy((char *)selectedProfileSetting->Identity, "0");

	value = websGetVar(wp, T("cert_password"), T(""));
	if (strcmp(value, "") != 0)
		strcpy((char *)selectedProfileSetting->Password, value);
	else
		strcpy((char *)selectedProfileSetting->Password, "0");

	value = websGetVar(wp, T("cert_client_cert_path"), T(""));
	if (strcmp(value, "") != 0) {
		strcpy((char *)selectedProfileSetting->ClientCert, value);

		value = websGetVar(wp, T("cert_private_key_path"), T(""));
		if (strcmp(value, "") != 0)
			strcpy((char *)selectedProfileSetting->PrivateKey, value);
		else
			strcpy((char *)selectedProfileSetting->PrivateKey, "0");

		value = websGetVar(wp, T("cert_private_key_password"), T(""));
		if (strcmp(value, "") != 0)
			strcpy((char *)selectedProfileSetting->PrivateKeyPassword, value);
		else
			strcpy((char *)selectedProfileSetting->PrivateKeyPassword, "0");
	}
	else {
		strcpy((char *)selectedProfileSetting->ClientCert, "0");
		strcpy((char *)selectedProfileSetting->PrivateKey, "0");
		strcpy((char *)selectedProfileSetting->PrivateKeyPassword, "0");
	}

	value = websGetVar(wp, T("cert_ca_cert_path"), T(""));
	if (strcmp(value, "") != 0)
		strcpy((char *)selectedProfileSetting->CACert, value);
	else
		strcpy((char *)selectedProfileSetting->CACert, "0");
#endif				

	if (selectedProfileSetting->Authentication <= Ndis802_11AuthModeShared) {
		if( strlen((char *)selectedProfileSetting->Key1) > 1 || strlen((char *)selectedProfileSetting->Key2) > 1 ||
			strlen((char *)selectedProfileSetting->Key4) > 1 || strlen((char *)selectedProfileSetting->Key3) > 1)
		{
			selectedProfileSetting->Encryption = Ndis802_11WEPEnabled;
		}
		else
			selectedProfileSetting->Encryption = Ndis802_11WEPDisabled;
	}
	else if (selectedProfileSetting->Authentication == Ndis802_11AuthModeMax) //802.1x
		selectedProfileSetting->Encryption = Ndis802_11WEPEnabled;

	if (selectedProfileSetting->Active)
	{
		NDIS_802_11_SSID Ssid;
		memset(&Ssid, 0x00, sizeof(NDIS_802_11_SSID));
		strcpy((char *)Ssid.Ssid ,(char *)selectedProfileSetting->SSID);
		Ssid.SsidLength = selectedProfileSetting->SsidLen;

		unsigned char Bssid[6];
#ifdef WPA_SUPPLICANT_SUPPORT
		if (selectedProfileSetting->Authentication == Ndis802_11AuthModeWPA ||
			selectedProfileSetting->Authentication == Ndis802_11AuthModeWPA2 ||
			selectedProfileSetting->Authentication == Ndis802_11AuthModeMax )//802.1x
		{
			char tmp_key[27];
			if (selectedProfileSetting->KeyDefaultId == 1) // 1~4
				strcpy(tmp_key, (char *)selectedProfileSetting->Key1);
			else if (selectedProfileSetting->KeyDefaultId == 2)
				strcpy(tmp_key, (char *)selectedProfileSetting->Key2);
			else if (selectedProfileSetting->KeyDefaultId == 3)
				strcpy(tmp_key, (char *)selectedProfileSetting->Key3);
			else if (selectedProfileSetting->KeyDefaultId == 4)
				strcpy(tmp_key, (char *)selectedProfileSetting->Key4);

			conf_WPASupplicant((char*)selectedProfileSetting->SSID, selectedProfileSetting->KeyMgmt, selectedProfileSetting->EAP, (char*)selectedProfileSetting->Identity, (char*)selectedProfileSetting->Password, (char*)selectedProfileSetting->CACert, (char*)selectedProfileSetting->ClientCert, (char*)selectedProfileSetting->PrivateKey, (char*)selectedProfileSetting->PrivateKeyPassword, tmp_key, selectedProfileSetting->KeyDefaultId-1, selectedProfileSetting->Encryption, selectedProfileSetting->Tunnel, selectedProfileSetting->Tunnel);
		}
		else
#endif
		sta_connection(selectedProfileSetting->NetworkType, selectedProfileSetting->Authentication, selectedProfileSetting->Encryption, selectedProfileSetting->KeyDefaultId, &Ssid, Bssid, (char *)selectedProfileSetting->WpaPsk, (char *)selectedProfileSetting->Key1, (char *)selectedProfileSetting->Key2, (char *)selectedProfileSetting->Key3, (char *)selectedProfileSetting->Key4, selectedProfileSetting->PreamType, selectedProfileSetting->RTSCheck, selectedProfileSetting->RTS, selectedProfileSetting->FragmentCheck, selectedProfileSetting->Fragment, selectedProfileSetting->PSmode, selectedProfileSetting->Channel);

		Active_flag = 1;
		Sleep(1);
	}

	// setp 2, write all profile into nvram
	writeProfileToNvram();

	selectedProfileSetting = NULL;
}

/*
 * description: goform - reset statistics counters
 */
static void resetStaCounters(webs_t wp, char_t *path, char_t *query)
{
	int s;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	OidSetInformation(RT_OID_802_11_RESET_COUNTERS, s, RA0, 0, 0);
	close(s);
	websRedirect(wp, "station/statistics.asp");
	return;
}

/*
 * description: goform - set 11n configuration
 */
static void setSta11nCfg(webs_t wp, char_t *path, char_t *query)
{
	char_t *a_mpdu_enable, *autoBA, *mpdu_density, *a_msdu_enable;
	int policy;
	int s;
	OID_BACAP_STRUC BACap;

	a_mpdu_enable = websGetVar(wp, T("a_mpdu_enable"), T("off"));
	autoBA = websGetVar(wp, T("autoBA"), T("0"));
	mpdu_density = websGetVar(wp, T("mpdu_density"), T("0"));
	a_msdu_enable = websGetVar(wp, T("a_msdu_enable"), T("off"));

	if (!strcmp(a_mpdu_enable, "on")) {
		policy = 1;
		nvram_bufset(RT2860_NVRAM, "staPolicy", "1"); //FIXME: typo?
	}
	else {
		policy = 0;
		nvram_bufset(RT2860_NVRAM, "staPolicy", "0");
	}
	nvram_bufset(RT2860_NVRAM, "HT_AutoBA", autoBA);
	nvram_bufset(RT2860_NVRAM, "HT_MpduDensity", mpdu_density);
	nvram_bufset(RT2860_NVRAM, "HT_AMSDU", strcmp(a_msdu_enable, "off")? "1":"0");
	nvram_commit(RT2860_NVRAM);

	s = socket(AF_INET, SOCK_DGRAM, 0);
	OidQueryInformation(RT_OID_802_11_QUERY_IMME_BA_CAP, s, RA0, &BACap, sizeof(BACap));
	BACap.Policy = policy;
	BACap.AutoBA = atoi(autoBA);
	BACap.MpduDensity = atoi(mpdu_density);
	if (!strcmp(a_msdu_enable, "on"))
		BACap.AmsduEnable = 1;

	OidSetInformation(RT_OID_802_11_SET_IMME_BA_CAP, s, RA0, &BACap, sizeof(BACap));
	close(s);

	//debug print
	websHeader(wp);
	websWrite(wp, T("<h3>11n configuration</h3><br>\n"));
	websWrite(wp, T("a_mpdu_enable: %s<br>\n"), a_mpdu_enable);
	websWrite(wp, T("autoBA: %s<br>\n"), autoBA);
	websWrite(wp, T("mpdu_density: %s<br>\n"), mpdu_density);
	websWrite(wp, T("a_msdu_enable: %s<br>\n"), a_msdu_enable);
	websFooter(wp);
	websDone(wp, 200);
}

/*
 * description: goform - set advanced configuration
 */
static void setStaAdvance(webs_t wp, char_t *path, char_t *query)
{
	char_t *w_mode, *cr_bg, *cr_a, *bg_prot, *rate, *burst;
	char_t *ht, *bw, *gi, *mcs, *rf;

	int s, ret, country_region_a=0, country_region_bg=0;
	int tx_burst=0, short_slot_time=0, wireless_mode=0, tx_rate=0;
	unsigned long country_region=0;
	RT_802_11_STA_CONFIG configStation;
	NDIS_802_11_RATES	 aryRates;
	unsigned char radio_status=0;

	w_mode = websGetVar(wp, T("wireless_mode"), T("0"));
	wireless_mode = atoi(w_mode);
	cr_bg = websGetVar(wp, T("country_region_bg"), T("0"));
	cr_a = websGetVar(wp, T("country_region_a"), T("0"));
	bg_prot = websGetVar(wp, T("bg_protection"), T("0"));
	rate = websGetVar(wp, T("tx_rate"), T("0"));
	tx_rate = atoi(rate);
	burst = websGetVar(wp, T("tx_burst"), T("off"));
	ht = websGetVar(wp, T("n_mode"), T("0"));
	bw = websGetVar(wp, T("n_bandwidth"), T("0"));
	gi = websGetVar(wp, T("n_gi"), T("0"));
	mcs = websGetVar(wp, T("n_mcs"), T("0"));
	rf = websGetVar(wp, T("radiohiddenButton"), T("2"));

	s = socket(AF_INET, SOCK_DGRAM, 0);

	radio_status = atoi(rf);
	if (radio_status < 2) {
		OidSetInformation(RT_OID_802_11_RADIO, s, RA0, &radio_status, sizeof(radio_status));
		if (radio_status == 1) {
			ret = OidSetInformation(OID_802_11_BSSID_LIST_SCAN, s, RA0, 0, 0);
			if (ret < 0)
				error(E_L, E_LOG, T("Set OID_802_11_BSSID_LIST_SCAN error = %d"), ret);
			Sleep(3);
			if (G_SSID.SsidLength > 0) {
				ret = OidSetInformation(OID_802_11_SSID, s, RA0, &G_SSID, sizeof(NDIS_802_11_SSID));
				if (ret < 0)
					error(E_L, E_LOG, T("Set OID_802_11_SSID error = %d"), ret);
			} 
			/*
			else {
				ret = OidSetInformation(OID_802_11_BSSID, s, "ra", &G_Bssid, 6);
				if (ret < 0)
					error(E_L, E_LOG, T("Set OID_802_11_BSSID error = %d"), ret);
			}
			*/
		}
		websRedirect(wp, "station/advance.asp");
		return;
	}
	nvram_bufset(RT2860_NVRAM, "WirelessMode", w_mode);
	nvram_bufset(RT2860_NVRAM, "CountryRegion", cr_bg);
	nvram_bufset(RT2860_NVRAM, "CountryRegionABand", cr_a);
	nvram_bufset(RT2860_NVRAM, "BGProtection", bg_prot);
	nvram_bufset(RT2860_NVRAM, "TxRate", rate);
	if (!strncmp(burst, "on", 3)) {
		nvram_bufset(RT2860_NVRAM, "TxBurst", "1");
			tx_burst = 1;
	}
	else
		nvram_bufset(RT2860_NVRAM, "TxBurst", "0");
	nvram_bufset(RT2860_NVRAM, "HT_OpMode", ht);
	nvram_bufset(RT2860_NVRAM, "HT_BW", bw);
	nvram_bufset(RT2860_NVRAM, "HT_GI", gi);
	nvram_bufset(RT2860_NVRAM, "HT_MCS", mcs);
	nvram_commit(RT2860_NVRAM);

	//set wireless mode
	ret = OidSetInformation(RT_OID_802_11_PHY_MODE, s, RA0, &wireless_mode, sizeof(wireless_mode));
	if (ret < 0)
		return websError(wp, 500, "setStaAdvance: Set RT_OID_802_11_PHY_MODE error = %d", ret);

	//set 11n ht phy mode
	if (wireless_mode >= PHY_11ABGN_MIXED) {
		OID_BACAP_STRUC    BACap;
		OID_SET_HT_PHYMODE phymode;

		phymode.PhyMode = wireless_mode;
		phymode.HtMode = atoi(ht);
		phymode.TransmitNo = 2; //Now always use 2
		phymode.BW = atoi(bw);
		phymode.SHORTGI = atoi(gi);
		phymode.MCS = atoi(mcs);

		ret = OidSetInformation(RT_OID_802_11_SET_HT_PHYMODE, s, RA0, &phymode, sizeof(phymode));
		if (ret < 0)
			return websError(wp, 500, "setStaAdvance: Set RT_OID_802_11_SET_HT_PHYMODE error = %d", ret);

		BACap.AutoBA = TRUE;
		ret = OidSetInformation(RT_OID_802_11_SET_IMME_BA_CAP, s, RA0, &BACap, sizeof(BACap));
		if (ret < 0)
			return websError(wp, 500, "setStaAdvance: Set RT_OID_802_11_SET_IMME_BA_CAP error = %d", ret);
	}
	else {
		OID_BACAP_STRUC	BACap;
		BACap.AutoBA = FALSE;
		OidSetInformation(RT_OID_802_11_SET_IMME_BA_CAP, s, RA0, &BACap, sizeof(BACap));
	}

	//set country region
	country_region_bg = atoi(cr_bg);
	country_region_a = atoi(cr_a);
	country_region = country_region_bg | (country_region_a << 8);
	ret = OidSetInformation(RT_OID_802_11_COUNTRY_REGION, s, RA0, &country_region, sizeof(country_region));
	if (ret < 0)
		return websError(wp, 500, "setStaAdvance: Set RT_OID_802_11_COUNTRY_REGION error = %d", ret);

	OidQueryInformation(RT_OID_802_11_STA_CONFIG, s, RA0, &configStation, sizeof(configStation));

	configStation.EnableTurboRate = 0; //false
	configStation.EnableTxBurst = tx_burst;
	configStation.UseBGProtection = atoi(bg_prot);
	configStation.UseShortSlotTime = short_slot_time;
	configStation.AdhocMode = wireless_mode;

	OidSetInformation(RT_OID_802_11_STA_CONFIG, s, RA0, &configStation, sizeof(configStation));

	// set rate
	memset(aryRates, 0x00, sizeof(NDIS_802_11_RATES));
	if (wireless_mode == PHY_11A || wireless_mode == PHY_11G)
	{
		switch (tx_rate)
		{
			case 0:
				aryRates[0] = 0x6c; // 54Mbps
				aryRates[1] = 0x60; // 48Mbps
				aryRates[2] = 0x48; // 36Mbps
				aryRates[3] = 0x30; // 24Mbps
				aryRates[4] = 0x24; // 18M
				aryRates[5] = 0x18; // 12M
				aryRates[6] = 0x12; // 9M
				aryRates[7] = 0x0c; // 6M
				break;
			case 1:
				aryRates[0] = 0x0c; // 6M
				break;
			case 2:
				aryRates[0] = 0x12; // 9M
				break;
			case 3:
				aryRates[0] = 0x18; // 12M
				break;
			case 4:
				aryRates[0] = 0x24; // 18M
				break;
			case 5:
				aryRates[0] = 0x30; // 24M
				break;
			case 6:
				aryRates[0] = 0x48; // 36M
				break;
			case 7:
				aryRates[0] = 0x60; // 48M
				break;
			case 8:
				aryRates[0] = 0x6c; // 54M
				break;
		}
	}
	else if ((wireless_mode == PHY_11BG_MIXED) || (wireless_mode == PHY_11B) ||
			(wireless_mode == PHY_11ABG_MIXED))
	{
		switch(tx_rate)
		{
			case 0:
				switch(wireless_mode)
				{
					case PHY_11BG_MIXED: // B/G Mixed
					case PHY_11ABG_MIXED: // A/B/G Mixed
						aryRates[0] = 0x6c; // 54Mbps
						aryRates[1] = 0x60; // 48Mbps
						aryRates[2] = 0x48; // 36Mbps
						aryRates[3] = 0x30; // 24Mbps
						aryRates[4] = 0x16; // 11Mbps
						aryRates[5] = 0x0b; // 5.5Mbps
						aryRates[6] = 0x04; // 2Mbps
						aryRates[7] = 0x02; // 1Mbps
						break;
					case PHY_11B: // B Only
						aryRates[0] = 0x16; // 11Mbps
						aryRates[1] = 0x0b; // 5.5Mbps
						aryRates[2] = 0x04; // 2Mbps
						aryRates[3] = 0x02; // 1Mbps
						break;
					case PHY_11A:
						break;   //Not be call, for avoid warning.
				}
				break;
			case 1:
				aryRates[0] = 0x02; // 1M
				break;
			case 2:
				aryRates[0] = 0x04; // 2M
				break;
			case 3:
				aryRates[0] = 0x0b; // 5.5M
				break;
			case 4:
				aryRates[0] = 0x16; // 11M
				break;
			case 5:
				aryRates[0] = 0x0c; // 6M
				break;
			case 6:
				aryRates[0] = 0x12; // 9M
				break;
			case 7:
				aryRates[0] = 0x18; // 12M
				break;
			case 8:
				aryRates[0] = 0x24; // 18M
				break;
			case 9:
				aryRates[0] = 0x30; // 24M
				break;
			case 10:
				aryRates[0] = 0x48; // 36M
				break;
			case 11:
				aryRates[0] = 0x60; // 48M
				break;
			case 12:
				aryRates[0] = 0x6c; // 54M
				break;
		}
	}
	if (wireless_mode < PHY_11ABGN_MIXED)
		OidSetInformation(OID_802_11_DESIRED_RATES, s, RA0, &aryRates, sizeof(NDIS_802_11_RATES));

	OidSetInformation(OID_802_11_BSSID_LIST_SCAN, s, RA0, 0, 0);
	close(s);

	doSystem("wan.sh"); //renew dhcp, pppoe etc
	websRedirect(wp,"station/advance.asp");
	return;
}

/*
 * description: goform - make the station connect to the AP with given SSID
 */
static void setStaConnect(webs_t wp, char_t *path, char_t *query)
{
	int  tmp_auth=0, tmp_encry=0, tmp_defaultkeyid=0, tmp_networktype=0;
	char_t *tmp_ssid, *tmp_wpapsk, *tmp_key1, *tmp_key2, *tmp_key3, *tmp_key4, *tmp_bssid;
#ifdef WPA_SUPPLICANT_SUPPORT
	int  tmp_keymgmt = Rtwpa_supplicantKeyMgmtNONE, tmp_eap = Rtwpa_supplicantEAPNONE, tmp_tunnel = Rtwpa_supplicantTUNNENONE;
	char_t *tmp_identity, *tmp_cacert, *tmp_clientcert, *tmp_privatekey, *tmp_privatekeypassword, *tmp_password;
#endif 
	char_t *value;

	tmp_auth  = Ndis802_11AuthModeOpen;
	tmp_encry = Ndis802_11WEPDisabled;

	//ssid, networktype, bssid
	tmp_ssid = websGetVar(wp, T("Ssid"), T(""));
	value = websGetVar(wp, T("network_type"), T("0"));
	tmp_networktype = atoi(value);
	tmp_bssid = websGetVar(wp, T("bssid"), T(""));

	//security mode
	value = websGetVar(wp, T("security_infra_mode"), T(""));
	if (strcmp(value, "") != 0)
		tmp_auth = atoi(value);
	value = websGetVar(wp, T("security_adhoc_mode"), T(""));
	if (strcmp(value, "") != 0)
		tmp_auth = atoi(value);
#ifdef WPA_SUPPLICANT_SUPPORT
	//key management
	if (tmp_auth == Ndis802_11AuthModeWPA || tmp_auth == Ndis802_11AuthModeWPA2)
		tmp_keymgmt = Rtwpa_supplicantKeyMgmtWPAEAP;
	else if (tmp_auth == Ndis802_11AuthMode8021x) //802.1x
		tmp_keymgmt= Rtwpa_supplicantKeyMgmtIEEE8021X;
	else 
		tmp_keymgmt = Rtwpa_supplicantKeyMgmtNONE;
#endif

	//wep key1~4
	tmp_key1 = websGetVar(wp, T("wep_key_1"), T(""));
	tmp_key2 = websGetVar(wp, T("wep_key_2"), T(""));
	tmp_key3 = websGetVar(wp, T("wep_key_3"), T(""));
	tmp_key4 = websGetVar(wp, T("wep_key_4"), T(""));
	if (strcmp(tmp_key1, "") || strcmp(tmp_key2, "") || strcmp(tmp_key3, "")
			|| strcmp(tmp_key4, ""))
	{
		// Auth mode OPEN might use encryption type: none or wep
		// if set wep key, the encry must be WEPEnable
		tmp_encry = Ndis802_11WEPEnabled;
	}

	//default key
	value = websGetVar(wp, T("wep_default_key"), T("0"));
	tmp_defaultkeyid = atoi(value);

	//cipher
	value = websGetVar(wp, T("cipher"), T(""));
	if (strcmp(value, "") != 0) {
		int enc = atoi(value);
		if (enc == 0) //TKIP
			tmp_encry= Ndis802_11Encryption2Enabled;
		else //AES
			tmp_encry = Ndis802_11Encryption3Enabled;
	}

	//passphrase
	tmp_wpapsk = websGetVar(wp, T("passphrase"), T(""));

#ifdef WPA_SUPPLICANT_SUPPORT
	//eap
	value = websGetVar(wp, T("cert_auth_type_from_1x"), T(""));
	if (strcmp(value, "") != 0)
		tmp_eap = (RT_WPA_SUPPLICANT_EAP)atoi(value);
	value = websGetVar(wp, T("cert_auth_type_from_wpa"), T(""));
	if (strcmp(value, "") != 0)
		tmp_eap = (RT_WPA_SUPPLICANT_EAP)atoi(value);

	//tunnel
	value = websGetVar(wp, T("cert_tunnel_auth_peap"), T(""));
	if (strcmp(value, "") != 0)
		tmp_tunnel = (RT_WPA_SUPPLICANT_TUNNEL)atoi(value);
	value = websGetVar(wp, T("cert_tunnel_auth_ttls"), T(""));
	if (strcmp(value, "") != 0)
		tmp_tunnel = (RT_WPA_SUPPLICANT_TUNNEL)atoi(value);

	//certificate
	tmp_identity = websGetVar(wp, T("cert_id"), T(""));
	tmp_password = websGetVar(wp, T("cert_password"), T(""));
	tmp_clientcert = websGetVar(wp, T("cert_client_cert_path"), T(""));
	tmp_privatekey = websGetVar(wp, T("cert_private_key_path"), T(""));
	tmp_privatekeypassword = websGetVar(wp, T("cert_private_key_password"), T(""));
	tmp_cacert = websGetVar(wp, T("cert_ca_cert_path"), T(""));
#endif

	//encryp
	if (tmp_auth <= Ndis802_11AuthModeShared)
	{
		if (strlen((char *)tmp_key1) > 1 || strlen((char *)tmp_key2) > 1 ||
				strlen((char *)tmp_key3) > 1 || strlen((char *)tmp_key4) > 1)
		{
			tmp_encry= Ndis802_11WEPEnabled;
		}
		else
			tmp_encry = Ndis802_11WEPDisabled;
	}
	else if (tmp_auth == Ndis802_11AuthModeMax) //802.1x
		tmp_encry = Ndis802_11WEPEnabled;

	RT_802_11_PREAMBLE                      tmp_preamtype = Rt802_11PreambleAuto;
	NDIS_802_11_RTS_THRESHOLD               tmp_rts = MAX_RTS_THRESHOLD;
	NDIS_802_11_FRAGMENTATION_THRESHOLD     tmp_fragment = 2346;
	NDIS_802_11_POWER_MODE                  tmp_psmode = Ndis802_11PowerModeCAM;
	NDIS_802_11_SSID						SSID;
	unsigned char							Bssid[6];
	int										s = socket(AF_INET, SOCK_DGRAM, 0);

	OidQueryInformation(RT_OID_802_11_PREAMBLE, s, RA0, &tmp_preamtype, sizeof(RT_802_11_PREAMBLE));
	OidQueryInformation(OID_802_11_POWER_MODE, s, RA0, &tmp_psmode, sizeof(NDIS_802_11_POWER_MODE));
	OidQueryInformation(OID_802_11_RTS_THRESHOLD, s, RA0, &tmp_rts, sizeof(NDIS_802_11_RTS_THRESHOLD));
	OidQueryInformation(OID_802_11_FRAGMENTATION_THRESHOLD, s, RA0, &tmp_fragment, sizeof(NDIS_802_11_FRAGMENTATION_THRESHOLD));
	// Set SSID
	memset(&SSID, 0x00, sizeof(NDIS_802_11_SSID));
	SSID.SsidLength = strlen(tmp_ssid);
	memcpy(SSID.Ssid, (const void *)tmp_ssid, SSID.SsidLength);

	// Set BSSID
	memset(Bssid, 0x00, sizeof(Bssid));
	AtoH(tmp_bssid, Bssid, 6);

	//site_survey_connect
#ifdef WPA_SUPPLICANT_SUPPORT
	if (tmp_auth == Ndis802_11AuthModeWPA ||
			tmp_auth == Ndis802_11AuthModeWPA2 ||
			tmp_auth == Ndis802_11AuthMode8021x )//802.1x
	{
		char tmp_key[27];
		if (tmp_defaultkeyid == 1) // 1~4
			strcpy(tmp_key, tmp_key1);
		else if (tmp_defaultkeyid == 2)
			strcpy(tmp_key, tmp_key2);
		else if (tmp_defaultkeyid == 3)
			strcpy(tmp_key, tmp_key3);
		else if (tmp_defaultkeyid == 4)
			strcpy(tmp_key, tmp_key4);

		tmp_defaultkeyid -=1;

		unsigned long CurrentWirelessMode;
		if (OidQueryInformation(RT_OID_802_11_PHY_MODE, s, RA0, &CurrentWirelessMode, sizeof(unsigned char)) < 0 )
		{
			websError(wp, 500, "Query OID_802_11_QUERY_WirelessMode error!");
			close(s);
			return;
		}

		if(tmp_encry == Ndis802_11Encryption2Enabled || tmp_encry == Ndis802_11WEPEnabled)
		{
			if(CurrentWirelessMode > 4)
			{

				doSystem("iwpriv %s set WirelessMode=3", RA0);
				sleep(2);
			}
		}
		else
		{
			if(CurrentWirelessMode < 5)
			{

				doSystem("iwpriv %s set WirelessMode=5", RA0);
				sleep(2);
			}
		}

		conf_WPASupplicant(tmp_ssid, tmp_keymgmt, tmp_eap, tmp_identity, tmp_password, tmp_cacert, tmp_clientcert, tmp_privatekey, tmp_privatekeypassword, tmp_key, tmp_defaultkeyid, tmp_encry, tmp_tunnel, tmp_auth);
	}
	else
#endif
		sta_connection(tmp_networktype, tmp_auth, tmp_encry, tmp_defaultkeyid, &SSID, Bssid, tmp_wpapsk, tmp_key1, tmp_key2, tmp_key3, tmp_key4, tmp_preamtype, 0, tmp_rts, 0, tmp_fragment, tmp_psmode, 0);  //tmp_channel 0 is auto.

#if defined CONFIG_USB
	initUSB();
#endif
}

/*
 * description: goform - set G_bdBm_ischeck (displaying dbm or % type)
 */
static void setStaDbm(webs_t wp, char_t *path, char_t *query)
{
	char_t *dbm;

	dbm = websGetVar(wp, T("dbmChecked"), T("off"));
	if (!strncmp(dbm, "on", 3))
		G_bdBm_ischeck = 1;
	else
		G_bdBm_ischeck = 0;
	websRedirect(wp, "station/link_status.asp");
	return;
}

/*
 * description: goform - add ampdu originator
 */
static void setStaProfile(webs_t wp, char_t *path, char_t *query)
{
	PRT_PROFILE_SETTING	previousProfileSetting = NULL;
	int selectedProfile=0 , i=0;
	char_t *value;
	
	if (headerProfileSetting == NULL) {
		error(E_L, E_LOG, T("headerProfileSetting is NULL"));
		return;
	}

	value = websGetVar(wp, T("selectedProfile"), T("0"));
	selectedProfile = atoi(value);
	if (selectedProfile <= 0) {
		error(E_L, E_LOG, T("selectedProfile(%d) is invalid"), selectedProfile);
		return;
	}

	previousProfileSetting = selectedProfileSetting = headerProfileSetting;
	for (i=2; i <= selectedProfile; i++) {
		selectedProfileSetting = selectedProfileSetting->Next;
		if (i == selectedProfile-1)
			previousProfileSetting = selectedProfileSetting;
	}
	if (selectedProfileSetting == headerProfileSetting) {
		previousProfileSetting = NULL;
	}

	value = websGetVar(wp, T("hiddenButton"), T(""));
	if (!strcmp(value, "edit")) {
		//do nothing
	}
	else if (!strcmp(value, "delete"))
	{
		if (selectedProfileSetting == headerProfileSetting) {
			if (headerProfileSetting->Next == NULL)
				selectedProfileSetting = headerProfileSetting = NULL;
			else
				headerProfileSetting = headerProfileSetting->Next;
			writeProfileToNvram();
		}
		else {
			if (previousProfileSetting != NULL && selectedProfileSetting != NULL) {
				previousProfileSetting->Next = selectedProfileSetting->Next;
				writeProfileToNvram();
			}
			selectedProfileSetting = NULL;
		}

		if (headerProfileSetting == NULL)
		{
			nvram_bufset(RT2860_NVRAM, "staProfile", "");
			nvram_bufset(RT2860_NVRAM, "staSSID", "");
			nvram_bufset(RT2860_NVRAM, "staNetworkType", "");
			nvram_bufset(RT2860_NVRAM, "staPSMode", "");
			nvram_bufset(RT2860_NVRAM, "staAdhocMode", "");
			nvram_bufset(RT2860_NVRAM, "staChannel", "");
			nvram_bufset(RT2860_NVRAM, "staPreamType", "");
			nvram_bufset(RT2860_NVRAM, "staRTSCheck", "");
			nvram_bufset(RT2860_NVRAM, "staFragmentCheck", "");
			nvram_bufset(RT2860_NVRAM, "staRTS", "");
			nvram_bufset(RT2860_NVRAM, "staFragment", "");
			nvram_bufset(RT2860_NVRAM, "staAuth", "");
			nvram_bufset(RT2860_NVRAM, "staEncrypt", "");
			nvram_bufset(RT2860_NVRAM, "staKeyDefaultId", "");
			nvram_bufset(RT2860_NVRAM, "staKey1Type", "");
			nvram_bufset(RT2860_NVRAM, "staKey2Type", "");
			nvram_bufset(RT2860_NVRAM, "staKey3Type", "");
			nvram_bufset(RT2860_NVRAM, "staKey4Type", "");
			nvram_bufset(RT2860_NVRAM, "staKey1Length", "");
			nvram_bufset(RT2860_NVRAM, "staKey2Length", "");
			nvram_bufset(RT2860_NVRAM, "staKey3Length", "");
			nvram_bufset(RT2860_NVRAM, "staKey4Length", "");
			nvram_bufset(RT2860_NVRAM, "staKey1", "");
			nvram_bufset(RT2860_NVRAM, "staKey2", "");
			nvram_bufset(RT2860_NVRAM, "staKey3", "");
			nvram_bufset(RT2860_NVRAM, "staKey4", "");
			nvram_bufset(RT2860_NVRAM, "staWpaPsk", "");
			nvram_bufset(RT2860_NVRAM, "sta8021xKeyMgmt", "");
			nvram_bufset(RT2860_NVRAM, "sta8021xEAP", "");
			nvram_bufset(RT2860_NVRAM, "sta8021xIdentity", "");
			nvram_bufset(RT2860_NVRAM, "sta8021xCACert", "");
			nvram_bufset(RT2860_NVRAM, "sta8021xClientCert", "");
			nvram_bufset(RT2860_NVRAM, "sta8021xPrivateKey", "");
			nvram_bufset(RT2860_NVRAM, "sta8021xPrivateKeyPassword", "");
			nvram_bufset(RT2860_NVRAM, "sta8021xPassword", "");
			nvram_bufset(RT2860_NVRAM, "sta8021xTunnel", "");
			nvram_commit(RT2860_NVRAM);
		}
	}
	else if (!strcmp(value, "activate"))
	{
		int tmp_auth, tmp_encry, tmp_defaultkeyid, tmp_networktype, tmp_preamtype, tmp_channel; //tmp_adhocmode,
		int s, ret;
		char tmp_wpapsk[65], tmp_key1[27], tmp_key2[27], tmp_key3[27], tmp_key4[27], tmp_bssid[13];
		char tmp_rtscheck=0, tmp_fragmentcheck=0;
		NDIS_802_11_RTS_THRESHOLD	tmp_rts;
		NDIS_802_11_FRAGMENTATION_THRESHOLD	tmp_fragment;
		NDIS_802_11_SSID			SSID;
		NDIS_802_11_POWER_MODE		tmp_psmode;

		currentProfileSetting = headerProfileSetting;
		do {
			currentProfileSetting->Active = 0;
			currentProfileSetting = currentProfileSetting->Next;
		} while (currentProfileSetting != NULL);

		selectedProfileSetting->Active = 1; // acivate
		//writeProfileToNvram();
		{
			char tmp_buffer[512] = {0}, tmp_data[8] = {0};
			currentProfileSetting = headerProfileSetting;
			do {
				sprintf(tmp_data, "%d", currentProfileSetting->Active);
				strcat(tmp_buffer, tmp_data);
				currentProfileSetting = currentProfileSetting->Next;
				if (currentProfileSetting != NULL)
					strcat(tmp_buffer, ";");	
			} while (currentProfileSetting != NULL);
			nvram_bufset(RT2860_NVRAM, "staActive", tmp_buffer);
			nvram_commit(RT2860_NVRAM);
		}

		memset(&SSID, 0x00, sizeof(SSID));
		bzero(tmp_bssid, sizeof(tmp_bssid));
		bzero(tmp_wpapsk, sizeof(tmp_wpapsk));
		bzero(tmp_key1, sizeof(tmp_key1));
		bzero(tmp_key2, sizeof(tmp_key2));
		bzero(tmp_key3, sizeof(tmp_key3));
		bzero(tmp_key4, sizeof(tmp_key4));
		memset(tmp_wpapsk, 0x00, sizeof(tmp_wpapsk));

		SSID.SsidLength = selectedProfileSetting->SsidLen;
		memcpy(SSID.Ssid, (const void *)selectedProfileSetting->SSID, selectedProfileSetting->SsidLen);

		tmp_networktype = selectedProfileSetting->NetworkType;
		tmp_auth  = selectedProfileSetting->Authentication;
		tmp_encry = selectedProfileSetting->Encryption;
		tmp_preamtype = selectedProfileSetting->PreamType;
		tmp_rts = selectedProfileSetting->RTS;
		tmp_rtscheck = selectedProfileSetting->RTSCheck;
		tmp_fragment = selectedProfileSetting->Fragment;
		tmp_fragmentcheck = selectedProfileSetting->FragmentCheck;
		tmp_psmode = selectedProfileSetting->PSmode;
		tmp_channel = selectedProfileSetting->Channel;
		tmp_defaultkeyid = selectedProfileSetting->KeyDefaultId;

		sprintf(tmp_wpapsk, "%s", selectedProfileSetting->WpaPsk);
		strcpy(tmp_key1, (char *)selectedProfileSetting->Key1);
		strcpy(tmp_key2, (char *)selectedProfileSetting->Key2);
		strcpy(tmp_key3, (char *)selectedProfileSetting->Key3);
		strcpy(tmp_key4, (char *)selectedProfileSetting->Key4);

		s = socket(AF_INET, SOCK_DGRAM, 0);
		//step 1: OID_802_11_INFRASTRUCTURE_MODE
		ret = OidSetInformation(OID_802_11_INFRASTRUCTURE_MODE, s, RA0, &tmp_networktype, sizeof(int));
		if (ret < 0)
			fprintf(stderr, "Set OID_802_11_INFRASTRUCTURE_MODE has error =%d, networktype=%d\n", ret, tmp_networktype);
		close(s);

		unsigned char Bssid[6];
		//activate
#ifdef WPA_SUPPLICANT_SUPPORT				
		if (selectedProfileSetting->Authentication == Ndis802_11AuthModeWPA ||
				selectedProfileSetting->Authentication == Ndis802_11AuthModeWPA2 ||
				selectedProfileSetting->Authentication == Ndis802_11AuthMode8021x )//802.1x
		{
			char tmp_key[27];
			if (tmp_defaultkeyid == 1) // 1~4
				strcpy(tmp_key, tmp_key1);
			else if (tmp_defaultkeyid == 2)
				strcpy(tmp_key, tmp_key2);
			else if (tmp_defaultkeyid == 3)
				strcpy(tmp_key, tmp_key3);
			else if (tmp_defaultkeyid == 4)
				strcpy(tmp_key, tmp_key4);

			conf_WPASupplicant((char*)selectedProfileSetting->SSID, selectedProfileSetting->KeyMgmt, selectedProfileSetting->EAP, (char*)selectedProfileSetting->Identity, (char*)selectedProfileSetting->Password, (char*)selectedProfileSetting->CACert, (char*)selectedProfileSetting->ClientCert, (char*)selectedProfileSetting->PrivateKey, (char*)selectedProfileSetting->PrivateKeyPassword, tmp_key, selectedProfileSetting->KeyDefaultId-1, selectedProfileSetting->Encryption, selectedProfileSetting->Tunnel, selectedProfileSetting->Authentication);
		}
		else
#endif
			sta_connection(tmp_networktype, tmp_auth, tmp_encry, tmp_defaultkeyid, &SSID, Bssid, tmp_wpapsk, tmp_key1, tmp_key2, tmp_key3, tmp_key4, tmp_preamtype, tmp_rtscheck, tmp_rts, tmp_fragmentcheck, tmp_fragment, tmp_psmode, tmp_channel);

		// Set SSID
		/*memset(&SSID, 0x00, sizeof(NDIS_802_11_SSID));
		  SSID.SsidLength = selectedProfileSetting->SsidLen;
		  memcpy(SSID.Ssid, (const void *)selectedProfileSetting->SSID, selectedProfileSetting->SsidLen);
		  */
		Active_flag = 1;
#if defined CONFIG_USB
		initUSB();
#endif
	}
	else {
		error(E_L, E_LOG, T("hiddenButton(%s) is invalid"), value);
		return;
	}
	websRedirect(wp, "station/profile.asp");
}

/*
 * description: goform - add ampdu originator
 */
static void setStaOrgAdd(webs_t wp, char_t *path, char_t *query)
{
	char_t *tid, *win_sz, *sbssid, *mac;
	int  s;
	char setflag = 0;
	unsigned char Bssid[6];
	OID_ADD_BA_ENTRY oriEntry;

	tid = websGetVar(wp, T("tid"), T("0"));
	win_sz = websGetVar(wp, T("ba_window_size"), T("32"));
	sbssid = websGetVar(wp, T("selectedbssid"), T("0"));
	mac = websGetVar(wp, T("mac"), T("0"));
	nConfig_flag = 1;

	if (strncmp(sbssid, "0", 2))
	{
		int i; char *tok;
		for (i = 0, tok = strtok(sbssid, ":");
				(i < 5) && tok;
				i++, tok = strtok(NULL, ":"))
		{
			Bssid[i] = (unsigned char)strtoul(tok, (char **)NULL, 16);
		}
		printf("bssid %02X:%02X:%02X:%02X:%02X:%02X\n", Bssid[0], Bssid[1], Bssid[2], Bssid[3], Bssid[4], Bssid[5]);
	}

	//FIXME: selectedbssid, mac might be different
	if (!strncmp(mac, "on", 3))
		setflag = 1;

	if (setflag) {
		s = socket(AF_INET, SOCK_DGRAM, 0);

		memcpy(oriEntry.MACAddr, Bssid, 6);
		oriEntry.IsRecipient = 0; //false
		oriEntry.BufSize = (unsigned char)atoi(win_sz);
		oriEntry.TID = (unsigned char)atoi(tid);
		oriEntry.TimeOut = 0;
		oriEntry.AllTid = 0; //false

		OidSetInformation(RT_OID_802_11_ADD_IMME_BA, s, RA0, &oriEntry, sizeof(oriEntry));
		close(s);
	}
}

/*
 * description: goform - delete ampdu originator
 */
static void setStaOrgDel(webs_t wp, char_t *path, char_t *query)
{
	char_t *button;

	button = websGetVar(wp, T("hiddenButton"), T("0"));
}

/*
 * description: goform - set station QoS parameters - wmm, dls setup, tear down
 */
static void setStaQoS(webs_t wp, char_t *path, char_t *query)
{
	int s;
	char_t *button;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	button = websGetVar(wp, T("button_type"), T("0"));

	if (!strncmp(button, "0", 2)) {
		close(s);
		websError(wp, 500, "unrecognized button type");
		return;
	}
	else if (!strncmp(button, "1", 2)) {
		unsigned long apsd;
		NDIS_802_11_SSID Ssid;
		int wmm_en, ps_en, acbe, acbk, acvi, acvo;
		char apsdac[8];

		wmm_en = websCompareVar(wp, T("wmm_enable"), T("on"));
		ps_en = websCompareVar(wp, T("wmm_ps_enable"), T("on"));
		acbe = websCompareVar(wp, T("wmm_ps_mode_acbe"), T("on"));
		acbk = websCompareVar(wp, T("wmm_ps_mode_acbk"), T("on"));
		acvi = websCompareVar(wp, T("wmm_ps_mode_acvi"), T("on"));
		acvo = websCompareVar(wp, T("wmm_ps_mode_acvo"), T("on"));

		nvram_bufset(RT2860_NVRAM, "WmmCapable", wmm_en? "1":"0");
		nvram_bufset(RT2860_NVRAM, "APSDCapable", ps_en? "1":"0");
		strncpy(apsdac, acbe? "1":"0", 2);
		strncat(apsdac, acbk? "1":"0", 2);
		strncat(apsdac, acvi? "1":"0", 2);
		strncat(apsdac, acvo? "1":"0", 2);
		nvram_bufset(RT2860_NVRAM, "APSDAC", apsdac);
		nvram_commit(RT2860_NVRAM);

		if (wmm_en) {
			OidQueryInformation(RT_OID_802_11_QUERY_APSD_SETTING, s, RA0, &apsd, sizeof(apsd));
			if (ps_en) {
				apsd |= 0x00000001;
				if (acbk)
					apsd |= 0x00000002;
				if (acbe)
					apsd |= 0x00000004;
				if (acvi)
					apsd |= 0x00000008;
				if (acvo)
					apsd |= 0x00000010;
				apsd &= 0x0000007F;  //set apsd bit be zero (xxxxxxx1)
			}
			else
				apsd &= 0x0000007E;  //set apsd bit be zero (xxxxxxx0)
		}
		else
			apsd &= 0x0000007E;  //set apsd bit be zero (xxxxxxx0)

		OidSetInformation(RT_OID_802_11_SET_APSD_SETTING, s, RA0, &apsd, sizeof(apsd));
		OidSetInformation(RT_OID_802_11_SET_WMM, s, RA0, &wmm_en, sizeof(wmm_en));

		OidQueryInformation(OID_802_11_SSID, s, RA0, &Ssid, sizeof(Ssid));
		OidSetInformation(OID_802_11_DISASSOCIATE, s, RA0, NULL, 0);
		Sleep(1);
		OidSetInformation(OID_802_11_SSID, s, RA0, &Ssid, sizeof(Ssid));
	}
	else if (!strncmp(button, "2", 2)) {
		int dls_en;
		char_t *mac0, *mac1, *mac2, *mac3, *mac4, *mac5;
		RT_802_11_DLS_UI dls;
		unsigned char mac[6];

		dls_en = websCompareVar(wp, T("wmm_dls_enable"), T("on"));
		mac0 = websGetVar(wp, T("mac0"), T("0"));
		mac1 = websGetVar(wp, T("mac1"), T("1"));
		mac2 = websGetVar(wp, T("mac2"), T("2"));
		mac3 = websGetVar(wp, T("mac3"), T("3"));
		mac4 = websGetVar(wp, T("mac4"), T("4"));
		mac5 = websGetVar(wp, T("mac5"), T("5"));
		dls.MacAddr[0] = (unsigned char)strtoul(mac0, (char **)NULL, 16);
		dls.MacAddr[1] = (unsigned char)strtoul(mac1, (char **)NULL, 16);
		dls.MacAddr[2] = (unsigned char)strtoul(mac2, (char **)NULL, 16);
		dls.MacAddr[3] = (unsigned char)strtoul(mac3, (char **)NULL, 16);
		dls.MacAddr[4] = (unsigned char)strtoul(mac4, (char **)NULL, 16);
		dls.MacAddr[5] = (unsigned char)strtoul(mac5, (char **)NULL, 16);
		dls.TimeOut = atoi(websGetVar(wp, T("timeout"), T("0")));

		if (dls.MacAddr[0] == 0 && dls.MacAddr[1] == 0 && dls.MacAddr[2] == 0 &&
				dls.MacAddr[3] == 0 && dls.MacAddr[4] == 0 && dls.MacAddr[5] == 0) {
			close(s);
			websError(wp, 500, "invalid DLS MAC address (00s)");
			return;
		}
		if (dls.MacAddr[0] == 0xff && dls.MacAddr[1] == 0xff && dls.MacAddr[2] == 0xff &&
				dls.MacAddr[3] == 0xff && dls.MacAddr[4] == 0xff && dls.MacAddr[5] == 0xff) {
			close(s);
			websError(wp, 500, "invalid DLS MAC address (FFs)");
			return;
		}
		if (OidQueryInformation(OID_802_3_CURRENT_ADDRESS, s, RA0, &mac, sizeof(mac)) >= 0) {
			if (dls.MacAddr[0] == mac[0] && dls.MacAddr[1] == mac[1] &&
					dls.MacAddr[2] == mac[2] && dls.MacAddr[3] == mac[3] &&
					dls.MacAddr[4] == mac[4] && dls.MacAddr[5] == mac[5]) {
				close(s);
				websError(wp, 500, "invalid DLS MAC address");
				return;
			}
		}
		nvram_bufset(RT2860_NVRAM, "DLSCapable", dls_en? "1":"0");

		dls.Valid = 1;
		OidSetInformation(RT_OID_802_11_SET_DLS, s, RA0, &dls_en, sizeof(unsigned long));
		OidSetInformation(RT_OID_802_11_SET_DLS_PARAM, s, RA0, &dls, sizeof(dls));
	}
	else if (!strncmp(button, "3", 2)) {
		RT_802_11_DLS_INFO dls_info;
		int s_dls;


		s_dls = atoi(websGetVar(wp, T("selected_dls"), T("0")));
		if (s_dls != 0) {
			OidQueryInformation(RT_OID_802_11_QUERY_DLS_PARAM, s, RA0, &dls_info, sizeof(dls_info));
			if (dls_info.Entry[s_dls-1].Valid == 1) {
				dls_info.Entry[s_dls-1].Valid = 0;
				dls_info.Entry[s_dls-1].Status = DLS_NONE;
				OidSetInformation(RT_OID_802_11_SET_DLS_PARAM, s, RA0, &dls_info.Entry[s_dls-1], sizeof(RT_802_11_DLS_UI));
			}
		}
	}
	close(s);
	sleep(1);
	websRedirect(wp, "station/qos.asp");
}


