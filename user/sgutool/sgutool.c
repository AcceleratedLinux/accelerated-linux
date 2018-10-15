/****************************************************************************/

/*
 *    sgutool.c -- tools that read and manipulate SGU images
 *                       binary image (SGU)
 *
 */

/****************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <endian.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <mtd/mtd-user.h>
#define MTD_CHAR_MAJOR 90

#ifndef CONFIG_USER_NETFLASH_CRYPTO_V2
#define	CONFIG_USER_NETFLASH_CRYPTO_V2	1
#endif
#include "../../user/netflash/crypto.h"
#include "image.h"

/****************************************************************************/

#define	PROGNAME	"sgutool"

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define	BSWAP(x)	(x)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define BSWAP(x)	(((x) << 24) | (((x) & 0xff00) << 8) | \
			 (((x) >> 8) & 0xff00) | ((x) >> 24))
#else
#error "Endian not defined?"
#endif

/****************************************************************************/

int verbose;
int trace;

char *sgu;
unsigned int sgusize;
unsigned int totsize;
unsigned int imgsize;
unsigned int fssize;
unsigned int fstype;
unsigned int kernelofs;
unsigned int kernelend;
unsigned int kernelsize;

#define	FS_UNKNOWN	0
#define	FS_ROMFS	1
#define	FS_CRAMFS	2
#define	FS_SQUASHFS	3

char *fs_names[] = {
	[FS_UNKNOWN] = "UNKNOWN",
	[FS_ROMFS] = "ROMfs",
	[FS_CRAMFS] = "CRAMfs",
	[FS_SQUASHFS] = "SQUASHfs",
};

/*
 * These taken from the netflash version.h (though the Makefile script
 * generation of these doesn't actually impose any limits on their size.
 * Modern image contain this info twice, once on the compatability
 * trailer, and also before the signed crypt header.
 */
#define MAX_VENDOR_SIZE		256
#define MAX_PRODUCT_SIZE	256
#define MAX_VERSION_SIZE	32

struct info {
	char vendor[MAX_VENDOR_SIZE];
	unsigned int vendorofs;
	unsigned int vendorsize;

	char product[MAX_PRODUCT_SIZE];
	unsigned int productofs;
	unsigned int productsize;

	char version[MAX_VERSION_SIZE];
	unsigned int versionofs;
	unsigned int versionsize;
};

struct info trailer;
struct info internal;

unsigned int checksum, checksummed;
unsigned int checksumofs, checksumsize;

int imgsigned;
int imgsummed;
int hashtype, hashlength;;

#define	HASH_NONE	0
#define HASH_UNKNOWN	1
#define	HASH_MD5	2
#define	HASH_SHA256	3

char *hash_names[] = {
	[HASH_NONE] = "NONE",
	[HASH_UNKNOWN] = "UNKNOWN",
	[HASH_MD5] = "MD5",
	[HASH_SHA256] = "SHA256",
};

unsigned char crypthash[SHA256_DIGEST_LENGTH];

struct little_header postcrypthdr;
unsigned int postcrypthdrofs, postcrypthdrsize;

struct header crypthdr;
unsigned int crypthdrofs, crypthdrsize;
void *crypthdrbuf, *decrypthdrbuf;
int cryptsigngood;

int publickeygot;
RSA *publickey;

/****************************************************************************/

static ssize_t xread_partial(int fd, void *buf, size_t len)
{
	ssize_t n;

	while (1) {
		n = read(fd, buf, len);
		if ((n < 0) && (errno == EAGAIN || errno == EINTR))
			continue;
		return n;
	}
}

static ssize_t xread(int fd, void *buf, size_t len)
{
	ssize_t n;

	while (len) {
		n = xread_partial(fd, buf, len);
		if (n < 0)
			return n;
		if (n == 0) {
			errno = EIO;
			return -1;
		}
		buf += n;
		len -= n;
	}

	return 0;
}

static ssize_t xread_offset(int fd, void *buf, size_t offset, size_t len)
{
	ssize_t n;

	n = lseek(fd, offset, SEEK_SET);
	if (n < 0)
		return n;
	n = xread(fd, buf, len);
	if (n < 0)
		return n;
	return 0;
}

static ssize_t xwrite_partial(int fd, const void *buf, size_t len)
{
	ssize_t n;

	while (1) {
		n = write(fd, buf, len);
		if ((n < 0) && (errno == EAGAIN || errno == EINTR))
			continue;
		return n;
	}
}

static ssize_t xwrite(int fd, const void *buf, size_t len)
{
	ssize_t n;

	while (len) {
		n = xwrite_partial(fd, buf, len);
		if (n < 0)
			return n;
		if (n == 0) {
			errno = EIO;
			return -1;
		}
		buf += n;
		len -= n;
	}

	return 0;
}

#define BUF_SIZE 8192
static ssize_t xcopy(int ofd, int ifd, size_t offset, size_t len)
{
	unsigned char buf[BUF_SIZE];
	ssize_t n;

	n = lseek(ifd, offset, SEEK_SET);
	if (n < 0)
		return n;

	while (len) {
		n = xread_partial(ifd, buf, len < BUF_SIZE ? len : BUF_SIZE);
		if (n < 0)
			return n;
		len -= n;
		n = xwrite(ofd, buf, n);
		if (n < 0)
			return n;
	}

	return 0;
}

/****************************************************************************/

unsigned int getsgusize(int ifd)
{
	struct stat st;

	if (fstat(ifd, &st) < 0) {
		printf("ERROR: stat image %s, %m\n", sgu);
		return 0;
	}

	if (S_ISCHR(st.st_mode) && major(st.st_rdev) == MTD_CHAR_MAJOR) {
		mtd_info_t mtd_info;

		if (ioctl(ifd, MEMGETINFO, &mtd_info) < 0) {
			printf("ERROR: MEMGETINFO image %s, %m\n", sgu);
			return 0;
		}

		return mtd_info.size;
	}

	return st.st_size;
}

/****************************************************************************/

/*
 * Determine the size of the image filesystem.
 * Can be either ROMfs or CRAmfs.
 */

unsigned int getfssize(int ifd)
{
	unsigned int magic, size;
	unsigned char ubuf[0x30];
	char *cbuf = (char *)ubuf;

	if (xread_offset(ifd, ubuf, 0, sizeof(ubuf)) < 0) {
		printf("ERROR: failed to read filesystem header: %m\n");
		return 0;
	}

	fstype = FS_UNKNOWN;

	if (trace > 1) {
		printf("magic: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
			ubuf[0], ubuf[1], ubuf[2], ubuf[3],
			ubuf[4], ubuf[5], ubuf[6], ubuf[7]);
	}

	if (memcmp(&cbuf[0], "-rom1fs-", 8) == 0) {
		fstype = FS_ROMFS;
                size = (ubuf[8] << 24) | (ubuf[9] << 16) | (ubuf[10] << 8) |
			ubuf[11];
                size = ((size + 1023) & ~1023);
		if (trace)
			printf("ROMfs size=%d\n", size);
		return size;
	}

	memcpy(&magic, &ubuf[0], sizeof(magic));
        if (magic == BSWAP(0x28cd3d45)) {
		fstype = FS_CRAMFS;
                memcpy(&size, &ubuf[4], sizeof(size));
                size = BSWAP(size);
		if ((cbuf[8] == 's') && (cbuf[9] == 'q') &&
		    (cbuf[10] == 's') && (cbuf[11] == 'h'))
			fstype = FS_SQUASHFS;
		if (trace)
			printf("%s size=%d\n", fs_names[fstype], size);
		return size;
        }

	if (memcmp(&cbuf[0], "hsqs", 4) == 0) {
		fstype = FS_SQUASHFS;
		size = (ubuf[0x2b] << 24) | (ubuf[0x2a] << 16) |
			(ubuf[0x29] << 8) | ubuf[0x28];
		size = (size + 0xfff) & 0xfffff000;
		if (trace)
			printf("%s size=%d\n", fs_names[fstype], size);
		return size;
	}

	printf("ERROR: unknown filesystem type?\n");
	return 0;
}

/****************************************************************************/

unsigned int getkernelsize(int ifd, unsigned int kernelofs)
{
	struct image_header image;

	if (xread_offset(ifd, &image, kernelofs, sizeof(image)) < 0)
		return 0;

	if (ntohl(image.ih_magic) == IH_MAGIC) {
		unsigned int size = ntohl(image.ih_size);
		if (trace)
			printf("uImage type=%d size=%d\n", image.ih_type, size);
		return sizeof(image) + size;
	}

	/* We'll work out the size later */
	return 0;
}

/****************************************************************************/

int extractkernel(int ifd, char *zImage)
{
	int ofd;

	if ((ofd = open(zImage, O_WRONLY | O_CREAT | O_TRUNC, 0660)) < 0) {
		printf("ERROR: cannot open(%s), %s\n", zImage, strerror(errno));
		return -1;
	}

	if (xcopy(ofd, ifd, kernelofs, kernelsize) < 0) {
		printf("ERROR: cannot write(%s), %s\n", zImage, strerror(errno));
		return -1;
	}

	if (trace)
		printf("zImage size=%d\n", kernelsize);

	close(ofd);
	return 0;
}

/****************************************************************************/

int extractfs(int ifd, char *fsimg)
{
	int ofd;

	if ((ofd = open(fsimg, O_WRONLY | O_CREAT | O_TRUNC, 0660)) < 0) {
		printf("ERROR: cannot open(%s), %s\n", fsimg, strerror(errno));
		return -1;
	}

	if (xcopy(ofd, ifd, 0, fssize) < 0) {
		printf("ERROR: cannot write(%s), %s\n", fsimg, strerror(errno));
		return -1;
	}

	if (trace)
		printf("filesystem size=%d\n", fssize);

	close(ofd);
	return 0;
}

/****************************************************************************/

int extractall(int ifd, char *allimg)
{
	int ofd;

	if ((ofd = open(allimg, O_WRONLY | O_CREAT | O_TRUNC, 0660)) < 0) {
		printf("ERROR: cannot open(%s), %s\n", allimg, strerror(errno));
		return -1;
	}

	if (xcopy(ofd, ifd, 0, totsize) < 0) {
		printf("ERROR: cannot write(%s), %s\n", allimg, strerror(errno));
		return -1;
	}

	if (trace)
		printf("image size=%d\n", totsize);

	close(ofd);
	return 0;
}

/****************************************************************************/

int extractchecksum(int ifd)
{
	if (imgsize - kernelend < 4)
		return 0;

	checksumsize = 4;
	checksumofs = imgsize - checksumsize;

	if (xread_offset(ifd, &checksum, checksumofs, checksumsize) < 0) {
		printf("ERROR: failed to read filesystem header: %m\n");
		return 0;
	}

	checksum = ntohl(checksum);
	if (checksum > 0xffff) {
		if (trace)
			printf("image has no final checksum\n");
		return 0;
	}

	imgsize = checksumofs;
	if (totsize < imgsize)
		totsize = imgsize;

	if (trace)
		printf("image checksum=0x%x\n", checksum);
	return 1;
}

/****************************************************************************/

unsigned int findbackwardstring(int ifd, unsigned int end, unsigned int max)
{
	unsigned int start, base;
	char ibuf[max + 1];

	/* Normally this would never be true, but lets be careful */
	if (max > end)
		max = end;
	base = end - max;

	if (xread_offset(ifd, ibuf, base, max + 1) < 0) {
		printf("ERROR: failed to read string buffer: %m\n");
		return 0;
	}

	for (start = end; (start > base); start--) {
		if (ibuf[start - base] == 0)
			return start;
		if (!isprint(ibuf[start - base]))
			return 0;
	}
	return 0;
}

int extractinfo(int ifd, struct info *ip)
{
	unsigned int end;

	end = imgsize - 1;

	/* Extract product name string */
	ip->productofs = findbackwardstring(ifd, end, MAX_PRODUCT_SIZE);
	if (ip->productofs == 0) {
		printf("WARNING: sgu doesn't appear to be versioned?\n");
		return -1;
	}
	ip->productsize = end - ip->productofs;
	if (ip->productsize > 0) {
		ip->productofs++;
		xread_offset(ifd, ip->product, ip->productofs, end - ip->productofs + 1);
		if (trace)
			printf("image product name=`%s`\n", ip->product);
		end = ip->productofs - 2;
	}

	/* Extract vendor name string */
	ip->vendorofs = findbackwardstring(ifd, end, MAX_VENDOR_SIZE);
	ip->vendorsize = end - ip->vendorofs;
	if (ip->vendorsize > 0) {
		ip->vendorofs++;
		xread_offset(ifd, ip->vendor, ip->vendorofs, end - ip->vendorofs + 1);
		if (trace)
			printf("image vendor name=`%s`\n", ip->vendor);
		end = ip->vendorofs - 2;
	}

	/* Extract version string */
	ip->versionofs = findbackwardstring(ifd, end, MAX_VERSION_SIZE);
	ip->versionsize = end - ip->versionofs;
	if (ip->versionsize > 0) {
		ip->versionofs++;
		xread_offset(ifd, ip->version, ip->versionofs, end - ip->versionofs + 1);
		if (trace)
			printf("image version=`%s`\n", ip->version);
		end = ip->versionofs - 2;
	}

	imgsize = end + 1;
	if (totsize < imgsize)
		totsize = imgsize;
	return 0;
}

/****************************************************************************/

int sha256image(int ifd)
{
	SHA256_CTX ctx;
	unsigned char buf[BUF_SIZE];
	ssize_t n;
	size_t len;

	SHA256_Init(&ctx);

	n = lseek(ifd, 0, SEEK_SET);
	if (n < 0)
		return n;

	len = imgsize;
	while (len) {
		n = xread_partial(ifd, buf, len < BUF_SIZE ? len : BUF_SIZE);
		if (n < 0)
			return n;
		SHA256_Update(&ctx, buf, n);
		len -= n;
	}

	SHA256_Final(&crypthash[0], &ctx);
	return 0;
}

/****************************************************************************/

int md5image(int ifd)
{
	MD5_CTX ctx;
	unsigned char buf[BUF_SIZE];
	ssize_t n;
	size_t len;

	MD5_Init(&ctx);

	n = lseek(ifd, 0, SEEK_SET);
	if (n < 0)
		return n;

	len = imgsize;
	while (len) {
		n = xread_partial(ifd, buf, len < BUF_SIZE ? len : BUF_SIZE);
		if (n < 0)
			return n;
		MD5_Update(&ctx, buf, n);
		len -= n;
	}

	MD5_Final(&crypthash[0], &ctx);
	return 0;
}

/****************************************************************************/

int extractcrypto(int ifd, unsigned int end)
{
	int len;

	hashtype = HASH_NONE;

	if (end - kernelend < sizeof(postcrypthdr))
		return -1;

	/* Extract the small pre-header to verify we have a signature */
	if (xread_offset(ifd, &postcrypthdr, end - sizeof(postcrypthdr), sizeof(postcrypthdr)) < 0) {
		printf("ERROR: failed to read postcrypthdr: %m\n");
		return -1;
	}
	postcrypthdr.magic = ntohs(postcrypthdr.magic);
	postcrypthdr.hlen = ntohs(postcrypthdr.hlen);

	if (postcrypthdr.magic != LITTLE_CRYPTO_MAGIC)
		return -1;
	if (end - kernelend < sizeof(postcrypthdr) + postcrypthdr.hlen)
		return -1;

	postcrypthdrsize = sizeof(postcrypthdr);
	postcrypthdrofs = end - sizeof(postcrypthdr);
	end = postcrypthdrofs;
	imgsigned = 1;

	if (trace)
		printf("postcrypthdr offset=%d size=%d\n", postcrypthdrofs, postcrypthdrsize);

	crypthdrofs = end - postcrypthdr.hlen;
	crypthdrsize = postcrypthdr.hlen;
	crypthdrbuf = malloc(crypthdrsize);
	imgsize = crypthdrofs;
	if (totsize < imgsize)
		totsize = imgsize;

	if (trace)
		printf("crypthdr offset=%d size=%d\n", crypthdrofs, crypthdrsize);

	hashtype = HASH_UNKNOWN;

	/* If we didn't get the public key then we are done. */
	if (! publickeygot)
		return 0;

	/* Default to SHA256 - we might fixup after reading crypt header */
	hashtype = HASH_SHA256;
	hashlength = SHA256_DIGEST_LENGTH;

	if (xread_offset(ifd, crypthdrbuf, crypthdrofs, crypthdrsize) < 0) {
		printf("ERROR: failed to read crypthdr: %m\n");
		return -1;
	}

	decrypthdrbuf = malloc(crypthdrsize);
	len = RSA_public_decrypt(crypthdrsize, crypthdrbuf, decrypthdrbuf,
		publickey, RSA_PKCS1_PADDING);
	if (len < 0)
		return -1;
	if (len == 54) {
		/* Fixed size check for older MD5 hash header */
		hashtype = HASH_MD5;
		hashlength = MD5_DIGEST_LENGTH;
	} else if (len != sizeof(crypthdr)) {
		printf("WARNING: crypt header different size?\n");
		return -1;
	}

	if (trace)
		printf("signed image, hash type=%s\n", hash_names[hashtype]);

	memcpy(&crypthdr, decrypthdrbuf, sizeof(crypthdr));

	crypthdr.magic = ntohl(crypthdr.magic);
	if (crypthdr.magic != CRYPTO_MAGIC) {
		printf("WARNING: crypt header wrong magic 0x%8x "
			"(expected 0x%08x)?\n", crypthdr.magic, CRYPTO_MAGIC);
		return -1;
	}

	/* FIXME: add decryption support */

	if (crypthdr.padsize)
		imgsize -= crypthdr.padsize;

	if (hashtype == HASH_SHA256)
		sha256image(ifd);
	else if (hashtype == HASH_MD5)
		md5image(ifd);
	else
		return 0;

	/* Check the hash matches */
	if (memcmp(crypthash, crypthdr.hash, hashlength) != 0) {
		printf("WARNING: crypt hash does not match?\n");
		return -1;
	}

	cryptsigngood = 1;
	return 0;
}

/****************************************************************************/

unsigned int genchecksum(int ifd)
{
	unsigned char buf[BUF_SIZE];
	ssize_t i, n;
	size_t len;

	n = lseek(ifd, 0, SEEK_SET);
	if (n < 0)
		return n;

	len = checksumofs;
	while (len) {
		n = xread_partial(ifd, buf, len < BUF_SIZE ? len : BUF_SIZE);
		if (n < 0) {
			printf("ERROR: failed read for checksum: %m\n");
			return n;
		}
		for (i = 0; i < n; i++)
			checksummed += buf[i];
		len -= n;
	}

	checksummed = (checksummed & 0xffff) + (checksummed >> 16);
	checksummed = (checksummed & 0xffff) + (checksummed >> 16);

	if (trace) {
		printf("calculated checksum=0x%x (%s)\n", checksummed,
			(checksum == checksummed) ? "good" : "BAD");
	}
	return checksummed;
}

/****************************************************************************/

int loadpublickey(char *pubkeyfile)
{
	BIO *bin;
	if ((bin = BIO_new(BIO_s_file()))  == NULL) {
		if (trace)
			printf("WARNING: failed to allocate crypt BIO?\n");
		return 0;
	}
	if (BIO_read_filename(bin, pubkeyfile) <= 0) {
		if (trace)
			printf("WARNING: cannot read public key file, %s\n",
				pubkeyfile);
		return 0;
	}
	publickey = PEM_read_bio_RSA_PUBKEY(bin, NULL, NULL, NULL);
	if (publickey == NULL) {
		if (trace)
			printf("WARNING: cannot read public key, %s\n",
                                pubkeyfile);
                return 0;
        }

	return 1;
}

/****************************************************************************/

void printinfo(void)
{
	printf("SGU %s:\n", sgu);
	printf("    total size =\t%d bytes\n", totsize);
	printf("    filesystem size =\t%d bytes\n", fssize);
	printf("    kernel size =\t%d bytes\n", kernelsize);
	printf("    filesystem type =\t%s\n", fs_names[fstype]);

	if (imgsummed) {
		printf("    old checksum =\t0x%04x (%s)\n", checksum,
			(checksum == checksummed) ? "good" : "bad");
		if (checksum != checksummed) {
			printf("    calculat checksum =\t0x%04x\n",
				checksummed);
		}
		if (trailer.vendorsize)
			printf("    vendor string =\t%s\n", trailer.vendor);
		if (trailer.productsize)
			printf("    product string =\t%s\n", trailer.product);
		if (trailer.versionsize)
			printf("    version string =\t%s\n", trailer.version);
	}

	printf("    signed image =\t%s (%s)\n", (imgsigned) ? "yes" : "no",
		(publickeygot ? (cryptsigngood ? "good" : "bad") : "unverified"));

	if (imgsigned) {
		printf("    signature type =\t%s\n", hash_names[hashtype]);
		if (internal.vendorsize)
			printf("    signed vendor =\t%s\n", internal.vendor);
		if (internal.productsize)
			printf("    signed product =\t%s\n", internal.product);
		if (internal.versionsize)
			printf("    signed version =\t%s\n", internal.version);
	}
}

/****************************************************************************/

void usage(int rc)
{
	printf("Usage: %s [-h?tvVPRCcs] [-p <public.key>] [-k <zImage>] "
		"[-f <fs.bin>] [-a <image.bin>] <file.sgu>\n", PROGNAME);
	printf("\n\t-h?\t\tthis help\n"
		"\t-v\t\tverbose output\n"
		"\t-t\t\ttrace output\n"
		"\t-V\t\treport VENDOR type of image\n"
		"\t-P\t\treport PRODUCT type of image\n"
		"\t-R\t\treport VERSION stamped in image\n"
		"\t-C\t\treport CHECKSUM stamped in image\n"
		"\t-c\t\treport generated image checksum\n"
		"\t-s\t\treport checksum status (good/bad)\n"
		"\t-p <public.key>\tkey file to use (default /etc/publickey.pem)\n"
		"\t-k <zImage>\twrite out kernel to file <zImage>\n"
		"\t-f <fs.bin>\twrite out filesystem to file <fs.bin>\n"
		"\t-a <image.bin>\twrite all of image to file <image.bin>\n"
	);
	exit(rc);
}

/****************************************************************************/

int main(int argc, char *argv[])
{
	char *zImage, *fsimg, *allimg, *pubkeyfile;
	int ifd, c;
	int dokernel, dofilesystem, doall;
	int dovendor, doproduct, doversion;
	int dochecksum, dochecksummed, dochecksumstatus;

	dokernel = 0;
	dofilesystem = 0;
	doall = 0;
	dovendor = 0;
	doproduct = 0;
	doversion = 0;
	dochecksum = 0;
	dochecksummed = 0;
	dochecksumstatus = 0;
	pubkeyfile = "/etc/publickey.pem";
	zImage = "zImage";
	fsimg = "filesystem";

	while ((c = getopt(argc, argv, "?hvtVPRCck:f:a:p:")) >= 0) {
		switch (c) {
		case 'V':
			dovendor = 1;
			break;
		case 'P':
			doproduct = 1;
			break;
		case 'R':
			doversion = 1;
			break;
		case 'C':
			dochecksum = 1;
			break;
		case 'c':
			dochecksummed = 1;
			break;
		case 's':
			dochecksumstatus = 1;
			break;
		case 'p':
			pubkeyfile = optarg;
			break;
		case 'k':
			dokernel = 1;
			zImage = optarg;
			break;
		case 'f':
			dofilesystem = 1;
			fsimg = optarg;
			break;
		case 'a':
			doall = 1;
			allimg = optarg;
			break;
		case 't':
			trace++;
			break;
		case 'v':
			verbose++;
			break;
		case 'h':
		case '?':
			usage(0);
			break;
		default:
			usage(1);
			break;
		}
	}

	if (argc != (optind + 1))
		usage(1);
	sgu = argv[optind++];

	if (trace)
		printf("file=%s\n", sgu);

	if ((ifd = open(sgu, O_RDONLY)) < 0) {
		printf("ERROR: cannot open(%s), %s\n", sgu, strerror(errno));
		return 1;
	}

	imgsize = sgusize = getsgusize(ifd);

	if (trace)
		printf("size=%d bytes\n", sgusize);

	publickeygot = loadpublickey(pubkeyfile);

	fssize = getfssize(ifd);
	if (fssize == 0)
		return 1;
	if (fssize > imgsize) {
		printf("WARNING: filesystem (%d) larger than image %d)?\n",
			fssize, imgsize);
		return 1;
	}

	kernelofs = fssize;
	kernelsize = getkernelsize(ifd, kernelofs);
	kernelend = kernelofs + kernelsize;
	totsize = kernelend;

	/* Allow for crypto header before checksum */
	extractcrypto(ifd, imgsize);

	imgsummed = extractchecksum(ifd);
	if (imgsummed) {
		/* This image has an unsigned trailer */
		extractinfo(ifd, &trailer);
		genchecksum(ifd);
		if (checksum != checksummed) {
			printf("WARNING: bad checksum, file=0x%04x, "
				"calculated=0x%04x\n", checksum, checksummed);
		}
		/* and it optionally has a signed trailer */
		if (!imgsigned) {
			extractcrypto(ifd, imgsize);
			if (imgsigned)
				extractinfo(ifd, &internal);
		}
	}

	/* If we don't know the kernel size then it's whatever is left */
	if (!kernelsize) {
		kernelsize = imgsize - kernelofs;
		kernelend = imgsize;
		if (totsize < kernelend)
			totsize = kernelend;
	} else if (kernelend != imgsize) {
		printf("WARNING: unknown data after kernel (%d bytes)\n",
				imgsize - kernelend);
	}

	if (verbose)
		printinfo();
	if (dokernel)
		extractkernel(ifd, zImage);
	if (dofilesystem)
		extractfs(ifd, fsimg);
	if (doall)
		extractall(ifd, allimg);
	if (dovendor)
		printf("%s\n", trailer.vendor);
	if (doproduct)
		printf("%s\n", trailer.product);
	if (doversion)
		printf("%s\n", trailer.version);
	if (dochecksum)
		printf("0x%04x\n", checksum);
	if (dochecksummed)
		printf("0x%04x\n", checksummed);
	if (dochecksumstatus) {
		printf("%s\n", (checksum == checksummed) ? "good" : "bad");
		exit((checksum == checksummed) ? 0 : 1);
	}

	close(ifd);
	return 0;
}

/****************************************************************************/
