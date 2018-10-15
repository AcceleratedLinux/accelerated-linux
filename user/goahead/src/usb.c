/* vi: set sw=4 ts=4 sts=4: */
/*
 *	usb.c -- USB Application Settings
 *
 *	Copyright (c) Ralink Technology Corporation All Rights Reserved.
 *
 *	$Id: usb.c,v 1.21 2009-12-25 10:44:04 chhung Exp $
 */

#include	<stdlib.h>
#include	<dirent.h>
#include	<arpa/inet.h>

#include	"webs.h"
#include	"nvram.h"
#include	"usb.h"
#include 	"utils.h"
#include 	"internet.h"
#include	"linux/autoconf.h"  //kernel config
#include	"config/autoconf.h" //user config

#if defined CONFIG_RALINKAPP_MPLAYER
#include	"media.h"
#endif

static void storageAdm(webs_t wp, char_t *path, char_t *query);
static void StorageAddUser(webs_t wp, char_t *path, char_t *query);
static void StorageEditUser(webs_t wp, char_t *path, char_t *query);
static void storageDiskAdm(webs_t wp, char_t *path, char_t *query);
static void storageDiskPart(webs_t wp, char_t *path, char_t *query);
#if defined CONFIG_USER_PROFTPD
static void storageFtpSrv(webs_t wp, char_t *path, char_t *query);
#endif
static int GetNthFree(char *entry);
static int ShowPartition(int eid, webs_t wp, int argc, char_t **argv);
static int ShowAllDir(int eid, webs_t wp, int argc, char_t **argv);
static void storageGetFirmwarePath(webs_t wp, char_t *path, char_t *query);
static int getCount(int eid, webs_t wp, int argc, char_t **argv);
#if defined CONFIG_USB && defined CONFIG_USER_STORAGE
static void setFirstPart(void);
#endif
static int getMaxVol(int eid, webs_t wp, int argc, char_t **argv);
#if defined CONFIG_USB && CONFIG_USER_SAMBA
static int ShowSmbDir(int eid, webs_t wp, int argc, char_t **argv);
static void storageSmbSrv(webs_t wp, char_t *path, char_t *query);
static void SmbDirAdd(webs_t wp, char_t *path, char_t *query);
static void SmbDirEdit(webs_t wp, char_t *path, char_t *query);
static void SetSambaSrv();
static int fetchSmbConfig(void);
#endif
#if defined CONFIG_USB && defined CONFIG_USER_USHARE
static void storageMediaSrv(webs_t wp, char_t *path, char_t *query);
static void MediaDirAdd(webs_t wp, char_t *path, char_t *query);
static int ShowMediaDir(int eid, webs_t wp, int argc, char_t **argv);
static void fetchMediaConfig(void);
static void RunMediaSrv();
#endif
#if defined CONFIG_USB && defined CONFIG_USER_UVC_STREAM
static void webcamra(webs_t wp, char_t *path, char_t *query);
#endif
#if defined CONFIG_USB && defined CONFIG_USER_P910ND
static void printersrv(webs_t wp, char_t *path, char_t *query);
#endif
#if defined CONFIG_USB && CONFIG_RTDEV_USB 
static void USBiNIC(webs_t wp, char_t *path, char_t *query);
#endif
#if defined CONFIG_USER_MTDAAPD 
static void iTunesSrv(webs_t wp, char_t *path, char_t *query);
#endif

#define	LSDIR_INFO		"/tmp/lsdir"
#define	MOUNT_INFO		"/proc/mounts"

#define USB_STORAGE_PATH    "/media"
#define USB_STORAGE_SIGN    "/media/sd"
#define MIN_SPACE_FOR_FIRMWARE			(1024 * 1024 * 8)// minimum space for firmware upload
#define MIN_FIRMWARE_SIZE				(1048576)		// minium firmware size(1MB)
#define IH_MAGIC			0x27051956					// firmware magic number in header

#define DEBUG(x) do{fprintf(stderr, #x); \
					fprintf(stderr, ": %s\n", x); \
				 }while(0)

char firmware_path[1024] = {0};

void formDefineUSB(void) {
	websAspDefine(T("ShowPartition"), ShowPartition);
	websAspDefine(T("ShowAllDir"), ShowAllDir);
	websAspDefine(T("getCount"), getCount);
	websAspDefine(T("getMaxVol"), getMaxVol);
#if defined CONFIG_USER_SAMBA
	websAspDefine(T("ShowSmbDir"), ShowSmbDir);
#endif
#if defined CONFIG_USB && defined CONFIG_USER_USHARE
	websAspDefine(T("ShowMediaDir"), ShowMediaDir);
#endif

	websFormDefine(T("storageAdm"), storageAdm);
	websFormDefine(T("StorageAddUser"), StorageAddUser);
	websFormDefine(T("StorageEditUser"), StorageEditUser);
	websFormDefine(T("storageDiskAdm"), storageDiskAdm);
	websFormDefine(T("storageDiskPart"), storageDiskPart);
	websFormDefine(T("storageGetFirmwarePath"), storageGetFirmwarePath);
#if defined CONFIG_USER_PROFTPD
	websFormDefine(T("storageFtpSrv"), storageFtpSrv);
#endif
#if defined CONFIG_USER_SAMBA
	websFormDefine(T("storageSmbSrv"), storageSmbSrv);
	websFormDefine(T("SmbDirAdd"), SmbDirAdd);
	websFormDefine(T("SmbDirEdit"), SmbDirEdit);
#endif
#if defined CONFIG_USB && defined CONFIG_USER_USHARE
	websFormDefine(T("storageMediaSrv"), storageMediaSrv);
	websFormDefine(T("MediaDirAdd"), MediaDirAdd);
#endif
#if defined CONFIG_USB && defined CONFIG_USER_UVC_STREAM
	websFormDefine(T("webcamra"), webcamra);
#endif
#if defined CONFIG_USB && defined CONFIG_USER_P910ND
	websFormDefine(T("printersrv"), printersrv);
#endif
#if defined CONFIG_RTDEV_USB
	websFormDefine(T("USBiNIC"), USBiNIC);
#endif
#if defined CONFIG_USER_MTDAAPD 
	websFormDefine(T("iTunesSrv"), iTunesSrv);
#endif
}

static int dir_count;
static int part_count;
static int media_dir_count;
static char first_part[12];

#if defined CONFIG_USER_SAMBA

#define		DIREXIST(bitmap,index)		bitmap[index/32] & 1<<(index%32)?1:0

struct smb_dir {
	char path[40];
	char name[20];
	char permit[1024];
};

struct smb_dir_config {
	int count;
	int bitmap[3];
	struct smb_dir dir_list[100];
};

static struct smb_dir_config smb_conf;
#endif

struct media_config {
	char path[40];
};


/* goform/storageAdm */
static void storageAdm(webs_t wp, char_t *path, char_t *query)
{
	char_t *submit;

	submit = websGetVar(wp, T("hiddenButton"), T(""));

	if (0 == strcmp(submit, "delete"))
	{
		char_t *user_select;
		char feild[20];

		feild[0] = '\0';
		user_select = websGetVar(wp, T("storage_user_select"), T(""));
		sprintf(feild, "User%s", user_select);
		nvram_bufset(RT2860_NVRAM, feild, "");
		doSystem("rm -rf \"%s/home/%s\"", first_part, nvram_bufget(RT2860_NVRAM, feild));
		sprintf(feild, "User%sPasswd", user_select);
		nvram_bufset(RT2860_NVRAM, feild, "");
		sprintf(feild, "FtpUser%s", user_select);
		nvram_bufset(RT2860_NVRAM, feild, "");
		sprintf(feild, "SmbUser%s", user_select);
		nvram_bufset(RT2860_NVRAM, feild, "");
		nvram_commit(RT2860_NVRAM);
		doSystem("storage.sh admin");
	}
	else if (0 == strcmp(submit, "apply"))
	{
		initUSB();
	}
	websRedirect(wp, "usb/STORAGEuser_admin.asp");
}

/* goform/StorageAddUser */
static void StorageAddUser(webs_t wp, char_t *path, char_t *query)
{
	char_t *name, *password;
	char_t *user_ftp_enable, *user_smb_enable;
	char mode[6], feild[20];
	int index; 

	mode[0] = '\0';
	feild[0] = '\0';

	// fetch from web input
	name = websGetVar(wp, T("adduser_name"), T(""));
	password = websGetVar(wp, T("adduser_pw"), T(""));
	user_ftp_enable = websGetVar(wp, T("adduser_ftp"), T(""));
	user_smb_enable = websGetVar(wp, T("adduser_smb"), T(""));
	/*
	DEBUG(name);
	DEBUG(password);
	DEBUG(user_ftp_enable);
	DEBUG(user_smb_enable);
	*/
	// get null user feild form nvram
	index = GetNthFree("User");
	// fprintf(stderr, "index: %d\n", index);

	// set to nvram
	if (0 != index) {
		sprintf(feild, "User%d", index);
		nvram_bufset(RT2860_NVRAM, feild, name);
		sprintf(feild, "User%dPasswd", index);
		nvram_bufset(RT2860_NVRAM, feild, password);
		sprintf(feild, "FtpUser%d", index);
		nvram_bufset(RT2860_NVRAM, feild, user_ftp_enable);
		sprintf(feild, "SmbUser%d", index);
		nvram_bufset(RT2860_NVRAM, feild, user_smb_enable);
		nvram_commit(RT2860_NVRAM);
		doSystem("storage.sh admin");

		websRedirect(wp, "usb/STORAGEuser_admin.asp");
	} else {
		websHeader(wp);
		websWrite(wp, T("<h2>User Quota is Full!</h2><br>\n"));
		websFooter(wp);
		websDone(wp, 200);
	}
}

/* goform/StorageEditUser */
static void StorageEditUser(webs_t wp, char_t *path, char_t *query)
{
	char_t *index, *password; 
	char_t *user_ftp_enable, *user_smb_enable;
	char feild[20];

	feild[0] = '\0';
	// fetch from web input
	index = websGetVar(wp, T("hiddenIndex"), T(""));
	password = websGetVar(wp, T("edituser_pw"), T(""));
	user_ftp_enable = websGetVar(wp, T("edituser_ftp"), T(""));
	user_smb_enable = websGetVar(wp, T("edituser_smb"), T(""));
	/*
	DEBUG(index);
	DEBUG(password);
	DEBUG(user_ftp_enable);
	DEBUG(user_smb_enable);
	*/

	// set to nvram
	sprintf(feild, "User%sPasswd", index);
	nvram_bufset(RT2860_NVRAM, feild, password);
	sprintf(feild, "FtpUser%s", index);
	// DEBUG(feild);
	nvram_bufset(RT2860_NVRAM, feild, user_ftp_enable);
	sprintf(feild, "SmbUser%s", index);
	// DEBUG(feild);
	nvram_bufset(RT2860_NVRAM, feild, user_smb_enable);
	nvram_commit(RT2860_NVRAM);
	doSystem("storage.sh admin");
		
	websRedirect(wp, "usb/STORAGEuser_admin.asp");
}

static void FormatPart(char *part, char *path)
{
		FILE *fp = NULL;
		char temp[30];

		if (0 == strcmp(part, "") && 0 < strcmp(path, "")) {
			if (NULL == (fp = fopen("/proc/mounts", "r"))) {
				perror(__FUNCTION__);
				return;
			}
			while(EOF != fscanf(fp, "%s %s %*s %*s %*s %*s\n", part, temp))
			{
				if (0 == strcmp(temp, path)) {
					break;
				}
				strcpy(part, "");
			}
			fclose(fp);
		}
		if (0 == strcmp(part, "")) {
				perror(__FUNCTION__);
				return;
		} else {
			char blk_size[30];
			if (NULL == (fp = fopen("/proc/partitions", "r"))) {
				perror(__FUNCTION__);
				return;
			}
			while(EOF != fscanf(fp, "%*s %*s %s %s\n", blk_size, temp)) {
				char temp_part[30];
				sprintf(temp_part, "/dev/%s", temp);
				if (0 == strcmp(part, temp_part)) {
					doSystem("storage.sh format %s %s", part, blk_size);
					break;
				}
			}
			fclose(fp);
		}
#if defined CONFIG_USER_SAMBA
		if (0 == strcmp(path, first_part))
			memset(&smb_conf, 0, sizeof(struct smb_dir_config));
#endif	
		return;
}

static void storageDiskAdm(webs_t wp, char_t *path, char_t *query)
{
	char_t *submit;

	submit = websGetVar(wp, T("hiddenButton"), T(""));

	if (0 == strcmp(submit, "delete"))
	{
		char_t *dir_path = websGetVar(wp, T("dir_path"), T(""));

#if defined CONFIG_USER_SAMBA
		FILE *fp = NULL; 
		char smb_config_file[25];
		int exist, index = 0;

		sprintf(smb_config_file, "%s/.smb_config", first_part);
		if (NULL == (fp = fopen(smb_config_file, "w")))
		{
			perror(__FUNCTION__);
			return;
		}
		while(96 > index)
		{
			exist = DIREXIST(smb_conf.bitmap, index);
			if ((exist) && 
				(0 == strcmp(dir_path, smb_conf.dir_list[index].path)))
			{
				// fprintf(stderr, "before set bitmap: %x%x%x\n", smb_conf.bitmap[2], smb_conf.bitmap[1], smb_conf.bitmap[0]);
				smb_conf.bitmap[index/32] &= ~(1<<(index%32));
				// fprintf(stderr, "after set bitmap: %x%x%x\n", smb_conf.bitmap[2], smb_conf.bitmap[1], smb_conf.bitmap[0]);
				smb_conf.count--;
				// fprintf(stderr, "smb dir count: %d\n", smb_conf.count);
				break;
			}
			index++;
		}
		fwrite(&smb_conf, sizeof(struct smb_dir_config), 1, fp);
		fclose(fp);
#endif	
		doSystem("rm -rf \"%s\"", dir_path);
		websRedirect(wp, "usb/STORAGEdisk_admin.asp");
	}
	else if (0 == strcmp(submit, "add"))
	{
		char_t *dir_name, *disk_part;

		dir_name = websGetVar(wp, T("adddir_name"), T(""));
		disk_part = websGetVar(wp, T("disk_part"), T(""));
		// DEBUG(dir_name);
		// DEBUG(disk_part);
		doSystem("mkdir -p \"%s/%s\"", disk_part, dir_name);	
		doSystem("chmod 777 \"%s/%s\"", disk_part, dir_name);	
	}
	else if (0 == strcmp(submit, "format"))
	{
		char_t *disk_part = websGetVar(wp, T("disk_part"), T(""));
		char part[30] = "";
		FormatPart(part, disk_part);
		sleep(5);
		doSystem("storage.sh restart");
		sleep(5);

		websRedirect(wp, "usb/STORAGEdisk_admin.asp");
	}
	else if (0 == strcmp(submit, "remove"))
	{
		FILE *fp_mount = NULL;
		char part[30];
		
		if (NULL == (fp_mount = fopen("/proc/mounts", "r")))
		{
			perror(__FUNCTION__);
			return;
		}
		while(EOF != fscanf(fp_mount, "%s %*s %*s %*s %*s %*s\n", part))
		{
			if (NULL != strstr(part, "/dev/sd"))
				doSystem("umount -l %s", part);
		}
		fclose(fp_mount);
		websRedirect(wp, "usb/STORAGEdisk_admin.asp");
	}
}

static void storageDiskPart(webs_t wp, char_t *path, char_t *query)
{
	char_t *part1_vol, *part2_vol, *part3_vol;
	int max_part = atoi(websGetVar(wp, T("max_part"), T("")));
	FILE *fp = NULL;
	char part[30];

	if (NULL == (fp = fopen("/proc/mounts", "r")))
	{
		perror(__FUNCTION__);
		return;
	}
	while(EOF != fscanf(fp, "%s %*s %*s %*s %*s %*s\n", part))
	{
		if (NULL != strstr(part, "/dev/sd"))
			doSystem("umount -l %s", part);
	}
	fclose(fp);
	part1_vol = websGetVar(wp, T("part1_vol"), T(""));
	part2_vol = websGetVar(wp, T("part2_vol"), T(""));
	part3_vol = websGetVar(wp, T("part3_vol"), T(""));
	doSystem("storage.sh reparted %s %s %s %d", 
			  part1_vol, part2_vol, part3_vol, max_part);
	sleep(50);
	memset(part, 0, 30);
	do {
		sprintf(part, "/dev/sda%d", max_part+4);
		FormatPart(part, "");
		sleep(5);
	} while (max_part--);
	doSystem("storage.sh restart");
	sleep(5);
	websRedirect(wp, "usb/STORAGEdisk_admin.asp");
}

#if defined CONFIG_USER_PROFTPD
/* goform/storageFtpSrv */
static void storageFtpSrv(webs_t wp, char_t *path, char_t *query)
{
	char_t *ftp, *name, *anonymous, *port, *max_sessions;
	char_t *adddir, *rename, *remove, *read, *write, *download, *upload;

	// fetch from web input
	ftp = websGetVar(wp, T("ftp_enabled"), T("0"));
	name = websGetVar(wp, T("ftp_name"), T("0"));
	anonymous = websGetVar(wp, T("ftp_anonymous"), T("0"));
	port = websGetVar(wp, T("ftp_port"), T(""));
	max_sessions = websGetVar(wp, T("ftp_max_sessions"), T(""));
	adddir = websGetVar(wp, T("ftp_adddir"), T("0"));
	rename = websGetVar(wp, T("ftp_rename"), T("0"));
	remove = websGetVar(wp, T("ftp_remove"), T("0"));
	read = websGetVar(wp, T("ftp_read"), T("0"));
	write = websGetVar(wp, T("ftp_write"), T("0"));
	download = websGetVar(wp, T("ftp_download"), T("0"));
	upload = websGetVar(wp, T("ftp_upload"), T("0"));

	// set to nvram
	nvram_bufset(RT2860_NVRAM, "FtpEnabled", ftp);
	nvram_bufset(RT2860_NVRAM, "FtpName", name);
	nvram_bufset(RT2860_NVRAM, "FtpAnonymous", anonymous);
	nvram_bufset(RT2860_NVRAM, "FtpPort", port);
	nvram_bufset(RT2860_NVRAM, "FtpMaxSessions", max_sessions);
	nvram_bufset(RT2860_NVRAM, "FtpAddDir", adddir);
	nvram_bufset(RT2860_NVRAM, "FtpRename", rename);
	nvram_bufset(RT2860_NVRAM, "FtpRemove", remove);
	nvram_bufset(RT2860_NVRAM, "FtpRead", read);
	nvram_bufset(RT2860_NVRAM, "FtpWrite", write);
	nvram_bufset(RT2860_NVRAM, "FtpDownload", download);
	nvram_bufset(RT2860_NVRAM, "FtpUpload", upload);
	nvram_commit(RT2860_NVRAM);

	// setup device
	doSystem("storage.sh ftp");

	// debug print
	websHeader(wp);
	websWrite(wp, T("<h2>ftp_enabled: %s</h2><br>\n"), ftp);
	websWrite(wp, T("ftp_anonymous: %s<br>\n"), anonymous);
	websWrite(wp, T("ftp_name: %s<br>\n"), name);
	websWrite(wp, T("ftp_port: %s<br>\n"), port);
	websWrite(wp, T("ftp_max_sessions: %s<br>\n"), max_sessions);
	websWrite(wp, T("ftp_adddir: %s<br>\n"), adddir);
	websWrite(wp, T("ftp_rename: %s<br>\n"), rename);
	websWrite(wp, T("ftp_remove: %s<br>\n"), remove);
	websWrite(wp, T("ftp_read: %s<br>\n"), read);
	websWrite(wp, T("ftp_write: %s<br>\n"), write);
	websWrite(wp, T("ftp_download: %s<br>\n"), download);
	websWrite(wp, T("ftp_upload: %s<br>\n"), upload);
	websFooter(wp);
	websDone(wp, 200);
}
#endif

#if defined CONFIG_USER_SAMBA
/* goform/storageSmbSrv */
static void storageSmbSrv(webs_t wp, char_t *path, char_t *query)
{
	char_t *submit;

	submit =  websGetVar(wp, T("hiddenButton"), T(""));

	if (0 == strcmp(submit, "delete"))
	{
		int index;
		FILE *fp;
		char smb_config_file[25];

		// strcpy(smb_config_file, "/var/.smb_config");
		sprintf(smb_config_file, "%s/.smb_config", first_part);
		fp = fopen(smb_config_file, "w");

		if (NULL == fp) {
			perror(__FUNCTION__);
			return;
		}

		index = atoi(websGetVar(wp, T("selectIndex"), T("")));
		// fprintf(stderr, "before set bitmap: %x%x%x\n", smb_conf.bitmap[2], smb_conf.bitmap[1], smb_conf.bitmap[0]);
		smb_conf.bitmap[index/32] &= ~(1<<(index%32));
		// fprintf(stderr, "after set bitmap: %x%x%x\n", smb_conf.bitmap[2], smb_conf.bitmap[1], smb_conf.bitmap[0]);
		smb_conf.count--;
		// fprintf(stderr, "smb dir count: %d\n", smb_conf.count);
		fwrite(&smb_conf, sizeof(struct smb_dir_config), 1, fp);
		fclose(fp);
		websRedirect(wp, "usb/SAMBAsmbsrv.asp");
	} 
	else if (0 == strcmp(submit, "apply"))
	{
		char_t *smb_enable, *wg, *netbios;
		
		// fetch from web input
		smb_enable = websGetVar(wp, T("smb_enabled"), T(""));
		wg = websGetVar(wp, T("smb_workgroup"), T(""));
		netbios = websGetVar(wp, T("smb_netbios"), T(""));

		// set to nvram
		nvram_bufset(RT2860_NVRAM, "SmbEnabled", smb_enable);
		nvram_bufset(RT2860_NVRAM, "HostName", wg);
		nvram_bufset(RT2860_NVRAM, "SmbNetBIOS", netbios);
		nvram_commit(RT2860_NVRAM);

		// setup device
		SetSambaSrv();

		// debug print
		websHeader(wp);
		websWrite(wp, T("<h2>smb_enabled: %s</h2><br>\n"), smb_enable);
		websWrite(wp, T("smb_workgroup: %s<br>\n"), wg);
		websWrite(wp, T("smb_netbios: %s<br>\n"), netbios);
		websFooter(wp);
		websDone(wp, 200);
	}
}

/* goform/SmbDirAdd */
static void SmbDirAdd(webs_t wp, char_t *path, char_t *query)
{
	char_t *dir_name, *dir_path, *allow_users;
	FILE *fp;
	int exist, index = 0;
	char smb_config_file[25];

	// strcpy(smb_config, "/var/.smb_config");
	sprintf(smb_config_file, "%s/.smb_config", first_part);
	fp = fopen(smb_config_file, "w");

	if (NULL == fp) {
		perror(__FUNCTION__);
		return;
	}

	// fetch from web input
	dir_name = websGetVar(wp, T("dir_name"), T(""));
	dir_path = websGetVar(wp, T("dir_path"), T(""));
	allow_users = websGetVar(wp, T("allow_user"), T(""));
	/*
	DEBUG(dir_name);
	DEBUG(dir_path);
	DEBUG(allow_users);
	*/

	while(96 > index)
	{
		exist = DIREXIST(smb_conf.bitmap, index);
		if (exist && 0 == strcmp(dir_name, smb_conf.dir_list[index].name))
		{
				fprintf(stderr, "Existed Samba Shared Dir: %s\n", dir_name);
				fwrite(&smb_conf, sizeof(struct smb_dir_config), 1, fp);
				fclose(fp);
				return;
		}
		index++;
	}
	index = 0;
	// fprintf(stderr, "before set bitmap: %x%x%x\n", smb_conf.bitmap[2], smb_conf.bitmap[1], smb_conf.bitmap[0]);
	while(96 > index)
	{
		exist = DIREXIST(smb_conf.bitmap, index);
		if (!exist)
		{
			strcpy(smb_conf.dir_list[index].path, dir_path);
			strcpy(smb_conf.dir_list[index].name, dir_name);
			strcpy(smb_conf.dir_list[index].permit, allow_users);
			smb_conf.bitmap[index/32] |= 1<<(index%32);
			smb_conf.count++;
			break;
		}
		index++;
	}
	/*
	fprintf(stderr, "after set bitmap: %x%x%x\n", smb_conf.bitmap[2], smb_conf.bitmap[1], smb_conf.bitmap[0]);
	DEBUG(smb_conf.dir_list[index].path);
	DEBUG(smb_conf.dir_list[index].name);
	DEBUG(smb_conf.dir_list[index].permit);
	*/
	fwrite(&smb_conf, sizeof(struct smb_dir_config), 1, fp);
	fclose(fp);
}

/* goform/SmbDirEdit */
static void SmbDirEdit(webs_t wp, char_t *path, char_t *query)
{
	char_t *allow_user;
	int index = 0;
	FILE *fp;
	char smb_config_file[25];

	// strcpy(smb_config, "/var/.smb_config");
	sprintf(smb_config_file, "%s/.smb_config", first_part);
	fp = fopen(smb_config_file, "w");

	if (NULL == fp) {
		perror(__FUNCTION__);
		return;
	}

	// fetch from web input
	index = atoi(websGetVar(wp, T("hidden_index"), T("")));
	allow_user = websGetVar(wp, T("allow_user"), T(""));

	strcpy(smb_conf.dir_list[index].permit, allow_user);
	fwrite(&smb_conf, sizeof(struct smb_dir_config), 1, fp);
	fclose(fp);
}

static int fetchSmbConfig(void)
{
	FILE *fp = NULL;
	char smb_config_file[25];

	memset(&smb_conf, 0, sizeof(struct smb_dir_config));
	sprintf(smb_config_file, "%s/.smb_config", first_part);
	if (NULL == (fp = fopen(smb_config_file, "r")))
	{
		perror(__FUNCTION__);
		return -1;
	}
	fread(&smb_conf, sizeof(struct smb_dir_config), 1, fp);
	fclose(fp);

	return 0;
}

static int ShowSmbDir(int eid, webs_t wp, int argc, char_t **argv)
{
	int exist, i = 0, count = 0;

	if (-1 == fetchSmbConfig())
	{
		return 0;
	}

	websWrite(wp, T("<tr align=\"center\">"));
	websWrite(wp, T("<td>--</td>"));
	websWrite(wp, T("<td>public</td>"));
	websWrite(wp, T("<td>%s/%s</td>"), first_part, "public");
	websWrite(wp, T("<td>All Users</td>"));
	websWrite(wp, T("</tr>"));
	while(smb_conf.count > 0 && i < 96)
	{
		exist = DIREXIST(smb_conf.bitmap, i);
		if (exist)
		{
			websWrite(wp, T("<tr align=\"center\">"));
			websWrite(wp, T("<td><input type=\"radio\" name=\"smb_dir\" value=\"%s\"></td>"), 
					  smb_conf.dir_list[i].name);
			websWrite(wp, T("<td>%s</td>"), smb_conf.dir_list[i].name);
			websWrite(wp, T("<input type=\"hidden\" name=\"smb_dir_path\" value=\"%s\">"), 
					  smb_conf.dir_list[i].path);
			websWrite(wp, T("<td>%s</td>"), smb_conf.dir_list[i].path);
			websWrite(wp, T("<input type=\"hidden\" name=\"smb_dir_permit\" value=\"%s\">"), 
					  smb_conf.dir_list[i].permit);
			if (0 == strlen(smb_conf.dir_list[i].permit))
				websWrite(wp, T("<td>%s</td>"), "<br />");
			else
				websWrite(wp, T("<td>%s</td>"), smb_conf.dir_list[i].permit);
			websWrite(wp, T("</tr>"));
			count++;
		}
		i++;
	}
	smb_conf.count = count;
	// fprintf(stderr, "smb_conf.count: %d\n", smb_conf.count);

	return 0;
}

static void SetSambaSrv()
{
	int i;
	const char *admin_id = nvram_bufget(RT2860_NVRAM, "Login");

	doSystem("storage.sh samba");
	if (1 == atoi(nvram_bufget(RT2860_NVRAM, "SmbEnabled")))
	{
		for(i=0;i<smb_conf.count;i++)
		{
			doSystem("samba_add_dir.sh \"%s\" \"%s\" \"%s\" \"%s\"", 
					smb_conf.dir_list[i].name, 
					smb_conf.dir_list[i].path, 
					admin_id,
					smb_conf.dir_list[i].permit);
		}
		doSystem("nmbd");
		doSystem("smbd");
	}
}
#endif

#if defined CONFIG_USB && defined CONFIG_USER_USHARE
static struct media_config media_conf[4];
static void storageMediaSrv(webs_t wp, char_t *path, char_t *query)
{
	char_t *submit;

	submit =  websGetVar(wp, T("hiddenButton"), T(""));

	if (0 == strcmp(submit, "delete"))
	{
		int index;
		FILE *fp;
		char media_config_file[25];

		// strcpy(smb_config, "/var/.smb_config");
		sprintf(media_config_file, "%s/.media_config", first_part);
		fp = fopen(media_config_file, "w");

		if (NULL == fp) {
			perror(__FUNCTION__);
			return;
		}

		index = atoi(websGetVar(wp, T("media_dir"), T("")));
		memset(&media_conf[index].path, 0, sizeof(media_conf[index].path));	
		fwrite(media_conf, sizeof(media_conf), 1, fp);
		fclose(fp);
		websRedirect(wp, "usb/USHAREmediasrv.asp");
	} 
	else if (0 == strcmp(submit, "apply"))
	{
		char_t *media_enabled, *media_name;
		int i;
		
		// fetch from web input
		media_enabled = websGetVar(wp, T("media_enabled"), T(""));
		media_name = websGetVar(wp, T("media_name"), T(""));

		// set to nvram
		nvram_bufset(RT2860_NVRAM, "mediaSrvEnabled", media_enabled);
		nvram_bufset(RT2860_NVRAM, "mediaSrvName", media_name);
		nvram_commit(RT2860_NVRAM);

		// setup device
		if (0 == strcmp(media_enabled, "0"))
			memset(media_conf, 0, sizeof(media_conf));
		RunMediaSrv();

		// debug print
		websHeader(wp);
		websWrite(wp, T("<h2>Media Server Settings</h2><br>\n"));
		websWrite(wp, T("media_enabled: %s<br>\n"), media_enabled);
		websWrite(wp, T("media_name: %s<br>\n"), media_name);
		for(i=0;i<4;i++)
			websWrite(wp, T("media dir%d path: %s<br>\n"), i, media_conf[i].path);
		websFooter(wp);
		websDone(wp, 200);
	}
}

static void MediaDirAdd(webs_t wp, char_t *path, char_t *query)
{
	char_t *dir_path;
	FILE *fp;
	char media_config_file[25];
	int index = 0;

	sprintf(media_config_file, "%s/.media_config", first_part);
	fp = fopen(media_config_file, "w");

	if (NULL == fp) {
		perror(__FUNCTION__);
		return;
	}

	// fetch from web input
	dir_path = websGetVar(wp, T("dir_path"), T(""));
	/*
	DEBUG(dir_path);
	*/
	while (4 > index)
	{
		if (0 == strcmp(dir_path, media_conf[index].path))
		{
			fprintf(stderr, "Existed Media Shared Dir: %s\n", dir_path);
			fwrite(media_conf, sizeof(media_conf), 1, fp);
			fclose(fp);
			return;
		}
		index++;
	}
	index = 0;
	while (4 > index)
	{
		if (0 == strlen(media_conf[index].path))
		{
			strcpy(media_conf[index].path, dir_path);
			break;
		}
		index++;
	}
	if (index == 5)
	{
		perror("Media Server Shared Dirs exceed 4");
		fclose(fp);
		return;
	}
	fwrite(media_conf, sizeof(media_conf), 1, fp);
	fclose(fp);
}

static int ShowMediaDir(int eid, webs_t wp, int argc, char_t **argv)
{
	int index;

	fetchMediaConfig();
	media_dir_count = 0;
	for(index=0;index<4;index++)
	{
		if (0 != strlen(media_conf[index].path)) 
		{
			websWrite(wp, T("<tr align=\"center\">"));
			websWrite(wp, T("<td><input type=\"radio\" name=\"media_dir\" value=\"%d\"></td>"), 
					  index);
			websWrite(wp, T("<td>%s</td>"), media_conf[index].path);
			websWrite(wp, T("</tr>"));
			media_dir_count++;
		}
	}

	return 0;
}

static void fetchMediaConfig(void)
{
	FILE *fp = NULL;
	char media_config_file[25];

	memset(media_conf, 0, sizeof(media_conf));
	sprintf(media_config_file, "%s/.media_config", first_part);
	if (NULL == (fp = fopen(media_config_file, "r")))
	{
		perror(__FUNCTION__);
		return;
	}
	fread(media_conf, sizeof(media_conf), 1, fp);
	fclose(fp);
}

static void RunMediaSrv()
{
	char mediasrv_dir[160];
	int i;

	memset(mediasrv_dir, 0, sizeof(mediasrv_dir));
	for(i=0;i<4;i++)
		if (0 != strlen(media_conf[i].path))
			sprintf(mediasrv_dir, "%s %s", mediasrv_dir, media_conf[i].path);

	doSystem("storage.sh media \"%s\"", mediasrv_dir);
}
#endif

#if defined CONFIG_USB && defined CONFIG_USER_UVC_STREAM
/* goform/webcamra */
static void webcamra(webs_t wp, char_t *path, char_t *query)
{
	char_t *enable, *resolution, *fps, *port;

	// fetch from web input
	enable = websGetVar(wp, T("enabled"), T(""));
	resolution = websGetVar(wp, T("resolution"), T(""));
	fps = websGetVar(wp, T("fps"), T(""));
	port = websGetVar(wp, T("port"), T(""));

	// set to nvram
	nvram_bufset(RT2860_NVRAM, "WebCamEnabled", enable);
	nvram_bufset(RT2860_NVRAM, "WebCamResolution", resolution);
	nvram_bufset(RT2860_NVRAM, "WebCamFPS", fps);
	nvram_bufset(RT2860_NVRAM, "WebCamPort", port);
	nvram_commit(RT2860_NVRAM);

	// setup device
	doSystem("killall uvc_stream");
	if (0 == strcmp(enable, "1"))
	{
		doSystem("uvc_stream -r %s -f %s -p %s -b", 
				  resolution, fps, port);
	}

	// debug print
	websHeader(wp);
	websWrite(wp, T("<h2>Web Camera Settings</h2><br>\n"));
	websWrite(wp, T("enabled: %s<br>\n"), enable);
	websWrite(wp, T("resolution: %s<br>\n"), resolution);
	websWrite(wp, T("fps: %s<br>\n"), fps);
	websWrite(wp, T("port: %s<br>\n"), port);
	websFooter(wp);
	websDone(wp, 200);
}
#endif

#if defined CONFIG_USB && defined CONFIG_USER_P910ND
static void printersrv(webs_t wp, char_t *path, char_t *query)
{
	char_t *enable;

	// fetch from web input
	enable = websGetVar(wp, T("enabled"), T(""));
	// set to nvram
	nvram_bufset(RT2860_NVRAM, "PrinterSrvEnabled", enable);
	nvram_commit(RT2860_NVRAM);

	// setup device
	doSystem("killall p910nd");
	if (0 == strcmp(enable, "1"))
	{
		doSystem("p910nd -b -f /dev/lp0");
	}

	// debug print
	websHeader(wp);
	websWrite(wp, T("<h2>Printer Server Settings</h2><br>\n"));
	websWrite(wp, T("enabled: %s<br>\n"), enable);
	websFooter(wp);
	websDone(wp, 200);
}
#endif

#if defined CONFIG_RTDEV_USB 
static void USBiNIC(webs_t wp, char_t *path, char_t *query)
{
	char_t *enable;

	// fetch from web input
	enable = websGetVar(wp, T("inic_enable"), T(""));
	// set to nvram
	nvram_bufset(RTDEV_NVRAM, "InicUSBEnable", enable);
	nvram_commit(RTDEV_NVRAM);

	// setup device
	initInternet();

	// debug print
	websHeader(wp);
	websWrite(wp, T("<h2>USV iNIC Settings</h2><br>\n"));
	websWrite(wp, T("inic_enable: %s<br>\n"), enable);
	websFooter(wp);
	websDone(wp, 200);
}
#endif

int initUSB(void)
{
	printf("\n##### USB init #####\n");
	sleep(1);			// wait for sub-storage initiation
#if defined CONFIG_USB && defined CONFIG_USER_STORAGE
	setFirstPart();
	doSystem("storage.sh admin");
#if defined CONFIG_USER_PROFTPD
	doSystem("storage.sh ftp");
#endif
#if defined CONFIG_USER_SAMBA
	fetchSmbConfig();
	SetSambaSrv();
#endif
#endif
#if defined CONFIG_USB && defined CONFIG_USER_USHARE
	fetchMediaConfig();
	RunMediaSrv();
#endif
#if defined CONFIG_USB && defined CONFIG_USER_UVC_STREAM
	printf("UVC init\n");
	const char *webcamebl = nvram_bufget(RT2860_NVRAM, "WebCamEnabled");
	doSystem("killall uvc_stream");
	if (0 == strcmp(webcamebl, "1"))
	{
	printf("UVC start\n");
		doSystem("uvc_stream -r %s -f %s -p %s -b", 
				  nvram_bufget(RT2860_NVRAM, "WebCamResolution"), 
				  nvram_bufget(RT2860_NVRAM, "WebCamFPS"), 
				  nvram_bufget(RT2860_NVRAM, "WebCamPort"));
	}
#endif
#if defined CONFIG_USB && defined CONFIG_USER_P910ND
	printf("P910ND init\n");
	const char *printersrvebl = nvram_bufget(RT2860_NVRAM, "PrinterSrvEnabled");
	doSystem("killall p910nd");
	if (0 == strcmp(printersrvebl, "1"))
	{
	printf("P910ND start\n");
		doSystem("p910nd -b -f /dev/lp0");
	}
#endif
#if defined CONFIG_USB && defined CONFIG_USER_MTDAAPD 
	doSystem("killall mt-daapd; killall mDNSResponder");
	if (strcmp(nvram_bufget(RT2860_NVRAM, "iTunesEnable"), "1") == 0)
		doSystem("config-iTunes.sh \"%s\" \"%s\" \"%s\"", 
				 nvram_bufget(RT2860_NVRAM, "iTunesSrvName"),
				 nvram_bufget(RT2860_NVRAM, "Password"),
				 nvram_bufget(RT2860_NVRAM, "iTunesDir"));
#endif

	return 0;
}

static int getCount(int eid, webs_t wp, int argc, char_t **argv)
{
	int type;
	char_t *field;
	char count[3];

	if (2 > ejArgs(argc, argv, T("%d %s"), &type, &field)) 
	{
		return websWrite(wp, T("Insufficient args\n"));
	}

	if (0 == strcmp(field, "AllDir"))
	{
		sprintf(count, "%d", dir_count);
		// fprintf(stderr,"AllDir: %s\n", count);
	}
	else if (0 == strcmp(field, "AllPart"))
	{
		sprintf(count, "%d", part_count);
		// fprintf(stderr,"AllPart: %s\n", count);
	}
#if defined CONFIG_USER_SAMBA
	else if (0 == strcmp(field, "AllSmbDir"))
	{
		sprintf(count, "%d", smb_conf.count);
		// fprintf(stderr,"AllSmbDir: %s\n", count);
	}
#endif
	else if (0 == strcmp(field, "AllMediaDir"))
	{
		sprintf(count, "%d", media_dir_count);
		// fprintf(stderr,"AllPart: %s\n", count);
	}

	if (1 == type) {
		if (!strcmp(count, ""))
			return websWrite(wp, T("0"));
		return websWrite(wp, T("%s"), count);
	}
	if (!strcmp(count, ""))
		ejSetResult(eid, "0");
	ejSetResult(eid, count);

	return 0;
}

#if defined CONFIG_USB && defined CONFIG_USER_STORAGE
static void setFirstPart(void)
{
	DIR *dp = NULL;
	char dir[12], c = 'a';
	int i;

	memset(first_part, 0, sizeof(first_part));
	do
	{
		for (i=1;i<10;i++)
		{
			sprintf(dir, "/media/sd%c%d", c, i);
			if (NULL == (dp = opendir(dir)))
			{
				closedir(dp);
				continue;
			}
			fprintf(stderr, "get the first partition: %s\n", dir);
			strcpy(first_part, dir);
			closedir(dp);
			return;
		}
		c++;
	} while(c <= 'z');
}
#endif

static int GetNthFree(char *entry)
{
	int index = 1;
	char feild[7];
	const char *user_name;

	feild[0] = '\0';
	do
	{
		sprintf(feild, "%s%d", entry, index);
		user_name = (const char *) nvram_bufget(RT2860_NVRAM, feild);
		if (strlen(user_name) == 0)
			return index;
		index++;
	} while (index <= 10);

	return 0;
}

static int ShowAllDir(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp_mount = fopen(MOUNT_INFO, "r");
	char part[30], path[50];
	char dir_name[30];
	int dir_len = 0;

	if (NULL == fp_mount) {
        perror(__FUNCTION__);
		return -1;
	}

	dir_count = 0;

	while(EOF != fscanf(fp_mount, "%s %s %*s %*s %*s %*s\n", part, path))
	{
		DIR *dp;
		struct dirent *dirp;
		struct stat statbuf;

		// if (strncmp(path, "/var", 4) != 0)
		if (0 != strncmp(path, "/media/sd", 9))
		{
			continue;
		}
		if (NULL == (dp = opendir(path)))
		{
			fprintf(stderr, "open %s error\n", path);
			return -1;
		}
		chdir(path);
		while(NULL != (dirp = readdir(dp)))
		{
			lstat(dirp->d_name, &statbuf);
			if(S_ISDIR(statbuf.st_mode))
			{
				if (0 == strncmp(dirp->d_name, ".", 1) ||
					0 == strcmp(dirp->d_name, "home"))
					continue;
				strcpy(dir_name, dirp->d_name);
				dir_len = strlen(dir_name);
				if (dir_len < 30 && dir_len > 0)
				{
					websWrite(wp, T("<tr><td><input type=\"radio\" name=\"dir_path\" value=\"%s/%s\"></td>"), 
							  path, dir_name);
					websWrite(wp, T("<td>%s/%s</td>"), path, dir_name);
					websWrite(wp, T("<input type=\"hidden\" name=\"dir_part\" value=\"%s\">"), 
							  part);
					websWrite(wp, T("<td>%s</td>"), part);
					websWrite(wp, T("</tr>"));
					dir_count++;
				}
			}
		}
		chdir("/");
		closedir(dp);
	}
	fclose(fp_mount);
	// fprintf(stderr, "dir_count: %d\n", dir_count);

	return 0;
}

static int ShowPartition(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp = fopen(MOUNT_INFO, "r");
	char part[30], path[50]; 
	if (NULL == fp) {
        perror(__FUNCTION__);
		return -1;
	}
	part_count = 0;

	while(EOF != fscanf(fp, "%s %s %*s %*s %*s %*s\n", part, path))
	{
		// if (strncmp(path, "/var", 4) != 0)
		if (0 != strncmp(path, "/media/sd", 9))
		{
			continue;
		}
		websWrite(wp, T("<tr align=center>"));
		websWrite(wp, T("<td><input type=\"radio\" name=\"disk_part\" value=\"%s\"></td>"), 
				  path);
		websWrite(wp, T("<td>%s</td>"), part);
		websWrite(wp, T("<td>%s</td>"), path);
		websWrite(wp, T("</tr>"));
		part_count++;
	}
	fclose(fp);
	// fprintf(stderr, "part_count: %d\n", part_count);

	return 0;
}

static int getMaxVol(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *pp = popen("fdisk -l /dev/sda", "r");
	char maxvol[5], unit[5];
	unsigned int result;

	fscanf(pp, "%*s %*s %s %s %*s %*s\n", maxvol, unit);
	result = atoi(maxvol);
	if (NULL != strstr(unit, "GB,"))
		websWrite(wp, T("%d"), result*1000);
	else if (NULL != strstr(unit, "MB,"))
		websWrite(wp, T("%d"), result);

	return 0;
}

static int isStorageExist(void)
{
	char buf[256];
	FILE *fp = fopen("/proc/mounts", "r");
	if(!fp){
		perror(__FUNCTION__);
		return 0;
	}

	while(fgets(buf, sizeof(buf), fp)){
		if(strstr(buf, USB_STORAGE_SIGN)){
			fclose(fp);
			return 1;
		}
	}

	fclose(fp);
	printf("no usb disk found\n.");
	return 0;
}

/*
 *  taken from "mkimage -l" with few modified....
 */
int checkIfFirmware(char *imagefile)
{
	unsigned int magic;
	FILE *fp = fopen(imagefile, "r");
	if(!fp){
		perror(imagefile);
		return 0;
	}
	if( fread(&magic, sizeof(magic), 1, fp) != 1){
		perror(imagefile);
		fclose(fp);
		return 0;
	}

	fclose(fp);

	if(magic == ntohl(IH_MAGIC))
		return 1;
	else
		return 0;
}


/*
 *  find the firmware recursively in usb disk.
 */
static void lookupFirmware(char *dir_path)
{
	struct dirent *dir;
	struct stat stat_buf;
	char whole_path[1024];

	DIR *d = opendir(dir_path);
	if(!d){
		perror(dir_path);
		return;			// give up to open the directory
	}

	while((dir = readdir(d))){
		if(!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
			continue;

		snprintf(whole_path, 1024, "%s/%s", dir_path, dir->d_name);
		if(stat( whole_path, &stat_buf) == -1){
			perror(dir_path);
			continue;
		}

		if( S_ISDIR( stat_buf.st_mode)){
			lookupFirmware(whole_path);	//recursive
			continue;
		}

		if(stat_buf.st_size < MIN_FIRMWARE_SIZE)
			continue;

		if(checkIfFirmware(whole_path)){
			if(!strlen(firmware_path))
				snprintf(firmware_path, sizeof(firmware_path), "%s", whole_path);
			else
				snprintf(firmware_path, sizeof(firmware_path), "%s\n%s", firmware_path, whole_path);
		}
	}
	closedir(d);
	return;
}

static int isSpaceEnough(char *part)
{
	// Write a big file to test the partition
	// TODO: I'm lazy to use fstatfs() to do the same thing.
	int count;
	char path[1024];
	snprintf(path, 1024, "%s/.delete_me", part);
	FILE *fp = fopen(path, "w");

	if(!fp){
        perror(__FUNCTION__);
		return 0;
	}
	for(count =0; count <MIN_SPACE_FOR_FIRMWARE; count += sizeof(path)){
		if( fwrite(path, sizeof(path), 1, fp) != 1){
			fclose(fp);
			unlink(path);
			sync();
			fprintf(stderr, __FILE__":no enough space left in %s.\n", part);
			return 0;
		}
	}
	fclose(fp);
	unlink(path);
	sync();
	return 1;
}

/*
 *  choose a suitable usb disk partition for the firmware upload.
 *  return NULL if not found.
 */
char *isStorageOK(void)
{
    char buf[256];
	char device_path[256];
	static char mount_path[256];
    FILE *fp = fopen(MOUNT_INFO, "r");
    if(!fp){
        perror(__FUNCTION__);
        return 0;
    }

    while(fgets(buf, sizeof(buf), fp)){
		sscanf(buf, "%s %s", device_path, mount_path);
		if(strstr(mount_path, USB_STORAGE_SIGN)){
			if(!isSpaceEnough(mount_path))
				continue;
			fclose(fp);
			fprintf(stderr, __FILE__":choose USB %s for firmrware space.\n", mount_path);
			return mount_path;
		}
    }

    fclose(fp);
	fprintf(stderr, "No suitable usb disk partition found \n.");
    return NULL;
}

void setFirmwarePath(void)
{
	firmware_path[0] = '\0';
	if(!isStorageExist())
		return;

	lookupFirmware(USB_STORAGE_PATH);
	sync();
	printf("firmware found: %s\n", firmware_path);
}

static void storageGetFirmwarePath(webs_t wp, char_t *path, char_t *query)
{
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain\nPragma: no-cache\nCache-Control: no-cache\n\n"));

	websWrite(wp, "%s", firmware_path);
	websDone(wp, 200);
}

#if defined CONFIG_USER_MTDAAPD 
static void iTunesSrv(webs_t wp, char_t *path, char_t *query)
{
	char_t *enable, *dir_path, *name;

	// fetch from web input
	enable = websGetVar(wp, T("enabled"), T("0"));
	dir_path = websGetVar(wp, T("dir_path"), T(""));
	name = websGetVar(wp, T("srv_name"), T(""));
	// set to nvram
	nvram_bufset(RT2860_NVRAM, "iTunesEnable", enable);
	nvram_bufset(RT2860_NVRAM, "iTunesDir", dir_path);
	nvram_bufset(RT2860_NVRAM, "iTunesSrvName", name);
	nvram_commit(RT2860_NVRAM);

	// setup device
	doSystem("killall mt-daapd; killall mDNSResponder");
	if (strcmp(enable, "1") == 0)
		doSystem("config-iTunes.sh \"%s\" \"%s\" \"%s\"", 
				 name,
				 nvram_bufget(RT2860_NVRAM, "Password"),
				 dir_path);

	// debug print
	websHeader(wp);
	websWrite(wp, T("<h2>iTunes Server Settings</h2><br>\n"));
	websWrite(wp, T("capability: %s<br>\n"), enable);
	websWrite(wp, T("srv_name: %s<br>\n"), name);
	websWrite(wp, T("media library: %s<br>\n"), dir_path);
	websFooter(wp);
	websDone(wp, 200);
}
#endif

/*
 *  Hotpluger signal handler
 */
void hotPluglerHandler(int signo)
{
#if defined CONFIG_RALINKAPP_MPLAYER
	cleanup_musiclist();
#endif
	initUSB();
}

