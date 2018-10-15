#ifndef __UPLOAD_CGI_H__
#define __UPLOAD_CGI_H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <wait.h>

#include "include/linux/autoconf.h"		/* !!! for CONFIG_MTD_KERNEL_PART_SIZ  !!! */
                                        /*   CONFIG_RT2880_ROOTFS_IN_FLASH */
                                        /*   CONFIG_RT2880_ROOTFS_IN_RAM   */

extern unsigned long crc32(unsigned long, const unsigned char *, unsigned int);

/*
 *  Uboot image header format
 *  (ripped from mkimage.c/image.h)
 */
#define IH_MAGIC    0x27051956
#define IH_NMLEN    32
typedef struct image_header {
    uint32_t    ih_magic;   /* Image Header Magic Number    */
    uint32_t    ih_hcrc;    /* Image Header CRC Checksum    */
    uint32_t    ih_time;    /* Image Creation Timestamp */
    uint32_t    ih_size;    /* Image Data Size      */
    uint32_t    ih_load;    /* Data  Load  Address      */
    uint32_t    ih_ep;      /* Entry Point Address      */
    uint32_t    ih_dcrc;    /* Image Data CRC Checksum  */
    uint8_t     ih_os;      /* Operating System     */
    uint8_t     ih_arch;    /* CPU architecture     */
    uint8_t     ih_type;    /* Image Type           */
    uint8_t     ih_comp;    /* Compression Type     */
    uint8_t     ih_name[IH_NMLEN];  /* Image Name       */
} image_header_t;


inline void write_flash_kernel_version(char *file, int offset)
{
#if 0
	unsigned char buf[128];
	char cmd[128];
	image_header_t *hdr;
	FILE *fp = fopen(file, "r");
	if(!fp){
		fprintf(stderr, "%s error\n", __FILE__);
		return;
	}

	fseek(fp, offset, SEEK_SET);
	if( fread(buf, 1, sizeof(buf), fp) != sizeof(buf))
		return;
	fclose(fp);
	hdr = (image_header_t *)buf;
	hdr->ih_name[IH_NMLEN] = '\0';
	fprintf(stderr, "hdr->name = %s\n", hdr->ih_name);

	sprintf(cmd, "nvram_set 2860 Expect_Firmware \"%s\"", hdr->ih_name);
	system(cmd);
#endif
	char cmd[512];
	char buf[512];
	FILE *fp = fopen("/proc/version", "r");
	fgets(buf, 512, fp);
	fclose(fp);

	sprintf(cmd, "nvram_set 2860 old_firmware \"%s\"", buf);
	system(cmd);
}

inline unsigned int getMTDPartSize(char *part)
{
	char buf[128], name[32], size[32], dev[32], erase[32];
	unsigned int result=0;
	FILE *fp = fopen("/proc/mtd", "r");
	if(!fp){
		fprintf(stderr, "mtd support not enable?");
		return 0;
	}
	while(fgets(buf, sizeof(buf), fp)){
		sscanf(buf, "%s %s %s %s", dev, size, erase, name);
		if(!strcmp(name, part)){
			result = strtol(size, NULL, 16);
			break;
		}
	}
	fclose(fp);
	return result;
}


inline int mtd_write_firmware(char *filename, int offset, int len)
{
#if 0 //DAVIDM
	char cmd[512];
	int status = 0;
#if defined (CONFIG_RT2880_FLASH_8M) || defined (CONFIG_RT2880_FLASH_16M)
    /* workaround: erase 8k sector by myself instead of mtd_erase */
    /* this is for bottom 8M NOR flash only */
    snprintf(cmd, sizeof(cmd), "/bin/flash -f 0x400000 -l 0x40ffff");
    system(cmd);
#endif

#if defined (CONFIG_RT2880_ROOTFS_IN_RAM)
    snprintf(cmd, sizeof(cmd), "/bin/mtd_write -o %d -l %d write %s Kernel", offset, len, filename);
    status = system(cmd);
#elif defined (CONFIG_RT2880_ROOTFS_IN_FLASH)
  #ifdef CONFIG_ROOTFS_IN_FLASH_NO_PADDING
    snprintf(cmd, sizeof(cmd), "/bin/mtd_write -o %d -l %d write %s Kernel_RootFS", offset, len, filename);
    status = system(cmd);
  #else
    snprintf(cmd, sizeof(cmd), "/bin/mtd_write -o %d -l %d write %s Kernel", offset,  CONFIG_MTD_KERNEL_PART_SIZ, filename);
    status = system(cmd);
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		return -1;
    if(len > CONFIG_MTD_KERNEL_PART_SIZ ){
		snprintf(cmd, sizeof(cmd), "/bin/mtd_write -o %d -l %d write %s RootFS", offset + CONFIG_MTD_KERNEL_PART_SIZ, len - CONFIG_MTD_KERNEL_PART_SIZ, filename);
		status = system(cmd);
    }
  #endif
#else
    fprintf(stderr, "goahead: no CONFIG_RT2880_ROOTFS defined!");
#endif
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		return -1;
#endif
    return 0;
}

inline void mtd_write_bootloader(char *filename, int offset, int len)
{
#if 0 //DAVIDM
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "/bin/mtd_write -o %d -l %d write %s Bootloader", offset, len, filename);
	printf("write bootloader");
	system(cmd);
    return ;
#endif
}

#endif /* __UPLOAD_CGI_H__ */
