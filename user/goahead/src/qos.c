#include    <stdlib.h>
#include    <sys/ioctl.h>
#include    <net/if.h>
#include    <net/route.h>
#include    <string.h>
#include    <dirent.h>
#include    "internet.h"
#include    "nvram.h"
#include    "webs.h"
#include    "utils.h"
#include    "firewall.h"
#include    "qos.h"
#include    "config/autoconf.h" //user config

extern char l7name[];							// in firewall.c
inline int getRuleNums(char *);

struct entry_s QOS_PROFILE[QOS_PROFILE_ENTRYS_MAX] = {
	{"QoSAF5Name","High"},
	{"QoSAF5Rate","30"},
	{"QoSAF5Ceil","100"},
	{"QoSAF2Name","Middle"},
	{"QoSAF2Rate","20"},
	{"QoSAF2Ceil","100"},
	{"QoSAF6Name","Default"},
	{"QoSAF6Rate","5"},
	{"QoSAF6Ceil","100"},
	{"QoSAF1Name","Low"},
	{"QoSAF1Rate","10"},
	{"QoSAF1Ceil","100"},
	{"QoSRules", "ICMP_HIGH,5,1,,ICMP,,,,,,,,,,,,EF;Small_Packet_HIGH,5,1,,,,,0,128,,,,,,,,EF;VoIP_H323_HIGH,5,1,,Application,,,,,,,,,h323,,,EF;VoIP_SIP_HIGH,5,1,,Application,,,,,,,,,sip,,,EF;VoIP_Skype1_HIGH,5,1,,Application,,,,,,,,,skypeout,,,EF;VoIP_Skype2_HIGH,5,1,,Application,,,,,,,,,skypetoskype,,,EF;RTP_HIGH,5,1,,Application,,,,,,,,,rtp,,,EF;SSH_HIGH,5,1,,Application,,,,,,,,,ssh,,,EF;MSN_Messenger_MIDDLE,2,1,,Application,,,,,,,,,msnmessenger,,,AF21;Yahoo_MIDDLE,2,1,,Application,,,,,,,,,yahoo,,,AF21;PoP3_LOW,1,1,,Application,,,,,,,,,msnmessenger,,,AF11;SMTP_LOW,1,1,,Application,,,,,,,,,smtp,,,AF11;P2P_eMule_LOW,1,1,,Application,,,,,,,,,edonkey,,,AF11;P2P_BT_LOW,1,1,,Application,,,,,,,,,bittorrent,,,AF11"},
	{ NULL, NULL}
};

inline void QoSRestart(void)
{
	FILE *fp = fopen("/bin/qos_run", "r");
	if(!fp)
		return;
	fclose(fp);
	doSystem("/bin/qos_run");
}

inline void QoSInit(void)
{
	char *cm;
	cm = nvram_bufget(RT2860_NVRAM, "wanConnectionMode");
	if (!strncmp(cm, "PPPOE", 6) || !strncmp(cm, "L2TP", 5) || !strncmp(cm, "PPTP", 5) 
#ifdef CONFIG_USER_3G
		|| !strncmp(cm, "3G", 3)
#endif
	){
		// Just return.
		// The PPP daemon would trigger "qos_run" after dial up successfully, so do nothing here.
		return;
	}

	QoSRestart();
}

static void QoSAFAttribute(webs_t wp, char_t *path, char_t *query)
{
	char tmp[512];
	char_t *index, *name, *rate, *ceil;
	int index_i;
	index = websGetVar(wp, T("af_index"), T(""));
	name = websGetVar(wp, T("af_name"), T(""));
	rate = websGetVar(wp, T("af_rate"), T(""));
	ceil = websGetVar(wp, T("af_ceil"), T(""));
	if(!index|| !name || !rate || !ceil)
		return;
	if(!strlen(index) || !strlen(name) || !strlen(rate) || !strlen(ceil))
		return;
	index_i = atoi(index);
	if(index_i > 6 || index_i <= 0)
		return;
	if(atoi(rate) > 100 || atoi(ceil) > 100)		//percentage
		return;

	sprintf(tmp, "QoSAF%dName", index_i);
	nvram_bufset(RT2860_NVRAM, tmp, name);
	sprintf(tmp, "QoSAF%dRate", index_i);
	nvram_bufset(RT2860_NVRAM, tmp, rate);
	sprintf(tmp, "QoSAF%dCeil", index_i);
	nvram_bufset(RT2860_NVRAM, tmp, ceil);
	nvram_commit(RT2860_NVRAM);

	QoSRestart();

    websWrite(wp, T("HTTP/1.0 200 OK\n"));
    websWrite(wp, T("Server: %s\r\n"), WEBS_NAME);
    websWrite(wp, T("Pragma: no-cache\n"));
    websWrite(wp, T("Cache-control: no-cache\n"));
    websWrite(wp, T("Content-Type: text/html\n"));
    websWrite(wp, T("\n"));
    websWrite(wp, T("<html>\n<head>\n"));
    websWrite(wp, T("<title>My Title</title>"));
    websWrite(wp, T("<link rel=\"stylesheet\" href=\"/style/normal_ws.css\" type=\"text/css\">"));
    websWrite(wp, T("<meta http-equiv=\"content-type\" content=\"text/html; charset=iso-8859-1\">"));
    websWrite(wp, T("</head>\n<body onload=\"opener.location.reload();window.close();\">\n"));

	websFooter(wp);
	websDone(wp, 200);
}

static int qosGetIndexByName(int af_index, int dp_index, char *match)
{
	int i;
	char rec[256];
	char asdp_str[512];
	char *rule, name[32];

	sprintf(asdp_str, "QoSAF%dDP%d", af_index, dp_index);
	rule = nvram_bufget(RT2860_NVRAM, asdp_str);
	if(!rule || !strlen(rule))
		return -1;

	i=0;
	while( (getNthValueSafe(i, rule, ';', rec, sizeof(rec)) != -1) ){
		if( (getNthValueSafe(0, rec, ',', name, sizeof(name)) == -1) )
			continue;
		if(!strcmp(name, match))
			return i;
		i++;
	}
	return -1;
}

static void qosClassifier(webs_t wp, char_t *path, char_t *query)
{
	char rule[4096];
	char *old_rule;
	char tmp[8];

	char_t *af_index, *dp_index, *comment, *mac_address, *dip_address, *sip_address, *pktlenfrom, *pktlento,
			*protocol, *dprf, *dprt, *sprf, *sprt, *layer7, *dscp, *ingress_if, *remark ;
	int af_index_i, dp_index_i, sprf_int, dprf_int, sprt_int, dprt_int;

	af_index = websGetVar(wp, T("af_index"), T(""));
	dp_index = websGetVar(wp, T("dp_index"), T(""));
	comment = websGetVar(wp, T("comment"), T(""));
	mac_address = websGetVar(wp, T("mac_address"), T(""));
	dip_address = websGetVar(wp, T("dip_address"), T(""));
	sip_address = websGetVar(wp, T("sip_address"), T(""));
	pktlenfrom = websGetVar(wp, T("pktlenfrom"), T(""));
	pktlento = websGetVar(wp, T("pktlento"), T(""));
	protocol = websGetVar(wp, T("protocol"), T(""));
	dprf = websGetVar(wp, T("dFromPort"), T(""));
	dprt = websGetVar(wp, T("dToPort"), T(""));
	sprf = websGetVar(wp, T("sFromPort"), T(""));
	sprt = websGetVar(wp, T("sToPort"), T(""));
	layer7 = websGetVar(wp, T("layer7"), T(""));
	dscp = websGetVar(wp, T("dscp"), T(""));
	/*ingress_if = websGetVar(wp, T("ingress_if"), T(""));*/
	ingress_if = "";
	remark =  websGetVar(wp, T("remark_dscp"), T(""));

	if(!af_index || !dp_index || !comment || !remark)
		return;

	if(!strlen(af_index) || !strlen(dp_index) || strlen(comment) > 32)
		return;

	// i know you will try to break our box... ;)
	if(strchr(comment, ';') || strchr(comment, ','))
		return;

	af_index_i = atoi(af_index);
	dp_index_i = atoi(dp_index);
	if(qosGetIndexByName(af_index_i, dp_index_i, comment) != -1){
		// This name is not unique.
		// return;
	}

	// pkt len from/to must be co-exist.
	if( (!strlen(pktlenfrom) && strlen(pktlento)) || 
		(strlen(pktlenfrom) && !strlen(pktlento)) )
		return;

	if(!strlen(protocol)){
		if(	!strlen(mac_address) && !strlen(sip_address) && !strlen(dip_address) &&
			!strlen(dscp) /*&& !strlen(ingress_if)*/ && !strlen(pktlenfrom) )
			return;
		layer7 = "";
	}else if(!strcmp(protocol, "TCP") || !strcmp(protocol, "UDP")){
		if(!strlen(dprf) && !strlen(sprf))
			return;
		layer7 = "";
	}else if(!strcmp(protocol, "Application")){
		if(!strlen(layer7))
			return;
	}else if(!strcmp(protocol, "ICMP")){
		layer7 = "";	// do nothing.
	}else
		return;

	// we dont trust user input.....
	if(strlen(mac_address)){
		if(!isMacValid(mac_address))
			return;
	}

	if(strlen(sip_address)){
		if(!isIpNetmaskValid(sip_address))
			return;
	}

    if(strlen(dip_address)){
        if(!isIpNetmaskValid(dip_address))
            return;
    }


	if(!strlen(sprf)){
		sprf_int = 0;
	}else{
		sprf_int = atoi(sprf);
		if(sprf_int == 0 || sprf_int > 65535)
			return;
	}
	if(!strlen(sprt)){
		sprt_int = 0;
	}else{
		sprt_int = atoi(sprt);
		if(sprt_int ==0 || sprt_int > 65535)
			return;
	}
	if(!strlen(dprf)){
		dprf_int = 0;
	}else{
		dprf_int = atoi(dprf);
		if(dprf_int == 0 || dprf_int > 65535)
			return;
	}
	if(!strlen(dprt)){
		dprt_int = 0;
	}else{
		dprt_int = atoi(dprt);
		if(dprt_int == 0 || dprt_int > 65535)
			return;
	}

	if(!strcmp(remark, "Auto")){
		if(af_index_i == 5 && dp_index_i == 1){				/* EF class*/
			remark = "EF";
		}else if(af_index_i == 6 && dp_index_i == 1){		/* BE class */
			remark = "BE";
		}else{                                  /* AF classes */
            snprintf(tmp, sizeof(tmp), "AF%d%d", af_index_i, dp_index_i);
            remark = tmp;
        }
	}

	old_rule = nvram_bufget(RT2860_NVRAM, "QoSRules");
	if(!old_rule || !strlen(old_rule))
		snprintf(rule, sizeof(rule), "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", comment, af_index, dp_index, mac_address, protocol, dip_address, sip_address, pktlenfrom, pktlento, dprf, dprt, sprf, sprt, layer7, dscp, ingress_if, remark);
	else
		snprintf(rule, sizeof(rule), "%s;%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s", old_rule, comment, af_index, dp_index, mac_address, protocol, dip_address, sip_address, pktlenfrom, pktlento, dprf, dprt, sprf, sprt, layer7, dscp, ingress_if, remark);

	nvram_bufset(RT2860_NVRAM, "QoSRules", rule);
	nvram_commit(RT2860_NVRAM);

	QoSRestart();

//	websHeader(wp);
    websWrite(wp, T("HTTP/1.0 200 OK\n"));
    websWrite(wp, T("Server: %s\r\n"), WEBS_NAME);
    websWrite(wp, T("Pragma: no-cache\n"));
    websWrite(wp, T("Cache-control: no-cache\n"));
    websWrite(wp, T("Content-Type: text/html\n"));
    websWrite(wp, T("\n"));
    websWrite(wp, T("<html>\n<head>\n"));
    websWrite(wp, T("<title>My Title</title>"));
    websWrite(wp, T("<link rel=\"stylesheet\" href=\"/style/normal_ws.css\" type=\"text/css\">"));
    websWrite(wp, T("<meta http-equiv=\"content-type\" content=\"text/html; charset=iso-8859-1\">"));
    websWrite(wp, T("</head>\n<body onload=\"opener.location.reload();window.close();\">\n"));

	websWrite(wp, T("name: %s<br>\n"), comment);
	websWrite(wp, T("mac: %s<br>\n"), mac_address);
	websWrite(wp, T("ingress interface: %s<br>\n"), ingress_if);
	websWrite(wp, T("sip_address: %s<br>\n"), sip_address);
	websWrite(wp, T("sFromPort: %s<br>\n"), sprf);
	websWrite(wp, T("sToPort: %s<br>\n"), sprt);
	websWrite(wp, T("dip_address: %s<br>\n"), dip_address);
	websWrite(wp, T("dFromPort: %s<br>\n"), dprf);
	websWrite(wp, T("dToPort: %s<br>\n"), dprt);
	websWrite(wp, T("protocol: %s<br>\n"), protocol);
	websWrite(wp, T("application: %s<br>\n"), layer7);
	websWrite(wp, T("dscp: %s<br>\n"), dscp);
	websWrite(wp, T("remark: %s<br>\n"), remark);	

	websFooter(wp);
	websDone(wp, 200);
}

static void QoSDelete(webs_t wp, char_t *path, char_t *query)
{
	char tmp[32];
	char *orig_rule, *new_rule;
	int del_index, af_i, dp_i;

	char_t af[2], dp[2], name[64];
    printf("query = %s\n", query);
    sscanf(query, "%2s %2s %64s", af, dp, name);
    printf("af = %s\n dp = %s\n name = %s\n", af, dp, name);

	af_i = atoi(af);
	dp_i = atoi(dp);

	snprintf(tmp, sizeof(tmp), "QoSAF%dDP%d", af_i, dp_i);
	orig_rule = nvram_bufget(RT2860_NVRAM, tmp);
	if(!orig_rule || !strlen(orig_rule))
		goto err;

	del_index = qosGetIndexByName(af_i, dp_i, name);
	printf("del_index = %d\n", del_index);
	if(del_index == -1)
		goto err;

	new_rule = strdup(orig_rule);
	deleteNthValueMulti(&del_index, 1, new_rule, ';');

	nvram_bufset(RT2860_NVRAM, tmp, new_rule);
    nvram_commit(RT2860_NVRAM);

	free(new_rule);

	doSystem("qos_run c");

err:

    websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain\nPragma: no-cache\nCache-Control: no-cache\n\n"));
    websWrite(wp, T("success\n"));
    websDone(wp, 200);
}

static void QoSDeleteRules(webs_t wp, char_t *path, char_t *query)
{
    int i, j, rule_count;
    char_t name_buf[16];
    char_t *value;
    int *deleArray;

    char *rules = nvram_bufget(RT2860_NVRAM, "QoSRules");
    if(!rules || !strlen(rules) )
        return;

    rule_count = getRuleNums(rules);
    if(!rule_count)
        return;

    deleArray = (int *)malloc(rule_count * sizeof(int));

    for(i=0, j=0; i< rule_count; i++){
        snprintf(name_buf, 16, "del_qos_%d", i);
        value = websGetVar(wp, name_buf, NULL);
        if(value){
            deleArray[j++] = i;
        }
    }
    if(!j){
	    free(deleArray);
        websHeader(wp);
        websWrite(wp, T("You didn't select any rules to delete.<br>\n"));
        websFooter(wp);
        websDone(wp, 200);
        return;
    }

    deleteNthValueMulti(deleArray, j, rules, ';');

    nvram_set(RT2860_NVRAM, "QoSRules", rules);
    nvram_commit(RT2860_NVRAM);

	QoSRestart();

    websHeader(wp);
	websWrite(wp, T("Delete Rules:\n") );
	for(i=0; i<j; i++)
		websWrite(wp, "%d ", (deleArray[i] + 1) );

    websFooter(wp);
    websDone(wp, 200);
    free(deleArray);
}


static void QoSLoadDefaultProfile(webs_t wp, char_t *path, char_t *query)
{
    int i;
	for(i=0; i<QOS_PROFILE_ENTRYS_MAX ; i++){
		if(QOS_PROFILE[i].name){
			nvram_bufset(RT2860_NVRAM, QOS_PROFILE[i].name, QOS_PROFILE[i].value);
		}else{
			break;
		}
	}
	nvram_commit(RT2860_NVRAM);
	QoSRestart();

    websHeader(wp);
    websWrite(wp, T("Load default profile successfully.\n"));
    websFooter(wp);
    websDone(wp, 200);	
}

static void QoSSetup(webs_t wp, char_t *path, char_t *query)
{
	char_t *qos_enable, *upload_bandwidth, *download_bandwidth, *upload_bandwidth_custom, *download_bandwidth_custom;
	qos_enable = websGetVar(wp, T("QoSSelect"), T(""));
	upload_bandwidth = websGetVar(wp, T("UploadBandwidth"), T(""));
	download_bandwidth = websGetVar(wp, T("DownloadBandwidth"), T(""));
	upload_bandwidth_custom = websGetVar(wp, T("UploadBandwidth_Custom"), T(""));
	download_bandwidth_custom = websGetVar(wp, T("DownloadBandwidth_Custom"), T(""));

	if(!qos_enable || !strlen(qos_enable) || !upload_bandwidth || !download_bandwidth)
		return;

	if(!strcmp(upload_bandwidth, "custom"))
			if(!upload_bandwidth_custom)
				return;
	if(!strcmp(download_bandwidth, "custom"))
			if(!download_bandwidth_custom)
				return;

	if(!strcmp(qos_enable, "1") || !strcmp(qos_enable, "2")){
		if(!strlen(upload_bandwidth)) /* in fact we don't care about the download bandwidth */
			return;
	}

	nvram_bufset(RT2860_NVRAM, "QoSEnable", qos_enable);

	if(!strcmp(qos_enable, "1") || !strcmp(qos_enable, "2")){
		char postfix[16];
		strncpy(postfix, upload_bandwidth_custom, sizeof(postfix));
		if(!strchr(postfix, 'k') && !strchr(postfix, 'K')  && !strchr(postfix, 'm') && !strchr(postfix, 'M') )
			strncat(postfix, "k", sizeof(postfix));
		nvram_bufset(RT2860_NVRAM, "QoSUploadBandwidth_custom", postfix);
		strncpy(postfix, download_bandwidth_custom, sizeof(postfix));
		if(!strchr(postfix, 'k') && !strchr(postfix, 'K')  && !strchr(postfix, 'm') && !strchr(postfix, 'M') )
			strncat(postfix, "k", sizeof(postfix));
		nvram_bufset(RT2860_NVRAM, "QoSDownloadBandwidth_custom", postfix);
		nvram_bufset(RT2860_NVRAM, "QoSUploadBandwidth", upload_bandwidth);
		nvram_bufset(RT2860_NVRAM, "QoSDownloadBandwidth", download_bandwidth);
	}

	nvram_commit(RT2860_NVRAM);

	QoSRestart();

	websHeader(wp);

	websWrite(wp, T("qos_enable: %s<br>\n"), qos_enable);
	websWrite(wp, T("upload bandwidth: %s<br>\n"), upload_bandwidth);
	websWrite(wp, T("download bandwidth: %s<br>\n"), download_bandwidth);
	websFooter(wp);
	websDone(wp, 200);	
}

void formDefineQoS()
{
	websFormDefine(T("QoSAFAttribute"), QoSAFAttribute);
	websFormDefine(T("qosClassifier"), qosClassifier);
	websFormDefine(T("QoSSetup"), QoSSetup);
	websFormDefine(T("QoSDelete"), QoSDelete);
	websFormDefine(T("QoSDeleteRules"), QoSDeleteRules);
	websFormDefine(T("QoSLoadDefaultProfile"), QoSLoadDefaultProfile);
}

