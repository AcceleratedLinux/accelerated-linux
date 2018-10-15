/* modules.c
 * This is the implementation of syslogd modules object.
 * This object handles plug-ins and build-in modules of all kind.
 *
 * Modules are reference-counted. Anyone who access a module must call
 * Use() before any function is accessed and Release() when he is done.
 * When the reference count reaches 0, rsyslog unloads the module (that
 * may be changed in the future to cache modules). Rsyslog does NOT
 * unload modules with a reference count > 0, even if the unload
 * method is called!
 *
 * File begun on 2007-07-22 by RGerhards
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
#include <time.h>
#include <assert.h>
#include <errno.h>

#include <dlfcn.h> /* TODO: replace this with the libtools equivalent! */

#include <unistd.h>
#include <sys/file.h>

#include "syslogd.h"
#include "cfsysline.h"
#include "modules.h"
#include "errmsg.h"

/* static data */
DEFobjStaticHelpers
DEFobjCurrIf(errmsg)

static modInfo_t *pLoadedModules = NULL;	/* list of currently-loaded modules */
static modInfo_t *pLoadedModulesLast = NULL;	/* tail-pointer */

/* config settings */
uchar	*pModDir = NULL; /* read-only after startup */


#ifdef DEBUG
/* we add some home-grown support to track our users (and detect who does not free us). In
 * the long term, this should probably be migrated into debug.c (TODO). -- rgerhards, 2008-03-11
 */

/* add a user to the current list of users (always at the root) */
static void
modUsrAdd(modInfo_t *pThis, char *pszUsr)
{
	modUsr_t *pUsr;

	BEGINfunc
	if((pUsr = calloc(1, sizeof(modUsr_t))) == NULL)
		goto finalize_it;

	if((pUsr->pszFile = strdup(pszUsr)) == NULL) {
		free(pUsr);
		goto finalize_it;
	}

	if(pThis->pModUsrRoot != NULL) {
		pUsr->pNext = pThis->pModUsrRoot;
	}
	pThis->pModUsrRoot = pUsr;

finalize_it:
	ENDfunc;
}


/* remove a user from the current user list
 * rgerhards, 2008-03-11
 */
static void
modUsrDel(modInfo_t *pThis, char *pszUsr)
{
	modUsr_t *pUsr;
	modUsr_t *pPrev = NULL;

	for(pUsr = pThis->pModUsrRoot ; pUsr != NULL ; pUsr = pUsr->pNext) {
		if(!strcmp(pUsr->pszFile, pszUsr))
			break;
		else
			pPrev = pUsr;
	}

	if(pUsr == NULL) {
		dbgprintf("oops - tried to delete user %s from module %s and it wasn't registered as one...\n",
			  pszUsr, pThis->pszName);
	} else {
		if(pPrev == NULL) {
			/* This was at the root! */
			pThis->pModUsrRoot = pUsr->pNext;
		} else {
			pPrev->pNext = pUsr->pNext;
		}
		/* free ressources */
		free(pUsr->pszFile);
		free(pUsr);
		pUsr = NULL; /* just to make sure... */
	}
}


/* print a short list all all source files using the module in question
 * rgerhards, 2008-03-11
 */
static void
modUsrPrint(modInfo_t *pThis)
{
	modUsr_t *pUsr;

	for(pUsr = pThis->pModUsrRoot ; pUsr != NULL ; pUsr = pUsr->pNext) {
		dbgprintf("\tmodule %s is currently in use by file %s\n",
			  pThis->pszName, pUsr->pszFile);
	}
}


/* print all loaded modules and who is accessing them. This is primarily intended
 * to be called at end of run to detect "module leaks" and who is causing them.
 * rgerhards, 2008-03-11
 */
//static void
void
modUsrPrintAll(void)
{
	modInfo_t *pMod;

	BEGINfunc
	for(pMod = pLoadedModules ; pMod != NULL ; pMod = pMod->pNext) {
		dbgprintf("printing users of loadable module %s, refcount %u, ptr %p, type %d\n", pMod->pszName, pMod->uRefCnt, pMod, pMod->eType);
		modUsrPrint(pMod);
	}
	ENDfunc
}

#endif /* #ifdef DEBUG */


/* Construct a new module object
 */
static rsRetVal moduleConstruct(modInfo_t **pThis)
{
	modInfo_t *pNew;

	if((pNew = calloc(1, sizeof(modInfo_t))) == NULL)
		return RS_RET_OUT_OF_MEMORY;

	/* OK, we got the element, now initialize members that should
	 * not be zero-filled.
	 */

	*pThis = pNew;
	return RS_RET_OK;
}


/* Destructs a module object. The object must not be linked to the
 * linked list of modules. Please note that all other dependencies on this
 * modules must have been removed before (e.g. CfSysLineHandlers!)
 */
static void moduleDestruct(modInfo_t *pThis)
{
	assert(pThis != NULL);
	if(pThis->pszName != NULL)
		free(pThis->pszName);
	if(pThis->pModHdlr != NULL) {
#	ifdef	VALGRIND
#		warning "dlclose disabled for valgrind"
#	else
		dlclose(pThis->pModHdlr);
#	endif
	}

	free(pThis);
}


/* The following function is the queryEntryPoint for host-based entry points.
 * Modules may call it to get access to core interface functions. Please note
 * that utility functions can be accessed via shared libraries - at least this
 * is my current shool of thinking.
 * Please note that the implementation as a query interface allows to take
 * care of plug-in interface version differences. -- rgerhards, 2007-07-31
 */
static rsRetVal queryHostEtryPt(uchar *name, rsRetVal (**pEtryPoint)())
{
	DEFiRet;

	if((name == NULL) || (pEtryPoint == NULL))
		return RS_RET_PARAM_ERROR;

	if(!strcmp((char*) name, "regCfSysLineHdlr")) {
		*pEtryPoint = regCfSysLineHdlr;
	} else if(!strcmp((char*) name, "objGetObjInterface")) {
		*pEtryPoint = objGetObjInterface;
	} else {
		*pEtryPoint = NULL; /* to  be on the safe side */
		ABORT_FINALIZE(RS_RET_ENTRY_POINT_NOT_FOUND);
	}

finalize_it:
	RETiRet;
}


/* get the name of a module
 */
static uchar *modGetName(modInfo_t *pThis)
{
	return((pThis->pszName == NULL) ? (uchar*) "" : pThis->pszName);
}


/* get the state-name of a module. The state name is its name
 * together with a short description of the module state (which
 * is pulled from the module itself.
 * rgerhards, 2007-07-24
 * TODO: the actual state name is not yet pulled
 */
static uchar *modGetStateName(modInfo_t *pThis)
{
	return(modGetName(pThis));
}


/* Add a module to the loaded module linked list
 */
static inline void
addModToList(modInfo_t *pThis)
{
	assert(pThis != NULL);

	if(pLoadedModules == NULL) {
		pLoadedModules = pLoadedModulesLast = pThis;
	} else {
		/* there already exist entries */
		pThis->pPrev = pLoadedModulesLast;
		pLoadedModulesLast->pNext = pThis;
		pLoadedModulesLast = pThis;
	}
}


/* Get the next module pointer - this is used to traverse the list.
 * The function returns the next pointer or NULL, if there is no next one.
 * The last object must be provided to the function. If NULL is provided,
 * it starts at the root of the list. Even in this case, NULL may be 
 * returned - then, the list is empty.
 * rgerhards, 2007-07-23
 */
static modInfo_t *GetNxt(modInfo_t *pThis)
{
	modInfo_t *pNew;

	if(pThis == NULL)
		pNew = pLoadedModules;
	else
		pNew = pThis->pNext;

	return(pNew);
}


/* this function is like GetNxt(), but it returns pointers to
 * modules of specific type only. As we currently deal just with output modules,
 * it is a dummy, to be filled with real code later.
 * rgerhards, 2007-07-24
 */
static modInfo_t *GetNxtType(modInfo_t *pThis, eModType_t rqtdType)
{
	modInfo_t *pMod = pThis;

	do {
		pMod = GetNxt(pMod);
	} while(!(pMod == NULL || pMod->eType == rqtdType)); /* warning: do ... while() */

	return pMod;
}


/* Prepare a module for unloading.
 * This is currently a dummy, to be filled when we have a plug-in
 * interface - rgerhards, 2007-08-09
 * rgerhards, 2007-11-21:
 * When this function is called, all instance-data must already have
 * been destroyed. In the case of output modules, this happens when the
 * rule set is being destroyed. When we implement other module types, we
 * need to think how we handle it there (and if we have any instance data).
 * rgerhards, 2008-03-10: reject unload request if the module has a reference
 * count > 0.
 */
static rsRetVal
modPrepareUnload(modInfo_t *pThis)
{
	DEFiRet;
	void *pModCookie;

	assert(pThis != NULL);

	if(pThis->uRefCnt > 0) {
		dbgprintf("rejecting unload of module '%s' because it has a refcount of %d\n",
			  pThis->pszName, pThis->uRefCnt);
		ABORT_FINALIZE(RS_RET_MODULE_STILL_REFERENCED);
	}

	CHKiRet(pThis->modGetID(&pModCookie));
	pThis->modExit(); /* tell the module to get ready for unload */
	CHKiRet(unregCfSysLineHdlrs4Owner(pModCookie));

finalize_it:
	RETiRet;
}


/* Add an already-loaded module to the module linked list. This function does
 * everything needed to fully initialize the module.
 */
static rsRetVal
doModInit(rsRetVal (*modInit)(int, int*, rsRetVal(**)(), rsRetVal(*)(), modInfo_t*), uchar *name, void *pModHdlr)
{
	DEFiRet;
	modInfo_t *pNew = NULL;
	rsRetVal (*modGetType)(eModType_t *pType);

	assert(modInit != NULL);

	if((iRet = moduleConstruct(&pNew)) != RS_RET_OK) {
		pNew = NULL;
		ABORT_FINALIZE(iRet);
	}

	CHKiRet((*modInit)(CURR_MOD_IF_VERSION, &pNew->iIFVers, &pNew->modQueryEtryPt, queryHostEtryPt, pNew));

	if(pNew->iIFVers != CURR_MOD_IF_VERSION) {
		ABORT_FINALIZE(RS_RET_MISSING_INTERFACE);
	}

	/* We now poll the module to see what type it is. We do this only once as this
	 * can never change in the lifetime of an module. -- rgerhards, 2007-12-14
	 */
	CHKiRet((*pNew->modQueryEtryPt)((uchar*)"getType", &modGetType));
	CHKiRet((iRet = (*modGetType)(&pNew->eType)) != RS_RET_OK);
	dbgprintf("module of type %d being loaded.\n", pNew->eType);
	
	/* OK, we know we can successfully work with the module. So we now fill the
	 * rest of the data elements. First we load the interfaces common to all
	 * module types.
	 */
	CHKiRet((*pNew->modQueryEtryPt)((uchar*)"modGetID", &pNew->modGetID));
	CHKiRet((*pNew->modQueryEtryPt)((uchar*)"modExit", &pNew->modExit));

	/* ... and now the module-specific interfaces */
	switch(pNew->eType) {
		case eMOD_IN:
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"runInput", &pNew->mod.im.runInput));
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"willRun", &pNew->mod.im.willRun));
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"afterRun", &pNew->mod.im.afterRun));
			break;
		case eMOD_OUT:
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"freeInstance", &pNew->freeInstance));
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"dbgPrintInstInfo", &pNew->dbgPrintInstInfo));
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"doAction", &pNew->mod.om.doAction));
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"parseSelectorAct", &pNew->mod.om.parseSelectorAct));
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"isCompatibleWithFeature", &pNew->isCompatibleWithFeature));
			CHKiRet((*pNew->modQueryEtryPt)((uchar*)"tryResume", &pNew->tryResume));
			break;
		case eMOD_LIB:
			break;
	}

	pNew->pszName = (uchar*) strdup((char*)name); /* we do not care if strdup() fails, we can accept that */
	pNew->pModHdlr = pModHdlr;
	/* TODO: take this from module */
	if(pModHdlr == NULL)
		pNew->eLinkType = eMOD_LINK_STATIC;
	else
		pNew->eLinkType = eMOD_LINK_DYNAMIC_LOADED;

	/* we initialized the structure, now let's add it to the linked list of modules */
	addModToList(pNew);

finalize_it:
	if(iRet != RS_RET_OK) {
		if(pNew != NULL)
			moduleDestruct(pNew);
	}

	RETiRet;
}

/* Print loaded modules. This is more or less a 
 * debug or test aid, but anyhow I think it's worth it...
 * This only works if the dbgprintf() subsystem is initialized.
 * TODO: update for new input modules!
 */
static void modPrintList(void)
{
	modInfo_t *pMod;

	pMod = GetNxt(NULL);
	while(pMod != NULL) {
		dbgprintf("Loaded Module: Name='%s', IFVersion=%d, ",
			(char*) modGetName(pMod), pMod->iIFVers);
		dbgprintf("type=");
		switch(pMod->eType) {
		case eMOD_OUT:
			dbgprintf("output");
			break;
		case eMOD_IN:
			dbgprintf("input");
			break;
		case eMOD_LIB:
			dbgprintf("library");
			break;
		}
		dbgprintf(" module.\n");
		dbgprintf("Entry points:\n");
		dbgprintf("\tqueryEtryPt:        0x%lx\n", (unsigned long) pMod->modQueryEtryPt);
		dbgprintf("\tdoAction:           0x%lx\n", (unsigned long) pMod->mod.om.doAction);
		dbgprintf("\tparseSelectorAct:   0x%lx\n", (unsigned long) pMod->mod.om.parseSelectorAct);
		dbgprintf("\tdbgPrintInstInfo:   0x%lx\n", (unsigned long) pMod->dbgPrintInstInfo);
		dbgprintf("\tfreeInstance:       0x%lx\n", (unsigned long) pMod->freeInstance);
		dbgprintf("\n");
		pMod = GetNxt(pMod); /* done, go next */
	}
}


/* unlink and destroy a module. The caller must provide a pointer to the module
 * itself as well as one to its immediate predecessor.
 * rgerhards, 2008-02-26
 */
static rsRetVal
modUnlinkAndDestroy(modInfo_t **ppThis)
{
	DEFiRet;
	modInfo_t *pThis;

	assert(ppThis != NULL);
	pThis = *ppThis;
	assert(pThis != NULL);

	/* first check if we are permitted to unload */
	if(pThis->eType == eMOD_LIB) {
		if(pThis->uRefCnt > 0) {
			dbgprintf("module %s NOT unloaded because it still has a refcount of %u\n",
				  pThis->pszName, pThis->uRefCnt);
#			ifdef DEBUG
			//modUsrPrintAll();
#			endif
			ABORT_FINALIZE(RS_RET_MODULE_STILL_REFERENCED);
		}
	}

	/* we need to unlink the module before we can destruct it -- rgerhards, 2008-02-26 */
	if(pThis->pPrev == NULL) {
		/* module is root, so we need to set a new root */
		pLoadedModules = pThis->pNext;
	} else {
		pThis->pPrev->pNext = pThis->pNext;
	}

	if(pThis->pNext == NULL) {
		pLoadedModulesLast = pThis->pPrev;
	} else {
		pThis->pNext->pPrev = pThis->pPrev;
	}

	/* finally, we are ready for the module to go away... */
	dbgprintf("Unloading module %s\n", modGetName(pThis));
	CHKiRet(modPrepareUnload(pThis));
	*ppThis = pThis->pNext;

	moduleDestruct(pThis);

finalize_it:
	RETiRet;
}


/* unload all loaded modules of a specific type (use eMOD_ALL if you want to
 * unload all module types). The unload happens only if the module is no longer
 * referenced. So some modules may survive this call.
 * rgerhards, 2008-03-11
 */
static rsRetVal
modUnloadAndDestructAll(eModLinkType_t modLinkTypesToUnload)
{
	DEFiRet;
	modInfo_t *pModCurr; /* module currently being processed */

	pModCurr = GetNxt(NULL);
	while(pModCurr != NULL) {
		if(modLinkTypesToUnload == eMOD_LINK_ALL || pModCurr->eLinkType == modLinkTypesToUnload) {
			if(modUnlinkAndDestroy(&pModCurr) == RS_RET_MODULE_STILL_REFERENCED) {
				pModCurr = GetNxt(pModCurr);
			}
			/* Note: if the module was successfully unloaded, it has updated the
			 * pModCurr pointer to the next module. So we do NOT need to advance
			 * to the next module on successful unload.
			 */
		} else {
			pModCurr = GetNxt(pModCurr);
		}
	}

#	ifdef DEBUG
		if(pLoadedModules != NULL) {
			dbgprintf("modules still loaded after module.UnloadAndDestructAll:\n");
			modUsrPrintAll();
		}
#	endif

	RETiRet;
}


/* load a module and initialize it, based on doModLoad() from conf.c
 * rgerhards, 2008-03-05
 * varmojfekoj added support for dynamically loadable modules on 2007-08-13
 * rgerhards, 2007-09-25: please note that the non-threadsafe function dlerror() is
 * called below. This is ok because modules are currently only loaded during
 * configuration file processing, which is executed on a single thread. Should we
 * change that design at any stage (what is unlikely), we need to find a
 * replacement.
 */
static rsRetVal
Load(uchar *pModName)
{
	DEFiRet;
	
	size_t iPathLen, iModNameLen;
	uchar szPath[PATH_MAX];
	uchar *pModNameCmp;
	int bHasExtension;
        void *pModHdlr, *pModInit;
	modInfo_t *pModInfo;

	assert(pModName != NULL);
	dbgprintf("Requested to load module '%s'\n", pModName);

	iModNameLen = strlen((char *) pModName);
	if(iModNameLen > 3 && !strcmp((char *) pModName + iModNameLen - 3, ".so")) {
		iModNameLen -= 3;
		bHasExtension = TRUE;
	} else
		bHasExtension = FALSE;

	pModInfo = GetNxt(NULL);
	while(pModInfo != NULL) {
		if(!strncmp((char *) pModName, (char *) (pModNameCmp = modGetName(pModInfo)), iModNameLen) &&
		   (!*(pModNameCmp + iModNameLen) || !strcmp((char *) pModNameCmp + iModNameLen, ".so"))) {
			dbgprintf("Module '%s' already loaded\n", pModName);
			ABORT_FINALIZE(RS_RET_OK);
		}
		pModInfo = GetNxt(pModInfo);
	}

	/* now build our load module name */
	if(*pModName == '/') {
		*szPath = '\0';	/* we do not need to append the path - its already in the module name */
		iPathLen = 0;
	} else {
		*szPath = '\0';
		strncat((char *) szPath, (pModDir == NULL) ? _PATH_MODDIR : (char*) pModDir, sizeof(szPath) - 1);
		iPathLen = strlen((char*) szPath);
		if((szPath[iPathLen - 1] != '/')) {
			if((iPathLen <= sizeof(szPath) - 2)) {
				szPath[iPathLen++] = '/';
				szPath[iPathLen] = '\0';
			} else {
				errmsg.LogError(NO_ERRCODE, "could not load module '%s', path too long\n", pModName);
				ABORT_FINALIZE(RS_RET_MODULE_LOAD_ERR_PATHLEN);
			}
		}
	}

	/* ... add actual name ... */
	strncat((char *) szPath, (char *) pModName, sizeof(szPath) - iPathLen - 1);

	/* now see if we have an extension and, if not, append ".so" */
	if(!bHasExtension) {
		/* we do not have an extension and so need to add ".so"
		 * TODO: I guess this is highly importable, so we should change the
		 * algo over time... -- rgerhards, 2008-03-05
		 */
		/* ... so now add the extension */
		strncat((char *) szPath, ".so", sizeof(szPath) - strlen((char*) szPath) - 1);
		iPathLen += 3;
	}

	if(iPathLen + strlen((char*) pModName) >= sizeof(szPath)) {
		errmsg.LogError(NO_ERRCODE, "could not load module '%s', path too long\n", pModName);
		ABORT_FINALIZE(RS_RET_MODULE_LOAD_ERR_PATHLEN);
	}

	/* complete load path constructed, so ... GO! */
	dbgprintf("loading module '%s'\n", szPath);
	if(!(pModHdlr = dlopen((char *) szPath, RTLD_NOW))) {
		errmsg.LogError(NO_ERRCODE, "could not load module '%s', dlopen: %s\n", szPath, dlerror());
		ABORT_FINALIZE(RS_RET_MODULE_LOAD_ERR_DLOPEN);
	}
	if(!(pModInit = dlsym(pModHdlr, "modInit"))) {
		errmsg.LogError(NO_ERRCODE, "could not load module '%s', dlsym: %s\n", szPath, dlerror());
		dlclose(pModHdlr);
		ABORT_FINALIZE(RS_RET_MODULE_LOAD_ERR_NO_INIT);
	}
	if((iRet = doModInit(pModInit, (uchar*) pModName, pModHdlr)) != RS_RET_OK) {
		errmsg.LogError(NO_ERRCODE, "could not load module '%s', rsyslog error %d\n", szPath, iRet);
		dlclose(pModHdlr);
		ABORT_FINALIZE(RS_RET_MODULE_LOAD_ERR_INIT_FAILED);
	}

finalize_it:
	RETiRet;
}


/* set the default module load directory. A NULL value may be provided, in
 * which case any previous value is deleted but no new one set. The caller-provided
 * string is duplicated. If it needs to be freed, that's the caller's duty.
 * rgerhards, 2008-03-07
 */
static rsRetVal
SetModDir(uchar *pszModDir)
{
	DEFiRet;

	dbgprintf("setting default module load directory '%s'\n", pszModDir);
	if(pModDir != NULL) {
		free(pModDir);
	}

	pModDir = (uchar*) strdup((char*)pszModDir);

	RETiRet;
}


/* Reference-Counting object access: add 1 to the current reference count. Must be
 * called by anyone interested in using a module. -- rgerhards, 20080-03-10
 */
static rsRetVal
Use(char *srcFile, modInfo_t *pThis)
{
	DEFiRet;

	assert(pThis != NULL);
	pThis->uRefCnt++;
	dbgprintf("source file %s requested reference for module '%s', reference count now %u\n",
		  srcFile, pThis->pszName, pThis->uRefCnt);

#	ifdef DEBUG
	modUsrAdd(pThis, srcFile);
#	endif

	RETiRet;

}


/* Reference-Counting object access: subract one from the current refcount. Must
 * by called by anyone who no longer needs a module. If count reaches 0, the 
 * module is unloaded. -- rgerhards, 20080-03-10
 */
static rsRetVal
Release(char *srcFile, modInfo_t **ppThis)
{
	DEFiRet;
	modInfo_t *pThis;

	assert(ppThis != NULL);
	pThis = *ppThis;
	assert(pThis != NULL);
	if(pThis->uRefCnt == 0) {
		/* oops, we are already at 0? */
		dbgprintf("internal error: module '%s' already has a refcount of 0 (released by %s)!\n",
			  pThis->pszName, srcFile);
	} else {
		--pThis->uRefCnt;
		dbgprintf("file %s released module '%s', reference count now %u\n",
			  srcFile, pThis->pszName, pThis->uRefCnt);
#		ifdef DEBUG
		modUsrDel(pThis, srcFile);
		modUsrPrint(pThis);
#		endif
	}

	if(pThis->uRefCnt == 0) {
		/* we have a zero refcount, so we must unload the module */
		dbgprintf("module '%s' has zero reference count, unloading...\n", pThis->pszName);
		modUnlinkAndDestroy(&pThis);
		/* we must NOT do a *ppThis = NULL, because ppThis now points into freed memory!
		 * If in doubt, see obj.c::ReleaseObj() for how we are called.
		 */
	}

	RETiRet;

}


/* exit our class
 * rgerhards, 2008-03-11
 */
BEGINObjClassExit(module, OBJ_IS_LOADABLE_MODULE) /* CHANGE class also in END MACRO! */
CODESTARTObjClassExit(module)
	/* release objects we no longer need */
	objRelease(errmsg, CORE_COMPONENT);

#	ifdef DEBUG
	modUsrPrintAll(); /* debug aid - TODO: integrate with debug.c, at least the settings! */
#	endif
ENDObjClassExit(module)


/* queryInterface function
 * rgerhards, 2008-03-05
 */
BEGINobjQueryInterface(module)
CODESTARTobjQueryInterface(module)
	if(pIf->ifVersion != moduleCURR_IF_VERSION) { /* check for current version, increment on each change */
		ABORT_FINALIZE(RS_RET_INTERFACE_NOT_SUPPORTED);
	}

	/* ok, we have the right interface, so let's fill it
	 * Please note that we may also do some backwards-compatibility
	 * work here (if we can support an older interface version - that,
	 * of course, also affects the "if" above).
	 */
	pIf->GetNxt = GetNxt;
	pIf->GetNxtType = GetNxtType;
	pIf->GetName = modGetName;
	pIf->GetStateName = modGetStateName;
	pIf->PrintList = modPrintList;
	pIf->UnloadAndDestructAll = modUnloadAndDestructAll;
	pIf->doModInit = doModInit;
	pIf->SetModDir = SetModDir;
	pIf->Load = Load;
	pIf->Use = Use;
	pIf->Release = Release;
finalize_it:
ENDobjQueryInterface(module)


/* Initialize our class. Must be called as the very first method
 * before anything else is called inside this class.
 * rgerhards, 2008-03-05
 */
BEGINAbstractObjClassInit(module, 1, OBJ_IS_CORE_MODULE) /* class, version - CHANGE class also in END MACRO! */
	uchar *pModPath;

	/* use any module load path specified in the environment */
	if((pModPath = (uchar*) getenv("RSYSLOG_MODDIR")) != NULL) {
		SetModDir(pModPath);
	}

	/* request objects we use */
	CHKiRet(objUse(errmsg, CORE_COMPONENT));
ENDObjClassInit(module)

/* vi:set ai:
 */
