#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/vfs.h>

#include <linux/autoconf.h>
#include <linux/version.h>
#include <config/autoconf.h>
#include <linux/major.h>
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
#if defined(CONFIG_MTD) || defined(CONFIG_MTD_MODULES)
#include <linux/jffs2.h>
#endif
#if defined(CONFIG_NFTL_RW) && !defined(NFTL_MAJOR)
 #define NFTL_MAJOR 93
#endif
#if defined(CONFIG_IDE) || defined(CONFIG_SCSI)
#include <linux/hdreg.h>
#endif
#ifdef CONFIG_LEDMAN
#include <linux/ledman.h>
#endif

#include "program.h"
#include "decompress.h"
#include "exit_codes.h"
#include "fileblock.h"
#include "util.h"
#include "versioning.h"

/****************************************************************************/

int sgsize;
long long devsize;

int offset;		/* Offset to start writing at */
int exitstatus;
int dolock, dounlock;	/* do we lock/unlock segments as we go */
int checkimage;		/* Compare with current flash contents */
int checkblank;		/* Do not erase if already blank */
unsigned char *check_buf;
int preserveconfig;	/* Preserve special bits of flash such as config fs */
int preserve;		/* Preserve and portions of flash not written to */
int stop_early;		/* stop at end of input data, do not write full dev. */
int dojffs2;		/* Write the jffs2 magic to unused segments */

/****************************************************************************/

static unsigned int writesize = 512;
static unsigned int flashtype;
static void (* program_segment)(int rd, unsigned char *sgdata,
		int sgpos, int sglength, int sgsize);

/****************************************************************************/

static int get_segment(int rd, unsigned char *sgdata, int sgpos, int sgsize)
{
	int sglength;
	int sgoffset;

	if (offset > sgpos)
		sgoffset = offset - sgpos;
	else
		sgoffset = 0;

	/* XXX: preserve case could be optimized to read less */
	if (preserve || sgoffset) {
		if (lseek(rd, sgpos, SEEK_SET) != sgpos) {
			error("lseek(%x) failed\n", sgpos);
			exit(BAD_SEG_SEEK);
		} else if (read(rd, sgdata, preserve ? sgsize : sgoffset) < 0) {
			error("read() failed, pos=%x, errno=%d\n",
					sgpos, errno);
			exit(BAD_SEG_READ);
		}
	}

	sgpos -= offset - sgoffset;
	sgdata += sgoffset;
	sgsize -= sgoffset;

#ifdef CONFIG_USER_NETFLASH_DECOMPRESS
	if (doinflate) {
		sglength = decompress(sgdata, sgsize);
	} else
#endif
	{
		sglength = fb_read(sgdata, sgsize);
	}

	if (sglength !=0) {
		if (preserve)
			sglength = sgsize;
		sglength += sgoffset;
	}

	return sglength;
}

static void check_segment(int rd, unsigned char *sgdata, int sgpos, int sglength)
{
	if (lseek(rd, sgpos, SEEK_SET) != sgpos) {
		error("lseek(%x) failed", sgpos);
		exitstatus = BAD_SEG_SEEK;
	} else if (read(rd, check_buf, sglength) < 0) {
		error("read failed, pos=%x, errno=%d",
				sgpos, errno);
		exitstatus = BAD_SEG_READ;
	} else if (memcmp(sgdata, check_buf, sglength) != 0) {
		int i;
		error("check failed, pos=%x", sgpos);
		for (i = 0; i < sglength; i++) {
			if (sgdata[i] != check_buf[i])
				printf("%x(%x,%x) ", sgpos + i,
						sgdata[i] & 0xff,
						check_buf[i] & 0xff);
		}
		printf("\n");
		exitstatus = BAD_SEG_CHECK;
	}
}

#if defined(CONFIG_MTD) || defined(CONFIG_MTD_MODULES)
static int erase_segment(int rd, erase_info_t *ei)
{
	if (checkblank) {
		int i, blank;

		if (read(rd, check_buf, ei->length) != ei->length) {
			error("pre segment read(%x) failed", ei->start);
			return -1;
		}
		if (lseek(rd, ei->start, SEEK_SET) != ei->start) {
			error("lseek(%x) failed", ei->start);
			return -1;
		}

		for (blank = 1, i = 0; (i < ei->length); i++) {
			if (check_buf[i] != 0xff) {
				blank = 0;
				break;
			}
		}

		if (blank)
			return 0;
	}

	if (ioctl(rd, MEMERASE, ei) < 0) {
		error("ioctl(MEMERASE) failed, errno=%d", errno);
		return -1;
	}

	return 0;
}

static void program_mtd_segment(int rd, unsigned char *sgdata,
		int sgpos, int sglength, int sgsize)
{
	erase_info_t erase_info;
	int pos;

	/* Unlock the segment to be reprogrammed.  */
	if (dounlock) {
		erase_info.start = sgpos;
		erase_info.length = sgsize;
		/* Don't bother checking for failure */
		ioctl(rd, MEMUNLOCK, &erase_info);
	}

	erase_info.start = sgpos;
	erase_info.length = sgsize;
	if (lseek(rd, sgpos, SEEK_SET) != sgpos) {
		error("lseek(%x) failed", sgpos);
		exitstatus = BAD_SEG_SEEK;
	} else if (erase_segment(rd, &erase_info) < 0) {
		exitstatus = ERASE_FAIL;
	} else if (sglength > 0) {
		if (lseek(rd, sgpos, SEEK_SET) != sgpos) {
			error("lseek(%x) failed", sgpos);
			exitstatus = BAD_SEG_SEEK;
		} else {
			for (pos = sgpos; (sglength >= writesize); ) {
				if (write(rd, sgdata, writesize) == -1) {
					error("write() failed, pos=0x%x, "
						"errno=%d", pos, errno);
					exitstatus = BAD_SEG_WRITE;
				}
				pos += writesize;
				sgdata += writesize;
				sglength -= writesize;
			}
			/*
			 * If there is a remainder, then still write a block
			 * size chunk, but preserve what is already there.
			 */
			if (sglength > 0) {
				char buf[writesize];

				if (lseek(rd, pos, SEEK_SET) != pos) {
					error("lseek(0x%x) failed", pos);
					exitstatus = BAD_SEG_SEEK;
				} else if (read(rd, buf, writesize) == -1) {
					error("read() failed, pos=0x%x, "
						"errno=%d", pos, errno);
					exitstatus = BAD_SEG_READ;
				} else if (lseek(rd, pos, SEEK_SET) != pos) {
					error("lseek(0x%x) failed", pos);
					exitstatus = BAD_SEG_SEEK;
				} else {
					memcpy(buf, sgdata, sglength);
					if (write(rd, buf, writesize) == -1) {
						error("write() failed, "
							"pos=0x%x, errno=%d",
							pos, errno);
						exitstatus = BAD_SEG_WRITE;
					}
				}
			}
		}
	} else if (dojffs2) {
		static struct jffs2_unknown_node marker = {
			JFFS2_MAGIC_BITMASK,
			JFFS2_NODETYPE_CLEANMARKER,
			sizeof(struct jffs2_unknown_node),
#if __BYTE_ORDER == __BIG_ENDIAN
			0xf060dc98
#else
			0xe41eb0b1
#endif
		};

		if (lseek(rd, sgpos, SEEK_SET) != sgpos) {
			error("lseek(%x) failed", sgpos);
			exitstatus = BAD_SEG_SEEK;
		} else if (write(rd, &marker, sizeof(marker)) < 0) {
			error("write() failed, pos=%x, "
				"errno=%d", sgpos, errno);
			exitstatus = BAD_SEG_WRITE;
		}
	}

	if (dolock) {
		erase_info.start = sgpos;
		erase_info.length = sgsize;
		if (ioctl(rd, MEMLOCK, &erase_info) < 0) {
			error("ioctl(MEMLOCK) failed, errno=%d", errno);
			exitstatus = ERASE_FAIL;
		}
	}
}
#elif defined(CONFIG_BLK_DEV_BLKMEM)
static void program_blkmem_segment(int rd, unsigned char *sgdata, int sgpos,
	int sglength, int sgsize)
{
	char buf[128];
	struct blkmem_program_t *prog = (struct blkmem_program_t *)buf;

	prog->magic1 = BMPROGRAM_MAGIC_1;
	prog->magic2 = BMPROGRAM_MAGIC_2;
	prog->reset = 0;
	prog->blocks = 1;
	prog->block[0].data = sgdata;
	prog->block[0].pos = sgpos;
	prog->block[0].length = sglength;
	prog->block[0].magic3 = BMPROGRAM_MAGIC_3;
	if (ioctl(rd, BMPROGRAM, prog) != 0) {
		error("ioctl(BMPROGRAM) failed, errno=%d", errno);
		exitstatus = BAD_SEG_WRITE;
	}
}
#endif

static int check_badblock_segment(int rd, int sgpos, int sgsize)
{
#if defined(CONFIG_MTD) || defined(CONFIG_MTD_MODULES)
	/* Check if bad block, we will skip if it is */
	if (flashtype == MTD_NANDFLASH) {
		loff_t offs = sgpos;
		int rc;

		rc = ioctl(rd, MEMGETBADBLOCK, &offs);
		if (rc < 0) {
			error("bad block check failed, pos=0x%x, errno=%d",
				sgpos, errno);
			exitstatus = ERASE_FAIL;
		}
		if (rc) {
			/* Bad block */
			return 1;
		}
	}
#endif
	return 0;
}

static void program_generic_segment(int rd, unsigned char *sgdata,
		int sgpos, int sglength, int sgsize)
{
	if (!stop_early && sglength < sgsize) {
		memset(sgdata + sglength, 0xff, sgsize - sglength);
		sglength = sgsize;
	}

	if (sglength > 0) {
		if (lseek(rd, sgpos, SEEK_SET) != sgpos) {
			error("lseek(%x) failed", sgpos);
			exitstatus = BAD_SEG_SEEK;
		} else if (write(rd, sgdata, sglength) < 0) {
			error("write() failed, pos=%x, "
					"errno=%d", sgpos, errno);
			exitstatus = BAD_SEG_WRITE;
		} else if (fdatasync(rd) < 0) {
			error("fdatasync() failed, pos=%x, "
					"errno=%d", sgpos, errno);
			exitstatus = BAD_SEG_CHECK;
		}
	}
}


void program_flash(int rd, unsigned long image_length, long long devsize, unsigned char *sgdata, int sgsize)
{
	int sgpos, sglength;
	unsigned long long total;
#ifdef CONFIG_LEDMAN
	int ledmancount = 0;
#endif

#ifdef CONFIG_LEDMAN
	ledman_cmd(LEDMAN_CMD_ALT_ON, LEDMAN_NVRAM_1);
	ledman_cmd(LEDMAN_CMD_ALT_ON, LEDMAN_NVRAM_2);
#ifdef LEDMAN_NVRAM_ALL
	ledman_cmd(LEDMAN_CMD_ALT_ON, LEDMAN_NVRAM_ALL);
	ledman_cmd(LEDMAN_CMD_OFF | LEDMAN_CMD_ALTBIT, LEDMAN_NVRAM_ALL);
#endif
#endif

	time_start();

	/* Round the image size up to the segment size */
	if (stop_early) {
		total = (image_length + sgsize - 1) & ~(sgsize - 1);
	} else {
		total = devsize;
	}

	/* Write the data one segment at a time */
	fb_seek_set(0);
	sgpos = offset - (offset % sgsize);

	for (; sgpos < devsize; sgpos += sgsize) {
		int show_progress = 0;

		if (check_badblock_segment(rd, sgpos, sgsize))
			continue;

		sglength = get_segment(rd, sgdata, sgpos, sgsize);

		if (stop_early && sglength <= 0)
			break;

		if (time_elapsed() >= time_rate_100) {
			show_progress = 1;
			time_start();
		}

		if (checkimage) {
			check_segment(rd, sgdata, sgpos, sglength);
		}
		else
#if defined(CONFIG_MTD_NETtel)
		if (!preserveconfig || sgpos < 0xe0000 || sgpos >= 0x100000) {
#endif
			program_segment(rd, sgdata, sgpos, sglength, sgsize);

#ifdef CONFIG_LEDMAN
			if (show_progress) {
				ledman_cmd(LEDMAN_CMD_OFF | LEDMAN_CMD_ALTBIT,
					ledmancount ? LEDMAN_NVRAM_1 : LEDMAN_NVRAM_2);
				ledman_cmd(LEDMAN_CMD_ON | LEDMAN_CMD_ALTBIT,
					ledmancount ? LEDMAN_NVRAM_2 : LEDMAN_NVRAM_1);
				ledmancount = (ledmancount + 1) & 1;
			}
#endif
#ifdef CONFIG_USER_NETFLASH_WATCHDOG
			if (watchdog_fd >= 0)
				write(watchdog_fd, "\0", 1); 
#endif

#if defined(CONFIG_MTD_NETtel)
		} /* if (!preserveconfig || ...) */
#endif
		if (show_progress) {
			printf("\r%5dK %3lld%%", (sgpos + sgsize) / 1024, (sgpos + sgsize) / (total / 100));
			fflush(stdout);
		}
	}
	printf("\r%5dK %3lld%%\n", sgpos / 1024, sgpos/(total/100));

	fflush(stdout);
#ifdef CONFIG_LEDMAN
	ledman_cmd(LEDMAN_CMD_ALT_OFF, LEDMAN_NVRAM_1);
	ledman_cmd(LEDMAN_CMD_ALT_OFF, LEDMAN_NVRAM_2);
#ifdef LEDMAN_NVRAM_ALL
	ledman_cmd(LEDMAN_CMD_ALT_OFF, LEDMAN_NVRAM_ALL);
#endif
#endif

	/* Put the flash back in read mode, some old boot loaders don't */
	lseek(rd, 0, SEEK_SET);
	read(rd, sgdata, 1);
}

int program_flash_open(const char *rdev)
{
	struct stat stat_rdev;
	int rd;

	rd = open(rdev, O_RDWR);
	if (rd < 0) {
		error("open(%s) failed: %s", rdev, strerror(errno));
		exit(BAD_OPEN_FLASH);
	}

	if (stat(rdev, &stat_rdev) != 0) {
		error("stat(%s) failed: %s", rdev, strerror(errno));
		exit(BAD_OPEN_FLASH);
	} else if (S_ISBLK(stat_rdev.st_mode)) {
#ifdef CONFIG_NFTL_RW
		if (major(stat_rdev.st_rdev) == NFTL_MAJOR) {
			unsigned long l;

			program_segment = program_generic_segment;
			preserveconfig = dolock = dounlock = 0;
			if (ioctl(rd, BLKGETSIZE, &l) < 0) {
				error("ioctl(BLKGETSIZE) failed, "
					"errno=%d", errno);
				exit(BAD_OPEN_FLASH);
			}
			/* Sectors are always 512 bytes */
			devsize = l * 512;
			/*
			 * Use a larger sgsize for efficiency, but it
			 * must divide evenly into devsize.
			 */
			for (sgsize = 512; sgsize < 64*1024; sgsize <<= 1)
				if (devsize & sgsize)
					break;
		}
#endif
#if defined(CONFIG_IDE) || defined(CONFIG_SCSI)
		if ((major(stat_rdev.st_rdev) == IDE0_MAJOR) ||
		    (major(stat_rdev.st_rdev) == IDE1_MAJOR) ||
		    (major(stat_rdev.st_rdev) == IDE2_MAJOR) ||
		    (major(stat_rdev.st_rdev) == IDE3_MAJOR) ||
		    (major(stat_rdev.st_rdev) == SCSI_DISK0_MAJOR) ||
		    (major(stat_rdev.st_rdev) == SCSI_DISK1_MAJOR) ||
		    (major(stat_rdev.st_rdev) == SCSI_DISK2_MAJOR) ||
		    (major(stat_rdev.st_rdev) == SCSI_DISK3_MAJOR)) {
			long size;

			program_segment = program_generic_segment;
			preserveconfig = dolock = dounlock = 0;
			if (ioctl(rd, BLKGETSIZE, &size) < 0) {
				error("ioctl(BLKGETSIZE) failed, errno=%d",
					errno);
				exit(BAD_OPEN_FLASH);
			}
			devsize = size * 512LL;
			/*
			 * Use a larger sgsize for efficiency, but it
			 * must divide evenly into devsize.
			 */
			for (sgsize = 512; sgsize < 64*1024; sgsize <<= 1)
				if (devsize & sgsize)
					break;
		}
#endif
	}
	if (!program_segment) {
#if defined(CONFIG_MTD) || defined(CONFIG_MTD_MODULES)
		mtd_info_t mtd_info, rootfs_info;

		program_segment = program_mtd_segment;

		if (ioctl(rd, MEMGETINFO, &mtd_info) < 0) {
			error("ioctl(MEMGETINFO) failed, errno=%d",
				errno);
			exit(BAD_OPEN_FLASH);
		}
		devsize = mtd_info.size;
		sgsize = mtd_info.erasesize;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
		/* if writesize is not set (will never happen),  or 1,
		 * use the default of 512, 1 makes reflashing impossibly slow */
		if (mtd_info.writesize > 1)
			writesize = mtd_info.writesize;
#endif
		flashtype = mtd_info.type;

		/*
		 * NETtel/x86 boards that boot direct from INTEL FLASH
		 * also have a boot sector at the top of the FLASH.
		 * When programming complete images we need to not
		 * overwrite this.
		 */
		if (preserveconfig) {
			int tmpfd;

			if ((tmpfd = open("/dev/flash/rootfs", O_RDONLY)) > 0) {
				if (ioctl(tmpfd, MEMGETINFO, &rootfs_info) >= 0) {
					if (rootfs_info.size & 0x000fffff) {
						devsize = devsize - (0x00100000 -
								(rootfs_info.size & 0x000fffff));
					}
				}
				close(tmpfd);
			}
		}
#elif defined(CONFIG_BLK_DEV_BLKMEM)
        	int old_devsize = 0;

		program_segment = program_blkmem_segment;

		if (ioctl(rd, BMGETSIZEB, &old_devsize) != 0) {
			error("ioctl(BMGETSIZEB) failed, errno=%d",
				errno);
			exit(BAD_OPEN_FLASH);
		} else {
			devsize = old_devsize;
		}
		if (ioctl(rd, BMSGSIZE, &sgsize) != 0) {
			error("ioctl(BMSGSIZE) failed, errno=%d",
				errno);
			exit(BAD_OPEN_FLASH);
		}
#endif
	}

	return rd;
}

void program_file_init(void)
{
	/* Should be checking space available on filesystem */
	sgsize = 128000;
	devsize = 0x0fffffff;
	program_segment = program_generic_segment;
}

char *program_file_autoname(const char *filename)
{
	char *version, *product, *p;
	char *rdev;

	if (filename == NULL) {
		rdev = malloc(MAX_VERSION_SIZE + 64);	/* for now, let's just fudge numbers */
	} else {
		rdev = malloc(MAX_VERSION_SIZE + strlen(filename) + 16);
	}
	version	= malloc(MAX_VERSION_SIZE + 16);
	product = malloc(MAX_PRODUCT_SIZE + 16);

	if (rdev == NULL || version == NULL || product == NULL) {
		exit(NO_MEMORY);
	}

	if (strlen(imageVersion))
		strncpy(version, imageVersion, MAX_VERSION_SIZE);
	else
		sprintf(version, "UnknownVersion");

	sprintf(rdev, "/sda1/%s", version);
	mkdir(rdev, 0777);

	if (filename == NULL) {
		int i, rv;
		char *newFile, suffix[3];
		struct stat buf;

		if (strlen(imageProductName))
			strncpy(product, imageProductName, MAX_PRODUCT_SIZE);
		else
			sprintf(product, "UnknownProduct");

		newFile = malloc(MAX_VERSION_SIZE + 64);
		if (newFile == NULL)
			exit(NO_MEMORY);

		suffix[0] = 0;
		for (i = 1; i < 10; i++) {
			if (i > 1) sprintf(suffix, "-%d", i);
			sprintf(newFile, "/sda1/%s/%s_%s%s.sgu", version, product, version, suffix);
			rv = stat(newFile, &buf);
			if (rv) {
				if (errno == ENOENT)
					break;
				error("unknown file error trying to stat %s.", newFile);
				exit(BAD_FILE);
			}
		}
		if (i >= 10) {
			syslog(LOG_ERR, "netflash: can't find a suitable file to write to (gave up after %s).", newFile);
			error("can't find a suitable file to write to (gave up after %s).", newFile);
			exit(BAD_FILE);
		}

		strcpy(rdev, newFile);
	} else {
		/* already have the file, so any '/'s can't
		 * be at the end, i.e. filename isn't a dir */
		p = strrchr(filename, '/');
		if (p) {
			sprintf(rdev, "/sda1/%s/%s", version, p+1);
		} else {
			sprintf(rdev, "/sda1/%s/%s", version, filename);
		}
	}

	return rdev;
}

int program_file_open(const char *rdev, int dobackup)
{
	struct stat st;
	char *bakupfile;
	int rd;

	/*
	 * If we are running with a rootfs in this image (so over-
	 * writing the current running system image file) then it is
	 * not enough to just unlink it. That will leave the fs in a
	 * dirty state on reboot. The dangling unlink will mean we
	 * cannot remount the flash fs as read-only just before we
	 * reboot. Move this file to a backup file before over-writing.
	 */
	if (dobackup && stat(rdev, &st) == 0) {
		bakupfile = malloc(strlen(rdev) + 8);
		sprintf(bakupfile, "%s.bak", rdev);
		rename(rdev, bakupfile);
	}

	if ((rd = open(rdev, O_RDWR|O_CREAT|O_TRUNC, 0400)) < 0) {
		error("open(%s) failed: %s", rdev, strerror(errno));
		exit(BAD_OPEN_FLASH);
	}

	return rd;
}
