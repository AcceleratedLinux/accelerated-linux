///
///	@file 	http.cpp
/// @brief 	Application level HTTP control class
///
///	One instance of the Http class is created per application.
///
////////////////////////////////////////////////////////////////////////////////
//
//	Copyright (c) Mbedthis Software LLC, 2003-2004. All Rights Reserved.
//	The latest version of this code is available at http://www.mbedthis.com
//
//	This software is open source; you can redistribute it and/or modify it 
//	under the terms of the GNU General Public License as published by the 
//	Free Software Foundation; either version 2 of the License, or (at your 
//	option) any later version.
//
//	This program is distributed WITHOUT ANY WARRANTY; without even the 
//	implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
//	See the GNU General Public License for more details at:
//	http://www.mbedthis.com/downloads/gplLicense.html
//	
//	This General Public License does NOT permit incorporating this software 
//	into proprietary programs. If you are unable to comply with the GPL, a 
//	commercial license for this software and support services are available
//	from Mbedthis Software at http://www.mbedthis.com
//
////////////////////////////////// Includes ////////////////////////////////////

#include	"http.h"

////////////////////////////// Forward Declarations ////////////////////////////

static int		allDigits(char *s);
static MaHttp	*maHttp;

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// Http //////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
 
MaHttp::MaHttp()
{
	mprAssert(maHttp == 0);

#if BLD_FEATURE_MULTITHREAD
	mutex = new MprMutex();
#endif

#if LINUX || MACOSX
	struct passwd	*pp;
	struct group	*gp;

	uid = getuid();
	if ((pp = getpwuid(uid)) == 0) {
		mprError(MPR_L, MPR_USER, "Can't read user credentials: %d", uid);
		user = 0;
	} else {
		user = mprStrdup(pp->pw_name);
	}
	
	gid = getgid();
	if ((gp = getgrgid(gid)) == 0) {
		mprError(MPR_L, MPR_USER, "Can't read group credentials: %d", gid);
		group = 0;
	} else {
		group = mprStrdup(gp->gr_name);
	}
#else
	uid = gid = -1;
	group = user = 0;
#endif
	maHttp = this;
}

////////////////////////////////////////////////////////////////////////////////

MaHttp::~MaHttp()
{
	unloadModules();
	if (user) {
		mprFree(user);
	}
	if (group) {
		mprFree(group);
	}
#if BLD_FEATURE_MULTITHREAD
	delete mutex;
#endif
}

////////////////////////////////////////////////////////////////////////////////

void MaHttp::insertServer(MaServer *sp)
{
	servers.insert(sp);
}

////////////////////////////////////////////////////////////////////////////////

MaServer *MaHttp::findServer(char *name)
{
	MaServer	*sp;

	sp = (MaServer*) servers.getFirst();
	while (sp) {
		if (strcmp(sp->getName(), name) == 0) {
			return sp;
		}
		sp = (MaServer*) servers.getNext(sp);
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

void MaHttp::insertHandlerService(MaHandlerService *hs)
{
	handlerServices.insert(hs);
}

////////////////////////////////////////////////////////////////////////////////

void MaHttp::removeHandlerService(MaHandlerService *hs)
{
	handlerServices.remove(hs);
}

////////////////////////////////////////////////////////////////////////////////

void MaHttp::startHandlers()
{
	MaHandlerService	*hs;

	hs = (MaHandlerService*) handlerServices.getFirst();
	while (hs) {
		mprLog(MPR_INFO, "Starting handler %s\n", hs->getName());
		hs->start();
		hs = (MaHandlerService*) handlerServices.getNext(hs);
	}
}

////////////////////////////////////////////////////////////////////////////////

void MaHttp::stopHandlers()
{
	MaHandlerService	*hs;

	hs = (MaHandlerService*) handlerServices.getFirst();
	while (hs) {
		hs->stop();
		hs = (MaHandlerService*) handlerServices.getNext(hs);
	}
}

////////////////////////////////////////////////////////////////////////////////

MaHandlerService* MaHttp::lookupHandlerService(char *name)
{
	MaHandlerService	*hs;

	hs = (MaHandlerService*) handlerServices.getFirst();
	while (hs) {
		if (strcmp(hs->getName(), name) == 0) {
			return hs;
		}
		hs = (MaHandlerService*) handlerServices.getNext(hs);
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

int MaHttp::start()
{
	MaServer	*sp;

	startModules();
	startHandlers();

	//
	//	Start servers (and hosts)
	//
	sp = (MaServer*) servers.getFirst();
	while (sp) {
		if (sp->start() < 0) {
			return MPR_ERR_CANT_INITIALIZE;
		}
		sp = (MaServer*) servers.getNext(sp);
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

int MaHttp::stop()
{
	MaServer	*sp;

	sp = (MaServer*) servers.getFirst();
	while (sp) {
		sp->stop();
		sp = (MaServer*) servers.getNext(sp);
	}
	
	stopHandlers();
	stopModules();
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

int MaHttp::setUser(char *newUser)
{
#if LINUX || MACOSX
	struct passwd	*pp;

    if (allDigits(newUser)) {
		uid = atoi(newUser);
		if ((pp = getpwuid(uid)) == 0) {
			mprError(MPR_L, MPR_USER, "Bad user id: %d", uid);
			return MPR_ERR_CANT_ACCESS;
		}
		newUser = pp->pw_name;

	} else {
		if ((pp = getpwnam(newUser)) == 0) {
			mprError(MPR_L, MPR_USER, "Bad user name: %s", newUser);
			return MPR_ERR_CANT_ACCESS;
		}
		uid = pp->pw_uid;
    }
#endif
	if (user) {
		mprFree(user);
	}
	user = mprStrdup(newUser);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

int MaHttp::setGroup(char *newGroup)
{
#if LINUX || MACOSX
	struct group	*gp;

    if (allDigits(newGroup)) {
		gid = atoi(newGroup);
		if ((gp = getgrgid(gid)) == 0) {
			mprError(MPR_L, MPR_USER, "Bad group id: %d", gid);
			return MPR_ERR_CANT_ACCESS;
		}
		newGroup = gp->gr_name;

	} else {
		if ((gp = getgrnam(newGroup)) == 0) {
			mprError(MPR_L, MPR_USER, "Bad group name: %s", newGroup);
			return MPR_ERR_CANT_ACCESS;
		}
		gid = gp->gr_gid;
    }
#endif
	if (group) {
		mprFree(group);
	}
	group = mprStrdup(newGroup);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

char *MaHttp::getUser()
{
	return user;
}

////////////////////////////////////////////////////////////////////////////////

char *MaHttp::getGroup()
{
	return group;
}

////////////////////////////////////////////////////////////////////////////////

int MaHttp::changeUser()
{
#if LINUX || MACOSX
    if (uid >= 0 && (setuid(uid)) != 0) {
		mprError(MPR_L, MPR_USER, "Can't change user to: %s: %d\n"
			"WARNING: This is a major security exposure", user, uid);
    }
#endif
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

int MaHttp::changeGroup()
{
#if LINUX || MACOSX
    if (gid >= 0 && setgid(gid) != 0) {
		mprError(MPR_L, MPR_USER, "Can't change group to %s: %d\n"
			"WARNING: This is a major security exposure", group, gid);
    }
#endif
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

void MaHttp::insertModule(MaModule *mp)
{
	modules.insert(mp);
#if UNUSED
	mp->setLoaded(0);
#endif
}

////////////////////////////////////////////////////////////////////////////////

void MaHttp::removeModule(MaModule *mp)
{
	modules.remove(mp);
}

////////////////////////////////////////////////////////////////////////////////

MaModule *MaHttp::findModule(char *name)
{
	MaModule	*mp;

	lock();
	mp = (MaModule*) modules.getFirst();
	while (mp) {
		if (strcmp(mp->name, name) == 0) {
			unlock();
			return mp;
		}
		mp = (MaModule*) modules.getNext(mp);
	}
	unlock();
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

void MaHttp::startModules()
{
	MaModule	*mp;

	lock();
	mp = (MaModule*) modules.getFirst();
	while (mp) {
		mprLog(MPR_INFO, "Starting module %s\n", mp->getName());
		mp->start();
		mp = (MaModule*) modules.getNext(mp);
	}
	unlock();
}

////////////////////////////////////////////////////////////////////////////////

void MaHttp::stopModules()
{
	MaModule	*mp;

	lock();
	mp = (MaModule*) modules.getFirst();
	while (mp) {
		mp->stop();
		mp = (MaModule*) modules.getNext(mp);
	}
	unlock();
}

////////////////////////////////////////////////////////////////////////////////

void MaHttp::unloadModules()
{
	MaModule	*mp, *nextMp;
	void		*handle;

	mp = (MaModule*) modules.getFirst();
	while (mp) {
		nextMp = (MaModule*) modules.getNext(mp);
		handle = mp->getHandle();
#if BLD_FEATURE_DLL
		if (mp->flags & MPR_MODULE_LOADED && mp->flags & MPR_MODULE_DLL) {
			delete mp;
			mprGetMpr()->unloadDll(handle);
		} else {
			delete mp;
		}
#endif
		mp = nextMp;
	}
	mprLog(MPR_CONFIG, "Unloaded all modules\n");
}

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// MaLimits ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

MaLimits::MaLimits()
{
	maxBody = MPR_HTTP_MAX_BODY;
	maxResponseBody = MPR_HTTP_MAX_RESPONSE_BODY;
	maxNumHeader = MPR_HTTP_MAX_NUM_HEADER;
	maxHeader = MPR_HTTP_MAX_HEADER;
	maxFirstLine = MPR_HTTP_MAX_FIRST_LINE;
	maxUrl = MPR_HTTP_MAX_URL;
	maxThreads = MPR_HTTP_DEFAULT_MAX_THREADS;
	minThreads = 0;

	//
	//	Zero means use O/S defaults
	//
	sendBufferSize = 0;
	threadStackSize = 0;
}

////////////////////////////////////////////////////////////////////////////////

MaLimits::~MaLimits()
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

MaHttp *maGetHttp()
{
	return maHttp;
}

////////////////////////////////////////////////////////////////////////////////

static int allDigits(char *s)
{
    return strspn(s, "1234567890") == strlen(s);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#if UNUSED
#if WIN
BOOL WINAPI maDllMain(HANDLE hinst, DWORD reason, LPVOID preserved)
{
	switch (reason)
	{
		case DLL_PROCESS_ATTACH:
			return TRUE;

		case DLL_PROCESS_DETACH:
			return TRUE;
	}
	return FALSE;
}

#endif	//	WIN
#endif
////////////////////////////////////////////////////////////////////////////////

//
// Local variables:
// tab-width: 4
// c-basic-offset: 4
// End:
// vim:tw=78
// vim600: sw=4 ts=4 fdm=marker
// vim<600: sw=4 ts=4
//
