/* ompgsql.c
 * This is the implementation of the build-in output module for PgSQL.
 *
 * NOTE: read comments in module-template.h to understand how this file
 *       works!
 *
 * File begun on 2007-10-18 by sur5r (converted from ommysql.c)
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
#include "config.h"
#include "rsyslog.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <libpq-fe.h>
#include "syslogd.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "template.h"
#include "ompgsql.h"
#include "module-template.h"
#include "errmsg.h"

MODULE_TYPE_OUTPUT

/* internal structures
 */
DEF_OMOD_STATIC_DATA
DEFobjCurrIf(errmsg)

typedef struct _instanceData {
	PGconn	*f_hpgsql;			/* handle to PgSQL */
	char	f_dbsrv[MAXHOSTNAMELEN+1];	/* IP or hostname of DB server*/ 
	char	f_dbname[_DB_MAXDBLEN+1];	/* DB name */
	char	f_dbuid[_DB_MAXUNAMELEN+1];	/* DB user */
	char	f_dbpwd[_DB_MAXPWDLEN+1];	/* DB user's password */
	ConnStatusType	eLastPgSQLStatus; 	/* last status from postgres */
} instanceData;


BEGINcreateInstance
CODESTARTcreateInstance
ENDcreateInstance


BEGINisCompatibleWithFeature
CODESTARTisCompatibleWithFeature
	if(eFeat == sFEATURERepeatedMsgReduction)
		iRet = RS_RET_OK;
ENDisCompatibleWithFeature


/* The following function is responsible for closing a
 * PgSQL connection.
 */
static void closePgSQL(instanceData *pData)
{
	assert(pData != NULL);

	if(pData->f_hpgsql != NULL) {	/* just to be on the safe side... */
		PQfinish(pData->f_hpgsql);
		pData->f_hpgsql = NULL;
	}
}

BEGINfreeInstance
CODESTARTfreeInstance
	closePgSQL(pData);
ENDfreeInstance


BEGINdbgPrintInstInfo
CODESTARTdbgPrintInstInfo
	/* nothing special here */
ENDdbgPrintInstInfo


/* log a database error with descriptive message.
 * We check if we have a valid handle. If not, we simply
 * report an error, but can not be specific. RGerhards, 2007-01-30
 */
static void reportDBError(instanceData *pData, int bSilent)
{
	char errMsg[512];
	ConnStatusType ePgSQLStatus;

	assert(pData != NULL);
	bSilent=0;

	/* output log message */
	errno = 0;
	if(pData->f_hpgsql == NULL) {
		errmsg.LogError(NO_ERRCODE, "unknown DB error occured - could not obtain PgSQL handle");
	} else { /* we can ask pgsql for the error description... */
		ePgSQLStatus = PQstatus(pData->f_hpgsql);
		snprintf(errMsg, sizeof(errMsg)/sizeof(char), "db error (%d): %s\n", ePgSQLStatus,
				PQerrorMessage(pData->f_hpgsql));
		if(bSilent || ePgSQLStatus == pData->eLastPgSQLStatus)
			dbgprintf("pgsql, DBError(silent): %s\n", errMsg);
		else {
			pData->eLastPgSQLStatus = ePgSQLStatus;
			errmsg.LogError(NO_ERRCODE, "%s", errMsg);
		}
	}
		
	return;
}


/* The following function is responsible for initializing a
 * PgSQL connection.
 */
static rsRetVal initPgSQL(instanceData *pData, int bSilent)
{
	DEFiRet;

	assert(pData != NULL);
	assert(pData->f_hpgsql == NULL);

	dbgprintf("host=%s dbname=%s uid=%s\n",pData->f_dbsrv,pData->f_dbname,pData->f_dbuid);

	/* Connect to database */
	if((pData->f_hpgsql=PQsetdbLogin(pData->f_dbsrv, NULL, NULL, NULL,
				pData->f_dbname, pData->f_dbuid, pData->f_dbpwd)) == NULL) {
		reportDBError(pData, bSilent);
		closePgSQL(pData); /* ignore any error we may get */
		iRet = RS_RET_SUSPENDED;
	}

	RETiRet;
}


/* The following function writes the current log entry
 * to an established PgSQL session.
 */
rsRetVal writePgSQL(uchar *psz, instanceData *pData)
{
	DEFiRet;

	assert(psz != NULL);
	assert(pData != NULL);

	dbgprintf("writePgSQL: %s", psz);

	/* try insert */
	PQexec(pData->f_hpgsql, (char*)psz);
	if(PQstatus(pData->f_hpgsql) != CONNECTION_OK) {
		/* error occured, try to re-init connection and retry */
		closePgSQL(pData); /* close the current handle */
		CHKiRet(initPgSQL(pData, 0)); /* try to re-open */
		PQexec(pData->f_hpgsql, (char*)psz);
		if(PQstatus(pData->f_hpgsql) != CONNECTION_OK) { /* re-try insert */
			/* we failed, giving up for now */
			reportDBError(pData, 0);
			closePgSQL(pData); /* free ressources */
			ABORT_FINALIZE(RS_RET_SUSPENDED);
		}
	}

finalize_it:
	if(iRet == RS_RET_OK) {
		pData->eLastPgSQLStatus = CONNECTION_OK; /* reset error for error supression */
	}

	RETiRet;
}


BEGINtryResume
CODESTARTtryResume
	if(pData->f_hpgsql == NULL) {
		iRet = initPgSQL(pData, 1);
	}
ENDtryResume

BEGINdoAction
CODESTARTdoAction
	dbgprintf("\n");
	iRet = writePgSQL(ppString[0], pData);
ENDdoAction


BEGINparseSelectorAct
	int iPgSQLPropErr = 0;
CODESTARTparseSelectorAct
CODE_STD_STRING_REQUESTparseSelectorAct(1)
	/* first check if this config line is actually for us
	 * The first test [*p == '>'] can be skipped if a module shall only
	 * support the newer slection syntax [:modname:]. This is in fact
	 * recommended for new modules. Please note that over time this part
	 * will be handled by rsyslogd itself, but for the time being it is
	 * a good compromise to do it at the module level.
	 * rgerhards, 2007-10-15
	 */

	if(!strncmp((char*) p, ":ompgsql:", sizeof(":ompgsql:") - 1)) {
		p += sizeof(":ompgsql:") - 1; /* eat indicator sequence (-1 because of '\0'!) */
	} else {
		ABORT_FINALIZE(RS_RET_CONFLINE_UNPROCESSED);
	}

	/* ok, if we reach this point, we have something for us */
	if((iRet = createInstance(&pData)) != RS_RET_OK)
		goto finalize_it;


	/* sur5r 2007-10-18: added support for PgSQL
	 * :ompgsql:server,dbname,userid,password
	 * Now we read the PgSQL connection properties 
	 * and verify that the properties are valid.
	 */
	if(getSubString(&p, pData->f_dbsrv, MAXHOSTNAMELEN+1, ','))
		iPgSQLPropErr++;
	dbgprintf("%p:%s\n",p,p);
	if(*pData->f_dbsrv == '\0')
		iPgSQLPropErr++;
	if(getSubString(&p, pData->f_dbname, _DB_MAXDBLEN+1, ','))
		iPgSQLPropErr++;
	if(*pData->f_dbname == '\0')
		iPgSQLPropErr++;
	if(getSubString(&p, pData->f_dbuid, _DB_MAXUNAMELEN+1, ','))
		iPgSQLPropErr++;
	if(*pData->f_dbuid == '\0')
		iPgSQLPropErr++;
	if(getSubString(&p, pData->f_dbpwd, _DB_MAXPWDLEN+1, ';'))
		iPgSQLPropErr++;
	/* now check for template
	 * We specify that the SQL option must be present in the template.
	 * This is for your own protection (prevent sql injection).
	 */
	if(*(p-1) == ';')
		--p;	/* TODO: the whole parsing of the MySQL module needs to be re-thought - but this here
			 *       is clean enough for the time being -- rgerhards, 2007-07-30
			 *       kept it for pgsql -- sur5r, 2007-10-19
			 */
	CHKiRet(cflineParseTemplateName(&p, *ppOMSR, 0, OMSR_RQD_TPL_OPT_SQL, (uchar*) " StdPgSQLFmt"));
	
	/* If we detect invalid properties, we disable logging, 
	 * because right properties are vital at this place.  
	 * Retries make no sense. 
	 */
	if (iPgSQLPropErr) { 
		errmsg.LogError(NO_ERRCODE, "Trouble with PgSQL connection properties. -PgSQL logging disabled");
		ABORT_FINALIZE(RS_RET_INVALID_PARAMS);
	} else {
		CHKiRet(initPgSQL(pData, 0));
	}

CODE_STD_FINALIZERparseSelectorAct
ENDparseSelectorAct

BEGINmodExit
CODESTARTmodExit
ENDmodExit


BEGINqueryEtryPt
CODESTARTqueryEtryPt
CODEqueryEtryPt_STD_OMOD_QUERIES
ENDqueryEtryPt


BEGINmodInit()
CODESTARTmodInit
	*ipIFVersProvided = CURR_MOD_IF_VERSION; /* we only support the current interface specification */
CODEmodInit_QueryRegCFSLineHdlr
	CHKiRet(objUse(errmsg, CORE_COMPONENT));
ENDmodInit
/*
 * vi:set ai:
 */
