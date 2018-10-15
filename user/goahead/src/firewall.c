/*
 *	firewall.c -- Firewall Settings 
 *
 *	Copyright (c) Ralink Technology Corporation All Rights Reserved.
 *
 *	$Id: firewall.c,v 1.33.2.2 2010-02-11 07:13:19 yy Exp $
 */

/*
 *	if  WAN or LAN ip changed, we must restart firewall.
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include "nvram.h"
#include "utils.h"
#include "webs.h"
#include "firewall.h"
#include "internet.h"

#define DD printf("---> %d\n", __LINE__);

static void websSysFirewall(webs_t wp, char_t *path, char_t *query);


char l7name[8192];						// export it for internet.c qos
										// (The actual string is about 7200 bytes.)

int getGoAHeadServerPort(void);

int isMacValid(char *str)
{
	int i, len = strlen(str);
	if(len != 17)
		return 0;

	for(i=0; i<5; i++){
		if( (!isxdigit( str[i*3])) || (!isxdigit( str[i*3+1])) || (str[i*3+2] != ':') )
			return 0;
	}
	return (isxdigit(str[15]) && isxdigit(str[16])) ? 1: 0;
}

static int isIpValid(char *str)
{
	struct in_addr addr;	// for examination
	if( (! strcmp(T("any"), str)) || (! strcmp(T("any/0"), str)))
		return 1;

	if(! (inet_aton(str, &addr))){
		printf("isIpValid(): %s is not a valid IP address.\n", str);
		return 0;
	}
	return 1;
}

static int isNumOnly(char *str){
	int i, len = strlen(str);
	for(i=0; i<len; i++){
		if((str[i] >= '0' && str[i] <= '9'))
			continue;
		return 0;
	}
	return 1;
}

static int isAllNumAndSlash(char *str){
	int i, len = strlen(str);
	for(i=0; i<len; i++){
		if( (str[i] >= '0' && str[i] <= '9') || str[i] == '.' || str[i] == '/' )
			continue;
		return 0;
	}
	return 1;
}

static int isOnlyOneSlash(char *str)
{
	int i, count=0;
	int len = strlen(str);
	for(i=0; i<len; i++)
		if( str[i] == '/')
			count++;
	return count <= 1 ? 1 : 0;
}

int isIpNetmaskValid(char *s)
{
	char str[32];
	char *slash;
	struct in_addr addr;    // for examination

	if(!s || !strlen(s)){
		return 0;
	}

	strncpy(str, s, sizeof(str));

    if( (!strcmp("any", str)) || (!strcmp("any/0", str)))
        return 1;

	if (!isAllNumAndSlash(str)){
		return 0;
	}

	if(!isOnlyOneSlash(str)){
		return 0;
	}

	slash = strchr(str, '/');
	if(slash){
		int mask;

		*slash = '\0';
		slash++;
		if(!strlen(slash)){
			return 0;
		}

		if(!isNumOnly(slash)){
			return 0;
		}

		mask = atoi(slash);
		if(mask < 0 || mask > 32){
			return 0;
		}
	}

	if(! (inet_aton(str, &addr))){
        printf("isIpNetmaskValid(): %s is not a valid IP address.\n", str);
        return 0;
    }
    return 1;
}

static int getDMZEnableASP(int eid, webs_t wp, int argc, char_t **argv)
{
	int type, value;
	char *dmze = (char *)nvram_bufget(RT2860_NVRAM, "DMZEnable");
	if(dmze)
		value = atoi(dmze);
	else
		value = 0;

	if( ejArgs(argc, argv, T("%d"), &type) == 1){
		if(type == value)
			websWrite(wp, T("selected"));
		else
			websWrite(wp, T(" "));
		return 0;
	}
	return -1;        
}

static int  getPortForwardEnableASP(int eid, webs_t wp, int argc, char_t **argv)
{
	int type, value;
	char *pfe = (char *)nvram_bufget(RT2860_NVRAM, "PortForwardEnable");

	if(pfe)
		value = atoi(pfe);
	else
		value = 0;

	if( ejArgs(argc, argv, T("%d"), &type) == 1){
		if(type == value)
			websWrite(wp, T("selected"));
		else
			websWrite(wp, T(" "));
		return 0;
	}
	return -1;
}

static int  getSinglePortForwardEnableASP(int eid, webs_t wp, int argc, char_t **argv)
{
	int type, value;
	char *spfe = (char *)nvram_bufget(RT2860_NVRAM, "SinglePortForwardEnable");

	if(spfe)
		value = atoi(spfe);
	else
		value = 0;

	if( ejArgs(argc, argv, T("%d"), &type) == 1){
		if(type == value)
			websWrite(wp, T("selected"));
		else
			websWrite(wp, T(" "));
		return 0;
	}
	return -1;
}

static int  getIPPortFilterEnableASP(int eid, webs_t wp, int argc, char_t **argv)
{
	int type, value;
	char *pfe = (char *)nvram_bufget(RT2860_NVRAM, "IPPortFilterEnable");

	if(pfe)
		value = atoi(pfe);
	else
		value = 0;

	if( ejArgs(argc, argv, T("%d"), &type) == 1){
		if(type == value)
			websWrite(wp, T("selected"));
		else
			websWrite(wp, T(" "));
		return 0;
	}
	return -1;
}

/*
 * hide the possible "error/warn" message when deleting a non-exist chain.
 */
static void iptablesForwardFilterClear(void)
{
	doSystem("iptables -F -t filter 1>/dev/null 2>&1");
}

/*
static void iptablesForwardFilterFlush(void)
{
	doSystem("iptables -t filter -F FORWARD  1>/dev/null 2>&1");
}
*/

static void iptablesIPPortFilterFlush(void)
{
	doSystem("iptables -F %s 1>/dev/null 2>&1", IPPORT_FILTER_CHAIN);
}

static void iptablesIPPortFilterClear(void)
{
	doSystem("iptables -D FORWARD -j %s 1>/dev/null 2>&1", IPPORT_FILTER_CHAIN);
	doSystem("iptables -F %s 1>/dev/null 2>&1", IPPORT_FILTER_CHAIN);
}

static void iptablesWebContentFilterClear(void)
{
	doSystem("iptables -D FORWARD -j %s  1>/dev/null 2>&1", WEB_FILTER_CHAIN);
	doSystem("iptables -F %s  1>/dev/null 2>&1", WEB_FILTER_CHAIN);
}

static void iptablesMaliciousFilterClear(void)
{
	doSystem("iptables -D FORWARD -j %s 1>/dev/null 2>&1", MALICIOUS_FILTER_CHAIN);
	doSystem("iptables -F %s  1>/dev/null 2>&1", MALICIOUS_FILTER_CHAIN);

	doSystem("iptables -D INPUT -j %s 1>/dev/null 2>&1", MALICIOUS_INPUT_FILTER_CHAIN);
	doSystem("iptables -F %s  1>/dev/null 2>&1", MALICIOUS_INPUT_FILTER_CHAIN);

}

static void iptablesMaliciousFilterFlush(void)
{
	doSystem("iptables -F %s  1>/dev/null 2>&1", MALICIOUS_FILTER_CHAIN);
	doSystem("iptables -F %s  1>/dev/null 2>&1", SYNFLOOD_FILTER_CHAIN);
	doSystem("iptables -A %s -p tcp --syn -j %s 1>/dev/null 2>&1", MALICIOUS_FILTER_CHAIN, SYNFLOOD_FILTER_CHAIN);

	doSystem("iptables -F %s  1>/dev/null 2>&1", MALICIOUS_INPUT_FILTER_CHAIN);
	doSystem("iptables -F %s  1>/dev/null 2>&1", SYNFLOOD_INPUT_FILTER_CHAIN);
	doSystem("iptables -A %s -p tcp --syn -j %s 1>/dev/null 2>&1", MALICIOUS_INPUT_FILTER_CHAIN, SYNFLOOD_INPUT_FILTER_CHAIN);

	/* disable tcp_syncookie capacity */
	doSystem("echo 0 > /proc/sys/net/ipv4/tcp_syncookies 2>&1 1>/dev/null");
}

static void iptablesDMZFlush(void){
    doSystem("iptables -t nat -F %s 1>/dev/null 2>&1", DMZ_CHAIN);
}

static void iptablesPortForwardFlush(void){
    doSystem("iptables -t nat -F %s 1>/dev/null 2>&1", PORT_FORWARD_CHAIN);
}

static void iptablesSinglePortForwardFlush(void){
    doSystem("iptables -t nat -F %s 1>/dev/null 2>&1", PORT_FORWARD_CHAIN);
}
static void iptablesDMZClear(void){
	doSystem("iptables -t nat -D PREROUTING -j %s 1>/dev/null 2>&1", DMZ_CHAIN);	// remove rule in PREROUTING chain
	doSystem("iptables -t nat -F %s 1>/dev/null 2>&1; iptables -t nat -X %s  1>/dev/null 2>&1", DMZ_CHAIN, DMZ_CHAIN);
}

static void iptablesPortForwardClear(void){
	doSystem("iptables -t nat -D PREROUTING -j %s 1>/dev/null 2>&1", PORT_FORWARD_CHAIN);
	doSystem("iptables -t nat -F %s  1>/dev/null 2>&1; iptables -t nat -X %s  1>/dev/null 2>&1", PORT_FORWARD_CHAIN, PORT_FORWARD_CHAIN);
}



static void iptablesAllFilterClear(void)
{
	iptablesForwardFilterClear();
	iptablesIPPortFilterClear();
	iptablesWebContentFilterClear();
	iptablesMaliciousFilterClear();

	doSystem("iptables -P INPUT ACCEPT");
	doSystem("iptables -P OUTPUT ACCEPT");
	doSystem("iptables -P FORWARD ACCEPT");
}

static void iptablesAllNATClear(void)
{
	iptablesPortForwardClear();
	iptablesDMZClear();
}

#if 0
char *insert(char *subs, int index, char *str, char delimit)
{
	int i=0;
	char *begin, *end;
	char *result = (char *)calloc(1, sizeof(char ) * (strlen(str) + strlen(subs) + 1 + 1));

	begin = str;
	end = strchr(str, delimit);
	while(end){
		if(i == index)
			break;
		begin = end;
		end = strchr(begin+1, delimit);
		i++;
	}
	if(begin == str){
		if(strlen(str) == 0){
			strcpy(result, subs);
		}else{
			if(index == 0){
				sprintf(result, "%s%c%s", subs, delimit, str);
			}else{
				sprintf(result, "%s%c%s", str, delimit, subs);
			}
		}
	}else if(end == NULL && i != index){
		sprintf(result, "%s%c%s", str, delimit, subs);
	}else{
		strncpy(result, str, begin - str);
		sprintf(result, "%s%c", result, delimit);
		strcat(result, subs);
		strcat(result, begin);
	}
	return result;
}

char *replace(char *subs, int index, char *str, char delimit)
{
	int del[1];
	char *result;
	char *dup = strdup(str);
	del[0] = index;
	deleteNthValueMulti(del, 1, dup, delimit);
	result = insert(subs, index, dup, delimit);
	free(dup);
	return result;
}
#endif
static int getNums(char *value, char delimit)
{
	char *pos = value;
    int count=1;
    if(!pos)
    	return 0;
	while( (pos = strchr(pos, delimit))){
		pos = pos+1;
		count++;
	}
	return count;
}

static void makeDMZRule(char *buf, int len, char *wan_name, char *ip_address)
{
	int rc = snprintf(buf, len, "iptables -t nat -A %s -j DNAT -i %s -p udp --dport ! %d --to %s", DMZ_CHAIN, wan_name, getGoAHeadServerPort(), ip_address);
	snprintf(buf+rc, len, ";iptables -t nat -A %s -j DNAT -i %s -p tcp --dport ! %d --to %s", DMZ_CHAIN, wan_name,	getGoAHeadServerPort(), ip_address);
}

/*
 *
 */
static void makeIPPortFilterRule(char *buf, int len, char *mac_address,
char *sip_1, char *sip_2, int sprf_int, int sprt_int, 
char *dip_1, char *dip_2, int dprf_int, int dprt_int, int proto, int action)
{
		int rc = 0;
		char *pos = buf;
        char *spifw = (char *)nvram_bufget(RT2860_NVRAM, "SPIFWEnabled");

		switch(action){
		case ACTION_DROP:
		    if (atoi(spifw) == 0)
			rc = snprintf(pos, len-rc, 
				"iptables -A %s ", IPPORT_FILTER_CHAIN);
		    else
			rc = snprintf(pos, len-rc, 
				"iptables -A %s -m state --state NEW,INVALID ", IPPORT_FILTER_CHAIN);
			break;
		case ACTION_ACCEPT:
			rc = snprintf(pos, len-rc, 
				"iptables -A %s ", IPPORT_FILTER_CHAIN);
			break;
		}
		pos = pos + rc;

		// write mac address
		if(mac_address && strlen(mac_address)){
			rc = snprintf(pos, len-rc, "-m mac --mac-source %s ", mac_address);
			pos = pos+rc;
		}

		// write source ip
		rc = snprintf(pos, len-rc, "-s %s ", sip_1);
		pos = pos+rc;
		
		// write dest ip
		rc = snprintf(pos, len-rc, "-d %s ", dip_1);
		pos = pos+rc;

		// write protocol type
		if(proto == PROTO_NONE){
			rc = snprintf(pos, len-rc, " ");
			pos = pos + rc;
		}else if(proto == PROTO_ICMP){
			rc = snprintf(pos, len-rc, "-p icmp ");
			pos = pos + rc;
		}else{
			if(proto == PROTO_TCP)
				rc = snprintf(pos, len-rc, "-p tcp ");
			else if (proto == PROTO_UDP)
				rc = snprintf(pos, len-rc, "-p udp ");
			pos = pos + rc;

			// write source port
			if(sprf_int){
				if(sprt_int)
					rc = snprintf(pos, len-rc, "--sport %d:%d ", sprf_int, sprt_int);
				else
					rc = snprintf(pos, len-rc, "--sport %d ", sprf_int);
				pos = pos+rc;
			}

			// write dest port
			if(dprf_int){
				if(dprt_int)
					rc = snprintf(pos, len-rc, "--dport %d:%d ", dprf_int, dprt_int);
				else
					rc = snprintf(pos, len-rc, "--dport %d ", dprf_int);
				pos = pos+rc;
			}
		}

		switch(action){
		case ACTION_DROP:			// 1 == ENABLE--DROP mode
			rc = snprintf(pos, len-rc, "-j DROP");
			break;
		case ACTION_ACCEPT:			// 2 == ENABLE--ACCEPT mode
			rc = snprintf(pos, len-rc, "-j ACCEPT");
			break;
		}
}

static void makePortForwardRule(char *buf, int len, char *wan_name, char *ip_address, int proto, int prf_int, int prt_int)
{
		int rc = 0;
		char *pos = buf;
		char wanname[16];
		char wan_ip[16];

		snprintf(wanname, sizeof(wanname), "%s.2", nvram_bufget(RT2860_NVRAM, "eth2"));
		getIfIp(wanname, wan_ip);
		rc = snprintf(pos, len-rc, 
			//"iptables -t nat -A %s -j DNAT -i %s ", PORT_FORWARD_CHAIN, wan_name);
			"iptables -t nat -A %s -j DNAT -d %s ", PORT_FORWARD_CHAIN, wan_ip);
		pos = pos + rc;

		// write protocol type
		if(proto == PROTO_TCP)
			rc = snprintf(pos, len-rc, "-p tcp ");
		else if (proto == PROTO_UDP)
			rc = snprintf(pos, len-rc, "-p udp ");
		else if (proto == PROTO_TCP_UDP)
			rc = snprintf(pos, len-rc, " ");
		pos = pos + rc;

		// write port
		if(prt_int != 0)
			rc = snprintf(pos, len-rc, "--dport %d:%d ", prf_int, prt_int);
		else
			rc = snprintf(pos, len-rc, "--dport %d ", prf_int);
		pos = pos + rc;

		// write remote ip
		rc = snprintf(pos, len-rc, "--to %s ", ip_address);
		pos = pos + rc;

		rc = snprintf(pos, len-rc, "; iptables -t nat -A POSTROUTING -j SNAT --to %s ", wan_ip);
}


static void makeSinglePortForwardRule(char *buf, int len, char *wan_name, char *ip_address, int proto, int publicPort_int, int privatePort_int)
{
		int rc = 0;
		char *pos = buf;

		rc = snprintf(pos, len-rc, 
			"iptables -t nat -A %s -j DNAT -i %s ", PORT_FORWARD_CHAIN, wan_name);
		pos = pos + rc;

		// write protocol type
		if(proto == PROTO_TCP)
			rc = snprintf(pos, len-rc, "-p tcp ");
		else if (proto == PROTO_UDP)
			rc = snprintf(pos, len-rc, "-p udp ");
		else if (proto == PROTO_TCP_UDP)
			rc = snprintf(pos, len-rc, " ");
		pos = pos + rc;

		// write public port
		rc = snprintf(pos, len-rc, "--dport %d ", publicPort_int);
		pos = pos + rc;

		// write remote ip & private port
		rc = snprintf(pos, len-rc, "--to-destination %s:%d", ip_address, privatePort_int);
		//rc = snprintf(pos, len-rc, "--to %s ", ip_address);
}


static void iptablesRemoteManagementRun(void)
{
	char *rmE = (char *)nvram_bufget(RT2860_NVRAM, "RemoteManagement");
	char *opmode = (char *)nvram_bufget(RT2860_NVRAM, "OperationMode");
	char *spifw = (char *)nvram_bufget(RT2860_NVRAM, "SPIFWEnabled");

	if(!opmode)
		return;

	// "Gateway mode" only
	if(strcmp(opmode , "1"))
		return;

	if(rmE && atoi(rmE) == 1)
		return;

	if (atoi(spifw) == 0)
		;//doSystem("iptables -A %s -i %s -j ACCEPT", getWanIfNamePPP());
	else
		doSystem("iptables -A %s -i %s -m state --state RELATED,ESTABLISHED -j ACCEPT", MALICIOUS_INPUT_FILTER_CHAIN, getWanIfNamePPP());
	if(getOnePortOnly()){
		// make the web server to be reachable.
		if (atoi(spifw) == 0)
			doSystem("iptables -A %s -i %s -d 172.32.1.254 -p tcp --dport 80 -j ACCEPT", MALICIOUS_INPUT_FILTER_CHAIN, getWanIfNamePPP());
		else
			doSystem("iptables -A %s -i %s -m state -d 172.32.1.254 -p tcp --dport 80 --state NEW,INVALID -j ACCEPT", MALICIOUS_INPUT_FILTER_CHAIN, getWanIfNamePPP());
	}
	if (atoi(spifw) == 0)
		doSystem("iptables -A %s -i %s -p tcp --dport 80 -j DROP", MALICIOUS_INPUT_FILTER_CHAIN , getWanIfNamePPP());
	else
		doSystem("iptables -A %s -i %s -m state -p tcp --dport 80 --state NEW,INVALID -j DROP", MALICIOUS_INPUT_FILTER_CHAIN, getWanIfNamePPP());

	return;
}

static void iptablesMaliciousFilterRun(void)
{
	char *wpf = (char *)nvram_bufget(RT2860_NVRAM, "WANPingFilter");
	char *bps = (char *)nvram_bufget(RT2860_NVRAM, "BlockPortScan");
	char *bsf = (char *)nvram_bufget(RT2860_NVRAM, "BlockSynFlood");

	if(!wpf || !bps || !bsf)
		return;

	if(!atoi(wpf)){
		;// do nothing
	}else{
		doSystem("iptables -A %s -i %s -p icmp -j DROP", MALICIOUS_INPUT_FILTER_CHAIN, getWanIfNamePPP());
	}

	if(!atoi(bps)){
		;//do nothing
	}else{
		/*
		 *  Port scan rules
		 */
		// nmap- Xmas
		doSystem("iptables -A %s -p tcp --tcp-flags ALL FIN,URG,PSH -j DROP", MALICIOUS_FILTER_CHAIN);
		// nmap- PSH
		doSystem("iptables -A %s -p tcp --tcp-flags ALL SYN,RST,ACK,FIN,URG -j DROP", MALICIOUS_FILTER_CHAIN);
		// Null
		doSystem("iptables -A %s -p tcp --tcp-flags ALL NONE -j DROP", MALICIOUS_FILTER_CHAIN);
		// SYN/RST
		doSystem("iptables -A %s -p tcp --tcp-flags SYN,RST SYN,RST -j DROP", MALICIOUS_FILTER_CHAIN);
		// SYN/FIN
		doSystem("iptables -A %s -p tcp --tcp-flags SYN,FIN SYN,FIN ", MALICIOUS_FILTER_CHAIN);

		doSystem("iptables -A %s -p tcp --tcp-flags ALL FIN,URG,PSH -j DROP", MALICIOUS_INPUT_FILTER_CHAIN);
		doSystem("iptables -A %s -p tcp --tcp-flags ALL SYN,RST,ACK,FIN,URG -j DROP", MALICIOUS_INPUT_FILTER_CHAIN);
		doSystem("iptables -A %s -p tcp --tcp-flags ALL NONE -j DROP", MALICIOUS_INPUT_FILTER_CHAIN);
		doSystem("iptables -A %s -p tcp --tcp-flags SYN,RST SYN,RST -j DROP", MALICIOUS_INPUT_FILTER_CHAIN);
		doSystem("iptables -A %s -p tcp --tcp-flags SYN,FIN SYN,FIN ", MALICIOUS_INPUT_FILTER_CHAIN);
	}

	if(!atoi(bsf)){
		;// do nothing
	}else{
		/*
		 * SYN flooding fules
		 */
		 //doSystem("iptables -A %s -m limit --limit 1/s --limit-burst 10 -j RETURN", SYNFLOOD_FILTER_CHAIN);
		 //doSystem("iptables -A %s -j DROP", SYNFLOOD_FILTER_CHAIN);
		 //doSystem("iptables -A %s -m limit --limit 1/s --limit-burst 10 -j RETURN", SYNFLOOD_INPUT_FILTER_CHAIN);
		 //doSystem("iptables -A %s -j DROP", SYNFLOOD_INPUT_FILTER_CHAIN);
		 doSystem("echo 1 > /proc/sys/net/ipv4/tcp_syncookies 2>&1 1>/dev/null");
	}
	return;
}


static void iptablesDMZRun(void)
{
//	char wan_ip[16];
	char cmd[1024], *ip_address;
	char *dmz_enable = (char *)nvram_bufget(RT2860_NVRAM, "DMZEnable");
	if(!dmz_enable){
		printf("Warning: can't find \"DMZEnable\" in flash\n");
		return;
	}
	if(!atoi(dmz_enable))
		return;

	ip_address = (char *)nvram_bufget(RT2860_NVRAM, "DMZIPAddress");
	if(!ip_address){
		printf("Warning: can't find \"DMZIPAddress\" in flash\n");
		return;
	}

//	if ( getIfIp(getWanIfNamePPP(), wan_ip) == -1)
//       return;
	makeDMZRule(cmd, sizeof(cmd), getWanIfNamePPP(), ip_address);
	doSystem(cmd);
	return;
}

static void iptablesIPPortFilterRun(void)
{
	int i=0;
	char rec[256];
	char cmd[1024];
	int sprf_int, sprt_int, proto, action;
	int dprf_int, dprt_int;
	char sprf[8], sprt[8], protocol[8];
	char dprf[8], dprt[8];
	char mac_address[32];
	char sip_1[32], sip_2[32], action_str[4];
	char dip_1[32], dip_2[32];
    char *firewall_enable, *default_policy, *rule;
    char *spifw = (char *)nvram_bufget(RT2860_NVRAM, "SPIFWEnabled");
    int mode;
	
    firewall_enable = (char *)nvram_bufget(RT2860_NVRAM, "IPPortFilterEnable");
    if(!firewall_enable){
        printf("Warning: can't find \"IPPortFilterEnable\" in flash.\n");
        return;
    }
    mode = atoi(firewall_enable);
    if(!mode)
		return;

	rule = (char *)nvram_bufget(RT2860_NVRAM, "IPPortFilterRules");
	if(!rule){
		printf("Warning: can't find \"IPPortFilterRules\" in flash.\n");
		return;
	}

	default_policy = (char *)nvram_bufget(RT2860_NVRAM, "DefaultFirewallPolicy");
	// add the default policy to the end of FORWARD chain
	if(!default_policy)
		default_policy = "0";

	if(atoi(default_policy) == 1){
		//the default policy is drop
	    if (atoi(spifw) == 0)
			;//doSystem("iptables -t filter -A %s -j ACCEPT", IPPORT_FILTER_CHAIN);
	    else
			doSystem("iptables -t filter -A %s -m state --state RELATED,ESTABLISHED -j ACCEPT", IPPORT_FILTER_CHAIN);
	}

	while( (getNthValueSafe(i++, rule, ';', rec, sizeof(rec)) != -1) ){
        // get sip 1
        if((getNthValueSafe(0, rec, ',', sip_1, sizeof(sip_1)) == -1)){
			continue;
		}
		if(!isIpNetmaskValid(sip_1)){
			continue;
		}

		// we dont support ip range yet.
        // get sip 2
        //if((getNthValueSafe(1, rec, ',', sip_2, sizeof(sip_2)) == -1))
        //	continue;
		//if(!isIpValid(sip_2))
		//	continue;

		// get source port range "from"
		if((getNthValueSafe(2, rec, ',', sprf, sizeof(sprf)) == -1)){
			continue;
		}
		if( (sprf_int = atoi(sprf)) > 65535)
			continue;
		// get dest port range "to"
		if((getNthValueSafe(3, rec, ',', sprt, sizeof(sprt)) == -1)){
			continue;
		}
		if( (sprt_int = atoi(sprt)) > 65535)
			continue;

		// Destination Part
        // get dip 1
		if((getNthValueSafe(4, rec, ',', dip_1, sizeof(dip_1)) == -1)){
			continue;
		}
		if(!isIpNetmaskValid(dip_1)){
			continue;
		}
		// we dont support ip range yet
        // get sip 2
        //if((getNthValueSafe(5, rec, ',', dip_2, sizeof(dip_2)) == -1))
        //    continue;
        //if(!isIpValid(dip_2))
        //    continue;

		// get source port range "from"
		if((getNthValueSafe(6, rec, ',', dprf, sizeof(dprf)) == -1)){
			continue;
		}
		if( (dprf_int = atoi(dprf)) > 65535)
			continue;

		// get dest port range "to"
		if((getNthValueSafe(7, rec, ',', dprt, sizeof(dprt)) == -1)){
			continue;
		}
		if( (dprt_int = atoi(dprt)) > 65535)
			continue;


		// get protocol
		if((getNthValueSafe(8, rec, ',', protocol, sizeof(protocol)) == -1))
			continue;
		proto = atoi(protocol);

		// get action
        if((getNthValueSafe(9, rec, ',', action_str, sizeof(action_str)) == -1)){
            continue;
        }
        action = atoi(action_str);

        // getNthValueSafe(10) is "comment".

        // get mac address
        if((getNthValueSafe(11, rec, ',', mac_address, sizeof(mac_address)) == -1))
            continue;
		if(strlen(mac_address)){
	        if(!isMacValid(mac_address))
	        	continue;
		}

        //TODO:
		// supposed to do validation here but  we didn't do it because code size.
/*
# iptables example
# iptables -t nat -A POSTROUTING -o eth0  -s 10.10.10.0/24 -j MASQUERADE
# iptables -A FORWARD -m physdev --physdev-in ra0 --physdev-out eth2 -m state --state ESTABLISHED,RELATED -j ACCEPT
# iptables -A FORWARD -m physdev --physdev-in eth0 --physdev-out eth2 -j DROP
# iptables -A FORWARD -i eth0 -o eth2 -j DROP
# iptables -A FORWARD -p tcp --dport 139 -j DROP
# iptables -A FORWARD -i eth0 -o eth2 -m state --state NEW,INVALID -p tcp --dport 80 -j DROP
*/
		makeIPPortFilterRule(cmd, sizeof(cmd), mac_address, sip_1, sip_2, sprf_int, sprt_int, dip_1, dip_2, dprf_int, dprt_int, proto, action);
		doSystem(cmd);
	}


	switch(atoi(default_policy)){
	case 0:
		doSystem("iptables -t filter -A %s -j ACCEPT", IPPORT_FILTER_CHAIN);
		break;
	case 1:
		doSystem("iptables -t filter -A %s -j DROP", IPPORT_FILTER_CHAIN);
		break;
	}

}

static void iptablesPortForwardRun(void)
{
	int i=0;
	char rec[256];
	char cmd[1024];
	char wan_name[16];

	int prf_int, prt_int, proto;
	char ip_address[32], prf[8], prt[8], protocol[8];

    char *firewall_enable, *rule;

    firewall_enable = (char *)nvram_bufget(RT2860_NVRAM, "PortForwardEnable");
    if(!firewall_enable){
        printf("Warning: can't find \"PortForwardEnable\" in flash\n");
        return;
    }
    if(atoi(firewall_enable)){
        rule = (char *)nvram_bufget(RT2860_NVRAM, "PortForwardRules");
        if(!rule){
            printf("Warning: can't find \"PortForwardRules\" in flash\n");
            return ;
        }
    }else
		return;

//  if ( getIfIp(getWanIfNamePPP(), wan_ip) == -1)
//      return;
	strncpy(wan_name, getWanIfNamePPP(), sizeof(wan_name)-1);

	while( (getNthValueSafe(i++, rule, ';', rec, sizeof(rec)) != -1) ){
		// get ip address
		if((getNthValueSafe(0, rec, ',', ip_address, sizeof(ip_address)) == -1)){
			printf("prf = %s\n", prf);	
			continue;
		}
		if(!isIpValid(ip_address))
			continue;

		// get port range "from"
		if((getNthValueSafe(1, rec, ',', prf, sizeof(prf)) == -1)){
			printf("prf = %s\n", prf);	
			continue;
		}
		if( (prf_int = atoi(prf)) == 0 || prf_int > 65535)
			continue;

		// get port range "to"
		if((getNthValueSafe(2, rec, ',', prt, sizeof(prt)) == -1)){
			printf("prt = %s\n", prt);	
			continue;
		}
		if( (prt_int = atoi(prt)) > 65535)
			continue;

		// get protocol
		if((getNthValueSafe(3, rec, ',', protocol, sizeof(protocol)) == -1))
			continue;
		proto = atoi(protocol);
		switch(proto){
			case PROTO_TCP:
			case PROTO_UDP:
				makePortForwardRule(cmd, sizeof(cmd), wan_name, ip_address, proto, prf_int, prt_int);
				doSystem(cmd);
				break;
			case PROTO_TCP_UDP:
				makePortForwardRule(cmd, sizeof(cmd), wan_name, ip_address, PROTO_TCP, prf_int, prt_int);
				doSystem(cmd);
				makePortForwardRule(cmd, sizeof(cmd), wan_name, ip_address, PROTO_UDP, prf_int, prt_int);
				doSystem(cmd);
				break;
			default:
				continue;
		}
	}
}

static void iptablesSinglePortForwardRun(void)
{
	int i=0;
	char rec[256];
	char cmd[1024];
	char wan_name[16];

	int publicPort_int, privatePort_int, proto;
	char ip_address[32], publicPort[8], privatePort[8], protocol[8];

    char *firewall_enable, *rule;

    firewall_enable = (char *)nvram_bufget(RT2860_NVRAM, "SinglePortForwardEnable");
    if(!firewall_enable){
        printf("Warning: can't find \"SinglePortForwardEnable\" in flash\n");
        return;
    }
    if(atoi(firewall_enable)){
        rule = (char *)nvram_bufget(RT2860_NVRAM, "SinglePortForwardRules");
        if(!rule){
            printf("Warning: can't find \"SinglePortForwardRules\" in flash\n");
            return ;
        }
    }else
		return;

//  if ( getIfIp(getWanIfNamePPP(), wan_ip) == -1)
//      return;
	strncpy(wan_name, getWanIfNamePPP(), sizeof(wan_name)-1);

	while( (getNthValueSafe(i++, rule, ';', rec, sizeof(rec)) != -1) ){
		// get ip address
		if((getNthValueSafe(0, rec, ',', ip_address, sizeof(ip_address)) == -1)){
			printf("ip_address = %s\n", ip_address);	
			continue;
		}
		if(!isIpValid(ip_address))
			continue;

		// get public port
		if((getNthValueSafe(1, rec, ',', publicPort, sizeof(publicPort)) == -1)){
			printf("publicPort = %s\n", publicPort);	
			continue;
		}
		if( (publicPort_int = atoi(publicPort)) == 0 || publicPort_int > 65535)
			continue;

		// get private port
		if((getNthValueSafe(2, rec, ',', privatePort, sizeof(privatePort)) == -1)){
			printf("privatePort = %s\n", privatePort);	
			continue;
		}
		if( (privatePort_int = atoi(privatePort)) == 0 || privatePort_int > 65535)
			continue;

		// get protocol
		if((getNthValueSafe(3, rec, ',', protocol, sizeof(protocol)) == -1))
			continue;
		proto = atoi(protocol);
		switch(proto){
			case PROTO_TCP:
			case PROTO_UDP:
				makeSinglePortForwardRule(cmd, sizeof(cmd), wan_name, ip_address, proto, publicPort_int, privatePort_int);
				doSystem(cmd);
				break;
			case PROTO_TCP_UDP:
				makeSinglePortForwardRule(cmd, sizeof(cmd), wan_name, ip_address, PROTO_TCP, publicPort_int, privatePort_int);
				doSystem(cmd);
				makeSinglePortForwardRule(cmd, sizeof(cmd), wan_name, ip_address, PROTO_UDP, publicPort_int, privatePort_int);
				doSystem(cmd);
				break;
			default:
				continue;
		}
	}
}

static void iptablesAllFilterRun(void)
{
	iptablesIPPortFilterRun();

	iptablesWebsFilterRun();

	/* system filter */
	iptablesRemoteManagementRun();

	iptablesMaliciousFilterRun();
}

static void iptablesAllNATRun(void)
{
	iptablesPortForwardRun();
	iptablesSinglePortForwardRun();
	iptablesDMZRun();
}

inline int getRuleNums(char *rules){
	return getNums(rules, ';');
}

static int getDefaultFirewallPolicyASP(int eid, webs_t wp, int argc, char_t **argv)
{
	int value;
	char *p = (char *)nvram_bufget(RT2860_NVRAM, "DefaultFirewallPolicy");
	int default_policy;
	if(!p)
		default_policy = 0;
	else
		default_policy = atoi(p);

	if( ejArgs(argc, argv, T("%d"), &value) != 1){
		return -1;
	}

	if(default_policy == value )
		websWrite(wp, T(" selected "));
	return 0;	
}

static int checkIfUnderBridgeModeASP(int eid, webs_t wp, int argc, char_t **argv)
{
	char *mode = (char *)nvram_bufget(RT2860_NVRAM, "OperationMode");
	if(!mode)
		return -1;	// fatal error, make ASP engine complained.
	if(atoi(mode) == 0)	// bridge mode
		websWrite(wp, T(HTML_NO_FIREWALL_UNDER_BRIDGE_MODE));
	return 0;
}

/*
 * ASP function
 */
static int getPortForwardRuleNumsASP(int eid, webs_t wp, int argc, char_t **argv)
{
    char *rules = (char *)nvram_bufget(RT2860_NVRAM, "PortForwardRules");
	if(!rules || !strlen(rules) ){
		websWrite(wp, T("0"));
		return 0;
	}
	websWrite(wp, T("%d"), getRuleNums(rules));
	return 0;
}

/*
 * ASP function
 */
static int getSinglePortForwardRuleNumsASP(int eid, webs_t wp, int argc, char_t **argv)
{
    char *rules = (char *)nvram_bufget(RT2860_NVRAM, "SinglePortForwardRules");
	if(!rules || !strlen(rules) ){
		websWrite(wp, T("0"));
		return 0;
	}
	websWrite(wp, T("%d"), getRuleNums(rules));
	return 0;
}


/*
 * ASP function
 */
static int getIPPortRuleNumsASP(int eid, webs_t wp, int argc, char_t **argv)
{
    char *rules = (char *)nvram_bufget(RT2860_NVRAM, "IPPortFilterRules");
	if(!rules || !strlen(rules) ){
		websWrite(wp, T("0"));
		return 0;
	}

	websWrite(wp, T("%d"), getRuleNums(rules));
	return 0;
}

/*
 * ASP function
 */
static int showPortForwardRulesASP(int eid, webs_t wp, int argc, char_t **argv)
{
	int i=0;
	int prf_int, prt_int, proto;
	char ip_address[32], prf[8], prt[8], comment[16], protocol[8];
	char rec[128];
	char *rules = (char *)nvram_bufget(RT2860_NVRAM, "PortForwardRules");
	if(!rules)
		return 0;
	if(!strlen(rules))
		return 0;

	/* format is :
	 * [ip],[port_from],[port_to],[protocol],[comment],;
	 */
	while(getNthValueSafe(i++, rules, ';', rec, sizeof(rec)) != -1 ){
		//printf("i=%d : \n",i);
		// get ip address
		if((getNthValueSafe(0, rec, ',', ip_address, sizeof(ip_address)) == -1)){
			//printf("ip fail!!\n");
			continue;
		}
		//printf("ip = %s \n");
		if(!isIpValid(ip_address)){
			continue;
		}

		// get port range "from"
		if((getNthValueSafe(1, rec, ',', prf, sizeof(prf)) == -1)){
			//printf("prf fail!!\n");
			continue;
		}
		if( (prf_int = atoi(prf)) == 0 || prf_int > 65535){
			continue;
		}
		//printf("prf = %d \n",prf_int);

		// get port range "to"
		if((getNthValueSafe(2, rec, ',', prt, sizeof(prt)) == -1)){
			//printf("prt fail!!\n");
			continue;
		}

		if( (prt_int = atoi(prt)) > 65535){
			continue;
		}
		//printf("prt = %d \n",prt_int);

		// get protocol
		if((getNthValueSafe(3, rec, ',', protocol, sizeof(protocol)) == -1)){
			//printf("proto fail!!\n");
			continue;
		}
		proto = atoi(protocol);
		//printf("proto = %d \n",proto);
		switch(proto){
			case PROTO_TCP:
			case PROTO_UDP:
			case PROTO_TCP_UDP:
				break;
			default:
				continue;
		}
	
		if((getNthValueSafe(4, rec, ',', comment, sizeof(comment)) == -1)){
			continue;
		}

		//printf("All items are ok!!\n");

		websWrite(wp, T("<tr>\n"));
		// output No.
		websWrite(wp, T("<td> %d&nbsp; <input type=\"checkbox\" name=\"delRule%d\"> </td>"), i, i-1 );

		// output IP address
		websWrite(wp, T("<td align=center> %s </td>"), ip_address);

		// output Port Range
		if(prt_int)
			websWrite(wp, T("<td align=center> %d - %d </td>"), prf_int, prt_int);
		else
			websWrite(wp, T("<td align=center> %d </td>"), prf_int);

		// output Protocol
        switch(proto){
            case PROTO_TCP:
				websWrite(wp, T("<td align=center> TCP </td>"));
				break;
            case PROTO_UDP:
				websWrite(wp, T("<td align=center> UDP </td>"));
				break;
            case PROTO_TCP_UDP:
				websWrite(wp, T("<td align=center> TCP + UDP </td>"));
				break;
		}

		// output Comment
		if(strlen(comment))
			websWrite(wp, T("<td align=center> %s</td>"), comment);
		else
			websWrite(wp, T("<td align=center> &nbsp; </td>"));
		websWrite(wp, T("</tr>\n"));
	}	  
	return 0;	
}

/*
 * ASP function
 */
static int showSinglePortForwardRulesASP(int eid, webs_t wp, int argc, char_t **argv)
{
	int i=0;
	int publicPort_int, privatePort_int, proto;
	char ip_address[32], publicPort[8], privatePort[8], comment[16], protocol[8];
	char rec[128];
	char *rules = (char *)nvram_bufget(RT2860_NVRAM, "SinglePortForwardRules");
	if(!rules)
		return 0;
	if(!strlen(rules))
		return 0;

	/* format is :
	 * [ip],[port_public],[port_private],[protocol],[comment],;
	 */
	while(getNthValueSafe(i++, rules, ';', rec, sizeof(rec)) != -1 ){
		// get ip address
		if((getNthValueSafe(0, rec, ',', ip_address, sizeof(ip_address)) == -1)){
			continue;
		}
		if(!isIpValid(ip_address)){
			continue;
		}

		// get public port
		if((getNthValueSafe(1, rec, ',', publicPort, sizeof(publicPort)) == -1)){
			continue;
		}
		if( (publicPort_int = atoi(publicPort)) == 0 || publicPort_int > 65535){
			continue;
		}

		// get private port
		if((getNthValueSafe(2, rec, ',', privatePort, sizeof(privatePort)) == -1)){
			continue;
		}
		if( (privatePort_int = atoi(privatePort)) == 0 || privatePort_int > 65535){
			continue;
		}

		// get protocol
		if((getNthValueSafe(3, rec, ',', protocol, sizeof(protocol)) == -1)){
			continue;
		}
		proto = atoi(protocol);
		switch(proto){
			case PROTO_TCP:
			case PROTO_UDP:
			case PROTO_TCP_UDP:
				break;
			default:
				continue;
		}

		if((getNthValueSafe(4, rec, ',', comment, sizeof(comment)) == -1)){
			continue;
		}

		websWrite(wp, T("<tr>\n"));
		// output No.
		websWrite(wp, T("<td> %d&nbsp; <input type=\"checkbox\" name=\"delRule%d\"> </td>"), i, i-1 );

		// output IP address
		websWrite(wp, T("<td align=center> %s </td>"), ip_address);

		// output Public port
		websWrite(wp, T("<td align=center> %d </td>"), publicPort_int);
	
		// output Private port
		websWrite(wp, T("<td align=center> %d </td>"), privatePort_int);

		// output Protocol
        switch(proto){
            case PROTO_TCP:
				websWrite(wp, T("<td align=center> TCP </td>"));
				break;
            case PROTO_UDP:
				websWrite(wp, T("<td align=center> UDP </td>"));
				break;
            case PROTO_TCP_UDP:
				websWrite(wp, T("<td align=center> TCP + UDP </td>"));
				break;
		}

		// output Comment
		if(strlen(comment))
			websWrite(wp, T("<td align=center> %s</td>"), comment);
		else
			websWrite(wp, T("<td align=center> &nbsp; </td>"));
		websWrite(wp, T("</tr>\n"));
	}	  
	return 0;	
}


static void getRulesPacketCount(webs_t wp, char_t *path, char_t *query)
{
	FILE *fp;
	int i, step_in_chains=0;
	char buf[1024], *default_policy;
	int default_drop_flag;
	int index=0, pkt_count;
	int *result;

	// check if the default policy is "drop" 
	default_policy = (char *)nvram_bufget(RT2860_NVRAM, "DefaultFirewallPolicy");
	if(!default_policy)
		default_policy = "0";
	default_drop_flag = atoi(default_policy);

	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain\nPragma: no-cache\nCache-Control: no-cache\n\n"));

	result = (int *)malloc(sizeof(int) * 128);
	if(!result)
		goto error;

	fp = popen("iptables -t filter -L -v", "r");
	if(!fp){
		free(result);
		goto error;
	}

	while(fgets(buf, 1024, fp) && index < 128){
		if(step_in_chains){
			if(buf[0] == '\n')
				break;
			if(buf[0] == ' ' && buf[1] == 'p' && buf[2] == 'k' && buf[3] == 't' )
				continue;
			// Skip the first one rule if default policy is drop.
			if(default_drop_flag){
				default_drop_flag = 0;
				continue;
			}
			sscanf(buf, "%d ", &pkt_count);
			result[index++] = pkt_count;
		}

		if(strstr(buf, "Chain " IPPORT_FILTER_CHAIN))
			step_in_chains = 1;
	}
	pclose(fp);

	if(index > 0)
		websWrite(wp, "%d", result[0]);
	for(i=1; i<index; i++)
		websWrite(wp, " %d", result[i]);

	free(result);
error:
	websDone(wp, 200);
	return;
}


/*
 * ASP function
 */
static int showIPPortFilterRulesASP(int eid, webs_t wp, int argc, char_t **argv)
{
	int i;
	int sprf_int, sprt_int, proto;
	char mac_address[32];
	char sip_1[32], sip_2[32], sprf[8], sprt[8], comment[16], protocol[8], action[4];
	char dip_1[32], dip_2[32], dprf[8], dprt[8];
	int dprf_int, dprt_int;
	char rec[256];
	char *default_policy;
	char *rules = (char *)nvram_bufget(RT2860_NVRAM, "IPPortFilterRules");
	if(!rules)
		return 0;

    default_policy = (char *)nvram_bufget(RT2860_NVRAM, "DefaultFirewallPolicy");
    // add the default policy to the end of FORWARD chain
    if(!default_policy)
		return 0;
	if(!strlen(default_policy))
		return 0;

	i=0;
	while(getNthValueSafe(i, rules, ';', rec, sizeof(rec)) != -1 && strlen(rec)){
		printf("i=%d, rec=%s, strlen(rec)=%d\n", i, rec, strlen(rec));
		// get ip 1
        if((getNthValueSafe(0, rec, ',', sip_1, sizeof(sip_1)) == -1)){
			i++;
			continue;
		}
        if(!isIpNetmaskValid(sip_1)){
			i++;
			continue;
		}
		// translate "any/0" to "any" for readable reason
		if( !strcmp(sip_1, "any/0"))
			strcpy(sip_1, "-");

		// get ip 2
        // get ip address
        if((getNthValueSafe(1, rec, ',', sip_2, sizeof(sip_2)) == -1)){
			i++;
			continue;
		}
		// dont verify cause we dont have ip range support
		//if(!isIpValid(sip_2))
        //    continue;

		// get port range "from"
		if((getNthValueSafe(2, rec, ',', sprf, sizeof(sprf)) == -1)){
			i++;
			continue;
		}
		if( (sprf_int = atoi(sprf)) > 65535){
			i++;
			continue;
		}

		// get port range "to"
		if((getNthValueSafe(3, rec, ',', sprt, sizeof(sprt)) == -1)){
			i++;
			continue;
		}
		if( (sprt_int = atoi(sprt)) > 65535){
			i++;
			continue;
		}

		// get ip 1
        if((getNthValueSafe(4, rec, ',', dip_1, sizeof(dip_1)) == -1)){
			i++;
            continue;
		}
        if(!isIpNetmaskValid(dip_1)){
			i++;
            continue;
		}
		// translate "any/0" to "any" for readable reason
		if( !strcmp(dip_1, "any/0"))
			strcpy(dip_1, "-");
		
		// get ip 2
        if((getNthValueSafe(5, rec, ',', dip_2, sizeof(dip_2)) == -1)){
			i++;
            continue;
		}
		// dont verify cause we dont have ip range support
		//if(!isIpValid(dip_2))
        //    continue;

		// get protocol
		if((getNthValueSafe(8, rec, ',', protocol, sizeof(protocol)) == -1)){
			i++;
			continue;
		}
		proto = atoi(protocol);
		switch(proto){
			case PROTO_TCP:
			case PROTO_UDP:
			case PROTO_NONE:
			case PROTO_ICMP:
				break;
			default:
				continue;
		}

		// get port range "from"
		if((getNthValueSafe(6, rec, ',', dprf, sizeof(dprf)) == -1)){
			i++;
			continue;
		}
		if( (dprf_int = atoi(dprf)) > 65535){
			i++;
			continue;
		}

		// get port range "to"
		if((getNthValueSafe(7, rec, ',', dprt, sizeof(dprt)) == -1)){
			i++;
			continue;
		}
		if( (dprt_int = atoi(dprt)) > 65535){
			i++;
			continue;
		}

		// get action
		if((getNthValueSafe(9, rec, ',', action, sizeof(action)) == -1)){
			i++;
			continue;
		}

		// get comment
		if((getNthValueSafe(10, rec, ',', comment, sizeof(comment)) == -1)){
			i++;
			continue;
		}

		// get mac address
		if((getNthValueSafe(11, rec, ',', mac_address, sizeof(mac_address)) == -1)){
			i++;
			continue;
		}
		if(!strlen(mac_address))
			gstrcpy(mac_address, T("-"));

		websWrite(wp, T("<tr>\n"));
		// output No.
		websWrite(wp, T("<td> %d&nbsp; <input type=\"checkbox\" name=\"delRule%d\"> </td>"), i+1, i );

		// output Mac address
		websWrite(wp, T("<td align=center> %s </td>"), mac_address);

		// output DIP
		websWrite(wp, T("<td align=center> %s </td>"), dip_1);
		// we dont support ip range 
		// websWrite(wp, T("<td align=center> %s-%s </td>"), ip_1, ip_2);

		// output SIP
		websWrite(wp, T("<td align=center> %s </td>"), sip_1);
		// we dont support ip range 
		// websWrite(wp, T("<td align=center> %s-%s </td>"), ip_1, ip_2);

		// output Protocol
        switch(proto){
            case PROTO_TCP:
				websWrite(wp, T("<td align=center> TCP </td>"));
				break;
            case PROTO_UDP:
				websWrite(wp, T("<td align=center> UDP </td>"));
				break;
            case PROTO_ICMP:
				websWrite(wp, T("<td align=center> ICMP </td>"));
				break;
            case PROTO_NONE:
				websWrite(wp, T("<td align=center> - </td>"));
				break;
		}

		// output dest Port Range
		if(dprt_int)
			websWrite(wp, T("<td align=center> %d - %d </td>"), dprf_int, dprt_int);
		else{
			// we re-descript the port number here because 
			// "any" word is more meanful than "0"
			if(!dprf_int){
				websWrite(wp, T("<td align=center> - </td>"), dprf_int);
			}else{
				websWrite(wp, T("<td align=center> %d </td>"), dprf_int);
			}
		}

		// output Source Port Range
		if(sprt_int)
			websWrite(wp, T("<td align=center> %d - %d </td>"), sprf_int, sprt_int);
		else{
			// we re-descript the port number here because 
			// "any" word is more meanful than "0"
			if(!sprf_int){
				websWrite(wp, T("<td align=center> - </td>"), sprf_int);
			}else{
				websWrite(wp, T("<td align=center> %d </td>"), sprf_int);
			}
		}


		// output action
        switch(atoi(action)){
            case ACTION_DROP:
				websWrite(wp, T("<td align=center id=portFilterActionDrop%d> Drop </td>"), i);
				break;
            case ACTION_ACCEPT:
				websWrite(wp, T("<td align=center id=portFilterActionAccept%d> Accept </td>"), i);
				break;
		}

		// output Comment
		if(strlen(comment))
			websWrite(wp, T("<td align=center> %s</td>"), comment);
		else
			websWrite(wp, T("<td align=center> &nbsp; </td>"));

		// output the id of "packet count"
		websWrite(wp, T("<td align=center id=pktCnt%d>-</td>"), i);

		websWrite(wp, T("</tr>\n"));

		i++;
	}	  

	switch(atoi(default_policy)){
		case 0:
			websWrite(wp, T("<tr><td align=center colspan=9 id=portCurrentFilterDefaultAccept> Others would be accepted.</td><td align=center id=pktCnt%d>-</td></tr>"), i);
			break;
		case 1:
			websWrite(wp, T("<tr><td align=center colspan=9 id=portCurrentFilterDefaultDrop> Others would be dropped.</td><td align=center id=pktCnt%d>-</td></tr>"), i);
			break;
	}

	return 0;	
}

static int showDMZIPAddressASP(int eid, webs_t wp, int argc, char_t **argv)
{
	char *DMZIPAddress = (char *)nvram_bufget(RT2860_NVRAM, "DMZIPAddress");
	if(!DMZIPAddress)
		return 0;
	if(!strlen(DMZIPAddress))
		return 0;

	websWrite(wp, T("%s"), DMZIPAddress);
	return 0;	
}

static void ipportFilterDelete(webs_t wp, char_t *path, char_t *query)
{
	int i, j, rule_count;
	char_t name_buf[16];
	char_t *value;
	int *deleArray;
//	char *firewall_enable;

	char *new_rules;
    const char *rules = (char *)nvram_bufget(RT2860_NVRAM, "IPPortFilterRules");
    if(!rules || !strlen(rules) )
        return;

	rule_count = getRuleNums((char *)rules);
	if(!rule_count)
		return;

	deleArray = (int *)malloc(rule_count * sizeof(int));
	if(!deleArray)
		return;
	
	new_rules = strdup(rules);
	if(!new_rules){
		free(deleArray);
		return;
	}

	for(i=0, j=0; i< rule_count; i++){
		snprintf(name_buf, 16, "delRule%d", i);
		value = websGetVar(wp, name_buf, NULL);
		if(value){
			deleArray[j++] = i;
		}
	}
	if(!j){
	    websHeader(wp);
	    websWrite(wp, T("You didn't select any rules to delete.<br>\n"));
	    websFooter(wp);
	    websDone(wp, 200);		
		return;
	}

	deleteNthValueMulti(deleArray, rule_count, new_rules, ';');
	free(deleArray);

	nvram_set(RT2860_NVRAM, "IPPortFilterRules", new_rules);
	nvram_commit(RT2860_NVRAM);
	free(new_rules);

	iptablesIPPortFilterFlush();
	iptablesIPPortFilterRun();

    websHeader(wp);
    websWrite(wp, T("s<br>\n") );
    websWrite(wp, T("fromPort: <br>\n"));
    websWrite(wp, T("toPort: <br>\n"));
    websWrite(wp, T("protocol: <br>\n"));
    websWrite(wp, T("comment: <br>\n"));
    websFooter(wp);
    websDone(wp, 200);

	return;
}

static void portForwardDelete(webs_t wp, char_t *path, char_t *query)
{
	int i, j, rule_count;
	char_t name_buf[16];
	char_t *value;
	int *deleArray;
	char *firewall_enable;

	char *new_rules;
    const char *rules = (char *)nvram_bufget(RT2860_NVRAM, "PortForwardRules");
    if(!rules || !strlen(rules) )
        return;

	rule_count = getRuleNums((char *)rules);
	if(!rule_count)
		return;

	deleArray = (int *)malloc(rule_count * sizeof(int));
	if(!deleArray)
		return;

	new_rules = strdup(rules);
	if(!new_rules){
		free(deleArray);
		return;
	}

	for(i=0, j=0; i< rule_count; i++){
		snprintf(name_buf, 16, "delRule%d", i);
		value = websGetVar(wp, T(name_buf), NULL);
		if(value){
			deleArray[j++] = i;
		}
	}

    if(!j){
        websHeader(wp);
        websWrite(wp, T("You didn't select any rules to delete.<br>\n"));
        websFooter(wp);
        websDone(wp, 200);
        return;
    }

	deleteNthValueMulti(deleArray, rule_count, new_rules, ';');
	free(deleArray);

	nvram_set(RT2860_NVRAM, "PortForwardRules", new_rules);
	nvram_commit(RT2860_NVRAM);
	free(new_rules);

	// restart iptables if it is running
	firewall_enable = (char *)nvram_bufget(RT2860_NVRAM, "PortForwardEnable");
	if(firewall_enable){
		if(atoi(firewall_enable)){
			// call iptables
			iptablesPortForwardFlush();
			iptablesPortForwardRun();
		}
	}

    websHeader(wp);
    websWrite(wp, T("s<br>\n") );
    websWrite(wp, T("fromPort: <br>\n"));
    websWrite(wp, T("toPort: <br>\n"));
    websWrite(wp, T("protocol: <br>\n"));
    websWrite(wp, T("comment: <br>\n"));
    websFooter(wp);
    websDone(wp, 200);

	return;
}
            
static void singlePortForwardDelete(webs_t wp, char_t *path, char_t *query)
{
	int i, j, rule_count;
	char_t name_buf[16];
	char_t *value;
	int *deleArray;
	char *firewall_enable;

	char *new_rules;
    const char *rules = (char *)nvram_bufget(RT2860_NVRAM, "SinglePortForwardRules");
    if(!rules || !strlen(rules) )
        return;

	rule_count = getRuleNums((char *)rules);
	if(!rule_count)
		return;

	deleArray = (int *)malloc(rule_count * sizeof(int));
	if(!deleArray)
		return;

	new_rules = strdup(rules);
	if(!new_rules){
		free(deleArray);
		return;
	}

	for(i=0, j=0; i< rule_count; i++){
		snprintf(name_buf, 16, "delRule%d", i);
		value = websGetVar(wp, T(name_buf), NULL);
		if(value){
			deleArray[j++] = i;
		}
	}

    if(!j){
        websHeader(wp);
        websWrite(wp, T("You didn't select any rules to delete.<br>\n"));
        websFooter(wp);
        websDone(wp, 200);
        return;
    }

	deleteNthValueMulti(deleArray, rule_count, new_rules, ';');
	free(deleArray);

	nvram_set(RT2860_NVRAM, "SinglePortForwardRules", new_rules);
	nvram_commit(RT2860_NVRAM);
	free(new_rules);

	// restart iptables if it is running
	firewall_enable = (char *)nvram_bufget(RT2860_NVRAM, "SinglePortForwardEnable");
	if(firewall_enable){
		if(atoi(firewall_enable)){
			// call iptables
			iptablesSinglePortForwardFlush();
			iptablesSinglePortForwardRun();
		}
	}

    websHeader(wp);
    websWrite(wp, T("s<br>\n") );
    websWrite(wp, T("publicPort: <br>\n"));
    websWrite(wp, T("privatePort: <br>\n"));
    websWrite(wp, T("protocol: <br>\n"));
    websWrite(wp, T("comment: <br>\n"));
    websFooter(wp);
    websDone(wp, 200);

	return;
}


static void ipportFilter(webs_t wp, char_t *path, char_t *query)
{
	char rule[8192];
	char *mac_address;
	char *sip_1, *sip_2, *sprf, *sprt, *protocol, *action_str, *comment;
	char *dip_1, *dip_2, *dprf, *dprt;
	char *IPPortFilterRules;
	
	int sprf_int, sprt_int, dprf_int, dprt_int, proto, action;

	mac_address = websGetVar(wp, T("mac_address"), T(""));

	sip_1 = websGetVar(wp, T("sip_address"), T("any"));
	sip_2 = websGetVar(wp, T("sip_address2"), T(""));
	sprf = websGetVar(wp, T("sFromPort"), T("0"));
	sprt = websGetVar(wp, T("sToPort"), T(""));

	dip_1 = websGetVar(wp, T("dip_address"), T("any"));
	dip_2 = websGetVar(wp, T("dip_address2"), T(""));
	dprf = websGetVar(wp, T("dFromPort"), T("0"));
	dprt = websGetVar(wp, T("dToPort"), T(""));

	protocol = websGetVar(wp, T("protocol"), T(""));
	action_str = websGetVar(wp, T("action"), T(""));
	comment = websGetVar(wp, T("comment"), T(""));

	if(!mac_address || !sip_1 || !dip_1 || !sprf || !dprf)
		return;

	if(!strlen(mac_address) && !strlen(sip_1) && !strlen(dip_1) && !strlen(sprf) && !strlen(dprf))
		return;

	// we dont trust user input.....
	if(strlen(mac_address)){
		if(!isMacValid(mac_address))
			return;
	}

	if(strlen(sip_1)){
		if(!isIpNetmaskValid(sip_1))
			return;
	}else
		sip_1 = T("any/0");

	if(strlen(dip_1)){
		if(!isIpNetmaskValid(dip_1))
			return;
	}else
    	dip_1 = T("any/0");

	sip_2 = dip_2 = "0";

	if(! strcmp(protocol, T("TCP")))
		proto = PROTO_TCP;
	else if( !strcmp(protocol, T("UDP")))
		proto = PROTO_UDP;
	else if( !strcmp(protocol, T("None")))
		proto = PROTO_NONE;
	else if( !strcmp(protocol, T("ICMP")))
		proto = PROTO_ICMP;
	else
		return;

	if(!strlen(sprf) || proto == PROTO_NONE || proto == PROTO_ICMP){
		sprf_int = 0;
	}else{
		sprf_int = atoi(sprf);
		if(sprf_int == 0 || sprf_int > 65535)
			return;
	}

	if(!strlen(sprt) || proto == PROTO_NONE || proto == PROTO_ICMP){
		sprt_int = 0;
	}else{
		sprt_int = atoi(sprt);
		if(sprt_int ==0 || sprt_int > 65535)
			return;
	}

	if(!strlen(dprf) || proto == PROTO_NONE || proto == PROTO_ICMP){
		dprf_int = 0;
	}else{
		dprf_int = atoi(dprf);
		if(dprf_int ==0 || dprf_int > 65535)
			return;
	}

	if(!strlen(dprt) || proto == PROTO_NONE || proto == PROTO_ICMP){
		dprt_int = 0;
	}else{
		dprt_int = atoi(dprt);
		if(dprt_int ==0 || dprt_int > 65535)
			return;
	}

	if(! (strcmp(action_str, T("Accept"))))
		action = ACTION_ACCEPT;
	else if(! (strcmp(action_str, T("Drop"))))
		action = ACTION_DROP;
	else
		return;

	if(strlen(comment) > 32)
		return;
	// i know you will try to break our box... ;) 
	if(strchr(comment, ';') || strchr(comment, ','))
		return;

	if(   ( IPPortFilterRules = (char *)nvram_bufget(RT2860_NVRAM, "IPPortFilterRules")) && strlen(IPPortFilterRules)){
		snprintf(rule, sizeof(rule), "%s;%s,%s,%d,%d,%s,%s,%d,%d,%d,%d,%s,%s", IPPortFilterRules, sip_1, sip_2, sprf_int, sprt_int, dip_1, dip_2, dprf_int, dprt_int, proto, action, comment, mac_address);
	}else{
		snprintf(rule, sizeof(rule), "%s,%s,%d,%d,%s,%s,%d,%d,%d,%d,%s,%s", sip_1, sip_2, sprf_int, sprt_int, dip_1, dip_2, dprf_int, dprt_int, proto, action, comment, mac_address);
	}

	nvram_set(RT2860_NVRAM, "IPPortFilterRules", rule);
	nvram_commit(RT2860_NVRAM);

	iptablesIPPortFilterFlush();
	iptablesIPPortFilterRun();

	websHeader(wp);
	websWrite(wp, T("mac: %s<br>\n"), mac_address);	
	websWrite(wp, T("sip1: %s<br>\n"), sip_1);	
	websWrite(wp, T("sip2: %s<br>\n"), sip_2);	
	websWrite(wp, T("sFromPort: %s<br>\n"), sprf);
	websWrite(wp, T("sToPort: %s<br>\n"), sprt);
	websWrite(wp, T("dip1: %s<br>\n"), dip_1);	
	websWrite(wp, T("dip2: %s<br>\n"), dip_2);	
	websWrite(wp, T("dFromPort: %s<br>\n"), dprf);
	websWrite(wp, T("dToPort: %s<br>\n"), dprt);
	websWrite(wp, T("protocol: %s<br>\n"), protocol);
	websWrite(wp, T("action: %s<br>\n"), action_str);
	websWrite(wp, T("comment: %s<br>\n"), comment);

    websFooter(wp);
    websDone(wp, 200);        
    return;
	
}


static void portForward(webs_t wp, char_t *path, char_t *query)
{
	char rule[8192];
	char *ip_address, *pfe, *prf, *prt, *protocol, *comment;
	char *PortForwardRules;

	int prf_int, prt_int, proto;

	pfe = websGetVar(wp, T("portForwardEnabled"), T(""));
	ip_address = websGetVar(wp, T("ip_address"), T(""));
	prf = websGetVar(wp, T("fromPort"), T(""));
	prt = websGetVar(wp, T("toPort"), T(""));
	protocol = websGetVar(wp, T("protocol"), T(""));
	comment = websGetVar(wp, T("comment"), T(""));

	if(!pfe && !strlen(pfe))
		return;

	if(!atoi(pfe)){
		nvram_set(RT2860_NVRAM, "PortForwardEnable", "0");
		iptablesPortForwardFlush();		//disable
		//no change in rules
		goto end;
	}

	if(!strlen(ip_address) && !strlen(prf) && !strlen(prt) && !strlen(comment)){	// user choose nothing but press "apply" only
		nvram_set(RT2860_NVRAM, "PortForwardEnable", "1");
		iptablesPortForwardFlush();
		iptablesPortForwardRun();
		// no change in rules
		goto end;
	}

	if(!ip_address && !strlen(ip_address))
		return;
	if(!isIpValid(ip_address))
		return;

	// we dont trust user input.....
	if(!prf && !strlen(prf))
		return;
	if(!(prf_int = atoi(prf)) )
		return;
	if(prf_int > 65535)
		return;

	if(!prt)
		return;
	if(strlen(prt)){
		if( !(prt_int = atoi(prt)) )
			return;
		if(prt_int < prf_int)
			return;
		if(prt_int > 65535)
			return;
	}else{
		prt_int = 0;
	}

	if(! strcmp(protocol, "TCP"))
		proto = PROTO_TCP;
	else if( !strcmp(protocol, "UDP"))
		proto = PROTO_UDP;
	else if( !strcmp(protocol, "TCP&UDP"))
		proto = PROTO_TCP_UDP;
	else
		return;

	if(strlen(comment) > 32)
		return;
	/* i know you will try to break our box... ;) */
	if(strchr(comment, ';') || strchr(comment, ','))
		return;

	nvram_set(RT2860_NVRAM, "PortForwardEnable", "1");

	if(( PortForwardRules = (char *)nvram_bufget(RT2860_NVRAM, "PortForwardRules")) && strlen( PortForwardRules) )
		snprintf(rule, sizeof(rule), "%s;%s,%d,%d,%d,%s",  PortForwardRules, ip_address, prf_int, prt_int, proto, comment);
	else
		snprintf(rule, sizeof(rule), "%s,%d,%d,%d,%s", ip_address, prf_int, prt_int, proto, comment);

	nvram_set(RT2860_NVRAM, "PortForwardRules", rule);
	nvram_commit(RT2860_NVRAM);

	iptablesPortForwardFlush();
	// call iptables
	iptablesPortForwardRun();

end:
	websHeader(wp);
	websWrite(wp, T("portForwardEnabled: %s<br>\n"), pfe);
	websWrite(wp, T("ip: %s<br>\n"), ip_address);
	websWrite(wp, T("fromPort: %s<br>\n"), prf);
	websWrite(wp, T("toPort: %s<br>\n"), prt);
	websWrite(wp, T("protocol: %s<br>\n"), protocol);
	websWrite(wp, T("comment: %s<br>\n"), comment);

    websFooter(wp);
    websDone(wp, 200);        
}

static void singlePortForward(webs_t wp, char_t *path, char_t *query)
{
	char rule[8192];
	char *ip_address, *spfe, *publicPort, *privatePort, *protocol, *comment;
	char *SinglePortForwardRules;

	int publicPort_int, privatePort_int, proto;

	spfe = websGetVar(wp, T("singlePortForwardEnabled"), T(""));
	ip_address = websGetVar(wp, T("ip_address"), T(""));
	publicPort = websGetVar(wp, T("publicPort"), T(""));
	privatePort = websGetVar(wp, T("privatePort"), T(""));
	protocol = websGetVar(wp, T("protocol"), T(""));
	comment = websGetVar(wp, T("comment"), T(""));

	if(!spfe && !strlen(spfe))
		return;

	if(!atoi(spfe)){
		nvram_set(RT2860_NVRAM, "SinglePortForwardEnable", "0");
		iptablesSinglePortForwardFlush();		//disable
		//no chainge in rules
		goto end;
	}

	if(!strlen(ip_address) && !strlen(publicPort) && !strlen(privatePort) && !strlen(comment)){	// user choose nothing but press "apply" only
		nvram_set(RT2860_NVRAM, "SinglePortForwardEnable", "1");
		iptablesSinglePortForwardFlush();
		iptablesSinglePortForwardRun();
		// no change in rules
		goto end;
	}

	if(!ip_address && !strlen(ip_address))
		return;
	if(!isIpValid(ip_address))
		return;

	// we dont trust user input.....
	if(!publicPort && !strlen(publicPort))
		return;
	if(!(publicPort_int = atoi(publicPort)) )
		return;
	if(publicPort_int > 65535)
		return;

	if(!privatePort && !strlen(privatePort))
		return;
	if(!(privatePort_int = atoi(privatePort)) )
		return;
	if(privatePort_int > 65535)
		return;

	if(! strcmp(protocol, "TCP"))
		proto = PROTO_TCP;
	else if( !strcmp(protocol, "UDP"))
		proto = PROTO_UDP;
	else if( !strcmp(protocol, "TCP&UDP"))
		proto = PROTO_TCP_UDP;
	else
		return;

	if(strlen(comment) > 32)
		return;
	/* i know you will try to break our box... ;) */
	if(strchr(comment, ';') || strchr(comment, ','))
		return;

	nvram_set(RT2860_NVRAM, "SinglePortForwardEnable", "1");

	if(( SinglePortForwardRules = (char *)nvram_bufget(RT2860_NVRAM, "SinglePortForwardRules")) && strlen( SinglePortForwardRules) )
		snprintf(rule, sizeof(rule), "%s;%s,%d,%d,%d,%s",  SinglePortForwardRules, ip_address, publicPort_int, privatePort_int, proto, comment);
	else
		snprintf(rule, sizeof(rule), "%s,%d,%d,%d,%s", ip_address, publicPort_int, privatePort_int, proto, comment);

	nvram_set(RT2860_NVRAM, "SinglePortForwardRules", rule);
	nvram_commit(RT2860_NVRAM);

	iptablesSinglePortForwardFlush();
	// call iptables
	iptablesSinglePortForwardRun();

end:
	websHeader(wp);
	websWrite(wp, T("singlePortForwardEnabled: %s<br>\n"), spfe);
	websWrite(wp, T("ip: %s<br>\n"), ip_address);
	websWrite(wp, T("publicPort: %s<br>\n"), publicPort);
	websWrite(wp, T("privatePort: %s<br>\n"), privatePort);
	websWrite(wp, T("protocol: %s<br>\n"), protocol);
	websWrite(wp, T("comment: %s<br>\n"), comment);

    websFooter(wp);
    websDone(wp, 200);        
}

static void iptablesBasicSetting(void)
{
	char *firewall_enable;

	firewall_enable = (char *)nvram_bufget(RT2860_NVRAM, "IPPortFilterEnable");

	// flush  ipport   filter   chain
    iptablesIPPortFilterFlush();

	if(!firewall_enable || !atoi(firewall_enable))
		return;
    iptablesIPPortFilterRun();

	return;
}

static void BasicSettings(webs_t wp, char_t *path, char_t *query)
{
	char *default_policy, *firewall_enable;

	firewall_enable = websGetVar(wp, T("portFilterEnabled"), T(""));
	default_policy = websGetVar(wp, T("defaultFirewallPolicy"), T("0"));

	switch(atoi(firewall_enable)){
	case 0:
		nvram_set(RT2860_NVRAM, "IPPortFilterEnable", "0");
		break;
	case 1:
		nvram_set(RT2860_NVRAM, "IPPortFilterEnable", "1");
		break;
	}

	switch(atoi(default_policy)){
	case 1:
		nvram_set(RT2860_NVRAM, "DefaultFirewallPolicy", "1");
		break;
	case 0:
	default:
		nvram_set(RT2860_NVRAM, "DefaultFirewallPolicy", "0");
		break;
	}
	nvram_commit(RT2860_NVRAM);

	iptablesBasicSetting();

	websHeader(wp);
	websWrite(wp, T("default_policy: %s<br>\n"), default_policy);
    websFooter(wp);
    websDone(wp, 200);        
}

static void DMZ(webs_t wp, char_t *path, char_t *query)
{
	char *dmzE, *ip_address;

	dmzE = websGetVar(wp, T("DMZEnabled"), T(""));
	ip_address = websGetVar(wp, T("DMZIPAddress"), T(""));

	// someone use malform page.....
	if(!dmzE && !strlen(dmzE))
		return;

	// we dont trust user input, check all things before doing changes
	if(atoi(dmzE) && !isIpValid(ip_address))	// enable && invalid mac address
		return;

	iptablesDMZFlush();
	if(atoi(dmzE) == 0){		// disable
		nvram_set(RT2860_NVRAM, "DMZEnable", "0");
	}else{					// enable
		nvram_set(RT2860_NVRAM, "DMZEnable", "1");
		if(strlen(ip_address)){
			nvram_set(RT2860_NVRAM, "DMZIPAddress", ip_address);
		}
	}

	nvram_commit(RT2860_NVRAM);
	iptablesDMZRun();

	websHeader(wp);
	websWrite(wp, T("DMZEnabled: %s<br>\n"), dmzE);
	websWrite(wp, T("ip_address: %s<br>\n"), ip_address);
    websFooter(wp);
    websDone(wp, 200);        
}

static void websSysFirewall(webs_t wp, char_t *path, char_t *query)
{
	char *rmE = websGetVar(wp, T("remoteManagementEnabled"), T(""));
	char *wpfE = websGetVar(wp, T("pingFrmWANFilterEnabled"), T(""));
	char *bpsE = websGetVar(wp, T("blockPortScanEnabled"), T(""));
	char *bsfE = websGetVar(wp, T("blockSynFloodEnabled"), T(""));
	char *spifw = websGetVar(wp, T("spiFWEnabled"), T("1"));

	// someone use malform page.....
	if(!rmE || !strlen(rmE))
		return;
	if(!wpfE || !strlen(wpfE))
		return;
	if(!spifw || !strlen(spifw))
		return;
	if(!bpsE || !strlen(bpsE))
		return;
	if(!bsfE || !strlen(bsfE))
		return;

	// TODO: make a new chain instead of flushing the whole INPUT chain
//	doSystem("iptables -t filter -F INPUT");

	if(atoi(rmE) == 0){		// disable
		nvram_bufset(RT2860_NVRAM, "RemoteManagement", "0");
	}else{					// enable
		nvram_bufset(RT2860_NVRAM, "RemoteManagement", "1");
	}

	if(atoi(wpfE) == 0){		// disable
		nvram_bufset(RT2860_NVRAM, "WANPingFilter", "0");
	}else{					// enable
		nvram_bufset(RT2860_NVRAM, "WANPingFilter", "1");
//		doSystem("iptables -t filter -A INPUT -i %s -p icmp -j DROP", getWanIfNamePPP());
	}

	if(atoi(spifw) == 0){		// disable
		nvram_bufset(RT2860_NVRAM, "SPIFWEnabled", "0");
	}else{					// enable
		nvram_bufset(RT2860_NVRAM, "SPIFWEnabled", "1");
	}

	if(atoi(bpsE) == 0){		// disable
		nvram_bufset(RT2860_NVRAM, "BlockPortScan", "0");
	}else{					// enable
		nvram_bufset(RT2860_NVRAM, "BlockPortScan", "1");
	}

	if(atoi(bsfE) == 0){		// disable
		nvram_bufset(RT2860_NVRAM, "BlockSynFlood", "0");
	}else{					// enable
		nvram_bufset(RT2860_NVRAM, "BlockSynFlood", "1");
	}
	nvram_commit(RT2860_NVRAM);

	iptablesRemoteManagementRun();

	iptablesIPPortFilterFlush();
	iptablesIPPortFilterRun();

	iptablesMaliciousFilterFlush();
	iptablesMaliciousFilterRun();

	websHeader(wp);
	websWrite(wp, T("RemoteManage: %s<br>\n"), rmE);
	websWrite(wp, T("WANPingFilter: %s<br>\n"), wpfE);
	websWrite(wp, T("SPIFWEnabled: %s<br>\n"), spifw);
	websWrite(wp, T("BlockPortScan: %s<br>\n"), bpsE);
	websWrite(wp, T("BlockSynFlood: %s<br>\n"), bsfE);
    websFooter(wp);
    websDone(wp, 200);        

}


/* Same as the file "linux/netfilter_ipv4/ipt_webstr.h" */
#define BLK_JAVA                0x01
#define BLK_ACTIVE              0x02
#define BLK_COOKIE              0x04
#define BLK_PROXY               0x08
void iptablesWebsFilterRun(void)
{
	int i;
	int content_filter = 0;
	char entry[256];
	char *url_filter = (char *)nvram_bufget(RT2860_NVRAM, "websURLFilters");
	char *host_filter = (char *)nvram_bufget(RT2860_NVRAM, "websHostFilters");
	char *proxy = (char *)nvram_bufget(RT2860_NVRAM, "websFilterProxy");
	char *java = (char *)nvram_bufget(RT2860_NVRAM, "websFilterJava");
	char *activex = (char *)nvram_bufget(RT2860_NVRAM, "websFilterActivex");
	char *cookies = (char *)nvram_bufget(RT2860_NVRAM, "websFilterCookies");

	if(!url_filter || !host_filter || !proxy || !java || !activex || !cookies)
		return;

	// Content filter
	if(!strcmp(java, "1"))
		content_filter += BLK_JAVA;
	if(!strcmp(activex, "1"))
		content_filter += BLK_ACTIVE;
	if(!strcmp(cookies, "1"))
		content_filter += BLK_COOKIE;
	if(!strcmp(proxy, "1"))
		content_filter += BLK_PROXY;

	if(content_filter){
		// Why only 3 ports are inspected?(This idea is from CyberTAN source code)
		// TODO: use layer7 to inspect HTTP
		doSystem("iptables -A " WEB_FILTER_CHAIN " -p tcp -m tcp --dport 80   -m webstr --content %d -j REJECT --reject-with tcp-reset", content_filter);
		doSystem("iptables -A " WEB_FILTER_CHAIN " -p tcp -m tcp --dport 3128 -m webstr --content %d -j REJECT --reject-with tcp-reset", content_filter);
		doSystem("iptables -A " WEB_FILTER_CHAIN " -p tcp -m tcp --dport 8080 -m webstr --content %d -j REJECT --reject-with tcp-reset", content_filter);
	}

	// URL filter
	i=0;
	while( (getNthValueSafe(i++, url_filter, ';', entry, sizeof(entry)) != -1) ){
		if(strlen(entry)){
			if(!strncasecmp(entry, "http://", strlen("http://")))
				strcpy(entry, entry + strlen("http://"));
			doSystem("iptables -A " WEB_FILTER_CHAIN " -p tcp -m tcp -m webstr --url  %s -j REJECT --reject-with tcp-reset", entry);
		}
	}

	// HOST(Keyword) filter
	i=0;
	while( (getNthValueSafe(i++, host_filter, ';', entry, sizeof(entry)) != -1) ){
		if(strlen(entry))
			doSystem("iptables -A " WEB_FILTER_CHAIN " -p tcp -m tcp -m webstr --host %s -j REJECT --reject-with tcp-reset", entry);
	}

	return;
}

static void websURLFilterDelete(webs_t wp, char_t *path, char_t *query)
{
	int i, j, rule_count;
	char_t name_buf[16];
	char_t *value;
	int *deleArray;

	char *new_rules;	
    const char *rules = (char *)nvram_bufget(RT2860_NVRAM, "websURLFilters");
    if(!rules || !strlen(rules) )
        return;

	rule_count = getRuleNums((char *)rules);
	if(!rule_count)
		return;

	
	deleArray = (int *)malloc(rule_count * sizeof(int));
	if(!deleArray)
		return;

    new_rules = strdup(rules);
    if(!new_rules){
        free(deleArray);
        return;
    }

	for(i=0, j=0; i< rule_count; i++){
		snprintf(name_buf, 16, "DR%d", i);
		value = websGetVar(wp, name_buf, NULL);
		if(value){
			deleArray[j++] = i;
		}
	}
	if(!j){
	    websHeader(wp);
	    websWrite(wp, T("You didn't select any rules to delete.<br>\n"));
	    websFooter(wp);
	    websDone(wp, 200);		
		return;
	}

	deleteNthValueMulti(deleArray, rule_count, new_rules, ';');
	free(deleArray);

	nvram_set(RT2860_NVRAM, "websURLFilters", new_rules);
	nvram_commit(RT2860_NVRAM);

	free(new_rules);

	doSystem("iptables -t filter -F " WEB_FILTER_CHAIN);
	iptablesWebsFilterRun();

    websHeader(wp);
    websWrite(wp, T("Delete : success<br>\n") );
    websFooter(wp);
    websDone(wp, 200);

	return;
}

static void websHostFilterDelete(webs_t wp, char_t *path, char_t *query)
{
	int i, j, rule_count;
	char_t name_buf[16];
	char_t *value;
	int *deleArray;

	char *new_rules;
	const char *rules = (char *)nvram_bufget(RT2860_NVRAM, "websHostFilters");
    if(!rules || !strlen(rules) )
        return;

	rule_count = getRuleNums((char *)rules);
	if(!rule_count)
		return;

	deleArray = (int *)malloc(rule_count * sizeof(int));
	if(!deleArray)
		return;

    new_rules = strdup(rules);
    if(!new_rules){
        free(deleArray);
        return;
    }

	for(i=0, j=0; i< rule_count; i++){
		snprintf(name_buf, 16, "DR%d", i);
		value = websGetVar(wp, name_buf, NULL);
		if(value){
			deleArray[j++] = i;
		}
	}
	if(!j){
	    websHeader(wp);
	    websWrite(wp, T("You didn't select any rules to delete.<br>\n"));
	    websFooter(wp);
	    websDone(wp, 200);		
		return;
	}

	deleteNthValueMulti(deleArray, rule_count, new_rules, ';');
	free(deleArray);

	nvram_set(RT2860_NVRAM, "websHostFilters", new_rules);
	nvram_commit(RT2860_NVRAM);

	free(new_rules);

	doSystem("iptables -t filter -F " WEB_FILTER_CHAIN);
	iptablesWebsFilterRun();

    websHeader(wp);
    websWrite(wp, T("Delete : success<br>\n") );
    websFooter(wp);
    websDone(wp, 200);

	return;
}

static void webContentFilter(webs_t wp, char_t *path, char_t *query)
{
	char *proxy = websGetVar(wp, T("websFilterProxy"), T(""));
	char *java = websGetVar(wp, T("websFilterJava"), T(""));
	char *activex = websGetVar(wp, T("websFilterActivex"), T(""));
	char *cookies = websGetVar(wp, T("websFilterCookies"), T(""));

	// someone use malform page.....
	if(!proxy || !java || !activex || !cookies)
		return;

	nvram_bufset(RT2860_NVRAM, "websFilterProxy",   atoi(proxy)   ? "1" : "0" );
	nvram_bufset(RT2860_NVRAM, "websFilterJava",    atoi(java)    ? "1" : "0" );
	nvram_bufset(RT2860_NVRAM, "websFilterActivex", atoi(activex) ? "1" : "0" );
	nvram_bufset(RT2860_NVRAM, "websFilterCookies", atoi(cookies) ? "1" : "0" );
	nvram_commit(RT2860_NVRAM);

	doSystem("iptables -t filter -F " WEB_FILTER_CHAIN);
	iptablesWebsFilterRun();

	websHeader(wp);
	websWrite(wp, T("Proxy: %s<br>\n"),  atoi(proxy) ? "enable" : "disable");
	websWrite(wp, T("Java: %s<br>\n"),   atoi(java) ? "enable" : "disable");
	websWrite(wp, T("Activex: %s<br>\n"), atoi(activex) ? "enable" : "disable");
	websWrite(wp, T("Cookies: %s<br>\n"), atoi(cookies) ? "enable" : "disable");
    websFooter(wp);
    websDone(wp, 200);
}

static void websURLFilter(webs_t wp, char_t *path, char_t *query)
{
	char *urlfilters = (char *)nvram_bufget(RT2860_NVRAM, "websURLFilters");
	char *rule = websGetVar(wp, T("addURLFilter"), T(""));
	char *new_urlfilters;
	if(!rule)
		return;
	if(strchr(rule, ';'))
		return;

	if(!urlfilters || !strlen(urlfilters))
		nvram_bufset(RT2860_NVRAM, "websURLFilters", rule);
	else{
		if(! (new_urlfilters = (char *)malloc(sizeof(char) * (strlen(urlfilters)+strlen(rule)+2))))
			return;
		new_urlfilters[0] = '\0';
		strcat(new_urlfilters, urlfilters);
		strcat(new_urlfilters, ";");
		strcat(new_urlfilters, rule);
		nvram_bufset(RT2860_NVRAM, "websURLFilters", new_urlfilters);
		free(new_urlfilters);
	}
	nvram_commit(RT2860_NVRAM);

	doSystem("iptables -t filter -F " WEB_FILTER_CHAIN);
	iptablesWebsFilterRun();

    websHeader(wp);
    websWrite(wp, T("add URL filter: %s<br>\n"), rule);
    websFooter(wp);
    websDone(wp, 200);

}

static void websHostFilter(webs_t wp, char_t *path, char_t *query)
{
	char *hostfilters = (char *)nvram_bufget(RT2860_NVRAM, "websHostFilters");
	char *rule = websGetVar(wp, T("addHostFilter"), T(""));
	char *new_hostfilters;
	if(!rule)
		return;
	if(strchr(rule, ';'))
		return;

	if(!hostfilters || !strlen(hostfilters))
		nvram_bufset(RT2860_NVRAM, "websHostFilters", rule);
	else{
		if(! (new_hostfilters = (char *)malloc(sizeof(char) * (strlen(hostfilters)+strlen(rule)+2))))
			return;
		new_hostfilters[0] = '\0';
		strcat(new_hostfilters, hostfilters);
		strcat(new_hostfilters, ";");
		strcat(new_hostfilters, rule);
		nvram_bufset(RT2860_NVRAM, "websHostFilters", new_hostfilters);
		free(new_hostfilters);
	}
	nvram_commit(RT2860_NVRAM);

	doSystem("iptables -t filter -F " WEB_FILTER_CHAIN);
	iptablesWebsFilterRun();

    websHeader(wp);
    websWrite(wp, T("add Host filter: %s<br>\n"), rule);
    websFooter(wp);
    websDone(wp, 200);
}

char *getNameIntroFromPat(char *filename)
{
	static char result[512];
	char buf[512], *begin, *end, *desh;
	char path_filename[512];
	char *rc;
	FILE *fp;

	sprintf(path_filename, "%s/%s", "/etc_ro/l7-protocols", filename);
	if(! (fp = fopen(path_filename, "r")))
		return NULL;
	result[0] = '\0';
	rc = fgets(buf, sizeof(buf), fp);
	if(rc){
		// find name
		begin = buf + 2;
		if(! ( desh = strchr(buf, '-'))){
			printf("warning: can't find %s name.\n", filename);
			fclose(fp);
			return "N/A#N/A";
		}
		end = desh;
		if(*(end-1) == ' ')
			end--;
		*end = '\0';
		strncat(result, begin, sizeof(result));
		strncat(result, "#", sizeof(result));

		// find intro
		if(!(end = strchr(desh+1, '\n'))){
			printf("warning: can't find %s intro.\n", filename);
			fclose(fp);
			return "N/A#N/A";
		}
		*end = '\0';
		strncat(result, desh + 2 , sizeof(result));
	}else{
		printf("warning: can't read %s intro.\n", filename);
		fclose(fp);
		return "N/A#N/A";
	}

	fclose(fp);
	return result;	
}


void LoadLayer7FilterName(void)
{
	char *delim;
	struct dirent *dir;
	DIR *d;
	char *intro;

	l7name[0] = '\0';
	if(!(d = opendir("/etc_ro/l7-protocols")))
		return;
	
	while((dir = readdir(d))){
		if(dir->d_name[0] == '.')
			continue;
		if(!(delim = strstr(dir->d_name, ".pat")) )
			continue;
		
		intro = getNameIntroFromPat(dir->d_name);

		*delim = '\0';
		if(l7name[0] == '\0'){
			strncat(l7name, dir->d_name, sizeof(l7name));
			strncat(l7name, "#", sizeof(l7name));
			strncat(l7name, intro, sizeof(l7name));
		}else{
			strncat(l7name, ";", sizeof(l7name));
			strncat(l7name, dir->d_name, sizeof(l7name));
			strncat(l7name, "#", sizeof(l7name));
			strncat(l7name, intro, sizeof(l7name));
		}
	}
	closedir(d);
}

static int getLayer7FiltersASP(int eid, webs_t wp, int argc, char_t **argv)
{
	websLongWrite(wp, l7name);
	return 0;	
}

void formDefineFirewall(void)
{
	websAspDefine(T("getDefaultFirewallPolicyASP"), getDefaultFirewallPolicyASP);
	websFormDefine(T("BasicSettings"), BasicSettings);

	websAspDefine(T("getIPPortFilterEnableASP"), getIPPortFilterEnableASP);
	websAspDefine(T("showIPPortFilterRulesASP"), showIPPortFilterRulesASP);
	websAspDefine(T("getIPPortRuleNumsASP"), getIPPortRuleNumsASP);
	websFormDefine(T("ipportFilter"), ipportFilter);
	websFormDefine(T("ipportFilterDelete"), ipportFilterDelete);
	websFormDefine(T("getRulesPacketCount"), getRulesPacketCount);

	websFormDefine(T("DMZ"), DMZ);
	websAspDefine(T("getDMZEnableASP"), getDMZEnableASP);
	websAspDefine(T("showDMZIPAddressASP"), showDMZIPAddressASP);

	websAspDefine(T("getPortForwardEnableASP"), getPortForwardEnableASP);
	websAspDefine(T("showPortForwardRulesASP"), showPortForwardRulesASP);
	websAspDefine(T("getPortForwardRuleNumsASP"), getPortForwardRuleNumsASP);
	websFormDefine(T("portForward"), portForward);
	websFormDefine(T("portForwardDelete"), portForwardDelete);
	
	websAspDefine(T("getSinglePortForwardEnableASP"), getSinglePortForwardEnableASP);
	websAspDefine(T("showSinglePortForwardRulesASP"), showSinglePortForwardRulesASP);
	websAspDefine(T("getSinglePortForwardRuleNumsASP"), getSinglePortForwardRuleNumsASP);
	websFormDefine(T("singlePortForward"), singlePortForward);
	websFormDefine(T("singlePortForwardDelete"), singlePortForwardDelete);

	websFormDefine(T("websSysFirewall"), websSysFirewall);

	websFormDefine(T("webContentFilter"), webContentFilter);
	websFormDefine(T("websURLFilterDelete"), websURLFilterDelete);
	websFormDefine(T("websHostFilterDelete"), websHostFilterDelete);
	websFormDefine(T("websHostFilter"), websHostFilter);
	websFormDefine(T("websURLFilter"), websURLFilter);

	websAspDefine(T("getLayer7FiltersASP"), getLayer7FiltersASP);

	websAspDefine(T("checkIfUnderBridgeModeASP"), checkIfUnderBridgeModeASP);
}

void firewall_init(void)
{
	LoadLayer7FilterName();

	// init filter
	iptablesAllFilterClear();
	// make a new chain
	doSystem("iptables -t filter -N %s 1>/dev/null 2>&1", WEB_FILTER_CHAIN);
	doSystem("iptables -t filter -N %s 1>/dev/null 2>&1", IPPORT_FILTER_CHAIN);
	doSystem("iptables -t filter -N %s 1>/dev/null 2>&1", MALICIOUS_FILTER_CHAIN);
	doSystem("iptables -t filter -N %s 1>/dev/null 2>&1", SYNFLOOD_FILTER_CHAIN);
	doSystem("iptables -t filter -N %s 1>/dev/null 2>&1", MALICIOUS_INPUT_FILTER_CHAIN);
	doSystem("iptables -t filter -N %s 1>/dev/null 2>&1", SYNFLOOD_INPUT_FILTER_CHAIN);

	doSystem("iptables -t filter -A FORWARD -j %s 1>/dev/null 2>&1", WEB_FILTER_CHAIN);
	doSystem("iptables -t filter -A FORWARD -j %s 1>/dev/null 2>&1", IPPORT_FILTER_CHAIN);
	doSystem("iptables -t filter -A FORWARD -j %s 1>/dev/null 2>&1", MALICIOUS_FILTER_CHAIN);
	doSystem("iptables -t filter -A %s -p tcp --syn -j %s 1>/dev/null 2>&1", MALICIOUS_FILTER_CHAIN, SYNFLOOD_FILTER_CHAIN);
	doSystem("iptables -t filter -A INPUT -j %s 1>/dev/null 2>&1", MALICIOUS_INPUT_FILTER_CHAIN);
	doSystem("iptables -t filter -A %s -p tcp --syn -j %s 1>/dev/null 2>&1", MALICIOUS_INPUT_FILTER_CHAIN, SYNFLOOD_INPUT_FILTER_CHAIN);
	doSystem("iptables -A FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu 1>/dev/null 2>&1");
	iptablesAllFilterRun();

	// init NAT(DMZ)
	// We use -I instead of -A here to prevent from default MASQUERADE NAT rule 
	// being in front of us.
	// So "port forward chain" has the highest priority in the system, and "DMZ chain" is the second one.
	iptablesAllNATClear();
	doSystem("iptables -t nat -N %s 1>/dev/null 2>&1; iptables -t nat -I PREROUTING 1 -j %s 1>/dev/null 2>&1", PORT_FORWARD_CHAIN, PORT_FORWARD_CHAIN);
	doSystem("iptables -t nat -N %s 1>/dev/null 2>&1; iptables -t nat -I PREROUTING 2 -j %s 1>/dev/null 2>&1", DMZ_CHAIN, DMZ_CHAIN);
	iptablesAllNATRun();
}

void firewall_fini(void)
{
	iptablesAllFilterClear();
	iptablesAllNATClear();
}
