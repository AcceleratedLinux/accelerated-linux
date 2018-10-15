/* msg.h
 * Header file for all msg-related functions.
 *
 * File begun on 2007-07-13 by RGerhards (extracted from syslogd.c)
 *
 * Copyright 2007 Rainer Gerhards and Adiscon GmbH.
 *
 * This file is part of rsyslog.
 *
 * Rsyslog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rsyslog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rsyslog.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
 */
#include "template.h" /* this is a quirk, but these two are too interdependant... */

#ifndef	MSG_H_INCLUDED
#define	MSG_H_INCLUDED 1

#include <pthread.h>
#include "obj.h"
#include "syslogd-types.h"
#include "template.h"

/* rgerhards 2004-11-08: The following structure represents a
 * syslog message. 
 *
 * Important Note:
 * The message object is used for multiple purposes (once it
 * has been created). Once created, it actully is a read-only
 * object (though we do not specifically express this). In order
 * to avoid multiple copies of the same object, we use a
 * reference counter. This counter is set to 1 by the constructer
 * and increased by 1 with a call to MsgAddRef(). The destructor
 * checks the reference count. If it is more than 1, only the counter
 * will be decremented. If it is 1, however, the object is actually
 * destroyed. To make this work, it is vital that MsgAddRef() is
 * called each time a "copy" is stored somewhere.
 */
struct msg {
	BEGINobjInstance;	/* Data to implement generic object - MUST be the first data element! */
	pthread_mutexattr_t mutAttr;
	pthread_mutex_t mut;
	int	iRefCount;	/* reference counter (0 = unused) */
	short	bParseHOSTNAME;	/* should the hostname be parsed from the message? */
	   /* background: the hostname is not present on "regular" messages
	    * received via UNIX domain sockets from the same machine. However,
	    * it is available when we have a forwarder (e.g. rfc3195d) using local
	    * sockets. All in all, the parser would need parse templates, that would
	    * resolve all these issues... rgerhards, 2005-10-06
	    */
	flowControl_t flowCtlType; /**< type of flow control we can apply, for enqueueing, needs not to be persisted because
				        once data has entered the queue, this property is no longer needed. */
	short	iSeverity;	/* the severity 0..7 */
	uchar *pszSeverity;	/* severity as string... */
	int iLenSeverity;	/* ... and its length. */
 	uchar *pszSeverityStr;   /* severity name... */
 	int iLenSeverityStr;    /* ... and its length. */
	short	iFacility;	/* Facility code 0 .. 23*/
	uchar *pszFacility;	/* Facility as string... */
	int iLenFacility;	/* ... and its length. */
 	uchar *pszFacilityStr;   /* facility name... */
 	int iLenFacilityStr;    /* ... and its length. */
	uchar *pszPRI;		/* the PRI as a string */
	int iLenPRI;		/* and its length */
	uchar	*pszRawMsg;	/* message as it was received on the
				 * wire. This is important in case we
				 * need to preserve cryptographic verifiers.
				 */
	int	iLenRawMsg;	/* length of raw message */
	uchar	*pszMSG;	/* the MSG part itself */
	int	iLenMSG;	/* Length of the MSG part */
	uchar	*pszUxTradMsg;	/* the traditional UNIX message */
	int	iLenUxTradMsg;/* Length of the traditional UNIX message */
	uchar	*pszTAG;	/* pointer to tag value */
	int	iLenTAG;	/* Length of the TAG part */
	uchar	*pszHOSTNAME;	/* HOSTNAME from syslog message */
	int	iLenHOSTNAME;	/* Length of HOSTNAME */
	uchar	*pszRcvFrom;	/* System message was received from */
	int	iLenRcvFrom;	/* Length of pszRcvFrom */
	short	iProtocolVersion;/* protocol version of message received 0 - legacy, 1 syslog-protocol) */
	cstr_t *pCSProgName;	/* the (BSD) program name */
	cstr_t *pCSStrucData;/* STRUCTURED-DATA */
	cstr_t *pCSAPPNAME;	/* APP-NAME */
	cstr_t *pCSPROCID;	/* PROCID */
	cstr_t *pCSMSGID;	/* MSGID */
	struct syslogTime tRcvdAt;/* time the message entered this program */
	char *pszRcvdAt3164;	/* time as RFC3164 formatted string (always 15 charcters) */
	char *pszRcvdAt3339;	/* time as RFC3164 formatted string (32 charcters at most) */
	char *pszRcvdAt_MySQL;	/* rcvdAt as MySQL formatted string (always 14 charcters) */
        char *pszRcvdAt_PgSQL;  /* rcvdAt as PgSQL formatted string (always 21 characters) */
	struct syslogTime tTIMESTAMP;/* (parsed) value of the timestamp */
	char *pszTIMESTAMP3164;	/* TIMESTAMP as RFC3164 formatted string (always 15 charcters) */
	char *pszTIMESTAMP3339;	/* TIMESTAMP as RFC3339 formatted string (32 charcters at most) */
	char *pszTIMESTAMP_MySQL;/* TIMESTAMP as MySQL formatted string (always 14 charcters) */
        char *pszTIMESTAMP_PgSQL;/* TIMESTAMP as PgSQL formatted string (always 21 characters) */
	int msgFlags;		/* flags associated with this message */
};
typedef struct msg msg_t;	/* new name */

/* function prototypes
 */
PROTOTYPEObjClassInit(msg);
char* getProgramName(msg_t*);
rsRetVal msgConstruct(msg_t **ppThis);
rsRetVal msgDestruct(msg_t **ppM);
msg_t* MsgDup(msg_t* pOld);
msg_t *MsgAddRef(msg_t *pM);
void setProtocolVersion(msg_t *pM, int iNewVersion);
int getProtocolVersion(msg_t *pM);
char *getProtocolVersionString(msg_t *pM);
int getMSGLen(msg_t *pM);
char *getRawMsg(msg_t *pM);
char *getUxTradMsg(msg_t *pM);
char *getMSG(msg_t *pM);
char *getPRI(msg_t *pM);
int getPRIi(msg_t *pM);
char *getTimeReported(msg_t *pM, enum tplFormatTypes eFmt);
char *getTimeGenerated(msg_t *pM, enum tplFormatTypes eFmt);
char *getSeverity(msg_t *pM);
char *getSeverityStr(msg_t *pM);
char *getFacility(msg_t *pM);
char *getFacilityStr(msg_t *pM);
rsRetVal MsgSetAPPNAME(msg_t *pMsg, char* pszAPPNAME);
char *getAPPNAME(msg_t *pM);
rsRetVal MsgSetPROCID(msg_t *pMsg, char* pszPROCID);
int getPROCIDLen(msg_t *pM);
char *getPROCID(msg_t *pM);
rsRetVal MsgSetMSGID(msg_t *pMsg, char* pszMSGID);
void MsgAssignTAG(msg_t *pMsg, uchar *pBuf);
void MsgSetTAG(msg_t *pMsg, char* pszTAG);
rsRetVal MsgSetFlowControlType(msg_t *pMsg, flowControl_t eFlowCtl);
char *getTAG(msg_t *pM);
int getHOSTNAMELen(msg_t *pM);
char *getHOSTNAME(msg_t *pM);
char *getRcvFrom(msg_t *pM);
rsRetVal MsgSetStructuredData(msg_t *pMsg, char* pszStrucData);
char *getStructuredData(msg_t *pM);
int getProgramNameLen(msg_t *pM);
char *getProgramName(msg_t *pM);
void MsgSetRcvFrom(msg_t *pMsg, char* pszRcvFrom);
void MsgAssignHOSTNAME(msg_t *pMsg, char *pBuf);
void MsgSetHOSTNAME(msg_t *pMsg, char* pszHOSTNAME);
int MsgSetUxTradMsg(msg_t *pMsg, char* pszUxTradMsg);
void MsgSetMSG(msg_t *pMsg, char* pszMSG);
void MsgSetRawMsg(msg_t *pMsg, char* pszRawMsg);
void moveHOSTNAMEtoTAG(msg_t *pM);
char *getMSGID(msg_t *pM);
char *MsgGetProp(msg_t *pMsg, struct templateEntry *pTpe,
                 cstr_t *pCSPropName, unsigned short *pbMustBeFreed);
char *textpri(char *pRes, size_t pResLen, int pri);
rsRetVal msgGetMsgVar(msg_t *pThis, cstr_t *pstrPropName, var_t **ppVar);
rsRetVal MsgEnableThreadSafety(void);

/* The MsgPrepareEnqueue() function is a macro for performance reasons.
 * It needs one global variable to work. This is acceptable, as it gains
 * us quite some performance and is fully abstracted using this header file.
 * The important thing is that no other module is permitted to actually
 * access that global variable! -- rgerhards, 2008-01-05
 */
extern void (*funcMsgPrepareEnqueue)(msg_t *pMsg);
#define MsgPrepareEnqueue(pMsg) funcMsgPrepareEnqueue(pMsg)

#endif /* #ifndef MSG_H_INCLUDED */
/*
 * vi:set ai:
 */
