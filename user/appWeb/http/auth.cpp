///
///	@file 	auth.cpp
/// @brief 	Authorization abstraction
///
///	This modules provides the linkage between authorization handlers and the
///	main http server. Auth objects usually inherit their settings from 
///	enclosing Directory or Location directives in the configuration file.
///	In particular, the user and group stores are pointers to the master stores.
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
//
//	MOB -- this whole module requires locking when modifying the 
//	user/group tables
//
////////////////////////////////// Includes ////////////////////////////////////

#include	"http.h"

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// MaAuth ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

MaAuth::MaAuth()
{
	anyValidUser = 0;
	authType = MPR_HTTP_AUTH_UNKNOWN;
	order = MPR_HTTP_DENY_ALLOW;
	allowSpec = 0;
	denySpec = 0;
	flags = 0;
	qop = mprStrdup("");
	requiredRealm = 0;
	requiredGroups = 0;
	requiredUsers = 0;

	//
	//	In most cases, the Auth object inherits its group and user hash 
	//	indicies from a parent Auth object. 
	//
	groupHash = 0;
	userHash = 0;
}

////////////////////////////////////////////////////////////////////////////////

MaAuth::~MaAuth()
{
	release();
}

////////////////////////////////////////////////////////////////////////////////

void MaAuth::release()
{
	mprFree(allowSpec);
	mprFree(denySpec);
	mprFree(qop);
	mprFree(requiredRealm);
	mprFree(requiredGroups);
	mprFree(requiredUsers);

	//
	//	Only free if this object owned the indicies
	//
	if (flags & MPR_HTTP_AUTH_USER_HASH) {
		delete userHash;
	}
	if (flags & MPR_HTTP_AUTH_GROUP_HASH) {
		delete groupHash;
	}
}

////////////////////////////////////////////////////////////////////////////////

int MaAuth::addGroup(char *group, MaAcl acl, bool enabled)
{
	//
	//	Create the index on demand
	//
	if (groupHash == 0) {
		groupHash = new MprHashTable();
		flags |= MPR_HTTP_AUTH_GROUP_HASH;
	}
	if (groupHash->lookup(group)) {
		return MPR_ERR_ALREADY_EXISTS;
	}
	groupHash->insert(new MaGroup(group, acl, enabled));
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
//	NOTE: passwd must be already encrypted
//

int MaAuth::addUser(char *user, char *realm, char *passwd, bool enabled)
{
	char	key[MPR_HTTP_MAX_USER * 2];

	if (userHash == 0) {
		userHash = new MprHashTable();
		flags |= MPR_HTTP_AUTH_USER_HASH;
	}
	mprSprintf(key, sizeof(key), "%s:%s", user, realm);

	if (userHash->lookup(key)) {
		return MPR_ERR_ALREADY_EXISTS;
	}

	userHash->insert(new MaUser(user, realm, passwd, enabled));
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

int MaAuth::addUsersToGroup(char *group, char *users)
{
	MaGroup		*gp;
	char		*tok, ubuf[80], *userName;

	if (groupHash == 0 || (gp = (MaGroup*) groupHash->lookup(group)) == 0) {
		return MPR_ERR_CANT_ACCESS;
	}

	userName = mprGetWordTok(ubuf, sizeof(ubuf), users, " \t", &tok);
	while (userName) {
		//	Ignore already exists errors
		gp->addUser(userName);
		userName = mprGetWordTok(ubuf, sizeof(ubuf), 0, " \t", &tok);
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

int MaAuth::disableGroup(char *group)
{
	MaGroup 	*gp;

	if (groupHash == 0 || (gp = (MaGroup*) groupHash->lookup(group)) == 0) {
		return MPR_ERR_CANT_ACCESS;
	}
	gp->setEnabled(0);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

int MaAuth::disableUser(char *user, char *realm)
{
	MaUser 	*up;
	char	key[MPR_HTTP_MAX_USER * 2];

	mprSprintf(key, sizeof(key), "%s:%s", user, realm);

	if (userHash == 0 || (up = (MaUser*) userHash->lookup(key)) == 0) {
		return MPR_ERR_CANT_ACCESS;
	}
	up->setEnabled(0);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

int MaAuth::enableGroup(char *group)
{
	MaGroup 	*gp;

	if (groupHash == 0 || (gp = (MaGroup*) groupHash->lookup(group)) == 0) {
		return MPR_ERR_CANT_ACCESS;
	}
	gp->setEnabled(1);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

int MaAuth::enableUser(char *user, char *realm)
{
	MaUser 	*up;
	char	key[MPR_HTTP_MAX_USER * 2];

	mprSprintf(key, sizeof(key), "%s:%s", user, realm);

	if (userHash == 0 || (up = (MaUser*) userHash->lookup(user)) == 0) {
		return MPR_ERR_CANT_ACCESS;
	}
	up->setEnabled(1);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

MaAcl MaAuth::getAcl(char *group)
{
	MaGroup 	*gp;

	if (groupHash == 0 || (gp = (MaGroup*) groupHash->lookup(group)) == 0) {
		return MPR_ERR_CANT_ACCESS;
	}
	return gp->getAcl();
}

////////////////////////////////////////////////////////////////////////////////

char *MaAuth::getAllowSpec()
{
	return allowSpec;
}

////////////////////////////////////////////////////////////////////////////////

bool MaAuth::getAnyValidUser()
{
	return anyValidUser;
}

////////////////////////////////////////////////////////////////////////////////

char *MaAuth::getDenySpec()
{
	return denySpec;
}

////////////////////////////////////////////////////////////////////////////////

void MaAuth::getGroups(MprStringList *list)
{
	MaGroup		*gp;
	
	gp = (MaGroup*) groupHash->getFirst();
	while (gp) {
		list->insert(gp->getName());
		gp = (MaGroup*) groupHash->getNext(gp);
	}
}

////////////////////////////////////////////////////////////////////////////////

MaAuthOrder MaAuth::getOrder()
{
	return order;
}

////////////////////////////////////////////////////////////////////////////////

int MaAuth::getPassword(char *passBuf, int passLen, char *user, char *realm)
{
	MaUser		*up;
	char		key[MPR_HTTP_MAX_USER * 2];

	up = 0;
	mprSprintf(key, sizeof(key), "%s:%s", user, realm);
	if (userHash) {
		up = (MaUser*) userHash->lookup(key);
	}
	if (up == 0) {
		return MPR_ERR_NOT_FOUND;
	}
	mprStrcpy(passBuf, passLen, up->getPassword());
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

char *MaAuth::getRealm()
{
	return requiredRealm;
}

////////////////////////////////////////////////////////////////////////////////

char *MaAuth::getRequiredGroups()
{
	return requiredGroups;
}

////////////////////////////////////////////////////////////////////////////////

char *MaAuth::getRequiredUsers()
{
	return requiredUsers;
}

////////////////////////////////////////////////////////////////////////////////

char *MaAuth::getQop()
{
	return qop;
}

////////////////////////////////////////////////////////////////////////////////

MaAuthType MaAuth::getType()
{
	return authType;
}

////////////////////////////////////////////////////////////////////////////////

void MaAuth::getUsers(MprStringList *list)
{
	MaUser		*up;
	
	up = (MaUser*) userHash->getFirst();
	while (up) {
		list->insert(up->getName());
		up = (MaUser*) userHash->getNext(up);
	}
}

////////////////////////////////////////////////////////////////////////////////

void MaAuth::inherit(MaAuth *master)
{
	release();
	anyValidUser = master->anyValidUser;
	order = master->order;
	allowSpec = mprStrdup(master->allowSpec);
	denySpec = mprStrdup(master->denySpec);
	qop = mprStrdup(master->qop);
	requiredRealm = mprStrdup(master->requiredRealm);
	requiredGroups = mprStrdup(master->requiredGroups);
	requiredUsers = mprStrdup(master->requiredUsers);

	//
	//	Point to the master user/group lookup hash tables
	//
	userHash = master->userHash;
	groupHash = master->groupHash;
	flags = 0;
};

////////////////////////////////////////////////////////////////////////////////

void MaAuth::inheritUserGroup(MaAuth *master)
{
	//
	//	Point to the master user/group lookup hash tables
	//
	userHash = master->userHash;
	groupHash = master->groupHash;
};

////////////////////////////////////////////////////////////////////////////////

bool MaAuth::isAuthRequired()
{
	return (flags & MPR_HTTP_AUTH_REQUIRED) != 0;
}

////////////////////////////////////////////////////////////////////////////////

bool MaAuth::isGroupEnabled(char *group)
{
	MaGroup 	*gp;

	if (groupHash == 0 || (gp = (MaGroup*) groupHash->lookup(group)) == 0) {
		return 0;
	}
	return gp->getEnabled();
}

////////////////////////////////////////////////////////////////////////////////

bool MaAuth::isUserEnabled(char *user, char *realm)
{
	MaUser 	*up;
	char	key[MPR_HTTP_MAX_USER * 2];

	mprSprintf(key, sizeof(key), "%s:%s", user, realm);

	if (userHash == 0 || (up = (MaUser*) userHash->lookup(key)) == 0) {
		return 0;
	}
	return up->getEnabled();
}

////////////////////////////////////////////////////////////////////////////////
//
//	Determine if this user is specified as being eligible for this realm
//	We examine the requiredUsers and requiredGroups.
//

bool MaAuth::isUserValid(char *userInQuestion, char *realm)
{
	MaGroup			*gp;
	MprStringList	*userList;
	MprStringData	*usp;
	char			ubuf[80], gbuf[80], key[MPR_HTTP_MAX_USER * 2];
	char			*userName, *groupName, *tok, *gtok;

	if (anyValidUser) {
		mprSprintf(key, sizeof(key), "%s:%s", userInQuestion, realm);
		if (userHash == 0) {
			return 0;
		}
		return userHash->lookup(key) != 0;
	}

	if (requiredUsers) {
		userName = mprGetWordTok(ubuf, sizeof(ubuf), requiredUsers, 
			" \t", &tok);
		while (userName) {
			if (strcmp(userInQuestion, userName) == 0) {
				return 1;
			}
			userName = mprGetWordTok(ubuf, sizeof(ubuf), 0, " \t", &tok);
		}
	}

	if (requiredGroups) {
		groupName = mprGetWordTok(gbuf, sizeof(gbuf), requiredGroups, 
			" \t", &gtok);
		//
		//	For each group, check all the users in the group.
		//
		while (groupName) {
			if (groupHash == 0) {
				gp = 0;
			} else {
				gp = (MaGroup*) groupHash->lookup(groupName);
			}
			if (gp == 0) {
				mprError(MPR_L, MPR_LOG, "Can't find group %s", groupName);
				groupName = mprGetWordTok(gbuf, sizeof(gbuf), 0, " \t", &gtok);
				continue;
			}

			userList = gp->getUsers();
			usp = (MprStringData*) userList->getFirst();
			while (usp) {
				if (strcmp(userInQuestion, usp->getValue()) == 0) {
					return 1;
				}
				usp = (MprStringData*) userList->getNext(usp);
			}
			groupName = mprGetWordTok(gbuf, sizeof(gbuf), 0, " \t", &gtok);
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
//	ACLs are simple hex numbers
//
MaAcl MaAuth::parseAcl(char *aclStr)
{
	MaAcl	acl = 0;
	int		c;

	if (aclStr) {
		if (aclStr[0] == '0' && aclStr[1] == 'x') {
			aclStr += 2;
		}
		for (; isxdigit(*aclStr); aclStr++) {
			c = (int) tolower(*aclStr);
			if ('0' <= c && c <= '9') {
				acl = (acl * 16) + c - '0';
			} else {
				acl = (acl * 16) + c - 'a' + 10;
			}
		}
	}
	return acl;
}

////////////////////////////////////////////////////////////////////////////////

int MaAuth::removeGroup(char *group)
{
	if (groupHash == 0 || !groupHash->lookup(group)) {
		return MPR_ERR_CANT_ACCESS;
	}
	groupHash->remove(group);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

int MaAuth::removeUser(char *user, char *realm)
{
	char	key[MPR_HTTP_MAX_USER * 2];

	mprSprintf(key, sizeof(key), "%s:%s", user, realm);
	if (userHash == 0 || !userHash->lookup(key)) {
		return MPR_ERR_CANT_ACCESS;
	}
	userHash->remove(key);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

int MaAuth::removeUsersFromGroup(char *group, char *users)
{
	MaGroup		*gp;
	char		ubuf[80], *userName, *tok;

	if (groupHash == 0 || (gp = (MaGroup*) groupHash->lookup(group)) == 0) {
		return MPR_ERR_CANT_ACCESS;
	}

	userName = mprGetWordTok(ubuf, sizeof(ubuf), users, " \t", &tok);
	while (userName) {
		gp->removeUser(userName);
		userName = mprGetWordTok(ubuf, sizeof(ubuf), 0, " \t", &tok);
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

void MaAuth::reset()
{
	//	MOB -- locking required
	delete groupHash;
	groupHash = 0;

	delete userHash;
	userHash = 0;
}

////////////////////////////////////////////////////////////////////////////////

int MaAuth::setAcl(char *group, MaAcl acl)
{
	MaGroup 	*gp;

	if (groupHash == 0 || (gp = (MaGroup*) groupHash->lookup(group)) == 0) {
		return MPR_ERR_CANT_ACCESS;
	}
	gp->setAcl(acl);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

void MaAuth::setAllowSpec(char *spec)
{
	mprFree(allowSpec);
	allowSpec = mprStrdup(spec);
}

////////////////////////////////////////////////////////////////////////////////

void MaAuth::setAnyValidUser()
{
	anyValidUser = 1;
	flags |= MPR_HTTP_AUTH_REQUIRED;
}

////////////////////////////////////////////////////////////////////////////////

void MaAuth::setDenySpec(char *spec)
{
	mprFree(denySpec);
	denySpec = mprStrdup(spec);
}

////////////////////////////////////////////////////////////////////////////////

void MaAuth::setOrder(MaAuthOrder o)
{
	order = o;
}

////////////////////////////////////////////////////////////////////////////////

void MaAuth::setRealm(char *realm)
{
	mprFree(requiredRealm);
	requiredRealm = mprStrdup(realm);
}

////////////////////////////////////////////////////////////////////////////////

void MaAuth::setRequiredGroups(char *groups)
{
	mprFree(requiredGroups);
	requiredGroups = mprStrdup(groups);
	flags |= MPR_HTTP_AUTH_REQUIRED;
}

////////////////////////////////////////////////////////////////////////////////

void MaAuth::setRequiredUsers(char *users)
{
	mprFree(requiredUsers);
	requiredUsers = mprStrdup(users);
	flags |= MPR_HTTP_AUTH_REQUIRED;
}

////////////////////////////////////////////////////////////////////////////////

void MaAuth::setType(MaAuthType typ)
{
	authType = typ;
}

////////////////////////////////////////////////////////////////////////////////

void MaAuth::setQop(char *qop)
{
	mprFree(this->qop);
	if (strcmp(qop, "auth") == 0 || strcmp(qop, "auth-int") == 0) {
		this->qop = mprStrdup(qop);
	} else {
		this->qop = mprStrdup("");
	}
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// MaUser ////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

MaUser::MaUser(char *user, char *realm, char *password, bool enabled)
{
	userName = mprStrdup(user);
	this->realm = mprStrdup(realm);
	this->password = mprStrdup(password);
	this->enabled = enabled;

	mprAllocSprintf(&userRealmKey, MPR_HTTP_MAX_USER * 2, "%s:%s", user, realm);
	setKey(userRealmKey);
}

////////////////////////////////////////////////////////////////////////////////

MaUser::~MaUser()
{
	mprFree(userName);
	mprFree(realm);
	mprFree(password);
	mprFree(userRealmKey);
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// MaGroup ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

MaGroup::MaGroup(char *group, MaAcl acl, bool enabled) : MprHashEntry(group)
{
	groupName = mprStrdup(group);
	this->acl = acl;
	this->enabled = enabled;
	userList = new MprStringList();
}

////////////////////////////////////////////////////////////////////////////////

MaGroup::~MaGroup()
{
	mprFree(groupName);
	delete userList;
}

////////////////////////////////////////////////////////////////////////////////

int MaGroup::addUser(char *user)
{
	MprStringData	*sp, *nextSp;

	sp = (MprStringData*) userList->getFirst();
	while (sp) {
		nextSp = (MprStringData*) userList->getNext(sp);
		if (strcmp(sp->getValue(), user) == 0) {
			return MPR_ERR_ALREADY_EXISTS;
		}
		sp = nextSp;
	}
	userList->insert(user);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

int MaGroup::removeUser(char *user)
{
	MprStringData	*sp, *nextSp;

	sp = (MprStringData*) userList->getFirst();
	while (sp) {
		nextSp = (MprStringData*) userList->getNext(sp);
		if (strcmp(sp->getValue(), user) == 0) {
			userList->remove(sp);
			return 0;
		}
		sp = nextSp;
	}
	return MPR_ERR_CANT_ACCESS;
}

////////////////////////////////////////////////////////////////////////////////
#if FUTURE

void MaAuth::writeInfoHeader()
{
	writeFmt(" Authentication-Info: qop=%s, rspauth=\"%s\", "
		"cnonce=\"%s\", nc=0x%08x\r\n", qop);
}

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
