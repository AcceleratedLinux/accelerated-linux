/* obj.c
 *
 * This file implements a generic object "class". All other classes can
 * use the service of this base class here to include auto-destruction and
 * other capabilities in a generic manner.
 *
 * As of 2008-02-29, I (rgerhards) am adding support for dynamically loadable
 * objects. In essence, each object will soon be available via its interface,
 * only. Before any object's code is accessed (including global static methods),
 * the caller needs to obtain an object interface. To do so, it needs to provide
 * the object name and the file where the object is expected to reside in. A
 * file may not be given, in which case the object is expected to reside in
 * the rsyslog core. The caller than receives an interface pointer which can
 * be utilized to access all the object's methods. This method enables rsyslog
 * to load library modules on demand. In order to keep overhead low, callers
 * should request object interface only once in the object Init function and
 * free them when they exit. The only exception is when a caller needs to
 * access an object only conditional, in which case a pointer to its interface
 * shall be aquired as need first arises but still be released only on exit
 * or when there definitely is no further need. The whole idea is to limit
 * the very performance-intense act of dynamically loading an objects library.
 * Of course, it is possible to violate this suggestion, but than you should
 * have very good reasoning to do so.
 *
 * Please note that there is one trick we need to do. Each object queries
 * the object interfaces and it does so via objUse(). objUse, however, is
 * part of the obj object's interface (implemented via the file you are
 * just reading). So in order to obtain a pointer to objUse, we need to
 * call it - obviously not possible. One solution would be that objUse is
 * hardcoded into all callers. That, however, would bring us into slight
 * trouble with actually dynamically loaded modules, as we should NOT
 * rely on the OS loader to resolve symbols back to the caller (this
 * is a feature not universally available and highly importable). Of course,
 * we can solve this with a pHostQueryEtryPoint() call. It still sounds
 * somewhat unnatural to call a regular interface function via a special
 * method. So what we do instead is define a special function called
 * objGetObjInterface() which delivers our own interface. That function
 * than will be defined global and be queriable via pHostQueryEtryPoint().
 * I agree, technically this is much the same, but from an architecture
 * point of view it looks cleaner (at least to me).
 * 
 * Please note that there is another egg-hen problem: we use a linked list,
 * which is provided by the linkedList object. However, we need to
 * initialize the linked list before we can provide the UseObj()
 * functionality. That, in turn, would probably be required by the 
 * linkedList object. So the solution is to use a backdoor just to
 * init the linked list and from then on use the usual interfaces.
 *
 * File begun on 2008-01-04 by RGerhards
 *
 * Copyright 2008 Rainer Gerhards and Adiscon GmbH.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

/* how many objects are supported by rsyslogd? */
#define OBJ_NUM_IDS 100 /* TODO change to a linked list?  info: 16 were currently in use 2008-02-29 */

#include "rsyslog.h"
#include "syslogd-types.h"
#include "srUtils.h"
#include "obj.h"
#include "stream.h"
#include "modules.h"
#include "errmsg.h"
#include "cfsysline.h"

/* static data */
DEFobjCurrIf(obj) /* we define our own interface, as this is expected by some macros! */
DEFobjCurrIf(var)
DEFobjCurrIf(module)
DEFobjCurrIf(errmsg)
static objInfo_t *arrObjInfo[OBJ_NUM_IDS]; /* array with object information pointers */


/* cookies for serialized lines */
#define COOKIE_OBJLINE   '<'
#define COOKIE_PROPLINE  '+'
#define COOKIE_ENDLINE   '>'
#define COOKIE_BLANKLINE '.'

/* forward definitions */
static rsRetVal FindObjInfo(cstr_t *pszObjName, objInfo_t **ppInfo);

/* methods */

/* This is a dummy method to be used when a standard method has not been
 * implemented by an object. Having it allows us to simply call via the 
 * jump table without any NULL pointer checks - which gains quite
 * some performance. -- rgerhards, 2008-01-04
 */
static rsRetVal objInfoNotImplementedDummy(void __attribute__((unused)) *pThis)
{
	return RS_RET_NOT_IMPLEMENTED;
}

/* and now the macro to check if something is not implemented
 * must be provided an objInfo_t pointer.
 */
#define objInfoIsImplemented(pThis, method) \
	(pThis->objMethods[method] != objInfoNotImplementedDummy)

/* construct an object Info object. Each class shall do this on init. The
 * resulting object shall be cached during the lifetime of the class and each
 * object shall receive a reference. A constructor and destructor MUST be provided for all
 * objects, thus they are in the parameter list.
 * pszID is the identifying object name and must point to constant pool memory. It is never freed.
 */
static rsRetVal
InfoConstruct(objInfo_t **ppThis, uchar *pszID, int iObjVers,
              rsRetVal (*pConstruct)(void *), rsRetVal (*pDestruct)(void *),
	      rsRetVal (*pQueryIF)(interface_t*), modInfo_t *pModInfo)
{
	DEFiRet;
	int i;
	objInfo_t *pThis;

	assert(ppThis != NULL);

	if((pThis = calloc(1, sizeof(objInfo_t))) == NULL)
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);

	pThis->pszID = pszID;
	pThis->lenID = strlen((char*)pszID);
	pThis->pszName = (uchar*)strdup((char*)pszID); /* it's OK if we have NULL ptr, GetName() will deal with that! */
	pThis->iObjVers = iObjVers;
	pThis->QueryIF = pQueryIF;
	pThis->pModInfo = pModInfo;

	pThis->objMethods[0] = pConstruct;
	pThis->objMethods[1] = pDestruct;
	for(i = 2 ; i < OBJ_NUM_METHODS ; ++i) {
		pThis->objMethods[i] = objInfoNotImplementedDummy;
	}

	*ppThis = pThis;

finalize_it:
	RETiRet;
}


/* destruct the objInfo object - must be done only when no more instances exist.
 * rgerhards, 2008-03-10
 */
static rsRetVal
InfoDestruct(objInfo_t **ppThis)
{
	DEFiRet;
	objInfo_t *pThis;

	assert(ppThis != NULL);
	pThis = *ppThis;
	assert(pThis != NULL);

	if(pThis->pszName != NULL)
		free(pThis->pszName);
	free(pThis);
	*ppThis = NULL;

	RETiRet;
}


/* set a method handler */
static rsRetVal
InfoSetMethod(objInfo_t *pThis, objMethod_t objMethod, rsRetVal (*pHandler)(void*))
{
	assert(pThis != NULL);
	assert(objMethod > 0 && objMethod < OBJ_NUM_METHODS);
	pThis->objMethods[objMethod] = pHandler;

	return RS_RET_OK;
}

/* destruct the base object properties.
 * rgerhards, 2008-01-29
 */
static rsRetVal
DestructObjSelf(obj_t *pThis)
{
	DEFiRet;

	ISOBJ_assert(pThis);
	if(pThis->pszName != NULL) {
		free(pThis->pszName);
	}

	RETiRet;
}


/* --------------- object serializiation / deserialization support --------------- */


/* serialize the header of an object
 * pszRecType must be either "Obj" (Object) or "OPB" (Object Property Bag)
 */
static rsRetVal objSerializeHeader(strm_t *pStrm, obj_t *pObj, uchar *pszRecType)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pStrm, strm);
	ISOBJ_assert(pObj);
	assert(!strcmp((char*) pszRecType, "Obj") || !strcmp((char*) pszRecType, "OPB"));

	/* object cookie and serializer version (so far always 1) */
	CHKiRet(strmWriteChar(pStrm, COOKIE_OBJLINE));
	CHKiRet(strmWrite(pStrm, (uchar*) pszRecType, 3)); /* record types are always 3 octets */
	CHKiRet(strmWriteChar(pStrm, ':'));
	CHKiRet(strmWriteChar(pStrm, '1'));

	/* object type, version and string length */
	CHKiRet(strmWriteChar(pStrm, ':'));
	CHKiRet(strmWrite(pStrm, pObj->pObjInfo->pszID, pObj->pObjInfo->lenID));
	CHKiRet(strmWriteChar(pStrm, ':'));
	CHKiRet(strmWriteLong(pStrm, objGetVersion(pObj)));

	/* record trailer */
	CHKiRet(strmWriteChar(pStrm, ':'));
	CHKiRet(strmWriteChar(pStrm, '\n'));

finalize_it:
	RETiRet;
}


/* begin serialization of an object
 * rgerhards, 2008-01-06
 */
static rsRetVal
BeginSerialize(strm_t *pStrm, obj_t *pObj)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pStrm, strm);
	ISOBJ_assert(pObj);
	
	CHKiRet(strmRecordBegin(pStrm));
	CHKiRet(objSerializeHeader(pStrm, pObj, (uchar*) "Obj"));

finalize_it:
	RETiRet;
}
	

/* begin serialization of an object's property bag
 * Note: a property bag is used to serialize some of an objects
 * properties, but not necessarily all. A good example is the queue
 * object, which at some stage needs to serialize a number of its 
 * properties, but not the queue data itself. From the object point
 * of view, a property bag can not be used to re-instantiate an object.
 * Otherwise, the serialization is exactly the same.
 * rgerhards, 2008-01-11
 */
static rsRetVal
BeginSerializePropBag(strm_t *pStrm, obj_t *pObj)
{
	DEFiRet;

	ISOBJ_TYPE_assert(pStrm, strm);
	ISOBJ_assert(pObj);
	
	CHKiRet(strmRecordBegin(pStrm));
	CHKiRet(objSerializeHeader(pStrm, pObj, (uchar*) "OPB"));

finalize_it:
	RETiRet;
}


/* append a property
 */
static rsRetVal
SerializeProp(strm_t *pStrm, uchar *pszPropName, propType_t propType, void *pUsr)
{
	DEFiRet;
	uchar *pszBuf = NULL;
	size_t lenBuf = 0;
	uchar szBuf[64];
	varType_t vType = VARTYPE_NONE;

	ISOBJ_TYPE_assert(pStrm, strm);
	assert(pszPropName != NULL);

	/*dbgprintf("objSerializeProp: strm %p, propName '%s', type %d, pUsr %p\n", pStrm, pszPropName, propType, pUsr);*/
	/* if we have no user pointer, there is no need to write this property.
	 * TODO: think if that's the righ point of view
	 * rgerhards, 2008-01-06
	 */
	if(pUsr == NULL) {
		ABORT_FINALIZE(RS_RET_OK);
	}

	/* TODO: use the stream functions for data conversion here - should be quicker */

	switch(propType) {
		case PROPTYPE_PSZ:
			pszBuf = (uchar*) pUsr;
			lenBuf = strlen((char*) pszBuf);
			vType = VARTYPE_STR;
			break;
		case PROPTYPE_SHORT:
			CHKiRet(srUtilItoA((char*) szBuf, sizeof(szBuf), (long) *((short*) pUsr)));
			pszBuf = szBuf;
			lenBuf = strlen((char*) szBuf);
			vType = VARTYPE_NUMBER;
			break;
		case PROPTYPE_INT:
			CHKiRet(srUtilItoA((char*) szBuf, sizeof(szBuf), (long) *((int*) pUsr)));
			pszBuf = szBuf;
			lenBuf = strlen((char*) szBuf);
			vType = VARTYPE_NUMBER;
			break;
		case PROPTYPE_LONG:
			CHKiRet(srUtilItoA((char*) szBuf, sizeof(szBuf), *((long*) pUsr)));
			pszBuf = szBuf;
			lenBuf = strlen((char*) szBuf);
			vType = VARTYPE_NUMBER;
			break;
		case PROPTYPE_INT64:
			CHKiRet(srUtilItoA((char*) szBuf, sizeof(szBuf), *((int64*) pUsr)));
			pszBuf = szBuf;
			lenBuf = strlen((char*) szBuf);
			vType = VARTYPE_NUMBER;
			break;
		case PROPTYPE_CSTR:
			pszBuf = rsCStrGetSzStrNoNULL((cstr_t *) pUsr);
			lenBuf = rsCStrLen((cstr_t*) pUsr);
			vType = VARTYPE_STR;
			break;
		case PROPTYPE_SYSLOGTIME:
			lenBuf = snprintf((char*) szBuf, sizeof(szBuf), "%d:%d:%d:%d:%d:%d:%d:%d:%d:%c:%d:%d",
					  ((syslogTime_t*)pUsr)->timeType,
					  ((syslogTime_t*)pUsr)->year,
					  ((syslogTime_t*)pUsr)->month,
					  ((syslogTime_t*)pUsr)->day,
					  ((syslogTime_t*)pUsr)->hour,
					  ((syslogTime_t*)pUsr)->minute,
					  ((syslogTime_t*)pUsr)->second,
					  ((syslogTime_t*)pUsr)->secfrac,
					  ((syslogTime_t*)pUsr)->secfracPrecision,
					  ((syslogTime_t*)pUsr)->OffsetMode,
					  ((syslogTime_t*)pUsr)->OffsetHour,
					  ((syslogTime_t*)pUsr)->OffsetMinute);
			if(lenBuf > sizeof(szBuf) - 1)
				ABORT_FINALIZE(RS_RET_PROVIDED_BUFFER_TOO_SMALL);
			vType = VARTYPE_SYSLOGTIME;
			pszBuf = szBuf;
			break;
		default:
			dbgprintf("invalid PROPTYPE %d\n", propType);
			break;
	}

	/* cookie */
	CHKiRet(strmWriteChar(pStrm, COOKIE_PROPLINE));
	/* name */
	CHKiRet(strmWrite(pStrm, pszPropName, strlen((char*)pszPropName)));
	CHKiRet(strmWriteChar(pStrm, ':'));
	/* type */
	CHKiRet(strmWriteLong(pStrm, (int) vType));
	CHKiRet(strmWriteChar(pStrm, ':'));
	/* length */
	CHKiRet(strmWriteLong(pStrm, lenBuf));
	CHKiRet(strmWriteChar(pStrm, ':'));

	/* data */
	CHKiRet(strmWrite(pStrm, (uchar*) pszBuf, lenBuf));

	/* trailer */
	CHKiRet(strmWriteChar(pStrm, ':'));
	CHKiRet(strmWriteChar(pStrm, '\n'));

finalize_it:
	RETiRet;
}


/* end serialization of an object. The caller receives a
 * standard C string, which he must free when no longer needed.
 */
static rsRetVal
EndSerialize(strm_t *pStrm)
{
	DEFiRet;

	assert(pStrm != NULL);

	CHKiRet(strmWriteChar(pStrm, COOKIE_ENDLINE));
	CHKiRet(strmWrite(pStrm, (uchar*) "End\n", sizeof("END\n") - 1));
	CHKiRet(strmWriteChar(pStrm, COOKIE_BLANKLINE));
	CHKiRet(strmWriteChar(pStrm, '\n'));

	CHKiRet(strmRecordEnd(pStrm));

finalize_it:
	RETiRet;
}


/* define a helper to make code below a bit cleaner (and quicker to write) */
#define NEXTC CHKiRet(strmReadChar(pStrm, &c))//;dbgprintf("c: %c\n", c);


/* de-serialize an embedded, non-octect-counted string. This is useful
 * for deserializing the object name inside the header. The string is
 * terminated by the first occurence of the ':' character.
 * rgerhards, 2008-02-29
 */
static rsRetVal
objDeserializeEmbedStr(cstr_t **ppStr, strm_t *pStrm)
{
	DEFiRet;
	uchar c;
	cstr_t *pStr = NULL;

	assert(ppStr != NULL);

	CHKiRet(rsCStrConstruct(&pStr));

	NEXTC;
	while(c != ':') {
		CHKiRet(rsCStrAppendChar(pStr, c));
		NEXTC;
	}
	CHKiRet(rsCStrFinish(pStr));

	*ppStr = pStr;

finalize_it:
	if(iRet != RS_RET_OK && pStr != NULL)
		rsCStrDestruct(&pStr);

	RETiRet;
}


/* de-serialize a number */
static rsRetVal objDeserializeNumber(number_t *pNum, strm_t *pStrm)
{
	DEFiRet;
	number_t i;
	int bIsNegative;
	uchar c;

	assert(pNum != NULL);

	NEXTC;
	if(c == '-') {
		bIsNegative = 1;
		NEXTC;
	} else {
		bIsNegative = 0;
	}

	/* we check this so that we get more meaningful error codes */
	if(!isdigit(c)) ABORT_FINALIZE(RS_RET_INVALID_NUMBER);

	i = 0;
	while(isdigit(c)) {
		i = i * 10 + c - '0';
		NEXTC;
	}

	if(c != ':') ABORT_FINALIZE(RS_RET_INVALID_DELIMITER);

	if(bIsNegative)
		i *= -1;

	*pNum = i;
finalize_it:
	RETiRet;
}


/* de-serialize a string, length must be provided but may be 0 */
static rsRetVal objDeserializeStr(cstr_t **ppCStr, int iLen, strm_t *pStrm)
{
	DEFiRet;
	int i;
	uchar c;
	cstr_t *pCStr = NULL;

	assert(ppCStr != NULL);
	assert(iLen >= 0);

	CHKiRet(rsCStrConstruct(&pCStr));

	NEXTC;
	for(i = 0 ; i < iLen ; ++i) {
		CHKiRet(rsCStrAppendChar(pCStr, c));
		NEXTC;
	}
	CHKiRet(rsCStrFinish(pCStr));

	/* check terminator */
	if(c != ':') ABORT_FINALIZE(RS_RET_INVALID_DELIMITER);

	*ppCStr = pCStr;

finalize_it:
	if(iRet != RS_RET_OK && pCStr != NULL)
		rsCStrDestruct(&pCStr);

	RETiRet;
}


/* de-serialize a syslogTime -- rgerhards,2008-01-08 */
#define	GETVAL(var)  \
	CHKiRet(objDeserializeNumber(&l, pStrm)); \
	pTime->var = l;
static rsRetVal objDeserializeSyslogTime(syslogTime_t *pTime, strm_t *pStrm)
{
	DEFiRet;
	number_t l;
	uchar c;

	assert(pTime != NULL);

	GETVAL(timeType);
	GETVAL(year);
	GETVAL(month);
	GETVAL(day);
	GETVAL(hour);
	GETVAL(minute);
	GETVAL(second);
	GETVAL(secfrac);
	GETVAL(secfracPrecision);
	/* OffsetMode is a single character! */
	NEXTC; pTime->OffsetMode = c;
	NEXTC; if(c != ':') ABORT_FINALIZE(RS_RET_INVALID_DELIMITER);
	GETVAL(OffsetHour);
	GETVAL(OffsetMinute);

finalize_it:
	RETiRet;
}
#undef GETVAL

/* de-serialize an object header
 * rgerhards, 2008-01-07
 */
static rsRetVal objDeserializeHeader(uchar *pszRecType, cstr_t **ppstrID, int* poVers, strm_t *pStrm)
{
	DEFiRet;
	number_t oVers;
	uchar c;

	assert(ppstrID != NULL);
	assert(poVers != NULL);
	assert(!strcmp((char*) pszRecType, "Obj") || !strcmp((char*) pszRecType, "OPB"));

	/* check header cookie */
	NEXTC; if(c != COOKIE_OBJLINE) ABORT_FINALIZE(RS_RET_INVALID_HEADER);
	NEXTC; if(c != pszRecType[0]) ABORT_FINALIZE(RS_RET_INVALID_HEADER_RECTYPE);
	NEXTC; if(c != pszRecType[1]) ABORT_FINALIZE(RS_RET_INVALID_HEADER_RECTYPE);
	NEXTC; if(c != pszRecType[2]) ABORT_FINALIZE(RS_RET_INVALID_HEADER_RECTYPE);
	NEXTC; if(c != ':') ABORT_FINALIZE(RS_RET_INVALID_HEADER);
	NEXTC; if(c != '1') ABORT_FINALIZE(RS_RET_INVALID_HEADER_VERS);
	NEXTC; if(c != ':') ABORT_FINALIZE(RS_RET_INVALID_HEADER_VERS);

	/* object type and version */
	CHKiRet(objDeserializeEmbedStr(ppstrID, pStrm));
	CHKiRet(objDeserializeNumber(&oVers, pStrm));

	/* and now we skip over the rest until the delemiting \n */
	NEXTC;
	while(c != '\n') {
		NEXTC;
	}

	*poVers = oVers;

finalize_it:
	RETiRet;
}


/* Deserialize a single property. Pointer must be positioned at begin of line. Whole line
 * up until the \n is read.
 */
static rsRetVal objDeserializeProperty(var_t *pProp, strm_t *pStrm)
{
	DEFiRet;
	number_t i;
	number_t iLen;
	uchar c;

	assert(pProp != NULL);

	/* check cookie */
	NEXTC;
	if(c != COOKIE_PROPLINE) {
		/* oops, we've read one char that does not belong to use - unget it first */
		CHKiRet(strmUnreadChar(pStrm, c));
		ABORT_FINALIZE(RS_RET_NO_PROPLINE);
	}

	/* get the property name first */
	CHKiRet(rsCStrConstruct(&pProp->pcsName));

	NEXTC;
	while(c != ':') {
		CHKiRet(rsCStrAppendChar(pProp->pcsName, c));
		NEXTC;
	}
	CHKiRet(rsCStrFinish(pProp->pcsName));

	/* property type */
	CHKiRet(objDeserializeNumber(&i, pStrm));
	pProp->varType = i;

	/* size (needed for strings) */
	CHKiRet(objDeserializeNumber(&iLen, pStrm));

	/* we now need to deserialize the value */
	switch(pProp->varType) {
		case VARTYPE_STR:
			CHKiRet(objDeserializeStr(&pProp->val.pStr, iLen, pStrm));
			break;
		case VARTYPE_NUMBER:
			CHKiRet(objDeserializeNumber(&pProp->val.num, pStrm));
			break;
		case VARTYPE_SYSLOGTIME:
			CHKiRet(objDeserializeSyslogTime(&pProp->val.vSyslogTime, pStrm));
			break;
		default:
			dbgprintf("invalid VARTYPE %d\n", pProp->varType);
			break;
	}

	/* we should now be at the end of the line. So the next char must be \n */
	NEXTC;
	if(c != '\n') ABORT_FINALIZE(RS_RET_INVALID_PROPFRAME);

finalize_it:
	RETiRet;
}


/* de-serialize an object trailer. This does not get any data but checks if the
 * format is ok.
 * rgerhards, 2008-01-07
 */
static rsRetVal objDeserializeTrailer(strm_t *pStrm)
{
	DEFiRet;
	uchar c;

	/* check header cookie */
	NEXTC; if(c != COOKIE_ENDLINE) ABORT_FINALIZE(RS_RET_INVALID_TRAILER);
	NEXTC; if(c != 'E')  ABORT_FINALIZE(RS_RET_INVALID_TRAILER);
	NEXTC; if(c != 'n')  ABORT_FINALIZE(RS_RET_INVALID_TRAILER);
	NEXTC; if(c != 'd')  ABORT_FINALIZE(RS_RET_INVALID_TRAILER);
	NEXTC; if(c != '\n') ABORT_FINALIZE(RS_RET_INVALID_TRAILER);
	NEXTC; if(c != COOKIE_BLANKLINE) ABORT_FINALIZE(RS_RET_INVALID_TRAILER);
	NEXTC; if(c != '\n') ABORT_FINALIZE(RS_RET_INVALID_TRAILER);

finalize_it:
	RETiRet;
}



/* This method tries to recover a serial store if it got out of sync.
 * To do so, it scans the line beginning cookies and waits for the object
 * cookie. If that is found, control is returned. If the store is exhausted,
 * we will receive an RS_RET_EOF error as part of NEXTC, which will also
 * terminate this function. So we may either return with somehting that
 * looks like a valid object or end of store.
 * rgerhards, 2008-01-07
 */
static rsRetVal objDeserializeTryRecover(strm_t *pStrm)
{
	DEFiRet;
	uchar c;
	int bWasNL;
	int bRun;

	assert(pStrm != NULL);
	bRun = 1;
	bWasNL = 0;

	while(bRun) {
		NEXTC;
		if(c == '\n')
			bWasNL = 1;
		else {
			if(bWasNL == 1 && c == COOKIE_OBJLINE)
				bRun = 0; /* we found it! */
			else
				bWasNL = 0;
		}
	}

	CHKiRet(strmUnreadChar(pStrm, c));

finalize_it:
	dbgprintf("deserializer has possibly been able to re-sync and recover, state %d\n", iRet);
	RETiRet;
}


/* De-serialize the properties of an object. This includes processing
 * of the trailer. Header must already have been processed.
 * rgerhards, 2008-01-11
 */
static rsRetVal objDeserializeProperties(obj_t *pObj, objInfo_t *pObjInfo, strm_t *pStrm)
{
	DEFiRet;
	var_t *pVar = NULL;

	ISOBJ_assert(pObj);
	ISOBJ_TYPE_assert(pStrm, strm);
	ASSERT(pObjInfo != NULL);

	CHKiRet(var.Construct(&pVar));
	CHKiRet(var.ConstructFinalize(pVar));

	iRet = objDeserializeProperty(pVar, pStrm);
	while(iRet == RS_RET_OK) {
		CHKiRet(pObjInfo->objMethods[objMethod_SETPROPERTY](pObj, pVar));
		/* re-init var object - TODO: method of var! */
		rsCStrDestruct(&pVar->pcsName); /* no longer needed */
		if(pVar->varType == VARTYPE_STR) {
			if(pVar->val.pStr != NULL)
				rsCStrDestruct(&pVar->val.pStr);
		}
		iRet = objDeserializeProperty(pVar, pStrm);
	}

	if(iRet != RS_RET_NO_PROPLINE)
		FINALIZE;

	CHKiRet(objDeserializeTrailer(pStrm)); /* do trailer checks */
finalize_it:
	if(pVar != NULL)
		var.Destruct(&pVar);

	RETiRet;
}


/* De-Serialize an object.
 * Params: Pointer to object Pointer (pObj) (like a obj_t**, but can not do that due to compiler warning)
 * expected object ID (to check against), a fixup function that can modify the object before it is finalized
 * and a user pointer that is to be passed to that function in addition to the object. The fixup function
 * pointer may be NULL, in which case none is called.
 * The caller must destruct the created object.
 * rgerhards, 2008-01-07
 */
static rsRetVal
Deserialize(void *ppObj, uchar *pszTypeExpected, strm_t *pStrm, rsRetVal (*fFixup)(obj_t*,void*), void *pUsr)
{
	DEFiRet;
	rsRetVal iRetLocal;
	obj_t *pObj = NULL;
	int oVers = 0;   /* after all, it is totally useless but takes up some execution time...    */
	cstr_t *pstrID = NULL;
	objInfo_t *pObjInfo;

	assert(ppObj != NULL);
	assert(pszTypeExpected != NULL);
	ISOBJ_TYPE_assert(pStrm, strm);

	/* we de-serialize the header. if all goes well, we are happy. However, if
	 * we experience a problem, we try to recover. We do this by skipping to
	 * the next object header. This is defined via the line-start cookies. In
	 * worst case, we exhaust the queue, but then we receive EOF return state,
	 * from objDeserializeTryRecover(), what will cause us to ultimately give up.
	 * rgerhards, 2008-07-08
	 */
	do {
		iRetLocal = objDeserializeHeader((uchar*) "Obj", &pstrID, &oVers, pStrm);
		if(iRetLocal != RS_RET_OK) {
			dbgprintf("objDeserialize error %d during header processing - trying to recover\n", iRetLocal);
			CHKiRet(objDeserializeTryRecover(pStrm));
		}
	} while(iRetLocal != RS_RET_OK);

	if(rsCStrSzStrCmp(pstrID, pszTypeExpected, strlen((char*)pszTypeExpected))) // TODO: optimize strlen() - caller shall provide
		ABORT_FINALIZE(RS_RET_INVALID_OID);

	CHKiRet(FindObjInfo(pstrID, &pObjInfo));

	CHKiRet(pObjInfo->objMethods[objMethod_CONSTRUCT](&pObj));

	/* we got the object, now we need to fill the properties */
	CHKiRet(objDeserializeProperties(pObj, pObjInfo, pStrm));

	/* check if we need to call a fixup function that modifies the object
	 * before it is finalized. -- rgerhards, 2008-01-13
	 */
	if(fFixup != NULL)
		CHKiRet(fFixup(pObj, pUsr));

	/* we have a valid object, let's finalize our work and return */
	if(objInfoIsImplemented(pObjInfo, objMethod_CONSTRUCTION_FINALIZER))
		CHKiRet(pObjInfo->objMethods[objMethod_CONSTRUCTION_FINALIZER](pObj));

	*((obj_t**) ppObj) = pObj;

finalize_it:
	if(iRet != RS_RET_OK && pObj != NULL)
		free(pObj); // TODO: check if we can call destructor 2008-01-13 rger

	if(pstrID != NULL)
		rsCStrDestruct(&pstrID);

	RETiRet;
}


/* De-Serialize an object, but treat it as property bag.
 * rgerhards, 2008-01-11
 */
rsRetVal
objDeserializeObjAsPropBag(obj_t *pObj, strm_t *pStrm)
{
	DEFiRet;
	rsRetVal iRetLocal;
	cstr_t *pstrID = NULL;
	int oVers = 0;   /* after all, it is totally useless but takes up some execution time...    */
	objInfo_t *pObjInfo;

	ISOBJ_assert(pObj);
	ISOBJ_TYPE_assert(pStrm, strm);

	/* we de-serialize the header. if all goes well, we are happy. However, if
	 * we experience a problem, we try to recover. We do this by skipping to
	 * the next object header. This is defined via the line-start cookies. In
	 * worst case, we exhaust the queue, but then we receive EOF return state
	 * from objDeserializeTryRecover(), what will cause us to ultimately give up.
	 * rgerhards, 2008-07-08
	 */
	do {
		iRetLocal = objDeserializeHeader((uchar*) "Obj", &pstrID, &oVers, pStrm);
		if(iRetLocal != RS_RET_OK) {
			dbgprintf("objDeserializeObjAsPropBag error %d during header - trying to recover\n", iRetLocal);
			CHKiRet(objDeserializeTryRecover(pStrm));
		}
	} while(iRetLocal != RS_RET_OK);

	if(rsCStrSzStrCmp(pstrID, pObj->pObjInfo->pszID, pObj->pObjInfo->lenID))
		ABORT_FINALIZE(RS_RET_INVALID_OID);

	CHKiRet(FindObjInfo(pstrID, &pObjInfo));

	/* we got the object, now we need to fill the properties */
	CHKiRet(objDeserializeProperties(pObj, pObjInfo, pStrm));

finalize_it:
	if(pstrID != NULL)
		rsCStrDestruct(&pstrID);

	RETiRet;
}



/* De-Serialize an object property bag. As a property bag contains only partial properties,
 * it is not instanciable. Thus, the caller must provide a pointer of an already-instanciated
 * object of the correct type.
 * Params: Pointer to object (pObj)
 * Pointer to be passed to the function
 * The caller must destruct the created object.
 * rgerhards, 2008-01-07
 */
static rsRetVal
DeserializePropBag(obj_t *pObj, strm_t *pStrm)
{
	DEFiRet;
	rsRetVal iRetLocal;
	cstr_t *pstrID = NULL;
	int oVers;
	objInfo_t *pObjInfo;

	ISOBJ_assert(pObj);
	ISOBJ_TYPE_assert(pStrm, strm);

	/* we de-serialize the header. if all goes well, we are happy. However, if
	 * we experience a problem, we try to recover. We do this by skipping to
	 * the next object header. This is defined via the line-start cookies. In
	 * worst case, we exhaust the queue, but then we receive EOF return state
	 * from objDeserializeTryRecover(), what will cause us to ultimately give up.
	 * rgerhards, 2008-07-08
	 */
	do {
		iRetLocal = objDeserializeHeader((uchar*) "OPB", &pstrID, &oVers, pStrm);
		if(iRetLocal != RS_RET_OK) {
			dbgprintf("objDeserializePropBag error %d during header - trying to recover\n", iRetLocal);
			CHKiRet(objDeserializeTryRecover(pStrm));
		}
	} while(iRetLocal != RS_RET_OK);

	if(rsCStrSzStrCmp(pstrID, pObj->pObjInfo->pszID, pObj->pObjInfo->lenID))
		ABORT_FINALIZE(RS_RET_INVALID_OID);

	CHKiRet(FindObjInfo(pstrID, &pObjInfo));

	/* we got the object, now we need to fill the properties */
	CHKiRet(objDeserializeProperties(pObj, pObjInfo, pStrm));

finalize_it:
	if(pstrID != NULL)
		rsCStrDestruct(&pstrID);

	RETiRet;
}

#undef NEXTC /* undef helper macro */


/* --------------- end object serializiation / deserialization support --------------- */


/* set the object (instance) name
 * rgerhards, 2008-01-29
 * TODO: change the naming to a rsCStr obj! (faster)
 */
static rsRetVal
SetName(obj_t *pThis, uchar *pszName)
{
	DEFiRet;

	if(pThis->pszName != NULL)
		free(pThis->pszName);

	pThis->pszName = (uchar*) strdup((char*) pszName);

	if(pThis->pszName == NULL)
		ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY);

finalize_it:
	RETiRet;
}


/* get the object (instance) name
 * Note that we use a non-standard calling convention. Thus function must never
 * fail, else we run into real big problems. So it must make sure that at least someting
 * is returned.
 * rgerhards, 2008-01-30
 */
static uchar *
GetName(obj_t *pThis)
{
	uchar *ret;
	uchar szName[128];

	BEGINfunc
	ISOBJ_assert(pThis);

	if(pThis->pszName == NULL) {
		snprintf((char*)szName, sizeof(szName)/sizeof(uchar), "%s %p", objGetClassName(pThis), pThis);
		SetName(pThis, szName);
		/* looks strange, but we NEED to re-check because if there was an
		 * error in objSetName(), the pointer may still be NULL
		 */
		if(pThis->pszName == NULL) {
			ret = objGetClassName(pThis);
		} else {
			ret = pThis->pszName;
		}
	} else {
		ret = pThis->pszName;
	}

	ENDfunc
	return ret;
}


/* Find the objInfo object for the current object
 * rgerhards, 2008-02-29
 */
static rsRetVal
FindObjInfo(cstr_t *pstrOID, objInfo_t **ppInfo)
{
	DEFiRet;
	int bFound;
	int i;

	assert(pstrOID != NULL);
	assert(ppInfo != NULL);

	bFound = 0;
	i = 0;
	while(!bFound && i < OBJ_NUM_IDS) {
#if 0
RUNLOG_VAR("%d", i);
if(arrObjInfo[i] != NULL) {
RUNLOG_VAR("%p", arrObjInfo[i]->pszID);
RUNLOG_VAR("%s", arrObjInfo[i]->pszID);
}
#endif
		if(arrObjInfo[i] != NULL && !rsCStrSzStrCmp(pstrOID, arrObjInfo[i]->pszID, arrObjInfo[i]->lenID)) {
			bFound = 1;
			break;
		}
		++i;
	}

	if(!bFound)
		ABORT_FINALIZE(RS_RET_NOT_FOUND);

	*ppInfo = arrObjInfo[i];

finalize_it:
	if(iRet == RS_RET_OK) {
		/* DEV DEBUG ONLY dbgprintf("caller requested object '%s', found at index %d\n", (*ppInfo)->pszID, i);*/
		/*EMPTY BY INTENSION*/;
	} else {
		dbgprintf("caller requested object '%s', not found (iRet %d)\n", rsCStrGetSzStr(pstrOID), iRet);
	}

	RETiRet;
}


/* register a classes' info pointer, so that we can reference it later, if needed to
 * (e.g. for de-serialization support).
 * rgerhards, 2008-01-07
 * In this function, we look for a free space in the object table. While we do so, we
 * also detect if the same object has already been registered, which is not valid.
 * rgerhards, 2008-02-29
 */
static rsRetVal
RegisterObj(uchar *pszObjName, objInfo_t *pInfo)
{
	DEFiRet;
	int bFound;
	int i;

	assert(pszObjName != NULL);
	assert(pInfo != NULL);

	bFound = 0;
	i = 0;
	while(!bFound && i < OBJ_NUM_IDS && arrObjInfo[i] != NULL) {
		if(   arrObjInfo[i] != NULL
		   && !strcmp((char*)arrObjInfo[i]->pszID, (char*)pszObjName)) {
			bFound = 1;
			break;
		}
		++i;
	}

	if(bFound)           ABORT_FINALIZE(RS_RET_OBJ_ALREADY_REGISTERED);
	if(i >= OBJ_NUM_IDS) ABORT_FINALIZE(RS_RET_OBJ_REGISTRY_OUT_OF_SPACE);

	arrObjInfo[i] = pInfo;
	dbgprintf("object '%s' successfully registered with index %d, qIF %p\n", pszObjName, i, pInfo->QueryIF);

finalize_it:
	if(iRet != RS_RET_OK) {
		errmsg.LogError(NO_ERRCODE, "registering object '%s' failed with error code %d", pszObjName, iRet);
	}

	RETiRet;
}


/* deregister a classes' info pointer, usually called because the class is unloaded.
 * After deregistration, the class can no longer be accessed, except if it is reloaded.
 * rgerhards, 2008-03-10
 */
static rsRetVal
UnregisterObj(uchar *pszObjName, objInfo_t *pInfo)
{
	DEFiRet;
	int bFound;
	int i;

	assert(pszObjName != NULL);
	assert(pInfo != NULL);

	bFound = 0;
	i = 0;
	while(!bFound && i < OBJ_NUM_IDS) {
		if(   arrObjInfo[i] != NULL
		   && !strcmp((char*)arrObjInfo[i]->pszID, (char*)pszObjName)) {
			bFound = 1;
			break;
		}
		++i;
	}

	if(!bFound)
		ABORT_FINALIZE(RS_RET_OBJ_NOT_REGISTERED);

	InfoDestruct(&arrObjInfo[i]);
	dbgprintf("object '%s' successfully unregistered with index %d\n", pszObjName, i);

finalize_it:
	if(iRet != RS_RET_OK) {
		dbgprintf("unregistering object '%s' failed with error code %d\n", pszObjName, iRet);
	}

	RETiRet;
}


/* This function shall be called by anyone who would like to use an object. It will
 * try to locate the object, load it into memory if not already present and return
 * a pointer to the objects interface.
 * rgerhards, 2008-02-29
 */
static rsRetVal
UseObj(char *srcFile, uchar *pObjName, uchar *pObjFile, interface_t *pIf)
{
	DEFiRet;
	cstr_t *pStr = NULL;
	objInfo_t *pObjInfo;


	dbgprintf("source file %s requests object '%s', ifIsLoaded %d\n", srcFile, pObjName, pIf->ifIsLoaded);

	if(pIf->ifIsLoaded == 1) {
		ABORT_FINALIZE(RS_RET_OK); /* we are already set */
	}
	if(pIf->ifIsLoaded == 2) {
		ABORT_FINALIZE(RS_RET_LOAD_ERROR); /* we had a load error and can not continue */
	}

	/* we must be careful that we do not enter in infinite loop if an error occurs during
	 * loading a module. ModLoad emits an error message in such cases and that potentially
	 * can trigger the same code here. So we initially set the module state to "load error"
	 * and set it to "fully initialized" when the load succeeded. It's a bit hackish, but
	 * looks like a good solution. -- rgerhards, 2008-03-07
	 */
	pIf->ifIsLoaded = 2;

	CHKiRet(rsCStrConstructFromszStr(&pStr, pObjName));
	iRet = FindObjInfo(pStr, &pObjInfo);
	if(iRet == RS_RET_NOT_FOUND) {
		/* in this case, we need to see if we can dynamically load the object */
		if(pObjFile == NULL) {
			FINALIZE; /* no chance, we have lost... */
		} else {
			CHKiRet(module.Load(pObjFile));
			/* NOW, we must find it or we have a problem... */
			CHKiRet(FindObjInfo(pStr, &pObjInfo));
		}
	} else if(iRet != RS_RET_OK) {
		FINALIZE; /* give up */
	}

	/* if we reach this point, we have a valid pObjInfo */
	//if(pObjInfo->pModInfo != NULL) { /* NULL means core module */
	if(pObjFile != NULL) { /* NULL means core module */
		module.Use(srcFile, pObjInfo->pModInfo); /* increase refcount */
	}

	CHKiRet(pObjInfo->QueryIF(pIf));
	pIf->ifIsLoaded = 1; /* we are happy */

finalize_it:
	if(pStr != NULL)
		rsCStrDestruct(&pStr);

	RETiRet;
}


/* This function shall be called when a caller is done with an object. Its primary
 * purpose is to keep the reference count correct, which is highly important for
 * modules residing in loadable modules.
 * rgerhards, 2008-03-10
 */
static rsRetVal
ReleaseObj(char *srcFile, uchar *pObjName, uchar *pObjFile, interface_t *pIf)
{
	DEFiRet;
	cstr_t *pStr = NULL;
	objInfo_t *pObjInfo;


	dbgprintf("source file %s requests object '%s', ifIsLoaded %d\n", srcFile, pObjName, pIf->ifIsLoaded);

	if(pObjFile == NULL)
		FINALIZE; /* if it is not a lodable module, we do not need to do anything... */

	if(pIf->ifIsLoaded == 0) {
		ABORT_FINALIZE(RS_RET_OK); /* we are already set */ /* TODO: flag an error? */
	}
	if(pIf->ifIsLoaded == 2) {
		pIf->ifIsLoaded = 0; /* clean up */
		ABORT_FINALIZE(RS_RET_OK); /* we had a load error and can not continue */
	}

	CHKiRet(rsCStrConstructFromszStr(&pStr, pObjName));
	CHKiRet(FindObjInfo(pStr, &pObjInfo));

	/* if we reach this point, we have a valid pObjInfo */
	//if(pObjInfo->pModInfo != NULL) { /* NULL means core module */
	module.Release(srcFile, &pObjInfo->pModInfo); /* decrease refcount */

	pIf->ifIsLoaded = 0; /* indicated "no longer valid" */

finalize_it:
	if(pStr != NULL)
		rsCStrDestruct(&pStr);

	RETiRet;
}


/* queryInterface function
 * rgerhards, 2008-02-29
 */
BEGINobjQueryInterface(obj)
CODESTARTobjQueryInterface(obj)
	if(pIf->ifVersion != objCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->UseObj = UseObj;
	pIf->ReleaseObj = ReleaseObj;
	pIf->InfoConstruct = InfoConstruct;
	pIf->DestructObjSelf = DestructObjSelf;
	pIf->BeginSerializePropBag = BeginSerializePropBag;
	pIf->InfoSetMethod = InfoSetMethod;
	pIf->BeginSerialize = BeginSerialize;
	pIf->SerializeProp = SerializeProp;
	pIf->EndSerialize = EndSerialize;
	pIf->RegisterObj = RegisterObj;
	pIf->UnregisterObj = UnregisterObj;
	pIf->Deserialize = Deserialize;
	pIf->DeserializePropBag = DeserializePropBag;
	pIf->SetName = SetName;
	pIf->GetName = GetName;
finalize_it:
ENDobjQueryInterface(obj)


/* This function returns a pointer to our own interface. It is used as the
 * hook that every object (including dynamically loaded ones) can use to
 * obtain a pointer to our interface which than can be used to obtain
 * pointers to any other interface in the system. This function must be
 * externally visible because of its special nature.
 * rgerhards, 2008-02-29 [nice - will have that date the next time in 4 years ;)]
 */
rsRetVal
objGetObjInterface(obj_if_t *pIf)
{
	DEFiRet;
	assert(pIf != NULL);
	objQueryInterface(pIf);
	RETiRet;
}


/* exit our class
 * rgerhards, 2008-03-11
 */
rsRetVal
objClassExit(void)
{
	DEFiRet;
	/* release objects we no longer need */
	objRelease(var, CORE_COMPONENT);
	objRelease(module, CORE_COMPONENT);
	objRelease(errmsg, CORE_COMPONENT);

	/* TODO: implement the class exits! */
#if 0
	errmsgClassInit(pModInfo);
	cfsyslineInit(pModInfo);
	varClassInit(pModInfo);
#endif
	moduleClassExit();
	RETiRet;
}


/* initialize our own class 
 * Please note that this also initializes those classes that we rely on.
 * Though this is a bit dirty, we need to do it - otherwise we can't get
 * around that bootstrap problem. We need to face the fact the the obj 
 * class is a little different from the rest of the system, as it provides
 * the core class loader functionality.
 * rgerhards, 2008-02-29
 */
rsRetVal
objClassInit(modInfo_t *pModInfo)
{
	DEFiRet;
	int i;
	
	/* first, initialize the object system itself. This must be done
	 * before any other object is created.
	 */
	for(i = 0 ; i < OBJ_NUM_IDS ; ++i) {
		arrObjInfo[i] = NULL;
	}

	/* request objects we use */
	CHKiRet(objGetObjInterface(&obj)); /* get ourselves ;) */

	/* init classes we use (limit to as few as possible!) */
	CHKiRet(errmsgClassInit(pModInfo));
	CHKiRet(cfsyslineInit());
	CHKiRet(varClassInit(pModInfo));
	CHKiRet(moduleClassInit(pModInfo));
	CHKiRet(objUse(var, CORE_COMPONENT));
	CHKiRet(objUse(module, CORE_COMPONENT));
	CHKiRet(objUse(errmsg, CORE_COMPONENT));

finalize_it:
	RETiRet;
}

/* vi:set ai:
 */
