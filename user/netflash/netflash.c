/****************************************************************************/

/*
 * netflash.c:  network FLASH loader.
 *
 * Copyright (C) 1999-2001,  Greg Ungerer (gerg@snapgear.com)
 * Copyright (C) 2000-2001,  Lineo (www.lineo.com)
 * Copyright (C) 2000-2002,  SnapGear (www.snapgear.com)
 *
 * Copied and hacked from rootloader.c which was:
 *
 * Copyright (C) 1998  Kenneth Albanowski <kjahds@kjahds.com>,
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <byteswap.h>
#include <ctype.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/vfs.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <syslog.h>

#include <linux/autoconf.h>
#include <linux/version.h>
#include <config/autoconf.h>
#include <linux/major.h>
#ifdef CONFIG_USER_NETFLASH_CRYPTO
#include "crypto.h"
#endif
#ifdef CONFIG_USER_NETFLASH_CRYPTO_V3
#include "cryptov3.h"
#endif
#ifdef CONFIG_USER_NETFLASH_ATECC508A
#include "crypto_atmel.h"
#endif
#if defined(CONFIG_MTD) || defined(CONFIG_MTD_MODULES)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,8)
#include <mtd/mtd-user.h>
#define MTD_CHAR_MAJOR 90
#define MTD_BLOCK_MAJOR 31
#else
#include <linux/mtd/mtd.h>
#endif
#elif defined(CONFIG_BLK_DEV_BLKMEM)
#include <linux/blkmem.h>
#endif
#ifdef CONFIG_LEDMAN
#include <linux/ledman.h>
#endif
#ifdef CONFIG_USER_NETFLASH_DECOMPRESS
#include <zlib.h>
#endif
#if defined(CONFIG_MTD) || defined(CONFIG_MTD_MODULES)
#include <linux/jffs2.h>
#endif
#if defined(CONFIG_NFTL_RW) && !defined(NFTL_MAJOR)
 #define NFTL_MAJOR 93
#endif
#if defined(CONFIG_IDE) || defined(CONFIG_SCSI)
#include <linux/hdreg.h>
#endif
#include <asm/byteorder.h>

#include "check.h"
#include "fileblock.h"
#include "exit_codes.h"
#include "versioning.h"
#include "netflash.h"
#include "program.h"
#include "util.h"

/****************************************************************************/

#ifdef CONFIG_USER_NETFLASH_HMACMD5
#define HMACMD5_OPTIONS "m:"
#else
#define HMACMD5_OPTIONS
#endif

#ifdef CONFIG_USER_NETFLASH_DECOMPRESS
#include "decompress.h"
#define DECOMPRESS_OPTIONS "z"
#else
#define DECOMPRESS_OPTIONS
#endif

#ifdef CONFIG_USER_NETFLASH_SETSRC
#define SETSRC_OPTIONS "I:"
#else
#define SETSRC_OPTIONS
#endif

#ifdef CONFIG_USER_NETFLASH_SHA256
#define	SHA256_OPTIONS "N"
#else
#define	SHA256_OPTIONS
#endif

#ifdef CONFIG_USER_NETFLASH_DUAL_IMAGES
#define	UBOOT_OPTIONS "U"
#ifndef CONFIG_USER_NETFLASH_DUAL_IMAGES_A
#define CONFIG_USER_NETFLASH_DUAL_IMAGES_A "/dev/flash/image"
#endif
#ifndef CONFIG_USER_NETFLASH_DUAL_IMAGES_B
#define CONFIG_USER_NETFLASH_DUAL_IMAGES_B "/dev/flash/image1"
#endif
#else
#define	UBOOT_OPTIONS
#endif

#define CMD_LINE_OPTIONS "abB:c:Cd:efFhiHjkKlL:Mno:pr:R:sStuv?" DECOMPRESS_OPTIONS HMACMD5_OPTIONS SETSRC_OPTIONS SHA256_OPTIONS UBOOT_OPTIONS

#define PID_DIR "/var/run"
#if defined(CONFIG_USER_DHCPCD_DHCPCD) || defined(CONFIG_USER_DHCPCD_NEW_DHCPCD)
#define DHCPCD_PID_FILE "dhcpcd-"
#elif defined(CONFIG_USER_BUSYBOX_UDHCPC)
#define DHCPCD_PID_FILE "udhcpc-"
#else
#warning "Unknown DHCP client pid file name, guessing ..."
#define DHCPCD_PID_FILE "dhcpc"
#endif
#define NETFLASH_KILL_LIST_FILE "/etc/netflash_kill_list.txt"

#ifdef CONFIG_USER_BUSYBOX_WATCHDOG
#define CONFIG_USER_NETFLASH_WATCHDOG 1
#endif

#ifdef CONFIG_USER_NETFLASH_WITH_CGI
#define MAX_WAIT_NETFLASH_FLUSH		20	/* seconds */
#endif

/****************************************************************************/

static char *version = "2.2.1";

static unsigned long image_length;

static int dothrow;		/* Check version info of image; no program */
#if CONFIG_USER_NETFLASH_WATCHDOG
static int watchdog = 1;	/* tickle watchdog while writing to flash */
static int watchdog_fd = -1;	/* ensure this is initalised to an invalid fd */
#endif
static int nostop_early;	/* no stop at end of input data, do write full dev. */
static int docgi;		/* Read options and data from stdin in mime multipart format */
static int dofilesave;		/* Save locally as file, not a flash device */
static int dofileautoname;	/* Put file in right directory automatically */
static int dobootcfg;		/* Update boot.cfg file to boot image */
#ifdef CONFIG_USER_NETFLASH_DUAL_IMAGES
static int dobootpart;		/* update uboot environment bootpart */
#endif
#if defined(CONFIG_USER_NETFLASH_WITH_CGI) && !defined(RECOVER_PROGRAM)
static char cgi_data[64];      /* CGI section name for the image part */
static char cgi_options[64];   /* CGI section name for the command line options part */
static char cgi_flash_region[20]; /* CGI section name for the flash region part */
extern size_t cgi_load(const char *data_name, const char *options_name, char options[64], const char *flash_region_name, char flash_region[20], int *error_code);
#endif

extern int tftpverbose;
extern int ftpverbose;

static FILE *nfd;

#ifdef CONFIG_USER_NETFLASH_SETSRC
static char *srcaddr;
#endif

static int killprocname(const char *name, int signo);

/****************************************************************************/

static int local_system(char **argv, int closefd)
{
	pid_t pid;
	int status;

	pid = vfork();
	if (pid == -1) {
		error("vfork() failed %m");
		exit(VFORK_FAIL);
	} else if (pid == 0) {
		if (closefd) {
			/* We don't want any output messages */
			close(0);
			close(1);
			close(2);
			open("/dev/null", O_RDONLY);
			open("/dev/null", O_WRONLY);
			dup(1);
		}
		execvp(argv[0], argv);
		_exit(1);
	}
	status = 0;
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status);
}

/****************************************************************************/

static void restartinit(void)
{
	notice("restarting init process...");
	killprocname("init", SIGCONT);
#ifdef CONFIG_USER_NETFLASH_WATCHDOG
	if (watchdog_fd >= 0)
		close(watchdog_fd);
	char *localargv[] = { "watchdog", "/dev/watchdog", NULL };
	local_system(localargv, 0);
#endif
}

/****************************************************************************/

/*
 *	Local copies of the open/close/write used by tftp loader.
 *	The idea is that we get tftp to do all the work of getting
 *	the file over the network. The following code back ends
 *	that process, preparing the read data for FLASH programming.
 */
int local_creat(char *name, int flags)
{
	return(fileno(nfd));
}

FILE *local_fdopen(int fd, char *flags)
{
	return(nfd);
}

FILE *local_fopen(const char *name, const char *flags)
{
	return(nfd);
}

int local_fclose(FILE *fp)
{
	return(0);
}

int local_fseek(FILE *fp, int offset, int whence)
{
	/* Shouldn't happen... */
	return(0);
}

int local_putc(int ch, FILE *fp)
{
	/* Shouldn't happen... */
	return(0);
}

#ifndef CONFIG_USER_NETFLASH_CRYPTO_V3
static void local_throw(void *buf, unsigned long count)
{
#ifdef CONFIG_USER_NETFLASH_CRYPTO
	update_crypto_hash(buf, count);
#endif
}
#endif

int local_write(int fd, void *buf, int count)
{
#ifdef CONFIG_USER_NETFLASH_WATCHDOG
	if (watchdog_fd >= 0)
		write(watchdog_fd, "\0", 1);
#endif

	if (!docgi) {
		static unsigned long total = 0;
		static unsigned lastk = 0;

		total += count;
		if (total / 1024 != lastk && time_elapsed() >= time_rate_100) {
			lastk = total / 1024;
			printf("\r%dK", lastk); fflush(stdout);
			time_start();
		}
	} else if (time_elapsed() >= time_rate_100)
		printf(".");

	update_chksum(buf, count);
#ifndef CONFIG_USER_NETFLASH_CRYPTO_V3
	if (dothrow)
		fb_throw(1024, local_throw);
#endif
	if (fb_write(buf, count) != 0) {
		error("Insufficient memory for image!");
		exit(NO_MEMORY);
	}
	return count;
}

/****************************************************************************/

#include "tftp.h"

/*
 * Call to tftp. This will initialize tftp and do a get operation.
 * This will call the local_write() routine with the data that is
 * fetched, and it will create the ioctl structure.
 */
static int tftpfetch(char *srvname, char *filename, int blocksize)
{
	char *tftpargv[8];
	int tftpmainargc = 0;
	char blocksize_str[16];
	/* If we get here assume blocksize is in range */
	sprintf(blocksize_str, "bs=%d", blocksize);

	tftpverbose = 0;	/* Set to 1 for tftp trace info */

	tftpargv[tftpmainargc++] = "tftp";
	tftpargv[tftpmainargc++] = srvname;
#ifdef CONFIG_USER_NETFLASH_SETSRC
	if (srcaddr != NULL)
		tftpargv[tftpmainargc++] = srcaddr;
#endif
	tftpmain(tftpmainargc, tftpargv);
	tftpsetbinary(1, tftpargv);

	notice("fetching file \"%s\" from %s\n", filename, srvname);
	tftpargv[0] = "get";
	tftpargv[1] = filename;
	tftpargv[2] = blocksize_str;
	tftpget(3, tftpargv);
	return 0;
}

/****************************************************************************/

extern void ftpmain(int argc, char *argv[]);
extern void setbinary(void);
extern void get(int argc, char *argv[]);
extern void quit(void);

/*
 * Call to ftp. This will initialize ftp and do a get operation.
 * This will call the local_write() routine with the data that is
 * fetched, and it will create the ioctl structure.
 */
static int ftpconnect(char *srvname)
{
#ifdef FTP
	char *ftpargv[4];

	ftpverbose = 0;	/* Set to 1 for ftp trace info */
	notice("login to remote host %s", srvname);

	ftpargv[0] = "ftp";
	ftpargv[1] = srvname;
	ftpmain(2, ftpargv);
	return 0;

#else
	error("no ftp support builtin");
	return -1;
#endif /* FTP */
}

static int ftpfetch(char *srvname, char *filename)
{
#ifdef FTP
	char *ftpargv[4];

	ftpverbose = 0;	/* Set to 1 for ftp trace info */
	notice("ftping file \"%s\" from %s", filename, srvname);
	setbinary(); /* make sure we are in binary mode */

	ftpargv[0] = "get";
	ftpargv[1] = filename;
	get(2, ftpargv);

	quit();
	return 0;

#else
	error("no ftp support builtin");
	return -1;
#endif /* FTP */
}

/****************************************************************************/

extern int openhttp(char *url);

/*
 *	When fetching file we need to even number of bytes in write
 *	buffers. Otherwise FLASH programming will fail. This is mostly
 *	only a problem with http for some reason.
 */

static int filefetch(char *filename)
{
	int fd, i, j;
	unsigned char buf[1024];

	if (strncmp(filename, "http://", 7) == 0)
		fd = openhttp(filename);
	else if (strcmp(filename, "-") == 0)
		fd = STDIN_FILENO;
#ifndef CONFIG_USER_NETFLASH_CRYPTO /* Need to decrypt in place */
	else if (fb_init_file(filename) == 0) {
		/*
		 * We're not loading the file into ram.
		 * The checksum is normally calculated while loading,
		 * so we have to do it explicitly here.
		 */
		calc_chksum();
		return 0;
	}
#endif
	else
		fd = open(filename, O_RDONLY);

	if (fd < 0)
		return -1;

	for (;;) {
		if ((i = read(fd, buf, sizeof(buf))) <= 0)
			break;
		if (i & 0x1) {
			/* Read more to make even sized buffer */
			if ((j = read(fd, &buf[i], 1)) > 0)
				i += j;
		}
		local_write(-1, buf, i);
	}

	close(fd);
	return 0;
}

/****************************************************************************/

static int samedev(struct stat *stat_dev, struct stat *stat_rootfs)
{
	if (S_ISBLK(stat_dev->st_mode)) {
		if (stat_dev->st_rdev == stat_rootfs->st_dev) {
			return 1;
		}
#if defined(CONFIG_NFTL_RW)
		/* Check for writing to nftla, with an nftla partition
		 * as the root device. */
		else if (major(stat_dev->st_rdev) == NFTL_MAJOR
				&& major(stat_rootfs->st_dev) == NFTL_MAJOR
				&& minor(stat_dev->st_rdev) == 0) {
			return 1;
		}
#endif
	}
#if defined(CONFIG_MTD) || defined(CONFIG_MTD_MODULES)
	/* Check for matching block/character mtd devices. */
	else if (S_ISCHR(stat_dev->st_mode)) {
		if (major(stat_dev->st_rdev) == MTD_CHAR_MAJOR
				&& major(stat_rootfs->st_dev) == MTD_BLOCK_MAJOR
				&& (minor(stat_dev->st_rdev) >> 1)
					== minor(stat_rootfs->st_dev)) {
			return 1;
		}
	}
#endif
	return 0;
}

/*
 *	Check if we are writing to the root filesystem.
 */
static int flashing_rootfs(char *rdev)
{
	struct stat stat_rootfs, stat_flash, stat_rdev;

	/* Check for device/file existence first */
	if (stat(rdev, &stat_rdev) != 0)
		return 0;

	/* First a generic check:
	 * is the rootfs device the same as the flash device?
	 */
	if (stat("/", &stat_rootfs) != 0) {
		error("stat(\"/\") failed: %m");
		exit(BAD_ROOTFS);
	}
	if (samedev(&stat_rdev, &stat_rootfs))
		return 1;

	/* Secondly, a platform specific check:
	 * /dev/flash/all and /dev/flash/image and /dev/flash/rootfs
	 * can overlap, check if we are writing to any of these, and the
	 * root device is /dev/flash/image or /dev/flash/rootfs.
	 * XXX: checking device numbers would be better than strcmp */
	else if (!strcmp(rdev, "/dev/flash/all")
			|| !strcmp(rdev, "/dev/flash/image")
			|| !strcmp(rdev, "/dev/flash/rootfs")) {
		if (stat("/dev/flash/image", &stat_flash) == 0
				&& samedev(&stat_flash, &stat_rootfs))
			return 1;
		if (stat("/dev/flash/rootfs", &stat_flash) == 0
				&& samedev(&stat_flash, &stat_rootfs))
			return 1;
	}
	return 0;
}

/****************************************************************************/

/*
 *	Search for a process and send a signal to it.
 */
static int killprocname(const char *name, int signo)
{
	DIR *dir;
	struct dirent *entry;
	FILE *f;
	char path[32];
	char line[64];
	int ret = 0;

	dir = opendir("/proc");
	if (!dir)
		return 0;

	while ((entry = readdir(dir)) != NULL) {
		if (!isdigit(*entry->d_name))
			continue;

		sprintf(path, "/proc/%s/status", entry->d_name);
		if ((f = fopen(path, "r")) == NULL)
			continue;

		while (fgets(line, sizeof(line), f) != NULL) {
			if (line[strlen(line)-1] == '\n') {
				line[strlen(line)-1] = '\0';
				if (strncmp(line, "Name:\t", 6) == 0
						&& strcmp(line+6, name) == 0) {
					kill(atoi(entry->d_name), signo);
					ret = 1;
				}
			}
		}

		fclose(f);
	}
	closedir(dir);
	return ret;
}

/****************************************************************************/

/*
 *  Read a process pid file and send a signal to it.
 */
static void killprocpid(char *file, int signo)
{
	FILE* f;
	pid_t pid;
	char value[16];

	f = fopen(file, "r");
	if (f == NULL)
		return;

	if (fread(value, 1, sizeof(value), f) > 0) {
		pid = atoi(value);
		if (pid)
			kill(pid, signo);
		unlink(file);
	}
	fclose(f);
}

/****************************************************************************/

/*
 * Wait for a (non-child) process to exit.
 */
static void waitprocpid(pid_t pid, int timeout)
{
	int status;

	while (timeout > 0) {
		status = kill(pid, 0);
		if (status == -1 && errno == ESRCH) {
			/*
			 * Allow a bit of time just in case data is 
			 * queued somewhere waiting to be transmitted.
			 */
			sleep(2);
			break;
		}
		sleep(1);
		timeout--;
	}
}

/****************************************************************************/

/*
 *	Find the current console device. We output trace to this device
 *	if it is the controlling tty at process start.
 */
static char *consolelist[] = {
	"/dev/console",
	"/dev/ttyS0",
	"/dev/cua0",
	"/dev/ttyS1",
	"/dev/cua1",
	"/dev/ttyAM0"
};

#define	clistsize	(sizeof(consolelist) / sizeof(char *))

static char *getconsole(void)
{
	struct stat	myst, st;
	int		i;

	if (fstat(0, &myst) < 0)
		goto err;

	for (i = 0; (i < clistsize); i++) {
		if (!stat(consolelist[i], &st) && 
				(myst.st_rdev == st.st_rdev))
			return(consolelist[i]);
	}

err:
	return "/dev/null";
}

/****************************************************************************/

/*
 * Kill off processes to reclaim some memory.  Only kills processes
 * that we know are unnecessary for obtaining the image.
 */
static void kill_processes_partial(void)
{
	int count;
	FILE *f;
	char line[64];
	char *newline;

	notice("killing unnecessary tasks...");
	sleep(1);

	killprocname("init", SIGTSTP);	/* Stop init from reforking tasks */
	atexit(restartinit);		/* If exit prematurely, restart init */
	sync();

	if (!dothrow) {
		/*
		 * Only kill ifmond if we're NOT in throw away mode and 
		 * need all the memory we can get.
		 */
		killprocpid("/var/run/ifmond.pid", SIGKILL);
	}

	/* Read the list of processes to kill from a file generated at compile time */
	f = fopen(NETFLASH_KILL_LIST_FILE, "r");
	if (f != NULL) {
		/* Ask them nicely. */
		count = 0;
		while (fgets(line, sizeof(line), f) != NULL) {
			/* Remove newline */
			newline = strchr(line, '\n');
			if (newline) {
				*newline = '\0';
			}
			count += killprocname(line, SIGTERM);
		}
		if (count) {
			sleep(8);	/* give em a moment... */
		}

		/* Re-read the list */
		rewind(f);

		/* Time for the no-nonsense approach. */
		count = 0;
		while (fgets(line, sizeof(line), f) != NULL) {
			/* Remove newline */
			newline = strchr(line, '\n');
			if (newline) {
				*newline = '\0';
			}
			count += killprocname(line, SIGKILL);
		}
		if (count) {
			sleep(4);	/* give em another moment... */
		}

		fclose(f);
	}

	/* If we couldn't open the process kill list then there isn't 
	 * much else we can do, so just keep going */
}


/*
 * Kill of processes now to reclaim some memory. Need this now so
 * we can buffer an entire firmware image...
 */
static void kill_processes(char *console)
{
	int ttyfd;
	struct termios tio;
	DIR *dir;
	struct dirent *dp;
	char filename[128];

	if (console == NULL)
		console = getconsole();

	ttyfd = open(console, O_RDWR|O_NDELAY|O_NOCTTY);
	if (ttyfd >= 0) {
		if (tcgetattr(ttyfd, &tio) >= 0) {
			tio.c_cflag |= CLOCAL;
			tcsetattr(ttyfd, TCSANOW, &tio);
		}
		close(ttyfd);
	}
	if (!docgi) {
		freopen(console, "w", stdout);
		freopen(console, "w", stderr);
	}

	notice("killing tasks...");
	fflush(stdout);
	sleep(1);

	killprocname("init", SIGTSTP);	/* Stop init from reforking tasks */
	atexit(restartinit);		/* If exit prematurely, restart init */
	sync();

	signal(SIGTERM,SIG_IGN);	/* Don't kill ourselves... */
	setpgrp();			/* Don't let our parent kill us */
	sleep(1);
	signal(SIGHUP, SIG_IGN);	/* Don't die if our parent dies due to
					 * a closed controlling terminal */

	killprocpid("/var/run/ifmond.pid", SIGKILL);

	/*Don't take down network interfaces that use dhcpcd*/
	dir = opendir(PID_DIR);
	if (dir) {
		while ((dp = readdir(dir)) != NULL) {
			if (strncmp(dp->d_name, DHCPCD_PID_FILE,
						sizeof(DHCPCD_PID_FILE)-1) != 0)
				continue;
			if (strcmp(dp->d_name + strlen(dp->d_name) - 4,
						".pid") != 0)
				continue;
			snprintf(filename, sizeof(filename), "%s/%s",
					PID_DIR, dp->d_name);
			killprocpid(filename, SIGKILL);
		}
		closedir(dir);
	}

	kill(-1, SIGTERM);		/* Kill everything that'll die */
	sleep(5);			/* give em a moment... (it may take a while for, e.g., pppd to shutdown cleanly */
	kill(-1, SIGKILL);		/* Really make sure that everything is dead */
	sleep(2);			/* give em another moment... */

	if (console)
		freopen(console, "w", stdout);
#if CONFIG_USER_NETFLASH_WATCHDOG
	if (watchdog) {
		watchdog_fd = open("/dev/watchdog", O_WRONLY);
	}
#endif
}

/****************************************************************************/

static void umount_all(void)
{
#if defined(CONFIG_USER_MOUNT_UMOUNT) || defined(CONFIG_USER_BUSYBOX_UMOUNT)
	char *localargv[] = {
		"/bin/umount", "-a", "-r", NULL
	};
	local_system(localargv, 1);
#endif
}

/****************************************************************************/

#define	BOOTCFG_MAX_ENTRIES	4

static void update_bootcfg(char *filename)
{
	char *cp, *line[BOOTCFG_MAX_ENTRIES * 4];
	static char ent[1024];
	int i, lines = 0;
	FILE *fp;
	struct stat st;

	if (memcmp(filename, "/sda1", 5) == 0)
		filename += 5;

	fp = fopen("/sda1/boot.cfg", "r");
	if (fp) {
		while (fgets(ent, sizeof(ent), fp)) {
			cp = strchr(ent, '\n');
			if (cp)
				*cp = '\0';
			/* check this file isn't the same */
			if (strcmp(ent, filename) == 0)
				continue;
			/* check that image file still exists, malloc failure means keep it in case */
			cp = (char *) malloc(strlen(ent) + 6);
			if (cp) {
				sprintf(cp, "/sda1%s", ent);
				if (stat(cp, &st) == -1) {
					free(cp);
					continue;
				}
				free(cp);
			}
			/* only interested in unique lines */
			for (i = 0; i < lines; i++)
				if (strcmp(ent, line[i]) == 0)
					break;
			if (i >= lines) {
				/* a unique line for an image that exists,  keep it */
				line[lines++] = strdup(ent);
				if (lines >= BOOTCFG_MAX_ENTRIES * 4)
					break;
			}
		}
		fclose(fp);
	}

	fp = fopen("/sda1/boot.cfg", "w");
	if (fp) {
		fprintf(fp, "%s\n", filename);
		/* prune: only write out BOOTCFG_MAX_ENTRIES - 1 extra entries */
		for (i = 0; i < lines && i < BOOTCFG_MAX_ENTRIES - 1; i++)
			fprintf(fp, "%s\n", line[i]);
		/* remove any freshly unreferenceded images */
		for (; i < lines; i++) {
			cp = malloc(strlen(line[i]) + 8);
			if (cp) {
				sprintf(cp, "/sda1%s", line[i]);
				unlink(cp);
				free(cp);
			}
			cp = malloc(strlen(line[i]) + 16);
			if (cp) {
				sprintf(cp, "/sda1%s.bak", line[i]);
				unlink(cp);
				free(cp);
			}
		}
		fclose(fp);
	}

	for (i = 0; i < lines; i++)
		free(line[i]);
}

/****************************************************************************/

static void flush_disk_cache(void)
{
	int fd;

	if ((fd = open("/dev/sda", O_RDONLY)) < 0) {
		printf("WARNING: failed to open /dev/sda for flushing?\n");
		return;
	}
	if (ioctl(fd, BLKFLSBUF, 0) < 0)
		printf("WARNING: failed to flush disk cache, %d?\n", errno);
	close(fd);
}

/****************************************************************************/

static void remount_disk_ro(void)
{
#if defined(CONFIG_USER_MOUNT_UMOUNT) || defined(CONFIG_USER_BUSYBOX_UMOUNT)
	char *localargv[] = {
		"/bin/mount", "-o", "remount,ro", "/dev/sda1", "/sda1", NULL
	};
	printf("Remounting flash as read-only (to write journal)\n");
	local_system(localargv, 0);
#endif
}

/****************************************************************************/
#ifdef CONFIG_USER_NETFLASH_DUAL_IMAGES

static int get_bootpart(void)
{
	FILE *f;
	size_t len;
	size_t n = 0;
	char *line = NULL, *s;
	char bootpart = 0;

	f = fopen("/proc/cmdline", "r");
	if (f) {
		if ((len = getline(&line, &n, f)) != -1) {
			s = strtok(line, " \n");
			while (s != NULL) {
				if (strncmp(s, "bootpart=", 9) == 0) {
					bootpart = s[9];
					break;
				}
				s = strtok(NULL, " \n");
			}
			free(line);
		}
		fclose(f);
	}

	return bootpart;
}

#ifdef CONFIG_USER_GRUB

/*
 * GRUB style dual booting configuration. We use a "grubenv" file to store
 * the default boot menu item.
 */
#define GRUBENVFILE	"/opt/boot/grubenv"
#define GRUBEDIT	"/usr/bin/grub-editenv"

static int set_bootpart(void)
{
	char *bootpart;
	char *partvalid;
	if (dobootpart == 'a') {
		bootpart = "default=0";
		partvalid = "part0_valid=true";
	} else if (dobootpart == 'b') {
		bootpart = "default=1";
		partvalid = "part1_valid=true";
	} else {
		return 0;
	}

	char *localargv[] = {
		GRUBEDIT, GRUBENVFILE, "set", bootpart, partvalid, NULL
	};
	return local_system(localargv, 0);
}
#else

/*
 * U-Boot style dual booting configuration. We store the bootpart info in
 * the u-boot environment.
 */
static int set_bootpart(void)
{
	int ret;
	char *bootpart;
	char *partvalid;
	if (dobootpart == 'a') {
		bootpart = "a";
		partvalid = "parta_valid";
	} else if (dobootpart == 'b') {
		bootpart = "b";
		partvalid = "partb_valid";
	} else {
		return 0;
	}

	char *localargv[] = {
		"/bin/fw_setenv", partvalid, "1", NULL
	};
	ret = local_system(localargv, 0);
	if (ret)
		return ret;

	localargv[1] = "bootpart";
	localargv[2] = bootpart;

	return local_system(localargv, 0);
}

#endif /* !CONFIG_USER_GRUB */
#endif /* CONFIG_USER_NETFLASH_DUAL_IMAGE */
/****************************************************************************/

static int usage(int rc)
{
	printf("usage: netflash [option]... [net-server] file-name\n\n"
	"\t-c device         console device\n"
	"\t-d delay          delay in seconds before killing processes\n"
#ifdef CONFIG_USER_NETFLASH_SETSRC
	"\t-I ip-address     originate TFTP request from this address\n"
#endif
#ifdef CONFIG_USER_NETFLASH_HMACMD5
	"\t-m hmac-md5-key   enable HMAC-MD5 and use this key\n"
#endif
	"\t-o offset         offset in flash device to write to\n"
	"\t-r device         flash device to write to\n"
	"\t-R file-name      real file to write to\n"
#ifdef CONFIG_USER_NETFLASH_CRYPTO_V3
	"\t--ca file-name    CA certificate for authentication\n"
	"\t--crl file-name   CRL for authentication\n"
#endif
	"\t-a\tdon't add filename to bootcfg file (if using -R)\n"
	"\t-b\tdon't reboot hardware when done\n"
	"\t-B N\tTFTP blocksize option 'N' bytes, 8-65464, default: 8192\n"
	"\t-C\tcheck that image was written correctly\n"
	"\t-d\tspecify seconds to wait before programming flash\n"
	"\t-e\tdo not erase flash segments if they are already blank\n"
	"\t-f\tuse FTP as load protocol\n"
	"\t-F\tforce overwrite (do not preserve special regions)\n"
	"\t-h\tprint help\n"
	"\t-H\tignore hardware type information\n"
	"\t-i\tignore any version information\n"
	"\t-j\timage is a JFFS2 filesystem\n"
	"\t-k\tdon't kill other processes (or delays kill until\n"
	"\t\tafter downloading when root filesystem is inside flash)\n"
	"\t-K\tonly kill unnecessary processes (or delays kill until\n"
	"\t\tafter downloading when root filesystem is inside flash)\n"
	"\t-l\tlock flash segments when done\n"
	"\t-L N\tlimit output to at most once every 'N' 100ths of a second\n"
#if defined(MINIMUM_VERSION)
	"\t-M\tIgnore minimum version incompatibility check\n"
#endif
	"\t-n\tfile with no checksum at end (implies no version information)\n"
#if CONFIG_USER_NETFLASH_SHA256
	"\t-N\tfile with no SHA256 checksum\n"
#endif
	"\t-p\tpreserve portions of flash segments not actually written.\n"
	"\t-s\tstop erasing/programming at end of input data\n"
	"\t-S\tdo not stop erasing/programming at end of input data\n"
	"\t-t\tcheck the image and then throw it away \n"
#if CONFIG_USER_NETFLASH_DUAL_IMAGES
	"\t-U\tupdate the bootpart environment variable of u-boot\n"
#endif
	"\t-u\tunlock flash segments before programming\n"
	"\t-v\tdisplay version number\n"
#if CONFIG_USER_NETFLASH_WATCHDOG
	"\t-w\tdon't tickle hardware watchdog\n"
#endif
#ifdef CONFIG_USER_NETFLASH_DECOMPRESS
	"\t-z\tdecompress image before writing\n"
#endif
	);

	exit(rc);
}

static struct option longopts[] = {
	{ .name = "help", .has_arg = no_argument, .flag = 0, .val = 'h' },
#ifdef CONFIG_USER_NETFLASH_CRYPTO_V3
	{ .name = "ca", .has_arg = required_argument, .flag = 0, .val = 0 },
	{ .name = "crl", .has_arg = required_argument, .flag = 0, .val = 0 },
#endif
	{ }
};

/****************************************************************************/
/*
 * when we call reboot,  we don't want anything in our way, most certainly
 * not logd !
 */

static inline int raw_reboot(int magic, int magic2, int flag)
{
	return syscall(__NR_reboot, magic, magic2, flag);
}

/****************************************************************************/

int netflashmain(int argc, char *argv[])
{
	char *srvname, *filename;
	char *rdev, *console;
	unsigned char *sgdata;
	int rd = 0, delay;
	int kill_processes_run = 0;
	int dokill, dokillpartial, doreboot, doftp;
	int dopreserve, doflashingrootfs;
	int blocksize = BLOCKSIZE_DEFAULT;
#if defined(CONFIG_USER_NETFLASH_WITH_CGI) && !defined(RECOVER_PROGRAM)
	char options[64];
	char flash_region[20];
	char *new_argv[10];
#endif
	struct check_opt check_opt = {
		.dochecksum = 1,
#if defined(MINIMUM_VERSION)
		.dominimumcheck = 1,
#else
		.dominimumcheck = 0,
#endif
#ifdef CONFIG_USER_NETFLASH_VERSION
		.doversion = 1,
#else
		.doversion = 0,
#endif
#ifdef CONFIG_USER_NETFLASH_HARDWARE
		.dohardwareversion = 1,
#else
		.dohardwareversion = 0,
#endif
		.doremoveversion = 1,
#ifdef CONFIG_USER_NETFLASH_SHA256
		.dosha256sum = 1,
#endif
#ifdef CONFIG_USER_NETFLASH_HMACMD5
		.hmacmd5key = NULL,
#endif
#ifdef CONFIG_USER_NETFLASH_CRYPTO_V3
		.ca = NULL,
		.crl = NULL,
#endif
	};

	rdev = "/dev/flash/image";
	srvname = NULL;
	filename = NULL;
	console = NULL;
	dokill = 1;
	dokillpartial = 0;
	doreboot = 1;
	dolock = 0;
	dounlock = 0;
	delay = 0;
	doftp = 0;
	dothrow = 0;
	dopreserve = 1;
	preserveconfig = 0;
	checkimage = 0;
	checkblank = 0;
	dojffs2 = 0;
#ifdef CONFIG_USER_NETFLASH_WITH_FILE
	dofileautoname = 1;
	dofilesave = 1;
	dobootcfg = 1;
	stop_early = 1;
#endif

	/* init the versioning data to empty strings; they're checked later on and could barf */
	imageVendorName[0] = 0;
	imageProductName[0] = 0;
	imageVersion[0] = 0;

#if defined(CONFIG_USER_NETFLASH_WITH_CGI) && !defined(RECOVER_PROGRAM)
	if (argc == 2 && strncmp(argv[1], "cgi://", 6) == 0) {
		char *pt;
		char *sep;
		int new_argc = 0;
		int rc;

		docgi = 1;
		syslog(LOG_INFO, "netflash: using CGI");

		/* Do the partial kill here before we download the image */
		kill_processes_partial();
		/* Wait a little bit for processes to die and release memory */
		sleep(5);

		/* Our "command line" options come from stdin for cgi
		 * Format of the command line is:
		 * cgi://dataname,optionsname,flash_regionname
		 */
		pt = argv[1] + 6;
		sep = strchr(pt, ',');
		if (sep) {
			int len = sep - pt;
			if (len >= sizeof(cgi_data)) {
				len = sizeof(cgi_data) - 1;
			}
			strncpy(cgi_data, pt, len);
			cgi_data[len] = 0;
			pt = sep + 1;
			sep = strchr(pt, ',');
			if (sep) {
				len = sep - pt;
				strncpy(cgi_options, pt, len);
				cgi_options[len] = 0;

				strncpy(cgi_flash_region, sep + 1, sizeof(cgi_flash_region));
				cgi_flash_region[sizeof(cgi_flash_region) - 1] = 0;
			} else {
				exit(BAD_CGI_FORMAT);
			}
		} else {
			exit(BAD_CGI_FORMAT);
		}

		if (cgi_load(cgi_data, cgi_options, options,
					cgi_flash_region, flash_region, &rc) <= 0) {
			exit(rc);
		}

		if (strcmp(flash_region, "bootloader") == 0) {
#ifndef CONFIG_USER_FLASH_BOOT_LOCKED
			const char *bootloader_params = " -np -r /dev/flash/boot";
#else
			const char *bootloader_params = " -np -r /dev/flash/boot -lu";
#endif
			if ((sizeof(options) - strlen(options)) > strlen(bootloader_params)) {
				strcat(options, bootloader_params);
			}
			else {
				exit(BAD_CGI_DATA);
			}
		}

		new_argv[new_argc++] = argv[0];

		/* Parse the options */
		pt = strtok(options, " \t");
		while (pt) {
			assert(new_argc < 10);
			new_argv[new_argc++] = pt;
			pt = strtok(0, " \t");
		}
		argc = new_argc;
		argv = new_argv;
	}
#endif

	while (1) {
		int longindex;
		int opt;

		opt = getopt_long(argc, argv, CMD_LINE_OPTIONS,
				longopts, &longindex);
		if (opt < 0)
			break;

		switch (opt) {
		case 'p':
			preserve = 1;
			stop_early = 1;
			break;
		case 's':
			stop_early = 1;
			break;
		case 'S':
			nostop_early = 1;
			break;
		case 'b':
			doreboot = 0;
			break;
		case 'B':
			blocksize = atoi(optarg);
			if (blocksize > BLOCKSIZE_MAX || blocksize < BLOCKSIZE_MIN) {
				error("Invalid blocksize!");
				usage(1);
			}
			break;
		case 'c':
			console = optarg;
			break;
		case 'C':
			checkimage = 1;
			break;
		case 'e':
			checkblank = 1;
			break;
		case 'd':
			delay = (atoi(optarg));
			break;
		case 'f':
			doftp = 1;
			break;
		case 'F':
			dopreserve = 0;
			break;
		case 'i': 
			check_opt.doversion = 0;
			break;
		case 'H': 
			check_opt.dohardwareversion = 0;
			break;
		case 'j':
			dojffs2 = 1;
			nostop_early = 1;
			break;
		case 'k':
			dokill = 0;
			break;
		case 'K':
			dokill = 1;
			dokillpartial = 1;
			break;
		case 'l':
			dolock++;
			break;
		case 'L':
			time_rate_100 = atoi(optarg);
			if (time_rate_100 > 100)
				time_rate_100 = 100;
			else if (time_rate_100 < 0)
				time_rate_100 = 0;
			break;
#if defined(MINIMUM_VERSION)
		case 'M':
			check_opt.dominimumcheck = 0;
			break;
#endif
#ifdef CONFIG_USER_NETFLASH_HMACMD5
		case 'm':
			check_opt.hmacmd5key = optarg;
			break;
#endif
#ifdef CONFIG_USER_NETFLASH_SHA256
		case 'N':
			check_opt.dosha256sum = 0;
			break;
#endif
		case 'n':
			/* No checksum implies no version */
			check_opt.dochecksum = 0;
			check_opt.doversion = 0;
			check_opt.dohardwareversion = 0;
			check_opt.doremoveversion = 0;
			break;
		case 'o':
			offset = strtol(optarg, NULL, 0);
			break;
		case 'a':
			dobootcfg = 0;
			break;
		case 'R':
			dofilesave = 1;
			dofileautoname = 0;
			dobootcfg = 1;
			stop_early = 1;
			rdev = optarg;
			break;
		case 'r':
			dofilesave = 0;
			dofileautoname = 0;
			stop_early = 0;
			dobootcfg = 0;
			rdev = optarg;
			break;
		case 't':
			dothrow = 1;
			break;
		case 'u':
			dounlock++;
			break;
		case 'v':
			notice("version %s", version);
			exit(0);
#ifdef CONFIG_USER_NETFLASH_DUAL_IMAGES
		case 'U':
			dobootpart = get_bootpart();
			if (dobootpart == 'a') {
				dobootpart = 'b';
				rdev = CONFIG_USER_NETFLASH_DUAL_IMAGES_B;
			} else {
				dobootpart = 'a';
				rdev = CONFIG_USER_NETFLASH_DUAL_IMAGES_A;
			}
			break;
#endif
#ifdef CONFIG_USER_NETFLASH_DECOMPRESS
		case 'z':
			doinflate = 1;
			break;
#endif
#ifdef CONFIG_USER_NETFLASH_SETSRC
		case 'I':
			srcaddr = optarg;
			break;
#endif
#ifdef CONFIG_USER_NETFLASH_WATCHDOG
		case 'w':
			watchdog = 0;
			break;
#endif
		case 'h':
		case '?':
			usage(0);
			break;
		case 0:
#ifdef CONFIG_USER_NETFLASH_CRYPTO_V3
			if (strcmp(longopts[longindex].name, "ca") == 0) {
				check_opt.ca = optarg;
			} else if (strcmp(longopts[longindex].name, "crl") == 0) {
				check_opt.crl = optarg;
			} else
#endif
			{
				notice("unhandled option: --%s",
						longopts[longindex].name);
			}
			break;
		}
	}

	doflashingrootfs = flashing_rootfs(rdev);

#ifdef CONFIG_USER_NETFLASH_DUAL_IMAGES
	if (dobootpart) {
		if (doflashingrootfs) {
			error("current root filesystem is new boot partition");
			exit(BAD_BOOTPART);
		}
	}

	/* Treat images not under /dev/ as normal files. */
	if (strncmp(rdev, "/dev/", strlen("/dev/")))
		dofilesave = 1;
#endif

	/*
	 * for /dev/flash/image* we want to stop early unless the user
	 * has told us not to (-S).  This allows us to preserve logd info
	 *
	 * So we override the default of not stopping early for /dev/flash/image
	 */
	if (strncmp(rdev, "/dev/flash/image", 16) == 0) {
		if (nostop_early == 0)
			stop_early = 1;
	}

	if ((nfd = fopen("/dev/null", "rw")) == NULL) {
		error("open(/dev/null) failed: %s", strerror(errno));
		exit(NO_DEV_NULL);
	}

	if (!docgi) {
		if (optind == (argc - 1)) {
			srvname = NULL;
			filename = argv[optind];
		} else if (optind == (argc - 2)) {
			srvname = argv[optind++];
			filename = argv[optind];
		} else {
			usage(1);
		}
	}

	if (delay > 0) {
		/* sleep the required time */
		notice("waiting %d seconds before updating flash...",delay);
		sleep(delay);
	}

	/*
	 *	Need to do any real FTP setup early, before killing processes
	 *	(and this losing association with the controlling tty).
	 */
	if (doftp) {
		if (ftpconnect(srvname)) {
			error("ftpconnect failed");
			exit(FTP_CONNECT_FAIL);
		}
	}

	/* CGI code has already called kill_processes_partial() */
	if (dokill && !docgi) {
		if (dokillpartial) {
			kill_processes_partial();
		} else {
			kill_processes(console);
			kill_processes_run = 1;
		}
	}

	/*
	 * Open the flash device and allocate a segment sized block.
	 * This is the largest block we need to allocate, so we do
	 * it first to try to avoid fragmentation effects.
	 */
	if (dopreserve && (strcmp(rdev, "/dev/flash/image") == 0))
		preserveconfig = 1;

	/*
	 * If we are writing to a real flash device then we open it now,
	 * and do the sizing checks. If writing to a regular file we hold
	 * off until we have the image (so we can do filename/directory name
	 * completion if required.
	 */
	if (dofilesave) {
		program_file_init();
	} else {
		rd = program_flash_open(rdev);
	}

	if (offset < 0) {
		error("offset is less than zero");
		exit(BAD_OFFSET);
	}
	if (offset >= devsize) {
		error("offset is greater than device size (%lld)", devsize);
		exit(BAD_OFFSET);
	}

	sgdata = malloc(sgsize);
	if (!sgdata) {
		error("Insufficient memory for image!");
		exit(NO_MEMORY);
	}

	if (checkimage || checkblank) {
		check_buf = malloc(sgsize);
		if (!check_buf) {
			error("Insufficient memory for check buffer!");
			exit(NO_MEMORY);
		}
	}

	/*
	 * Fetch file into memory buffers. Exactly how depends on the exact
	 * load method. Support for tftp, http and local file currently.
	 */
	time_start();
	if (!docgi) {
		if (srvname) {
			if (doftp)
				ftpfetch(srvname, filename);
			else
				tftpfetch(srvname, filename, blocksize);
		} else if (filefetch(filename) < 0) {
				error("failed to find %s", filename);
				exit(NO_IMAGE);
		}
		printf("\r%dK\n", fb_len() / 1024); fflush(stdout);
		fflush(stdout);
		sleep(1);
	}

	if (fb_len() == 0) {
		error("failed to load new image");
		exit(NO_IMAGE);
	}

	if (!docgi) {
		notice("got \"%s\", length=%ld", filename, fb_len());
	}

#if defined(CONFIG_USER_NETFLASH_ATECC508A)
	check_crypto_atmel();
#endif

#if defined(CONFIG_USER_NETFLASH_CRYPTO_V3)
	check_crypto_v3(&check_opt);
#elif defined(CONFIG_USER_NETFLASH_CRYPTO_V2)
	check_crypto_v2(&check_opt);
#elif defined(CONFIG_USER_NETFLASH_CRYPTO)
	check_crypto_v1(&check_opt);
#else
	check(&check_opt);
#endif

#ifdef CONFIG_USER_NETFLASH_DECOMPRESS
	image_length = check_decompression();
#else
	image_length = fb_len();
#endif

	if (dofilesave) {
		struct statfs fs;

		if (dofileautoname)
			rdev = program_file_autoname(filename);

		rd = program_file_open(rdev, doflashingrootfs);

		if (fstatfs(rd, &fs) == -1) {
			error("Cannot determine available space: %d", errno);
			exit(BAD_OPEN_FLASH);
		}

		if ((image_length + fs.f_bsize - 1) / fs.f_bsize > fs.f_bfree) {
			error("image too large for FLASH device (size=%lu)",
					fs.f_bsize * fs.f_bfree);
			exit(IMAGE_TOO_BIG);
		}

		/*
		 * We fake out the file size here so that the percentage
		 * display looks correct as output.
		 */
		devsize = image_length;
	}

	/* Check image that we fetched will actually fit in the FLASH device. */
	if (image_length > devsize - offset) {
		error("image too large for FLASH device (size=%lld)",
			devsize - offset);
		exit(IMAGE_TOO_BIG);
	}

	if (dothrow) {
		notice("the image is good.");
		exit(IMAGE_GOOD);
	}
#if defined(CONFIG_USER_NETFLASH_WITH_CGI) && !defined(RECOVER_PROGRAM)
	if (docgi) {
		/* let's let our parent know it's ok. */
		kill(getppid(), SIGUSR1);
	}
#endif

	if (doflashingrootfs) {
#if defined(CONFIG_USER_NETFLASH_WITH_CGI) && !defined(RECOVER_PROGRAM)
		/* Wait for netflash (parent) to write out data and exit */
		if (docgi) {
			waitprocpid(getppid(), MAX_WAIT_NETFLASH_FLUSH);
		}
#endif

		/*
		 * Our filesystem is live, so we MUST kill processes if we
		 * haven't done it already.
		 */
		notice("flashing root filesystem, kill is forced");
		if (!kill_processes_run) {
			kill_processes(console);
		}

		/* A new filesystem means we must reboot */
		doreboot = 1;

		/*
		 * If we are flashing root then unmount everything now.
		 * If not flashing root then we can defer the unmountall
		 * until after flashing (and ready to reboot).
		 */
		umount_all();
	}

#ifdef CONFIG_PROP_LOGD_LOGD
	log_upgrade(rdev);
#endif

#ifdef CONFIG_JFFS_FS
	/* Stop the JFFS garbage collector */
	killprocname("jffs_gcd", SIGSTOP);
#endif
#ifdef CONFIG_JFFS2_FS
	/* Stop the JFFS2 garbage collector */
	killprocname("jffs2_gcd_mtd1", SIGSTOP);
#endif

	/*
	 * Program the FLASH device.
	 */
	fflush(stdout);
	sleep(1);
	notice("programming %s %s",
		(dofilesave ? "file image" : "FLASH device"), rdev);

	program_flash(rd, image_length, devsize, sgdata, sgsize);

	if (dobootcfg) {
		if (exitstatus) {
			notice("Refusing to update bootcfg due to exit status %d",
					exitstatus);
		} else {
			update_bootcfg(rdev);
		}
	}
#ifdef CONFIG_USER_NETFLASH_DUAL_IMAGES
	if (dobootpart && set_bootpart()) {
		error("error setting boot partition");
		exit(BOOTPART_FAIL);
	}
#endif
	if (dofilesave) {
		fclose(nfd);
		close(rd);

		sync();
		sleep(2);
		if (doreboot) {
			remount_disk_ro();
			sleep(1);
		}
		flush_disk_cache();
		sleep(1);
	}
	if (doreboot) {
#ifdef CONFIG_USER_NETFLASH_WATCHDOG
		killprocname("watchdog", SIGKILL);
#endif
		if (!doflashingrootfs)
			umount_all();
		sync();
		usleep(1000000);
		raw_reboot(0xfee1dead, 672274793, 0x01234567);
	}

	return exitstatus;
}
/****************************************************************************/
