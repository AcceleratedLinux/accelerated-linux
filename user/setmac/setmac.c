/****************************************************************************/

/*
 *	setmac.c --  Set MAC addresses for eth devices from FLASH
 *
 *	(C) Copyright 2004, Greg Ungerer <gerg@snapgear.com>
 */

/****************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

/****************************************************************************/

/*
 *	Define the maxiumum number of ethernet devices we should try
 *	and configure. Also define the default number we try to configure.
 */
#define	MAXETHS		16
#define	DEFAULTETHS	16

#ifndef ETHPREFIX
#define ETHPREFIX "eth"
#endif

#ifndef SETMAC_PREFIX
#define SETMAC_PREFIX 0x00, 0x27, 0x04, 0x03, 0x02
#endif

/*
 *	Define the default flash device to use to get MAC addresses from.
 */
#define	DEFAULTFLASH	"/dev/flash/ethmac"

/****************************************************************************/

/*
 *	Define the table of default MAC addresses. What to use if we can't
 *	find any other good MAC addresses.
 */
unsigned char mactable[MAXETHS * 6] = {
	SETMAC_PREFIX, 0x01,
	SETMAC_PREFIX, 0x02,
	SETMAC_PREFIX, 0x03,
	SETMAC_PREFIX, 0x04,
	SETMAC_PREFIX, 0x05,
	SETMAC_PREFIX, 0x06,
	SETMAC_PREFIX, 0x07,
	SETMAC_PREFIX, 0x08,
	SETMAC_PREFIX, 0x09,
	SETMAC_PREFIX, 0x0a,
	SETMAC_PREFIX, 0x0b,
	SETMAC_PREFIX, 0x0c,
	SETMAC_PREFIX, 0x0d,
	SETMAC_PREFIX, 0x0e,
	SETMAC_PREFIX, 0x0f,
	SETMAC_PREFIX, 0x10,
};

int starteths = 0;
int numeths = DEFAULTETHS;
int debug = 0;
char *ethname[MAXETHS];

/****************************************************************************/

/*
 * parse a comma seperated list of interface names into our ethname array
 * for alternate device naming
 */

void process_interface_list(char *list)
{
	int i = 0;
	char *cp = strtok(list, " ,");;

	while (cp && i < MAXETHS) {
		ethname[i++] = cp;
		cp = strtok(NULL, " ,");
	}
}

/****************************************************************************/

/*
 *	Search for a mtd partition in /proc/mtd.
 *	Assumes that each line starts with the device name followed
 *	by a ':', and the partition name is enclosed by quotes.
 */
char *findmtddevice(char *mtdname)
{
	FILE *f;
	char buf[80];
	int found;
	static char device[80];
	char *p, *q;

	f = fopen("/proc/mtd", "r");
	if (!f) {
		perror("setmac: open /proc/mtd failed");
		return NULL;
	}

	found = 0;
	while (!found && fgets(buf, sizeof(buf), f)) {
		p = strchr(buf, ':');
		if (!p)
			continue;
		*p++ = '\0';

		p = strchr(p, '"');
		if (!p)
			continue;
		p++;

		q = strchr(p, '"');
		if (!q)
			continue;
		*q = '\0';

		if (strcmp(p, mtdname) == 0) {
			found = 1;
			break;
		}
	}
	fclose(f);

	if (found) {
		sprintf(device, "/dev/%s", buf);
		return device;
	} else {
		fprintf(stderr, "setmac: mtd device '%s' not found\n", mtdname);
		return NULL;
	}
}

/****************************************************************************/

void *memstr(void *m, const char *s, size_t n)
{
	int slen;
	void *end;

	slen = strlen(s);
	if (!slen || slen > n)
		return NULL;

	for (end=m+n-slen; m<=end; m++)
		if (memcmp(m, s, slen)==0)
			return m;

	return NULL;

}

/****************************************************************************/

#define REDBOOTSIZE 4096

void readmacredboot(char *flash, char *redbootconfig)
{
	int fd, i;
	off_t flashsize;
	void *m, *mac;
	char name[32];

	if ((fd = open(flash, O_RDONLY)) < 0) {
		perror("setmac: failed to open MAC flash");
		return;
	}

	m = malloc(REDBOOTSIZE);
	if (!m) {
		fprintf(stderr, "setmac: malloc failed\n");
		close(fd);
		return;
	}

	flashsize = read(fd, m, REDBOOTSIZE);
	if (flashsize < 0) {
		perror("setmac: failed to read MAC flash");
		close(fd);
		free(m);
		return;
	}

	for (i = starteths; i < (starteths + numeths); i++) {
		snprintf(name, sizeof(name), redbootconfig, i);
		mac = memstr(m, name, flashsize);
		if (!mac) {
			fprintf(stderr, "setmac: redboot config '%s' not found\n",
					name);
			continue;
		}
		mac += strlen(name)+1;
		memcpy(&mactable[i*6], mac, 6);
	}

	free(m);
	close(fd);
}

/****************************************************************************/

void readmacflash(char *flash, off_t macoffset)
{
	int fd, i;
	off_t off;
	unsigned char mac[6];


	/*
	 *	Not that many possible MAC addresses, so lets just
	 *	read them all at once and cache them locally.
	 */
	if ((fd = open(flash, O_RDONLY)) < 0) {
		perror("setmac: failed to read MAC flash");
		return;
	}

	for (i = starteths; i < (starteths + numeths); i++) {
		off = macoffset + (i * 6);
		if (lseek(fd, off, SEEK_SET) != off) {
			perror("setmac: failed to find eth MACS");
			break;
		}

		if (read(fd, &mac[0], 6) < 0) {
			perror("setmac: failed to read eth MACS");
			break;
		}

		/* Do simple checks for a valid MAC address */
		if ((mac[0] == 0) && (mac[1] == 0) && (mac[2] == 0) &&
		    (mac[3] == 0) && (mac[4] == 0) && (mac[5] == 0))
			continue;
		if ((mac[0] == 0xff) && (mac[1] == 0xff) && (mac[2] == 0xff) &&
		    (mac[3] == 0xff) && (mac[4] == 0xff) && (mac[5] == 0xff))
			continue;

		memcpy(&mactable[i*6], &mac[0], 6);
	}

	close(fd);
}

/****************************************************************************/

void runflashmac(void)
{
	FILE *fp;
	char cmd[32], result[32], *cp;
	unsigned int i, mac[6];

	for (i = starteths; i < (starteths + numeths); i++) {

		sprintf(cmd, "flash mac%d", i);
		fp = popen(cmd, "r");
		if (!fp)
			continue;
		cp = fgets(result, sizeof(result), fp);
		pclose(fp);

		if (!cp)
			continue;

		if (sscanf(cp, "%02x %02x %02x %02x %02x %02x", &mac[0], &mac[1],
				   	&mac[2], &mac[3], &mac[4], &mac[5]) != 6)
			continue;

		/* Do simple checks for a valid MAC address */
		if ((mac[0] == 0) && (mac[1] == 0) && (mac[2] == 0) &&
		    (mac[3] == 0) && (mac[4] == 0) && (mac[5] == 0))
			continue;
		if ((mac[0] == 0xff) && (mac[1] == 0xff) && (mac[2] == 0xff) &&
		    (mac[3] == 0xff) && (mac[4] == 0xff) && (mac[5] == 0xff))
			continue;

		mactable[i*6+0] = mac[0];
		mactable[i*6+1] = mac[1];
		mactable[i*6+2] = mac[2];
		mactable[i*6+3] = mac[3];
		mactable[i*6+4] = mac[4];
		mactable[i*6+5] = mac[5];
	}
}

/****************************************************************************/

void readmacuboot(void)
{
	char cmd[40], ethnam[16];
	char result[40], *ap;
	unsigned int i, mac[6];
	FILE *fp;

	for (i = starteths; i < (starteths + numeths); i++) {

		if (i == 0)
			sprintf(ethnam, "ethaddr");
		else
			sprintf(ethnam, "eth%daddr", i);
		sprintf(cmd, "fw_printenv -n %s", ethnam);

		fp = popen(cmd, "r");
		if (fp == NULL)
			continue;
		ap = fgets(result, sizeof(result), fp);
		pclose(fp);

		if (ap == NULL)
			continue;

		if (sscanf(ap, "%02x:%02x:%02x:%02x:%02x:%02x",
		    &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) != 6)
			continue;

		/* Do simple checks for a valid MAC address */
		if ((mac[0] == 0) && (mac[1] == 0) && (mac[2] == 0) &&
		    (mac[3] == 0) && (mac[4] == 0) && (mac[5] == 0))
			continue;
		if ((mac[0] == 0xff) && (mac[1] == 0xff) && (mac[2] == 0xff) &&
		    (mac[3] == 0xff) && (mac[4] == 0xff) && (mac[5] == 0xff))
			continue;

		mactable[i*6+0] = mac[0];
		mactable[i*6+1] = mac[1];
		mactable[i*6+2] = mac[2];
		mactable[i*6+3] = mac[3];
		mactable[i*6+4] = mac[4];
		mactable[i*6+5] = mac[5];
	}
}

/****************************************************************************/

void getmac(int port, unsigned char *mac)
{
	memcpy(mac, &mactable[port*6], 6);
}

/****************************************************************************/

void setmac(int port, unsigned char *mac)
{
	int pid, status;
	char eths[32];
	char macs[32];

	if (ethname[port])
		snprintf(eths, sizeof(eths), "%s", ethname[port]);
	else
		snprintf(eths, sizeof(eths), "%s%d", ETHPREFIX, port);
	sprintf(macs, "%02x:%02x:%02x:%02x:%02x:%02x",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	if ((pid = vfork()) < 0) {
		perror("setmac: failed to fork()");
		return;
	}

	if (pid == 0) {
		/* we do not want to see the output unless debug is enabled */
		if (!debug) {
			close(0);
			close(1);
			close(2);
			open("/dev/null", O_RDWR);
			dup(0);
			dup(0);
		}
#ifdef USE_IP
		/* we expect the interface to be down before we are called */
		execlp("ip", "ip", "link", "set", "dev", eths, "address", macs, NULL);
#else
		execlp("ifconfig", "ifconfig", eths, "hw", "ether", macs, NULL);
#endif
		exit(1);
	}

	waitpid(pid, &status, 0);

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		printf("Set %s to MAC address %s\n", eths, macs);
}

/****************************************************************************/

static void incmac(unsigned int *mac)
{
	int i;

	for (i = 5; i > 0; i--) {
		mac[i]++;
		if (mac[i] != 256)
			break;
		mac[i] = 0;
	}
}

/****************************************************************************/

static int basemac(const char *p)
{
	unsigned int i, mac[6];

	if (sscanf(p, "%02x%*c%02x%*c%02x%*c%02x%*c%02x%*c%02x",
				mac, mac+1, mac+2, mac+3, mac+4, mac+5) != 6) {
		fprintf(stderr, "ERROR: invalid base MAC\n");
		return -1;
	}

	for (i = 0; i < starteths; i++)
		incmac(mac);

	for (; i < (starteths + numeths); i++) {
		mactable[i*6+0] = mac[0];
		mactable[i*6+1] = mac[1];
		mactable[i*6+2] = mac[2];
		mactable[i*6+3] = mac[3];
		mactable[i*6+4] = mac[4];
		mactable[i*6+5] = mac[5];

		incmac(mac);
	}

	return 0;
}

static int laamac(const char *p)
{
	unsigned int i, mac[6];

	if (sscanf(p, "%02x%*c%02x%*c%02x%*c%02x%*c%02x%*c%02x",
				mac, mac+1, mac+2, mac+3, mac+4, mac+5) != 6) {
		fprintf(stderr, "ERROR: invalid base MAC\n");
		return -1;
	}

	for (i = starteths; i < (starteths + numeths); i++) {
		unsigned char *m = &mactable[i*6];
		m[0] = mac[0];
		m[1] = mac[1];
		m[2] = mac[2];
		m[3] = mac[3];
		m[4] = mac[4];
		m[5] = mac[5];

		/* Generate LAA (locally administered address) for MACx where (x > 0) */
		if (i != 0) {
			m[0] |= 0x2;
			m[0] ^= (i - 1) << 2;
		}
	}

	return 0;
}

/****************************************************************************/

static void printmac(unsigned char *mac)
{
	printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
		   mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/****************************************************************************/

void usage(int rc)
{
	printf("usage: setmac [-hs?] [OPTION]...\n"
		"\t-b <base-mac>\n"
		"\t-s\n"
		"\t-f <flash-device>\n"
		"\t-m <mtd-name>\n"
		"\t-a <eth-offset>\n"
		"\t-n <num-eth-interfaces>\n"
		"\t-o <offset>\n"
		"\t-p\n"
		"\t-u\n"
		"\t-i interface[,interface[,interface[,...]]]\n"
		"\t-r <redboot-config-name>\n"
		"\t-e\n"
		"\t-l\n");
	exit(rc);
}

/****************************************************************************/

int main(int argc, char *argv[])
{
	int i, p, c;
	unsigned char mac[6];
	char *flash = DEFAULTFLASH;
	char *mtdname = NULL;
	off_t macoffset = 0x24000;
	char *redboot = NULL;
	int swapmacs = 0;
	int runflash = 0;
	int uboot = 0;
	int printonly = 0;
	const char *macbase = NULL;
	int laa = 0;

	while ((c = getopt(argc, argv, "h?a:b:dspui:m:n:o:r:f:el")) > 0) {
		switch (c) {
		case '?':
		case 'h':
			usage(0);
		case 'a':
			starteths = atoi(optarg);
			break;
		case 'b':
			macbase = optarg;
			flash = NULL;
			break;
		case 's':
			swapmacs++;
			break;
		case 'p':
			runflash++;
			break;
		case 'u':
			uboot++;
			break;
		case 'd':
			debug++;
			break;
		case 'e':
			printonly++;
			break;
		case 'f':
			flash = optarg;
			break;
		case 'm':
			mtdname = optarg;
			break;
		case 'i':
			process_interface_list(optarg);
			break;
		case 'n':
			numeths = atoi(optarg);
			break;
		case 'o':
			macoffset = strtoul(optarg, NULL, 0);
			break;
		case 'r':
			redboot = optarg;
			break;
		case 'l':
			laa++;
			break;
		default:
			usage(1);
		}
	}

	if (starteths < 0 || numeths < 0 || (starteths + numeths) > MAXETHS) {
		fprintf(stderr,
				"ERROR: bad Ethernet offset or number of Ethernets\n");
		exit(1);
	}

	if (macbase) {
		if (laa) {
			if (laamac(macbase) < 0)
				exit(1);
		} else {
			if (basemac(macbase) < 0)
				exit(1);
		}
	}

	if (mtdname)
		flash = findmtddevice(mtdname);

	if (uboot) {
		readmacuboot();
	} else if (runflash) {
		runflashmac();
	} else if (flash) {
		if (redboot)
			readmacredboot(flash, redboot);
		else
			readmacflash(flash, macoffset);
	}

	for (i = starteths; i < (starteths + numeths); i++) {
		p = (swapmacs) ? (i^1) : i;
		getmac(p, &mac[0]);
		if (printonly)
			printmac(&mac[0]);
		else
			setmac(i, &mac[0]);
	}

	return 0;
}

/****************************************************************************/
