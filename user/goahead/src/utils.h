/*
 *	utils.h -- System Utilitie Header
 *
 *	Copyright (c) Ralink Technology Corporation All Rights Reserved.
 *
 *	$Id: utils.h,v 1.25.16.1 2010-02-25 05:15:54 winfred Exp $
 */
#include 	"webs.h"
#include	"uemf.h"

#define LED_ON				1
#define LED_OFF				0

#define WPS_LED_RESET			1
#define WPS_LED_PROGRESS		2
#define WPS_LED_ERROR			3
#define WPS_LED_SESSION_OVERLAP		4
#define WPS_LED_SUCCESS			5

int doKillPid(char_t *fmt, ...);
int doSystem(char_t *fmt, ...);
char *getNthValue(int index, char *values);
char *setNthValue(int index, char *old_values, char *new_value);
int deleteNthValueMulti(int index[], int count, char *value, char delimit);
int getValueCount(char *values);
void formDefineUtilities(void);
int checkSemicolon(char *str);
int isInLan(char *radius_ip_addr);
int getNthValueSafe(int index, char *value, char delimit, char *result, int len);
int setTimer(int mili, void ((*sigroutine)(int)));
void stopTimer(void);
int ledAlways(int gpio, int on);
int ledWps(int gpio, int mode);
unsigned int Sleep(unsigned int secs);
char *racat(char *s, int i);
void arplookup(char *ip, char *arp);
void websLongWrite(webs_t wp, char *longstr);
int netmask_aton(const char *ip);
int getOnePortOnly(void);
