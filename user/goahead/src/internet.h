/*
 *	internet.h -- Internet Configuration Header
 *
 *	Copyright (c) Ralink Technology Corporation All Rights Reserved.
 *
 *	$Id: internet.h,v 1.8 2008-03-17 07:47:16 yy Exp $
 */

void formDefineInternet(void);
int getIfLive(char *ifname);
int getIfMac(char *ifname, char *if_hw);
int getIfIp(char *ifname, char *if_addr);
int getIfIsUp(char *ifname);
int getIfNetmask(char *ifname, char *if_net);
char* getWanIfName(void);
char* getWanIfNamePPP(void);
char* getLanIfName(void);
char *getLanWanNamebyIf(char *ifname);
int initInternet(void);


