/*
 * kwbootflash -- tool to program NAND flash boot loaders on Kirkwood
 *
 * (C) Copyright 2016, Greg Ungerer <greg.ungerer@accelerated.com>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 or (at your option) any
 * later version.
 *
 * This file is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <mtd/mtd-abi.h>
#include "ecc_kw.h"

#define DEVICE "/dev/flash/boot"

/*
 * Vital mtd/flash device info.
 * These are retrieved from the real flash device through MTD.
 */
unsigned int mtdsize;
unsigned int erasesize;
unsigned int pagesize;
unsigned int oobsize;
unsigned int useecc;

/*
 * Vital file/boot image info.
 * "image" is an in-memory copy of the boot loader binary (which may be
 * read from flash or from a file). It contains both the raw data and the
 * OOB data interleaved on flash pages.
 */
unsigned char *image;
unsigned int imagememsize;
unsigned int imagefilesize;

/*
 * Simple trace macro. I don't want anything too compilicated. We just need
 * to be able to get some feedback on what is happening (file stats, etc)
 * when running on a target. Hopefully won't need them for real on the box,
 * but better to leave it all in now for easier debugging later.
 */
int verbose;
#define trace	if (verbose) printf
#define trace2	if (verbose > 1) printf

void hexdump(void *buf, int len)
{
	unsigned char *p = buf;
	int i;

	for (i = 0; i < len; i++, p++) {
		if ((i % 16) == 0) printf("0x%08x:  ", i);
		printf("%02x ", *p);
		if (((i + 1) % 16) == 0) printf("\n");
	}
	if (i % 16)
		printf("\n");
}

void getmtdinfo(char *name)
{
	struct mtd_info_user info;
	int fd;

	fd = open(name, O_RDONLY);
	if (fd < 0) {
		printf("ERROR: cannot open flash device\"%s\"\n", name);
		exit(2);
	}

	if (ioctl(fd, MEMGETINFO, &info) < 0) {
		printf("ERROR: cannot get flash specifics, errno=%d\n", errno);
		exit(2);
	}

	mtdsize = info.size;
	erasesize = info.erasesize;
	pagesize = info.writesize;
	oobsize = info.oobsize;

	trace("kwbootflash: flash size=%d\n", mtdsize);
	trace("kwbootflash: flash erasesize=%d\n", erasesize);
	trace("kwbootflash: flash pagesize=%d\n", pagesize);
	trace("kwbootflash: flash oobsize=%d\n", oobsize);

	/* These are not strictly enforced, just what we expect */
	if (mtdsize != 2*1024*1024)
		printf("WARNING: flash partition size %d != 2Mb\n", mtdsize);
	if (pagesize != 2048)
		printf("WARNING: page size %d != 2048\n", pagesize);
	if (oobsize != 64)
		printf("WARNING: OOB size %d != 64\n", oobsize);

	close(fd);
}

/*
 * Determine if the current flash pages use Marvell Reed-Soloman ECC.
 * We look at the OOB data and see how big the ECC bytes are. The Linux
 * hamming code (1-bit correcting) only uses 24 bytes to store ECC.
 * The Linux BCH code (4-bit correcting) uses 28 bytes to store ECC.
 * The Marvell kirkwood reed-solomon ECC uses 40 bytes.
 */
int checkecc(char *name)
{
	unsigned char buf[64];
	struct mtd_oob_buf oob;
	loff_t offs;
	int fd, i;

	trace("kwbootflash: checking ECC type on flash\n");
	if (oobsize != 64) {
		printf("ERROR: unsupported flash type for ECC check\n");
		exit(2);
	}

	fd = open(name, O_RDONLY);
	if (fd < 0) {
		printf("ERROR: cannot open flash device\"%s\"\n", name);
		exit(2);
	}

	for (offs = 0; offs < mtdsize; offs += pagesize) {
		if (ioctl(fd, MEMGETBADBLOCK, &offs) == 0)
			break;
	}

	oob.start = offs;
	oob.length = 64;
	oob.ptr = buf;
	if (ioctl(fd, MEMREADOOB, &oob) < 0) {
		printf("ERROR: failed to read flash OOB, "
			"offset=0x%llx, errno=%d\n", offs, errno);
		exit(2);
	}
	close(fd);

	for (i = 24; i < 36; i++) {
		if (buf[i] != 0xff) {
			trace("kwbootflash: looks like RS ECC in use\n");
			return 0;
		}
	}
	for (i = 36; i < 40; i++) {
		if (buf[i] != 0xff) {
			trace("kwbootflash: looks like BCH ECC in use\n");
			return 1;
		}
	}

	trace("kwbootflash: looks like Hamming ECC in use\n");
	return 1;
}

/*
 * Determine size of image file. If we are reading from the existing flash
 * then use mtd info size information instead.
 */
unsigned int getsize(char *name, int fd)
{
	struct stat st;

	if (fstat(fd, &st) < 0) {
		printf("ERROR: cannot stat image \"%s\"\n", name);
		exit(3);
	}
	trace("kwbootflash: file image size=%d\n", st.st_size);
	if (st.st_size == 0) {
		/* Assume flash if size s zero */
		st.st_size = mtdsize;
		trace("kwbootflash: using flash image size=%d\n", st.st_size);
	}
	if (st.st_size == 0) {
		printf("ERROR: cannot determine image size?\n");
		exit(3);
	}
	return st.st_size;
}

/*
 * Take the image size and pad it out for whole number of pages, and also
 * enough space for the OOB data for each page.
 */
unsigned int padsize(unsigned int size)
{
	size = (size + (pagesize - 1)) & ~(pagesize - 1);
	size += ((size / pagesize) * oobsize);
	return size;;
}

/*
 * Read the source image file into a single memory chunk. Leave enough room
 * for the OOB (and thus ECC) data to fit in as well.
 */
void readimage(char *name)
{
	unsigned char *bp;
	loff_t offs;
	int fd, i;

	fd = open(name, O_RDONLY);
	if (fd < 0) {
		printf("ERROR: cannot open image file \"%s\"\n", name);
		exit(3);
	}

	/* Disable flash page ECC checks, ignored for file */
	if (useecc == 0)
		ioctl(fd, MTDFILEMODE, MTD_FILE_MODE_RAW);

	imagefilesize = getsize(name, fd);
	trace("kwbootflash: image file size=%d\n", imagefilesize);
	imagememsize = padsize(imagefilesize);
	trace("kwbootflash: image memory size=%d\n", imagememsize);
	image = malloc(imagememsize);
	if (image == NULL) {
		printf("ERROR: failed to allocate %d bytes\n", imagememsize);
		exit(3);
	}
	memset(image, 0xff, imagememsize);

	bp = image;
	for (i = 0, offs = 0; i < imagefilesize; offs += pagesize) {
		if (ioctl(fd, MEMGETBADBLOCK, &offs) > 0) {
			trace2("kwbootflash: skipping bad block 0x%llx\n", offs);
			continue;
		}
		lseek(fd, offs, SEEK_SET);
		if (read(fd, bp, pagesize) < 0) {
			printf("ERROR: failed to read image offset=0x%llx, "
				"errno=%d\n", offs, errno);
			exit(3);
		}
		bp += pagesize + oobsize;
		i += pagesize;
	}

	close(fd);
}

/*
 * If we read the image from flash then most likely it contains a lot of
 * empty (and erased) blocks at the end. Trim them off so we just program
 * the image and no more.
 */
void trim(void)
{
	unsigned int pageoob, i;

	for (i = imagememsize - 1; i > 0; i--) {
		if (image[i] != 0xff)
			break;
	}

	pageoob = pagesize + oobsize;
	imagememsize = ((i + pageoob - 1) / pageoob) * pageoob;
	imagefilesize = (imagememsize / pageoob) * pagesize;
	trace("kwbootflash: trimed file size=%d\n", imagefilesize);
	trace("kwbootflash: trimed memory size=%d\n", imagememsize);
}

/*
 * For each page in the image generate reed-solomon ECC bits - and put them
 * in the OOB data section at the correct location. This is currently hard
 * coded for a 2k page size. Need to fix that.
 */
void mkecc(void)
{
	unsigned char *pp, *oobp;
	int i;

	trace("kwbootflash: generating reed-solomon ECC bits\n");

	pp = image;
	for (i = 0; i < imagememsize; i += pagesize + oobsize) {
		oobp = pp + pagesize;
		nand_calculate_ecc_kw(pp, oobp + 24);
		nand_calculate_ecc_kw(pp + 512, oobp + 34);
		nand_calculate_ecc_kw(pp + 1024, oobp + 44);
		nand_calculate_ecc_kw(pp + 1536, oobp + 54);
		pp += pagesize + oobsize;
	}
}

void writeimage(char *name)
{
	int fd;

	trace("kwbootflash: writing image to output file\n");

	fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd < 0) {
		printf("ERROR: cannot open output file \"%s\"\n", name);
		exit(5);
	}
	if (write(fd, image, imagememsize) < imagememsize) {
		printf("ERROR: failed to write output file \"%s\"\n", name);
		exit(5);
	}
	close(fd);
}

/*
 * Merge the unused OOB bytes into our in-memory image. We don't want to lose
 * the OOB bytes that are not updated (all the non-ECC bytes) - they may
 * contain bad blocking or other useful info.
 */
void mergeoob(char *name)
{
	struct mtd_oob_buf oob;
	loff_t offs;
	int fd, i, j;

	trace("kwbootflash: merging OOB bytes\n");

	fd = open(name, O_RDONLY);
	if (fd < 0) {
		printf("ERROR: cannot open flash device file \"%s\"\n", name);
		exit(6);
	}

	j = pagesize;
	for (i = 0, offs = 0; i < imagefilesize; offs += pagesize) {
		if (ioctl(fd, MEMGETBADBLOCK, &offs) > 0)
			continue;
		oob.start = offs;
		oob.length = 24;
		oob.ptr = &image[j];
		if (ioctl(fd, MEMREADOOB, &oob) < 0) {
			printf("ERROR: failed to read flash OOB, "
				"offset=0x%llx, errno=%d\n", offs, errno);
			exit(6);
		}
		j += pagesize + oobsize;
		i += pagesize;
	}

	close(fd);
}

/*
 * Check if the image we have in memory is the same as the image on flash.
 * We check the actual data. You may be comparing if an image boot loader
 * file is the sames as actually in flash, or you may want to check if the
 * ECC bits in flash are actually the correct reed-solomon ECC bits.
 */
void checkimage(char *name)
{
	struct mtd_oob_buf oob;
	unsigned char buf[pagesize];
	loff_t offs;
	int fd, i, j;

	trace("kwbootflash: checkimg image and ECC\n");

	fd = open(name, O_RDONLY);
	if (fd < 0) {
		printf("ERROR: cannot open flash device file \"%s\"\n", name);
		exit(7);
	}

	/* Disable flash page ECC checks */
	ioctl(fd, MTDFILEMODE, MTD_FILE_MODE_RAW);

	for (i = 0, j = 0, offs = 0; i < imagefilesize; offs += pagesize) {
		if (ioctl(fd, MEMGETBADBLOCK, &offs) > 0) {
			trace2("kwbootflash: skipping bad block 0x%llx\n", offs);
			continue;
		}

		lseek(fd, offs, SEEK_SET);
		if (read(fd, buf, pagesize) != pagesize) {
			printf("ERROR: failed to read flash page, "
				"offset=0x%llx, errno=%d\n", offs, errno);
			exit(7);
		}
		if (memcmp(&image[j], buf, pagesize)) {
			printf("kwbootflash: image and flash data differ\n");
			if (verbose > 1) {
				printf("kwbootflash: image page offset=0x%llx "
					"data:\n", i);
				hexdump(&image[j], pagesize);
				printf("kwbootflash: flash page offset=0x%llx "
					"data:\n", offs);
				hexdump(buf, pagesize);
			}
			exit(10);
		}

		oob.start = offs;
		oob.length = oobsize;
		oob.ptr = buf;
		if (ioctl(fd, MEMREADOOB, &oob) < 0) {
			printf("ERROR: failed to read flash OOB, "
				"offset=0x%llx, errno=%d\n", offs, errno);
			exit(7);
		}
		if (memcmp(&image[j + pagesize], buf, oobsize)) {
			printf("kwbootflash: flash ECC not correct\n");
			if (verbose > 1) {
				printf("kwbootflash: image page offset=0x%llx "
					"OOB:\n", i);
				hexdump(&image[j + pagesize], oobsize);
				printf("kwbootflash: flash page offset=0x%llx "
					"OOB:\n", offs);
				hexdump(buf, oobsize);
			}
			
			exit(11);
		}

		j += pagesize + oobsize;
		i += pagesize;
	}

	printf("kwbootflash: image and ECC flash contents match\n");
	close(fd);
}

void flashimage(char *name)
{
	struct erase_info_user erase;
	struct mtd_oob_buf oob;
	loff_t offs;
	int fd, i, j;

	fd = open(name, O_RDWR);
	if (fd < 0) {
		printf("ERROR: cannot open flash device file \"%s\"\n", name);
		exit(8);
	}

	/* Disable flash page ECC generation */
	ioctl(fd, MTDFILEMODE, MTD_FILE_MODE_RAW);

	trace("kwbootflash: erasing flash\n");
	for (offs = 0; offs < mtdsize; offs += erasesize) {
		if (ioctl(fd, MEMGETBADBLOCK, &offs) > 0) {
			trace2("kwbootflash: skipping erase of bad "
				"block 0x%llx\n", offs);
			continue;
		}
		trace2("kwbootflash: flash erasing 0x%llx\n", offs);
		erase.start = offs;
		erase.length = erasesize;
		if (ioctl(fd, MEMERASE, &erase) < 0) {
			printf("ERROR: failed to erase flash, "
				"offset=0x%llx errno=%d\n", offs, errno);
			exit(8);
		}
	}

	trace("kwbootflash: programming image into flash\n");
	for (i = 0, j = 0, offs = 0; i < imagefilesize; offs += pagesize) {
		if (offs >= mtdsize) {
			trace("kwbootflash: not enough room for all image?\n");
			exit(8);
		}
		if (ioctl(fd, MEMGETBADBLOCK, &offs) > 0) {
			trace2("kwbootflash: skipping bad block 0x%llx\n", offs);
			continue;
		}

		trace2("kwbootflash: flash write page 0x%llx\n", offs);
		lseek(fd, offs, SEEK_SET);
		if (write(fd, &image[j], pagesize) != pagesize) {
			printf("ERROR: failed to write flash page, "
				"offset=0x%llx, errno=%d\n", offs, errno);
			exit(8);
		}
		j += pagesize;

		trace2("kwbootflash: flash write OOB 0x%llx\n", offs);
		oob.start = offs;
		oob.length = oobsize;
		oob.ptr = &image[j];
		if (ioctl(fd, MEMWRITEOOB, &oob) < 0) {
			printf("ERROR: failed to write flash OOB, "
				"offset=0x%llx, errno=%d\n", offs, errno);
			exit(8);
		}
		j += oobsize;

		i += pagesize;
	}

	close(fd);
}

#if 0
int dobiterror;
int dobadblock;

/*
 * This code only used during develeopment testing. It intentionally
 * introduces bit errors in the data *AFTER* ECC calculation. This lets
 * us test the reader side (typically CPU internal boot loader) and makes
 * sure it can correct bit errors.
 */
void setbiterror(unsigned int page, unsigned int offset, unsigned int bit)
{
	unsigned int addr;

	addr = page * pagesize + offset;
	printf("    PAGE=%d OFFSET=%d ADDR=0x%x  --->  ", page, offset, addr);
	addr = page * (pagesize + oobsize) + offset;
	printf("was=0x%02x ", image[addr]);
	image[addr] ^= bit;
	printf("now=0x%02x\n", image[addr]);
}

void injecterror(void)
{
	int nr, i;

	nr = 0;
	printf("kwbootflash: INJECTING BIT ERRORS!\n");

	/* Put lots of single bit errors on lots of pages */
	for (i = 1; i < 100; i++, nr++)
		setbiterror(i, i - 1, (0x1 << (i & 0x7)));

	if (dobiterror > 1) {
		/* Put 2-bit errors on some pages */
		for (i = 2; i < 100; i += 2, nr += 2)
			setbiterror(i, (i * 7 + i) % pagesize, 0x28);
	}
	if (dobiterror > 2) {
		/* Put yet another bit error on some pages */
		for (i = 3; i < 100; i += 3, nr += 1)
			setbiterror(i, i + 17 , (0x1 << ((i + 1) & 0x7)));
	}
	if (dobiterror > 3) {
		/* Put yet another bit error on some pages */
		for (i = 3; i < 100; i += 3, nr += 1)
			setbiterror(i, (i + 151) % pagesize , (0x1 << ((i + 2) & 0x7)));
	}

	printf("kwbootflash: injected %d bit errors\n", nr);
}

/*
 * Mark some blocks as bad using the invalid block byte of the OOB area.
 * This is used to test the block block skipping for read/check/write.
 */
void setbadblock(int fd, int blknr)
{
	unsigned char buf[oobsize];
	struct mtd_oob_buf oob;
	unsigned int addr;

	addr = blknr * erasesize;

	printf("    BLOCK=%d ADDR=0x%x  --->  ", blknr, addr);

	oob.start = addr;
	oob.length = oobsize;
	oob.ptr = buf;
	if (ioctl(fd, MEMREADOOB, &oob) < 0) {
		printf("ERROR: failed to read flash OOB, "
			"offset=0x%x, errno=%d\n", addr, errno);
		exit(6);
	}
	printf("was=0x%02x ", buf[0]);

	buf[0] = 0;

	oob.start = addr;
	oob.length = oobsize;
	oob.ptr = buf;
	if (ioctl(fd, MEMWRITEOOB, &oob) < 0) {
		printf("ERROR: failed to write flash OOB, "
			"offset=0x%x, errno=%d\n", addr, errno);
		exit(8);
	}
	printf("now=0x%02x\n", buf[0]);
}

void injectbadblock(char *name)
{
	struct erase_info_user erase;
	loff_t offs;
	int fd;

	printf("kwbooflash: injecting bad blocks\n");

	fd = open(name, O_RDWR);
	if (fd < 0) {
		printf("ERROR: cannot open flash device file \"%s\"\n", name);
		exit(6);
	}

	printf("kwbootflash: erasing ALL\n");
	for (offs = 0; offs < mtdsize; offs += erasesize) {
		erase.start = offs;
		erase.length = erasesize;
		if (ioctl(fd, MEMERASE, &erase) < 0) {
			printf("ERROR: failed to erase flash, "
				"offset=0x%llx errno=%d\n", offs, errno);
		}
	}

	setbadblock(fd, 1);
	setbadblock(fd, 3);
	setbadblock(fd, 4);
	setbadblock(fd, 5);

	close(fd);
}
#endif

void usage(int err)
{
	printf("usage: kwbootflash [-hvcw] [-f file] [-o output] "
		"[-d flash-device]\n\n"
		"\t-h          print this help\n"
		"\t-v          verbose trace output\n"
		"\t-c          check current flash image and ECC\n"
		"\t-w          write the new image and ECC to flash\n"
		"\t-u          only update flash if fixing ECC\n"
		"\t-f file     image file to read (default is flash)\n"
		"\t-o output   write new image to output file)\n"
		"\t-d device   flash device to program\n\n");
	exit(err);
}

int main(int argc, char *argv[])
{
	char *filename = DEVICE;
	char *flashname = DEVICE;
	char *outputname = NULL;
	int checkflash, updateflash, writeflash, writefile;
	int c;

	checkflash = 0;
	updateflash = 0;
	writeflash = 0;
	writefile = 0;

	while ((c = getopt(argc, argv, "hvcwuf:o:d:")) != -1) {
		switch (c) {
		case 'v':
			verbose++;
			break;
		case 'c':
			checkflash = 1;
			break;
		case 'w':
			writeflash = 1;
			break;
		case 'u':
			updateflash = 1;
			break;
		case 'o':
			writefile = 1;
			outputname = optarg;
			break;
		case 'f':
			filename = optarg;
			break;
		case 'd':
			flashname = optarg;
			break;
		case 'h':
			usage(0);
		default:
			printf("ERROR: unkown arg '%c'\n\n", c);
			usage(1);
		}
	}
	if (optind < argc) {
		printf("ERROR: too many arguments\n\n");
		usage(1);
	}

	trace("kwbootflash: starting\n");
	trace("kwbootflash: using image file \"%s\"\n", filename);
	trace("kwbootflash: using output file \"%s\"\n", outputname);
	trace("kwbootflash: using flash device \"%s\"\n", flashname);

	getmtdinfo(flashname);
	if (updateflash) {
		if (checkecc(flashname)) {
			printf("kwbootflash: updating old flash contents\n");
			writeflash = 1;
			useecc = 1;
		} else {
			trace("kwbootflash: not updating flash contents\n");
			return 0;
		}
	}
	readimage(filename);
	trim();
	mkecc();
	mergeoob(flashname);
	if (writefile)
		writeimage(outputname);
	if (checkflash)
		checkimage(flashname);
	if (writeflash)
		flashimage(flashname);

	trace("kwbootflash: done\n");
	return 0;
}

