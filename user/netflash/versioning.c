/* netflash.c:
 *
 * Copyright (C) 2000,  Lineo (www.lineo.com)
 * Copyright (C) 1999-2000,  Greg Ungerer (gerg@snapgear.com)
 *
 * Copied and hacked from rootloader.c which was:
 *
 * Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <sys/mount.h>
#include <string.h>
#ifndef VERSIONTEST
#include <linux/autoconf.h>
#include <config/autoconf.h>
#endif
#include <ctype.h>
#include "fileblock.h"
#include "versioning.h"

#ifndef VENDOR
#define VENDOR "Vendor"
#endif
#ifndef PRODUCT
#define PRODUCT "Product"
#endif
#ifndef VERSION
#define VERSION "1.0.0"
#endif

#define	MAX_DOT_NUMS	4			/* N[.N[.N[.N]]] */
#define	TOTAL_VER_NUMS	(MAX_DOT_NUMS + 2)	/* [pN|bN|uN] */

/****************************************************************************/

char imageVendorName[MAX_VENDOR_SIZE];
char imageProductName[MAX_PRODUCT_SIZE];
char imageVersion[MAX_VERSION_SIZE];

/****************************************************************************/

extern struct blkmem_program_t * prog;

static const char our_vendor_name[] = VENDOR;
static const char our_product_name[] = PRODUCT;
static char our_image_version[] = VERSION;
#if defined(MINIMUM_VERSION)
static char min_image_version[MAX_VERSION_SIZE];
#endif

/****************************************************************************/

static int get_string(char *str, int len);
static int check_version_info(char *version, char *new_version);
static int get_version_bits(char *version, int num[TOTAL_VER_NUMS], char *lang);

/**
 * name is a simple product or vendor name.
 * namelist is either a comma separated list of names (may just be one)
 * 
 * Returns true if the name exists in the list or false if not.
 *
 * e.g. check_match("SG550", "SG530,SME530,SG550,SME550") returns true
 *  */
static int check_match(const char *name, const char *namelist)
{
	char *checklist = strdup(namelist);
	int ret = 0;

	const char *token = strtok(checklist, ",");
	while (token) {
		if (strcmp(name, token) == 0) {
			ret = 1;
			break;
		}
		token = strtok(0, ",");
	}
	free(checklist);
	return ret;
}

/****************************************************************************/
#ifndef VERSIONTEST

/*
 * Code to check that we are putting the correct type of flash into this
 * unit.
 * This code also removes the versioning information from the end
 * of the memory buffer.
 *
 * ret:
 *		0 - everything is correct.
 *		1 - the product name is incorrect.
 *		2 - the vendor name is incorrect.
 *		3 - the version is the same.
 *		4 - the version is older.
 *		5 - the version is invalid.
 *		6 - the version language is different.
 */

/*
 * The last few bytes of the image look like the following:
 *
 *  \0version\0vendore_name\0product_namechksum
 *	the chksum is 16bits wide, and the version is no more than 20bytes.
 *
 * version is w.x.y[nz], where n is ubp, and w, x, y and z are 1,2 or 3 digit
 * numbers.
 *
 * vendorName and productName may be a comma separated list of names
 * which are acceptable
 */
int check_vendor(int endOffset, int *versionLength)
{
	int versionInfo;

	/*
	 * Point to what should be the last byte in the product name string.
	 */
	if (fb_seek_end(endOffset + 1) != 0)
		return 5;

	*versionLength = 0;
	/*
	 * Now try to get the vendor/product/version strings, from the end of the
	 * image, and figure out the length of the strings to return as well
	 */
	if (get_string(imageProductName, MAX_PRODUCT_SIZE) != 0)
		return 5;
	*versionLength += strlen(imageProductName) + 1;

	if (get_string(imageVendorName, MAX_VENDOR_SIZE) != 0)
		return 5;
	*versionLength += strlen(imageVendorName) + 1;

	if (get_string(imageVersion, MAX_VERSION_SIZE) != 0)
		return 5;
	*versionLength += strlen(imageVersion) + 1;

	/*
	 * Check the product name. Our product name may be a comma separated list of names.
	 */
	if (!check_match(imageProductName, our_product_name)) {
		return 1;
	}

	/*
	 * Check the vendor name. Our vendor name may be a comma separated list of names.
	 */
	if (!check_match(imageVendorName, our_vendor_name)) {
		return 2;
	}

	/*
	 * Check the version number.
	 */
	versionInfo = check_version_info(our_image_version, imageVersion);

	return versionInfo;
}


/* get_string
 *
 * This gets a printable string from the memory buffer.
 * It searchs backwards for a non-printable character or a NULL terminator.
 *
 * inputs:
 *
 * str/len - the buffer to store the string in.
 *
 * ret:
 *
 * -1 - we couldn't find the string.
 * 0 - success
 */
int get_string(char *str, int len)
{
	int i, j;
	char c;

	for (i = 0; i < len; i++) {
		fb_peek(str + i, 1);
		if (fb_seek_dec(1) != 0)
			return -1;
		if (!str[i])
			break;
		if (!isprint(str[i]))
			return -1;
	}
	if (i == 0 || i >= len)
		return -1;

	/* We read string in reverse order, so reverse it again */
	for (j=0; j<i/2; j++) {
		c = str[j];
		str[j] = str[i-j-1];
		str[i-j-1] = c;
	}

	return 0;
}

#endif /* VERSIONTEST */
/****************************************************************************/
/* check_version_info
 *
 * Check with the version number in imageVersion is a valid
 * upgrade to the current version.
 * The version is ALWAYS of the form major.minor.minor or it is invalid.
 * We determine whether something is older (less than) or newer,
 * by simply using a strcmp.  This functionality will change over
 * time to reflect intuitive notions of what constitutes reasonable versioing.
 *
 * inputs:
 *
 * curr_version - the version of the current flash image.
 * recv_version - the version of the new flash image we just received.
 *
 * ret:
 * 		0 - it all worked perfectly and the version looks okay.
 *		3 - the new version is the same.
 *		4 - the new version is older.
 *		5 - the new/old version is invalid.
 *		6 - the version language is different.
 *		7 - the version is incompatible with the current version (kernel changes)
 */
int check_version_info(char *curr_version, char *recv_version)
{
	char old_version[MAX_VERSION_SIZE];
	char new_version[MAX_VERSION_SIZE];
	char new_lang[MAX_LANG_SIZE];
	char old_lang[MAX_LANG_SIZE];
	int new_ver[TOTAL_VER_NUMS];
	int old_ver[TOTAL_VER_NUMS];
#if defined(MINIMUM_VERSION)
	int min_ver[TOTAL_VER_NUMS];
	char min_lang[MAX_LANG_SIZE];
	char old_past_minimum = 1;
#endif
	int i;
	
	strncpy(old_version, curr_version, sizeof(old_version));
	old_version[sizeof(old_version)-1] = '\0';
	strncpy(new_version, recv_version, sizeof(new_version));
	new_version[sizeof(new_version)-1] = '\0';
	
	if (!get_version_bits(new_version, new_ver, new_lang))
		return 5;

	if (!get_version_bits(old_version, old_ver, old_lang))
		return 5;
	
	if (strcmp(old_lang, new_lang) != 0)
		return 6;


#if defined(MINIMUM_VERSION)
	strncpy(min_image_version, MINIMUM_VERSION, MAX_VERSION_SIZE);
	if (!get_version_bits(min_image_version, min_ver, min_lang))
		return 5;

	for (i = 0; i < TOTAL_VER_NUMS; i++) {
		if( old_ver[i] < min_ver[i] ) {
			old_past_minimum = 0;
			break;
		}
		if( old_ver[i] > min_ver[i] ) {
			break;
		}
	}
	if (old_past_minimum) {
		for (i = 0; i < TOTAL_VER_NUMS; i++) {
			if( new_ver[i] < min_ver[i] ) {
				return 7;
			}
			if( new_ver[i] > min_ver[i] ) {
				break;
			}
		}
	}
#endif

	for (i = 0; i < TOTAL_VER_NUMS; i++) {
		if (old_ver[i] < new_ver[i])
			return 0;
		if (old_ver[i] > new_ver[i])
			return 4;
	}
	return 3;
} 

/*
 * we support up to 3 x 3 digit versions followed by a suffix (letter number)
 *
 * for example:    1.2.3p5
 *                 1.2p5
 *
 * the letters can only be b=beta, u=update, p=prerelease
 */

static int get_version_bits(char *version, int num[TOTAL_VER_NUMS], char *lang)
{
	int i;
	char *tmp;
	int nums = 0;
	char *eptr;

	/* set everything to 0 */
	for (i = 0; i < TOTAL_VER_NUMS; i++)
		num[i] = 0;

	/* Extract the language suffix */
	eptr = strchr(version, '\0');
	while (--eptr > version && isupper(*eptr))
		;
	if (eptr == version)
		return 0;
	eptr++;
	for (i = 0; (lang[i] = eptr[i]) != '\0'; i++)
		;
	*eptr-- = '\0';

	/* strip any trailing non-version-ending letters or chars,  ie., jj */
	/* iif there is no '-',  which would mean a newer git version */
	if (strchr(version, '-') == NULL)
		while (eptr > version && strchr("bup0123456789", *eptr) == NULL)
			*eptr-- = '\0';
	tmp = version;
	while (tmp != NULL && *tmp) {
		num[nums] = strtol(tmp, &eptr, 10);

		/* when we come through this loop,  we expect a number */
		if (eptr == tmp)
			return 1;

		if (*eptr == '\0') {
			if (nums == 0)
				return 0;
			return 1;
		} else if (*eptr == '.') {
			nums++;
			if (nums >= MAX_DOT_NUMS)
				return 0; /* we have more N.N.N's than we can handle */
			tmp = eptr + 1;
		} else if (*eptr == '-') {
			/* sorry,  not a real version */
			if (nums == 0)
				return 0;
			/* git version */
			nums = TOTAL_VER_NUMS - 1;
			/* -dirty means really dodgy (lower) */
			if (strstr(eptr, "-dirty"))
				nums[num] -= 10000;
			/* another '-str' means hand built */
			if (strcmp(eptr, "-dirty") != 0)
				nums[num] -= 1000;
			return 1;
		} else if (strchr("bup", *eptr)) {
			nums = MAX_DOT_NUMS; /* skip to trailer (ie., p10, u12 bit) */

			/* set the priority appropriately p < b < release < u */
			switch (*eptr) {
			case 'p': num[nums] = -20; break;
			case 'b': num[nums] = -10; break;
			/*                      0; real releases are implied 0 */
			case 'u': num[nums] =  10; break;
			}
			nums++; /* time to read the number after the letter */
			tmp = eptr + 1;
		} else
			return 1;  /* something we do not understand */
	}
	return nums ? 1 : 0;
}

/****************************************************************************/

#ifdef VERSIONTEST
int main(int argc, char *argv[])
{
	int nums[TOTAL_VER_NUMS];
	char lang[10];
	int i, rc;

	if (argc == 2) {
		rc = get_version_bits(argv[1], nums, lang);
		printf("rc:   %d\n", rc);
		if (rc) {
			printf("lang: %s\n", lang);
			for (i = 0; i < TOTAL_VER_NUMS; i++)
				printf("v[%d]: %d\n", i, nums[i]);
			exit(0);
		}
		exit(1);
	} else if (argc == 3) {
		printf("old_ver = %s\n", argv[1]);
		printf("new_ver = %s\n", argv[2]);
		rc = check_version_info(argv[1], argv[2]);
		printf("rc:   %d\n", rc);
		switch (rc) {
		case 0: printf("Ok to upgrade, versions looks okay.\n"); break;
		case 3: printf("the new version is the same.\n"); break;
		case 4: printf("the new version is older.\n"); break;
		case 5: printf("the new/old version is invalid.\n"); break;
		case 6: printf("the version language is different.\n"); break;
		default: printf("unknown\n"); break;
		}
		exit(rc);
	} else {
		fprintf(stderr, "usage: versiontest <version>\n");
		fprintf(stderr, "       versiontest <old version> <new version>\n");
		exit(1);
	}
	exit(1); /* not reached */
}
#endif

#ifdef CONFIG_PROP_LOGD_LOGD
void log_upgrade(char *devname) {
	char *av[20];
	int ac = 0;
	pid_t pid;

	av[ac++] = "logd";
	av[ac++] = "firmware";
	if (imageVersion[0] == '\0') {
		av[ac++] = our_image_version;
		av[ac++] = (devname) ? devname : "UNKNOWN";
	} else {
		av[ac++] = our_image_version;
		av[ac++] = imageVersion;
	}
	av[ac++] = NULL;

	pid = vfork();
	if (pid == 0) {
		execv("/bin/logd", av);
		_exit(1);
	}
	if (pid != -1)
		while (waitpid(pid, NULL, 0) == -1 && errno == EINTR);
}
#endif
