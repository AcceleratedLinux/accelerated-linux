/* vi: set sw=4 ts=4 sts=4: */
/*
 *	wireless.c -- Wireless Settings 
 *
 *	Copyright (c) Ralink Technology Corporation All Rights Reserved.
 *
 *	$Id: media.c,v 1.9 2009-02-11 10:09:05 chhung Exp $
 */

#include	<stdlib.h>
#include	<dirent.h>
#include	<signal.h>
#include	<unistd.h> 

#include	"webs.h"
#include	"nvram.h"
#include	"media.h"
#include	"config/autoconf.h" //user config

static int getMediaOpt(int eid, webs_t wp, int argc, char_t **argv);
static int getMediaMode(int eid, webs_t wp, int argc, char_t **argv);
static int getMediaOption(int eid, webs_t wp, int argc, char_t **argv);
static int getMediaTarget(int eid, webs_t wp, int argc, char_t **argv);
// static int getMediaVolume(int eid, webs_t wp, int argc, char_t **argv);
// static int getMediaMute(int eid, webs_t wp, int argc, char_t **argv);
static int getMediaConnected(int eid, webs_t wp, int argc, char_t **argv);
static void mediaPlayer(webs_t wp, char_t *path, char_t *query);
static void mediaPlayerList(webs_t wp, char_t *path, char_t *query);
static int showUnSelectedList(int eid, webs_t wp, int argc, char_t **argv);
static int showSelectedList(int eid, webs_t wp, int argc, char_t **argv);

struct status {
	char operate[10];
	char service_mode[2];
	char option[10];
	char target[50];
	// char volume[4];
	// char mute[2];
	char connected[5];
};

struct list {
	char name[230];
	char path[1024];
	int	 selected;
}; 

#define DEBUG(x) do{fprintf(stderr, #x); \
					fprintf(stderr, ": %s\n", x); \
				 }while(0)

//struct status media_state = {"\0", "\0", "\0", "\0", "30", "0"};
struct status media_state;

struct list all_list[1024];

void formDefineMedia(void) {
	websAspDefine(T("getMediaOpt"), getMediaOpt);
	websAspDefine(T("getMediaMode"), getMediaMode);
	websAspDefine(T("getMediaOption"), getMediaOption);
	websAspDefine(T("getMediaTarget"), getMediaTarget);
//	websAspDefine(T("getMediaVolume"), getMediaVolume);
//	websAspDefine(T("getMediaMute"), getMediaMute);
	websAspDefine(T("getMediaConnected"), getMediaConnected);
	websAspDefine(T("showUnSelectedList"), showUnSelectedList);
	websAspDefine(T("showSelectedList"), showSelectedList);

	websFormDefine(T("mediaPlayer"), mediaPlayer);
	websFormDefine(T("mediaPlayerList"), mediaPlayerList);
}

static void ChStatus(int sig)
{
	if (sig == SIGWINCH)
	{
		strcpy(media_state.connected, "ok");
		signal(SIGWINCH, SIG_DFL);
	}
	else if (sig == SIGPWR)
	{
		strcpy(media_state.connected, "fail");
		signal(SIGPWR, SIG_DFL);
	}
}

/* goform/mediaPlayer */
static void mediaPlayer(webs_t wp, char_t *path, char_t *query)
{
	char_t *opt, *target;

	opt = websGetVar(wp, T("hidden_opt"), T(""));
	// DEBUG(opt);

	signal(SIGWINCH, ChStatus);
	signal(SIGPWR, ChStatus);
	if (strcmp(opt, "play") == 0)
	{
		char_t *service_mode = websGetVar(wp, T("service_mode"), T(""));

		memset(&media_state.operate, 0, sizeof(media_state.operate));
		memset(&media_state.service_mode, 0, sizeof(media_state.service_mode));
		memset(&media_state.option, 0, sizeof(media_state.option));
		memset(&media_state.target, 0, sizeof(media_state.target));
		memset(&media_state.connected, 0, sizeof(media_state.connected));
		// DEBUG(service_mode);
		if (strcmp(service_mode, "0") == 0)
		{
			target = websGetVar(wp, T("inet_radio"), T(""));
			doSystem("killall mplayer");
			doSystem("mplayer -cache 256 %s &", target);
		}
		else if (strcmp(service_mode, "1") == 0)
		{
			char_t *option;

			option = websGetVar(wp, T("play_mode"), T(""));
			target = websGetVar(wp, T("play_list"), T(""));
			if (strcmp(option, "random") == 0)
			{
				doSystem("killall mplayer");
				doSystem("mplayer -shuffle -playlist %s &", "/media/sda1/track.txt");
			}
			else
			{
				doSystem("killall mplayer");
				doSystem("mplayer -playlist %s &", "/media/sda1/track.txt");
			}
			strcpy(media_state.option, option);
		}
		else if (strcmp(service_mode, "2") == 0)
		{
			target = websGetVar(wp, T("radio_url"), T(""));
			doSystem("killall mplayer");
			doSystem("mplayer %s &", target);
		}
		strcpy(media_state.operate, opt);
		strcpy(media_state.service_mode, service_mode);
		strcpy(media_state.target, target);
		strcpy(media_state.connected, "load");
	}
	else if (strcmp(opt, "stop") == 0)
	{
		memset(&media_state.operate, 0, sizeof(media_state.operate));
		memset(&media_state.service_mode, 0, sizeof(media_state.service_mode));
		memset(&media_state.option, 0, sizeof(media_state.option));
		memset(&media_state.target, 0, sizeof(media_state.target));
		doSystem("killall mplayer");
		strcpy(media_state.operate, opt);
	}
	websRedirect(wp, "media/player.asp");
}

static int getIndex(char **select)
{
	char *temp;
	int index;

	temp = strstr(*select, " ");
	if (NULL != temp)
	{
		*temp = '\0';
		index = atoi(*select);
		*select = temp+1;
	}
	else if (0 != strlen(*select))
	{
		index = atoi(*select);
		**select = '\0';
	}
	else
		index = 1024;

	return index;
}

/* goform/mediaPlayerList */
static void mediaPlayerList(webs_t wp, char_t *path, char_t *query)
{
	char_t *opt, *select;
	int i;
	FILE *fp_list = fopen("/media/sda1/track.txt", "w");

	opt = websGetVar(wp, T("list_opt"), T(""));
	//DEBUG(opt);

	if (0 == strcmp(opt, "add"))
	{

		select = websGetVar(wp, T("unselected_list"), T(""));
		//DEBUG(select);
		do {
			i = getIndex(&select);
			if (i < 1024)
				all_list[i].selected = 1;
		} while(0 != strlen(select));
	}
	else if (0 == strcmp(opt, "del"))
	{
		select = websGetVar(wp, T("selected_list"), T(""));
		//DEBUG(select);
		do {
			i = getIndex(&select);
			if (i < 1024)
				all_list[i].selected = 0;
		} while(0 != strlen(select));
	}
	i = 0;
	while (0 != strlen(all_list[i].name) && i < 1024)
	{
		if (1 == all_list[i].selected)
		{
			fprintf(fp_list, "%s\n", all_list[i].path);
		}
		i++;
	}
	fclose(fp_list);
	strcpy(media_state.service_mode, "1");

	websRedirect(wp, "media/player.asp");
}

static int getMediaOpt(int eid, webs_t wp, int argc, char_t **argv)
{
	return websWrite(wp, T("%s"), media_state.operate);
}

static int getMediaMode(int eid, webs_t wp, int argc, char_t **argv)
{
	return websWrite(wp, T("%s"), media_state.service_mode);
}

static int getMediaOption(int eid, webs_t wp, int argc, char_t **argv)
{
	return websWrite(wp, T("%s"), media_state.option);
}

static int getMediaTarget(int eid, webs_t wp, int argc, char_t **argv)
{
	return websWrite(wp, T("%s"), media_state.target);
}

/*
static int getMediaVolume(int eid, webs_t wp, int argc, char_t **argv)
{
	return websWrite(wp, T("%s"), media_state.volume);
}

static int getMediaMute(int eid, webs_t wp, int argc, char_t **argv)
{
	return websWrite(wp, T("%s"), media_state.mute);
}
*/

static int getMediaConnected(int eid, webs_t wp, int argc, char_t **argv)
{
	return websWrite(wp, T("%s"), media_state.connected);
}

static void lookupAllList(char *dir_path)
{
	struct dirent *dir;
	struct stat stat_buf;
	char *loc, whole_path[1024];
	DIR *dirp = opendir(dir_path);
	int i = 0;

	if (!dirp)
	{
		perror(dir_path);
		return;			// give up to open the directory
	}

	while((dir = readdir(dirp)))
	{
		if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
			continue;

		snprintf(whole_path, 1024, "%s/%s", dir_path, dir->d_name);
		if (stat(whole_path, &stat_buf) == -1)
		{
			perror(dir_path);
			continue;
		}

		if (S_ISDIR(stat_buf.st_mode))
		{
			lookupAllList(whole_path);	//recursive
			continue;
		}

		if (((NULL != (loc=strstr(dir->d_name, ".mp3"))) || 
		    (NULL != (loc=strstr(dir->d_name, ".MP3")))) && 
			(4 == strlen(dir->d_name)-(int)loc+(int)dir->d_name))
		{
			strcpy(all_list[i].name, dir->d_name);
			strcpy(all_list[i].path, whole_path);
			i++;
		}
	}

	closedir(dirp);
	return;
}

static void lookupSelectList(void)
{
	FILE *fp = fopen("/media/sda1/track.txt", "r");
	char entry[1024];
	int	 elen, i = 0;

	if (NULL == fp) 
	{
		perror(__FUNCTION__);
	} 
	else 
	{
		while(!feof(fp))
		{
			if (NULL != fgets(entry, 1024, fp))
			{
				elen = strlen(entry);
				entry[elen-1] = '\0';
				while(0 != strlen(all_list[i].name) && i < 1024)
				{
					if (0 == strcmp(all_list[i].path, entry))
					{
						all_list[i].selected = 1;
						break;
					}
					i++;
				}
				i = 0;
			}
		}
		fclose(fp);
	}

	return;
}

static int showUnSelectedList(int eid, webs_t wp, int argc, char_t **argv)
{
	int i = 0;

	if (0 == strlen(all_list[0].name))
	{
		memset(all_list, 0, sizeof(all_list));
		lookupAllList("/media");
		lookupSelectList();
	}

	while(0 != strlen(all_list[i].name) && i < 1024)
	{
		if (0 == all_list[i].selected)
		{
			websWrite(wp, T("<option value=\"%d\">%s</option>"), i, all_list[i].name);
		}
		i++;
	}

	return 0;
}

static int showSelectedList(int eid, webs_t wp, int argc, char_t **argv)
{
	int i = 0;

	while(0 != strlen(all_list[i].name) && i < 1024)
	{
		if (1 == all_list[i].selected)
		{
			websWrite(wp, T("<option value=\"%d\">%s</option>"), i, all_list[i].name);
		}
		i++;
	}

	return 0;
}

void cleanup_musiclist(void)
{
	memset(all_list, 0, sizeof(all_list));
}
