/* 
   Unix SMB/CIFS implementation.
   SMB client
   Copyright (C) Andrew Tridgell          1994-1998
   Copyright (C) Simo Sorce               2001-2002
   Copyright (C) Jelmer Vernooij          2003
   Copyright (C) Gerald (Jerry) Carter    2004
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "includes.h"
#include "client/client_proto.h"
#include "include/rpc_client.h"
#ifndef REGISTER
#define REGISTER 0
#endif

extern BOOL AllowDebugChange;
extern BOOL override_logfile;
extern char tar_type;
extern BOOL in_client;
static int port = 0;
pstring cur_dir = "\\";
static pstring cd_path = "";
static pstring service;
static pstring desthost;
static pstring username;
static pstring calling_name;
static BOOL grepable=False;
static char *cmdstr = NULL;

static int io_bufsize = 64512;

static int name_type = 0x20;
extern int max_protocol;

static int process_tok(pstring tok);
static int cmd_help(void);

static TALLOC_CTX *ctx;
#define CREATE_ACCESS_READ READ_CONTROL_ACCESS
static pstring cwd;

/* 30 second timeout on most commands */
#define CLIENT_TIMEOUT (30*1000)
#define SHORT_TIMEOUT (5*1000)

/* value for unused fid field in trans2 secondary request */
#define FID_UNUSED (0xFFFF)

time_t newer_than = 0;
static int archive_level = 0;

static BOOL translation = False;
static BOOL have_ip;

/* clitar bits insert */
extern int blocksize;
extern BOOL tar_inc;
extern BOOL tar_reset;
/* clitar bits end */
 

static BOOL prompt = True;

static BOOL recurse = False;
static BOOL showacls = False;
BOOL lowercase = False;

static struct in_addr dest_ip;

#define SEPARATORS " \t\n\r"

static BOOL abort_mget = True;

static pstring fileselection = "";

extern file_info def_finfo;

/* timing globals */
SMB_BIG_UINT get_total_size = 0;
unsigned int get_total_time_ms = 0;
static SMB_BIG_UINT put_total_size = 0;
static unsigned int put_total_time_ms = 0;

/* totals globals */
static double dir_total;

/* root cli_state connection */

struct cli_state *cli;

static char CLI_DIRSEP_CHAR = '\\';
static char CLI_DIRSEP_STR[] = { '\\', '\0' };

/****************************************************************************
 Write to a local file with CR/LF->LF translation if appropriate. Return the 
 number taken from the buffer. This may not equal the number written.
****************************************************************************/

static int writefile(int f, char *b, int n)
{
	int i;

	if (!translation) {
		return write(f,b,n);
	}

	i = 0;
	while (i < n) {
		if (*b == '\r' && (i<(n-1)) && *(b+1) == '\n') {
			b++;i++;
		}
		if (write(f, b, 1) != 1) {
			break;
		}
		b++;
		i++;
	}
  
	return(i);
}

/****************************************************************************
 Read from a file with LF->CR/LF translation if appropriate. Return the 
 number read. read approx n bytes.
****************************************************************************/

static int readfile(char *b, int n, XFILE *f)
{
	int i;
	int c;

	if (!translation)
		return x_fread(b,1,n,f);
  
	i = 0;
	while (i < (n - 1) && (i < BUFFER_SIZE)) {
		if ((c = x_getc(f)) == EOF) {
			break;
		}
      
		if (c == '\n') { /* change all LFs to CR/LF */
			b[i++] = '\r';
		}
      
		b[i++] = c;
	}
  
	return(i);
}
 
/****************************************************************************
 Send a message.
****************************************************************************/

static void send_message(void)
{
	int total_len = 0;
	int grp_id;

	if (!cli_message_start(cli, desthost, username, &grp_id)) {
		d_printf("message start: %s\n", cli_errstr(cli));
		return;
	}


	d_printf("Connected. Type your message, ending it with a Control-D\n");

	while (!feof(stdin) && total_len < 1600) {
		int maxlen = MIN(1600 - total_len,127);
		pstring msg;
		int l=0;
		int c;

		ZERO_ARRAY(msg);

		for (l=0;l<maxlen && (c=fgetc(stdin))!=EOF;l++) {
			if (c == '\n')
				msg[l++] = '\r';
			msg[l] = c;   
		}

		if (!cli_message_text(cli, msg, l, grp_id)) {
			d_printf("SMBsendtxt failed (%s)\n",cli_errstr(cli));
			return;
		}      
		
		total_len += l;
	}

	if (total_len >= 1600)
		d_printf("the message was truncated to 1600 bytes\n");
	else
		d_printf("sent %d bytes\n",total_len);

	if (!cli_message_end(cli, grp_id)) {
		d_printf("SMBsendend failed (%s)\n",cli_errstr(cli));
		return;
	}      
}

/****************************************************************************
 Check the space on a device.
****************************************************************************/

static int do_dskattr(void)
{
	int total, bsize, avail;
	struct cli_state *targetcli;
	pstring targetpath;

	if ( !cli_resolve_path( "", cli, cur_dir, &targetcli, targetpath ) ) {
		d_printf("Error in dskattr: %s\n", cli_errstr(cli));
		return 1;
	}

	if (!cli_dskattr(targetcli, &bsize, &total, &avail)) {
		d_printf("Error in dskattr: %s\n",cli_errstr(targetcli)); 
		return 1;
	}

	d_printf("\n\t\t%d blocks of size %d. %d blocks available\n",
		 total, bsize, avail);

	return 0;
}

/****************************************************************************
 Show cd/pwd.
****************************************************************************/

static int cmd_pwd(void)
{
	d_printf("Current directory is %s",service);
	d_printf("%s\n",cur_dir);
	return 0;
}

/****************************************************************************
 Change directory - inner section.
****************************************************************************/

static int do_cd(char *newdir)
{
	char *p = newdir;
	pstring saved_dir;
	pstring dname;
	pstring targetpath;
	struct cli_state *targetcli;
	SMB_STRUCT_STAT sbuf;
	uint32 attributes;
	int ret = 1;
      
	dos_format(newdir);

	/* Save the current directory in case the new directory is invalid */

	pstrcpy(saved_dir, cur_dir);

	if (*p == CLI_DIRSEP_CHAR) {
		pstrcpy(cur_dir,p);
	} else {
		pstrcat(cur_dir,p);
		if ((cur_dir[0] != '\0') && (*(cur_dir+strlen(cur_dir)-1) != CLI_DIRSEP_CHAR)) {
			pstrcat(cur_dir, CLI_DIRSEP_STR);
		}
	}
	
	clean_name(cur_dir);
	pstrcpy( dname, cur_dir );
	
	if ( !cli_resolve_path( "", cli, dname, &targetcli, targetpath ) ) {
		d_printf("cd %s: %s\n", dname, cli_errstr(cli));
		pstrcpy(cur_dir,saved_dir);
		goto out;
	}

	if (strequal(targetpath,CLI_DIRSEP_STR )) {
		return 0;
	}
		
	/* Use a trans2_qpathinfo to test directories for modern servers.
	   Except Win9x doesn't support the qpathinfo_basic() call..... */ 
	
	if ( targetcli->protocol >  PROTOCOL_LANMAN2 && !targetcli->win95 ) {
		if ( !cli_qpathinfo_basic( targetcli, targetpath, &sbuf, &attributes ) ) {
			d_printf("cd %s: %s\n", dname, cli_errstr(targetcli));
			pstrcpy(cur_dir,saved_dir);
			goto out;
		}
		
		if ( !(attributes&FILE_ATTRIBUTE_DIRECTORY) ) {
			d_printf("cd %s: not a directory\n", dname);
			pstrcpy(cur_dir,saved_dir);
			goto out;
		}		
	} else {
		pstrcat( targetpath, CLI_DIRSEP_STR );
		clean_name( targetpath );
		
		if ( !cli_chkpath(targetcli, targetpath) ) {
			d_printf("cd %s: %s\n", dname, cli_errstr(targetcli));
			pstrcpy(cur_dir,saved_dir);
			goto out;
		}
	}

	ret = 0;

out:
	
	pstrcpy(cd_path,cur_dir);
	return ret;
}

/****************************************************************************
 Change directory.
****************************************************************************/

static int cmd_cd(void)
{
	pstring buf;
	int rc = 0;
		
	if (next_token_nr(NULL,buf,NULL,sizeof(buf)))
		rc = do_cd(buf);
	else
		d_printf("Current directory is %s\n",cur_dir);

	return rc;
}

/*******************************************************************
 Decide if a file should be operated on.
********************************************************************/

static BOOL do_this_one(file_info *finfo)
{
	if (finfo->mode & aDIR)
		return(True);

	if (*fileselection && 
	    !mask_match(finfo->name,fileselection,False)) {
		DEBUG(3,("mask_match %s failed\n", finfo->name));
		return False;
	}

	if (newer_than && finfo->mtime_ts.tv_sec < newer_than) {
		DEBUG(3,("newer_than %s failed\n", finfo->name));
		return(False);
	}

	if ((archive_level==1 || archive_level==2) && !(finfo->mode & aARCH)) {
		DEBUG(3,("archive %s failed\n", finfo->name));
		return(False);
	}
	
	return(True);
}

/****************************************************************************
 Display info about a file.
****************************************************************************/

static void display_finfo(file_info *finfo)
{
	if (do_this_one(finfo)) {
		time_t t = finfo->mtime_ts.tv_sec; /* the time is assumed to be passed as GMT */
		if (!showacls) {
			d_printf("  %-30s%7.7s %8.0f  %s",
				 finfo->name,
				 attrib_string(finfo->mode),
			 	(double)finfo->size,
				time_to_asc(t));
			dir_total += finfo->size;
		} else {
			pstring afname;
			int fnum;

			/* skip if this is . or .. */
			if ( strequal(finfo->name,"..") || strequal(finfo->name,".") )
				return;
			/* create absolute filename for cli_nt_create() FIXME */
			pstrcpy( afname, cwd);
			pstrcat( afname, CLI_DIRSEP_STR);
			pstrcat( afname, finfo->name);
			/* print file meta date header */
			d_printf( "FILENAME:%s\n", afname);
			d_printf( "MODE:%s\n", attrib_string(finfo->mode));
			d_printf( "SIZE:%.0f\n", (double)finfo->size);
			d_printf( "MTIME:%s", time_to_asc(t));
			fnum = cli_nt_create(finfo->cli, afname, CREATE_ACCESS_READ);
			if (fnum == -1) {
				DEBUG( 0, ("display_finfo() Failed to open %s: %s\n",
					afname,
					cli_errstr( finfo->cli)));
			} else {
				SEC_DESC *sd = NULL;
				sd = cli_query_secdesc(finfo->cli, fnum, ctx);
				if (!sd) {
					DEBUG( 0, ("display_finfo() failed to "
						"get security descriptor: %s",
						cli_errstr( finfo->cli)));
				} else {
					display_sec_desc(sd);
				}
			}
		}
	}
}

/****************************************************************************
 Accumulate size of a file.
****************************************************************************/

static void do_du(file_info *finfo)
{
	if (do_this_one(finfo)) {
		dir_total += finfo->size;
	}
}

static BOOL do_list_recurse;
static BOOL do_list_dirs;
static char *do_list_queue = 0;
static long do_list_queue_size = 0;
static long do_list_queue_start = 0;
static long do_list_queue_end = 0;
static void (*do_list_fn)(file_info *);

/****************************************************************************
 Functions for do_list_queue.
****************************************************************************/

/*
 * The do_list_queue is a NUL-separated list of strings stored in a
 * char*.  Since this is a FIFO, we keep track of the beginning and
 * ending locations of the data in the queue.  When we overflow, we
 * double the size of the char*.  When the start of the data passes
 * the midpoint, we move everything back.  This is logically more
 * complex than a linked list, but easier from a memory management
 * angle.  In any memory error condition, do_list_queue is reset.
 * Functions check to ensure that do_list_queue is non-NULL before
 * accessing it.
 */

static void reset_do_list_queue(void)
{
	SAFE_FREE(do_list_queue);
	do_list_queue_size = 0;
	do_list_queue_start = 0;
	do_list_queue_end = 0;
}

static void init_do_list_queue(void)
{
	reset_do_list_queue();
	do_list_queue_size = 1024;
	do_list_queue = (char *)SMB_MALLOC(do_list_queue_size);
	if (do_list_queue == 0) { 
		d_printf("malloc fail for size %d\n",
			 (int)do_list_queue_size);
		reset_do_list_queue();
	} else {
		memset(do_list_queue, 0, do_list_queue_size);
	}
}

static void adjust_do_list_queue(void)
{
	/*
	 * If the starting point of the queue is more than half way through,
	 * move everything toward the beginning.
	 */

	if (do_list_queue == NULL) {
		DEBUG(4,("do_list_queue is empty\n"));
		do_list_queue_start = do_list_queue_end = 0;
		return;
	}
		
	if (do_list_queue_start == do_list_queue_end) {
		DEBUG(4,("do_list_queue is empty\n"));
		do_list_queue_start = do_list_queue_end = 0;
		*do_list_queue = '\0';
	} else if (do_list_queue_start > (do_list_queue_size / 2)) {
		DEBUG(4,("sliding do_list_queue backward\n"));
		memmove(do_list_queue,
			do_list_queue + do_list_queue_start,
			do_list_queue_end - do_list_queue_start);
		do_list_queue_end -= do_list_queue_start;
		do_list_queue_start = 0;
	}
}

static void add_to_do_list_queue(const char* entry)
{
	long new_end = do_list_queue_end + ((long)strlen(entry)) + 1;
	while (new_end > do_list_queue_size) {
		do_list_queue_size *= 2;
		DEBUG(4,("enlarging do_list_queue to %d\n",
			 (int)do_list_queue_size));
		do_list_queue = (char *)SMB_REALLOC(do_list_queue, do_list_queue_size);
		if (! do_list_queue) {
			d_printf("failure enlarging do_list_queue to %d bytes\n",
				 (int)do_list_queue_size);
			reset_do_list_queue();
		} else {
			memset(do_list_queue + do_list_queue_size / 2,
			       0, do_list_queue_size / 2);
		}
	}
	if (do_list_queue) {
		safe_strcpy_base(do_list_queue + do_list_queue_end, 
				 entry, do_list_queue, do_list_queue_size);
		do_list_queue_end = new_end;
		DEBUG(4,("added %s to do_list_queue (start=%d, end=%d)\n",
			 entry, (int)do_list_queue_start, (int)do_list_queue_end));
	}
}

static char *do_list_queue_head(void)
{
	return do_list_queue + do_list_queue_start;
}

static void remove_do_list_queue_head(void)
{
	if (do_list_queue_end > do_list_queue_start) {
		do_list_queue_start += strlen(do_list_queue_head()) + 1;
		adjust_do_list_queue();
		DEBUG(4,("removed head of do_list_queue (start=%d, end=%d)\n",
			 (int)do_list_queue_start, (int)do_list_queue_end));
	}
}

static int do_list_queue_empty(void)
{
	return (! (do_list_queue && *do_list_queue));
}

/****************************************************************************
 A helper for do_list.
****************************************************************************/

static void do_list_helper(const char *mntpoint, file_info *f, const char *mask, void *state)
{
	char *dir_end;

	/* save the directory */
	pstrcpy( f->dir, mask );
	if ( (dir_end = strrchr( f->dir, CLI_DIRSEP_CHAR )) != NULL ) {
		*dir_end = '\0';
	}

	if (f->mode & aDIR) {
		if (do_list_dirs && do_this_one(f)) {
			do_list_fn(f);
		}
		if (do_list_recurse && 
		    !strequal(f->name,".") && 
		    !strequal(f->name,"..")) {
			pstring mask2;
			char *p;

			if (!f->name[0]) {
				d_printf("Empty dir name returned. Possible server misconfiguration.\n");
				return;
			}

			pstrcpy(mask2, mntpoint);
			pstrcat(mask2, mask);
			p = strrchr_m(mask2,CLI_DIRSEP_CHAR);
			if (!p)
				return;
			p[1] = 0;
			pstrcat(mask2, f->name);
			pstrcat(mask2,CLI_DIRSEP_STR);
			pstrcat(mask2,"*");
			add_to_do_list_queue(mask2);
		}
		return;
	}

	if (do_this_one(f)) {
		do_list_fn(f);
	}
}

/****************************************************************************
 A wrapper around cli_list that adds recursion.
****************************************************************************/

void do_list(const char *mask,uint16 attribute,void (*fn)(file_info *),BOOL rec, BOOL dirs)
{
	static int in_do_list = 0;
	struct cli_state *targetcli;
	pstring targetpath;

	if (in_do_list && rec) {
		fprintf(stderr, "INTERNAL ERROR: do_list called recursively when the recursive flag is true\n");
		exit(1);
	}

	in_do_list = 1;

	do_list_recurse = rec;
	do_list_dirs = dirs;
	do_list_fn = fn;

	if (rec) {
		init_do_list_queue();
		add_to_do_list_queue(mask);
		
		while (! do_list_queue_empty()) {
			/*
			 * Need to copy head so that it doesn't become
			 * invalid inside the call to cli_list.  This
			 * would happen if the list were expanded
			 * during the call.
			 * Fix from E. Jay Berkenbilt (ejb@ql.org)
			 */
			pstring head;
			pstrcpy(head, do_list_queue_head());
			
			/* check for dfs */
			
			if ( !cli_resolve_path( "", cli, head, &targetcli, targetpath ) ) {
				d_printf("do_list: [%s] %s\n", head, cli_errstr(cli));
				remove_do_list_queue_head();
				continue;
			}
			
			cli_list(targetcli, targetpath, attribute, do_list_helper, NULL);
			remove_do_list_queue_head();
			if ((! do_list_queue_empty()) && (fn == display_finfo)) {
				char* next_file = do_list_queue_head();
				char* save_ch = 0;
				if ((strlen(next_file) >= 2) &&
				    (next_file[strlen(next_file) - 1] == '*') &&
				    (next_file[strlen(next_file) - 2] == CLI_DIRSEP_CHAR)) {
					save_ch = next_file +
						strlen(next_file) - 2;
					*save_ch = '\0';
					if (showacls) /* cwd is only used if showacls is on */
						pstrcpy( cwd, next_file);
				}
				if (!showacls) /* don't disturbe the showacls output */
					d_printf("\n%s\n",next_file);
				if (save_ch) {
					*save_ch = CLI_DIRSEP_CHAR;
				}
			}
		}
	} else {
		/* check for dfs */
			
		if ( cli_resolve_path( "", cli, mask, &targetcli, targetpath ) ) {
			if (cli_list(targetcli, targetpath, attribute, do_list_helper, NULL) == -1) 
				d_printf("%s listing %s\n", cli_errstr(targetcli), targetpath);
		}
		else
			d_printf("do_list: [%s] %s\n", mask, cli_errstr(cli));
		
	}

	in_do_list = 0;
	reset_do_list_queue();
}

/****************************************************************************
 Get a directory listing.
****************************************************************************/

static int cmd_dir(void)
{
	uint16 attribute = aDIR | aSYSTEM | aHIDDEN;
	pstring mask;
	pstring buf;
	char *p=buf;
	int rc;
	
	dir_total = 0;
	if (strcmp(cur_dir, CLI_DIRSEP_STR) != 0) {
		pstrcpy(mask,cur_dir);
		if ((mask[0] != '\0') && (mask[strlen(mask)-1]!=CLI_DIRSEP_CHAR))
			pstrcat(mask,CLI_DIRSEP_STR);
	} else {
		pstrcpy(mask, CLI_DIRSEP_STR);
	}
	
	if (next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		dos_format(p);
		if (*p == CLI_DIRSEP_CHAR)
			pstrcpy(mask,p + 1);
		else
			pstrcat(mask,p);
	} else {
		pstrcat(mask,"*");
	}

	if (showacls) {
		/* cwd is only used if showacls is on */
		pstrcpy(cwd, cur_dir);
	}

	do_list(mask, attribute, display_finfo, recurse, True);

	rc = do_dskattr();

	DEBUG(3, ("Total bytes listed: %.0f\n", dir_total));

	return rc;
}

/****************************************************************************
 Get a directory listing.
****************************************************************************/

static int cmd_du(void)
{
	uint16 attribute = aDIR | aSYSTEM | aHIDDEN;
	pstring mask;
	pstring buf;
	char *p=buf;
	int rc;
	
	dir_total = 0;
	pstrcpy(mask,cur_dir);
	if ((mask[0] != '\0') && (mask[strlen(mask)-1]!=CLI_DIRSEP_CHAR))
		pstrcat(mask,CLI_DIRSEP_STR);
	
	if (next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		dos_format(p);
		if (*p == CLI_DIRSEP_CHAR)
			pstrcpy(mask,p);
		else
			pstrcat(mask,p);
	} else {
		pstrcat(mask,"*");
	}

	do_list(mask, attribute, do_du, recurse, True);

	rc = do_dskattr();

	d_printf("Total number of bytes: %.0f\n", dir_total);

	return rc;
}

/****************************************************************************
 Get a file from rname to lname
****************************************************************************/

static int do_get(char *rname, char *lname, BOOL reget)
{  
	int handle = 0, fnum;
	BOOL newhandle = False;
	char *data;
	struct timeval tp_start;
	int read_size = io_bufsize;
	uint16 attr;
	SMB_OFF_T size;
	off_t start = 0;
	off_t nread = 0;
	int rc = 0;
	struct cli_state *targetcli;
	pstring targetname;


	if (lowercase) {
		strlower_m(lname);
	}

	if ( !cli_resolve_path( "", cli, rname, &targetcli, targetname ) ) {
		d_printf("Failed to open %s: %s\n", rname, cli_errstr(cli));
		return 1;
	}

	GetTimeOfDay(&tp_start);
	
	fnum = cli_open(targetcli, targetname, O_RDONLY, DENY_NONE);

	if (fnum == -1) {
		d_printf("%s opening remote file %s\n",cli_errstr(cli),rname);
		return 1;
	}

	if(!strcmp(lname,"-")) {
		handle = fileno(stdout);
	} else {
		if (reget) {
			handle = sys_open(lname, O_WRONLY|O_CREAT, 0644);
			if (handle >= 0) {
				start = sys_lseek(handle, 0, SEEK_END);
				if (start == -1) {
					d_printf("Error seeking local file\n");
					return 1;
				}
			}
		} else {
			handle = sys_open(lname, O_WRONLY|O_CREAT|O_TRUNC, 0644);
		}
		newhandle = True;
	}
	if (handle < 0) {
		d_printf("Error opening local file %s\n",lname);
		return 1;
	}


	if (!cli_qfileinfo(targetcli, fnum, 
			   &attr, &size, NULL, NULL, NULL, NULL, NULL) &&
	    !cli_getattrE(targetcli, fnum, 
			  &attr, &size, NULL, NULL, NULL)) {
		d_printf("getattrib: %s\n",cli_errstr(targetcli));
		return 1;
	}

	DEBUG(1,("getting file %s of size %.0f as %s ", 
		 rname, (double)size, lname));

	if(!(data = (char *)SMB_MALLOC(read_size))) { 
		d_printf("malloc fail for size %d\n", read_size);
		cli_close(targetcli, fnum);
		return 1;
	}

	while (1) {
		int n = cli_read(targetcli, fnum, data, nread + start, read_size);

		if (n <= 0)
			break;
 
		if (writefile(handle,data, n) != n) {
			d_printf("Error writing local file\n");
			rc = 1;
			break;
		}
      
		nread += n;
	}

	if (nread + start < size) {
		DEBUG (0, ("Short read when getting file %s. Only got %ld bytes.\n",
			    rname, (long)nread));

		rc = 1;
	}

	SAFE_FREE(data);
	
	if (!cli_close(targetcli, fnum)) {
		d_printf("Error %s closing remote file\n",cli_errstr(cli));
		rc = 1;
	}

	if (newhandle) {
		close(handle);
	}

	if (archive_level >= 2 && (attr & aARCH)) {
		cli_setatr(cli, rname, attr & ~(uint16)aARCH, 0);
	}

	{
		struct timeval tp_end;
		int this_time;
		
		GetTimeOfDay(&tp_end);
		this_time = 
			(tp_end.tv_sec - tp_start.tv_sec)*1000 +
			(tp_end.tv_usec - tp_start.tv_usec)/1000;
		get_total_time_ms += this_time;
		get_total_size += nread;
		
		DEBUG(1,("(%3.1f kb/s) (average %3.1f kb/s)\n",
			 nread / (1.024*this_time + 1.0e-4),
			 get_total_size / (1.024*get_total_time_ms)));
	}
	
	return rc;
}

/****************************************************************************
 Get a file.
****************************************************************************/

static int cmd_get(void)
{
	pstring lname;
	pstring rname;
	char *p;

	pstrcpy(rname,cur_dir);
	pstrcat(rname,CLI_DIRSEP_STR);
	
	p = rname + strlen(rname);
	
	if (!next_token_nr(NULL,p,NULL,sizeof(rname)-strlen(rname))) {
		d_printf("get <filename>\n");
		return 1;
	}
	pstrcpy(lname,p);
	clean_name(rname);
	
	next_token_nr(NULL,lname,NULL,sizeof(lname));
	
	return do_get(rname, lname, False);
}

/****************************************************************************
 Do an mget operation on one file.
****************************************************************************/

static void do_mget(file_info *finfo)
{
	pstring rname;
	pstring quest;
	pstring saved_curdir;
	pstring mget_mask;

	if (strequal(finfo->name,".") || strequal(finfo->name,".."))
		return;

	if (abort_mget)	{
		d_printf("mget aborted\n");
		return;
	}

	if (finfo->mode & aDIR)
		slprintf(quest,sizeof(pstring)-1,
			 "Get directory %s? ",finfo->name);
	else
		slprintf(quest,sizeof(pstring)-1,
			 "Get file %s? ",finfo->name);

	if (prompt && !yesno(quest))
		return;

	if (!(finfo->mode & aDIR)) {
		pstrcpy(rname,cur_dir);
		pstrcat(rname,finfo->name);
		do_get(rname, finfo->name, False);
		return;
	}

	/* handle directories */
	pstrcpy(saved_curdir,cur_dir);

	pstrcat(cur_dir,finfo->name);
	pstrcat(cur_dir,CLI_DIRSEP_STR);

	unix_format(finfo->name);
	if (lowercase)
		strlower_m(finfo->name);
	
	if (!directory_exist(finfo->name,NULL) && 
	    mkdir(finfo->name,0777) != 0) {
		d_printf("failed to create directory %s\n",finfo->name);
		pstrcpy(cur_dir,saved_curdir);
		return;
	}
	
	if (chdir(finfo->name) != 0) {
		d_printf("failed to chdir to directory %s\n",finfo->name);
		pstrcpy(cur_dir,saved_curdir);
		return;
	}

	pstrcpy(mget_mask,cur_dir);
	pstrcat(mget_mask,"*");
	
	do_list(mget_mask, aSYSTEM | aHIDDEN | aDIR,do_mget,False, True);
	chdir("..");
	pstrcpy(cur_dir,saved_curdir);
}

/****************************************************************************
 View the file using the pager.
****************************************************************************/

static int cmd_more(void)
{
	pstring rname,lname,pager_cmd;
	char *pager;
	int fd;
	int rc = 0;

	pstrcpy(rname,cur_dir);
	pstrcat(rname,CLI_DIRSEP_STR);
	
	slprintf(lname,sizeof(lname)-1, "%s/smbmore.XXXXXX",tmpdir());
	fd = smb_mkstemp(lname);
	if (fd == -1) {
		d_printf("failed to create temporary file for more\n");
		return 1;
	}
	close(fd);

	if (!next_token_nr(NULL,rname+strlen(rname),NULL,sizeof(rname)-strlen(rname))) {
		d_printf("more <filename>\n");
		unlink(lname);
		return 1;
	}
	clean_name(rname);

	rc = do_get(rname, lname, False);

	pager=getenv("PAGER");

	slprintf(pager_cmd,sizeof(pager_cmd)-1,
		 "%s %s",(pager? pager:PAGER), lname);
	system(pager_cmd);
	unlink(lname);
	
	return rc;
}

/****************************************************************************
 Do a mget command.
****************************************************************************/

static int cmd_mget(void)
{
	uint16 attribute = aSYSTEM | aHIDDEN;
	pstring mget_mask;
	pstring buf;
	char *p=buf;

	*mget_mask = 0;

	if (recurse)
		attribute |= aDIR;
	
	abort_mget = False;

	while (next_token_nr(NULL,p,NULL,sizeof(buf))) {
		pstrcpy(mget_mask,cur_dir);
		if ((mget_mask[0] != '\0') && (mget_mask[strlen(mget_mask)-1]!=CLI_DIRSEP_CHAR))
			pstrcat(mget_mask,CLI_DIRSEP_STR);
		
		if (*p == CLI_DIRSEP_CHAR)
			pstrcpy(mget_mask,p);
		else
			pstrcat(mget_mask,p);
		do_list(mget_mask, attribute,do_mget,False,True);
	}

	if (!*mget_mask) {
		pstrcpy(mget_mask,cur_dir);
		if(mget_mask[strlen(mget_mask)-1]!=CLI_DIRSEP_CHAR)
			pstrcat(mget_mask,CLI_DIRSEP_STR);
		pstrcat(mget_mask,"*");
		do_list(mget_mask, attribute,do_mget,False,True);
	}
	
	return 0;
}

/****************************************************************************
 Make a directory of name "name".
****************************************************************************/

static BOOL do_mkdir(char *name)
{
	struct cli_state *targetcli;
	pstring targetname;
	
	if ( !cli_resolve_path( "", cli, name, &targetcli, targetname ) ) {
		d_printf("mkdir %s: %s\n", name, cli_errstr(cli));
		return False;
	}

	if (!cli_mkdir(targetcli, targetname)) {
		d_printf("%s making remote directory %s\n",
			 cli_errstr(targetcli),name);
		return(False);
	}

	return(True);
}

/****************************************************************************
 Show 8.3 name of a file.
****************************************************************************/

static BOOL do_altname(char *name)
{
	pstring altname;
	if (!NT_STATUS_IS_OK(cli_qpathinfo_alt_name(cli, name, altname))) {
		d_printf("%s getting alt name for %s\n",
			 cli_errstr(cli),name);
		return(False);
	}
	d_printf("%s\n", altname);

	return(True);
}

/****************************************************************************
 Exit client.
****************************************************************************/

static int cmd_quit(void)
{
	cli_cm_shutdown();
	talloc_destroy( ctx);
	exit(0);
	/* NOTREACHED */
	return 0;
}

/****************************************************************************
 Make a directory.
****************************************************************************/

static int cmd_mkdir(void)
{
	pstring mask;
	pstring buf;
	char *p=buf;
  
	pstrcpy(mask,cur_dir);

	if (!next_token_nr(NULL,p,NULL,sizeof(buf))) {
		if (!recurse)
			d_printf("mkdir <dirname>\n");
		return 1;
	}
	pstrcat(mask,p);

	if (recurse) {
		pstring ddir;
		pstring ddir2;
		struct cli_state *targetcli;
		pstring targetname;
		*ddir2 = 0;
		
		if ( !cli_resolve_path( "", cli, mask, &targetcli, targetname ) ) {
			return 1;
		}

		pstrcpy(ddir,targetname);
		trim_char(ddir,'.','\0');
		p = strtok(ddir,"/\\");
		while (p) {
			pstrcat(ddir2,p);
			if (!cli_chkpath(targetcli, ddir2)) { 
				do_mkdir(ddir2);
			}
			pstrcat(ddir2,CLI_DIRSEP_STR);
			p = strtok(NULL,"/\\");
		}	 
	} else {
		do_mkdir(mask);
	}
	
	return 0;
}

/****************************************************************************
 Show alt name.
****************************************************************************/

static int cmd_altname(void)
{
	pstring name;
	pstring buf;
	char *p=buf;
  
	pstrcpy(name,cur_dir);

	if (!next_token_nr(NULL,p,NULL,sizeof(buf))) {
		d_printf("altname <file>\n");
		return 1;
	}
	pstrcat(name,p);

	do_altname(name);

	return 0;
}

/****************************************************************************
 Put a single file.
****************************************************************************/

static int do_put(char *rname, char *lname, BOOL reput)
{
	int fnum;
	XFILE *f;
	SMB_OFF_T start = 0;
	off_t nread = 0;
	char *buf = NULL;
	int maxwrite = io_bufsize;
	int rc = 0;
	struct timeval tp_start;
	struct cli_state *targetcli;
	pstring targetname;
	
	if ( !cli_resolve_path( "", cli, rname, &targetcli, targetname ) ) {
		d_printf("Failed to open %s: %s\n", rname, cli_errstr(cli));
		return 1;
	}
	
	GetTimeOfDay(&tp_start);

	if (reput) {
		fnum = cli_open(targetcli, targetname, O_RDWR|O_CREAT, DENY_NONE);
		if (fnum >= 0) {
			if (!cli_qfileinfo(targetcli, fnum, NULL, &start, NULL, NULL, NULL, NULL, NULL) &&
			    !cli_getattrE(targetcli, fnum, NULL, &start, NULL, NULL, NULL)) {
				d_printf("getattrib: %s\n",cli_errstr(cli));
				return 1;
			}
		}
	} else {
		fnum = cli_open(targetcli, targetname, O_RDWR|O_CREAT|O_TRUNC, DENY_NONE);
	}
  
	if (fnum == -1) {
		d_printf("%s opening remote file %s\n",cli_errstr(targetcli),rname);
		return 1;
	}

	/* allow files to be piped into smbclient
	   jdblair 24.jun.98

	   Note that in this case this function will exit(0) rather
	   than returning. */
	if (!strcmp(lname, "-")) {
		f = x_stdin;
		/* size of file is not known */
	} else {
		f = x_fopen(lname,O_RDONLY, 0);
		if (f && reput) {
			if (x_tseek(f, start, SEEK_SET) == -1) {
				d_printf("Error seeking local file\n");
				return 1;
			}
		}
	}

	if (!f) {
		d_printf("Error opening local file %s\n",lname);
		return 1;
	}
  
	DEBUG(1,("putting file %s as %s ",lname,
		 rname));
  
	buf = (char *)SMB_MALLOC(maxwrite);
	if (!buf) {
		d_printf("ERROR: Not enough memory!\n");
		return 1;
	}
	while (!x_feof(f)) {
		int n = maxwrite;
		int ret;

		if ((n = readfile(buf,n,f)) < 1) {
			if((n == 0) && x_feof(f))
				break; /* Empty local file. */

			d_printf("Error reading local file: %s\n", strerror(errno));
			rc = 1;
			break;
		}

		ret = cli_write(targetcli, fnum, 0, buf, nread + start, n);

		if (n != ret) {
			d_printf("Error writing file: %s\n", cli_errstr(cli));
			rc = 1;
			break;
		} 

		nread += n;
	}

	if (!cli_close(targetcli, fnum)) {
		d_printf("%s closing remote file %s\n",cli_errstr(cli),rname);
		x_fclose(f);
		SAFE_FREE(buf);
		return 1;
	}

	
	if (f != x_stdin) {
		x_fclose(f);
	}

	SAFE_FREE(buf);

	{
		struct timeval tp_end;
		int this_time;
		
		GetTimeOfDay(&tp_end);
		this_time = 
			(tp_end.tv_sec - tp_start.tv_sec)*1000 +
			(tp_end.tv_usec - tp_start.tv_usec)/1000;
		put_total_time_ms += this_time;
		put_total_size += nread;
		
		DEBUG(1,("(%3.1f kb/s) (average %3.1f kb/s)\n",
			 nread / (1.024*this_time + 1.0e-4),
			 put_total_size / (1.024*put_total_time_ms)));
	}

	if (f == x_stdin) {
		cli_cm_shutdown();
		exit(0);
	}
	
	return rc;
}

/****************************************************************************
 Put a file.
****************************************************************************/

static int cmd_put(void)
{
	pstring lname;
	pstring rname;
	pstring buf;
	char *p=buf;
	
	pstrcpy(rname,cur_dir);
	pstrcat(rname,CLI_DIRSEP_STR);
  
	if (!next_token_nr(NULL,p,NULL,sizeof(buf))) {
		d_printf("put <filename>\n");
		return 1;
	}
	pstrcpy(lname,p);
  
	if (next_token_nr(NULL,p,NULL,sizeof(buf)))
		pstrcat(rname,p);      
	else
		pstrcat(rname,lname);
	
	clean_name(rname);

	{
		SMB_STRUCT_STAT st;
		/* allow '-' to represent stdin
		   jdblair, 24.jun.98 */
		if (!file_exist(lname,&st) &&
		    (strcmp(lname,"-"))) {
			d_printf("%s does not exist\n",lname);
			return 1;
		}
	}

	return do_put(rname, lname, False);
}

/*************************************
 File list structure.
*************************************/

static struct file_list {
	struct file_list *prev, *next;
	char *file_path;
	BOOL isdir;
} *file_list;

/****************************************************************************
 Free a file_list structure.
****************************************************************************/

static void free_file_list (struct file_list *list_head)
{
	struct file_list *list, *next;
	
	for (list = list_head; list; list = next) {
		next = list->next;
		DLIST_REMOVE(list_head, list);
		SAFE_FREE(list->file_path);
		SAFE_FREE(list);
	}
}

/****************************************************************************
 Seek in a directory/file list until you get something that doesn't start with
 the specified name.
****************************************************************************/

static BOOL seek_list(struct file_list *list, char *name)
{
	while (list) {
		trim_string(list->file_path,"./","\n");
		if (strncmp(list->file_path, name, strlen(name)) != 0) {
			return(True);
		}
		list = list->next;
	}
      
	return(False);
}

/****************************************************************************
 Set the file selection mask.
****************************************************************************/

static int cmd_select(void)
{
	pstrcpy(fileselection,"");
	next_token_nr(NULL,fileselection,NULL,sizeof(fileselection));

	return 0;
}

/****************************************************************************
  Recursive file matching function act as find
  match must be always set to True when calling this function
****************************************************************************/

static int file_find(struct file_list **list, const char *directory, 
		      const char *expression, BOOL match)
{
	SMB_STRUCT_DIR *dir;
	struct file_list *entry;
        struct stat statbuf;
        int ret;
        char *path;
	BOOL isdir;
	const char *dname;

        dir = sys_opendir(directory);
	if (!dir)
		return -1;
	
        while ((dname = readdirname(dir))) {
		if (!strcmp("..", dname))
			continue;
		if (!strcmp(".", dname))
			continue;
		
		if (asprintf(&path, "%s/%s", directory, dname) <= 0) {
			continue;
		}

		isdir = False;
		if (!match || !gen_fnmatch(expression, dname)) {
			if (recurse) {
				ret = stat(path, &statbuf);
				if (ret == 0) {
					if (S_ISDIR(statbuf.st_mode)) {
						isdir = True;
						ret = file_find(list, path, expression, False);
					}
				} else {
					d_printf("file_find: cannot stat file %s\n", path);
				}
				
				if (ret == -1) {
					SAFE_FREE(path);
					sys_closedir(dir);
					return -1;
				}
			}
			entry = SMB_MALLOC_P(struct file_list);
			if (!entry) {
				d_printf("Out of memory in file_find\n");
				sys_closedir(dir);
				return -1;
			}
			entry->file_path = path;
			entry->isdir = isdir;
                        DLIST_ADD(*list, entry);
		} else {
			SAFE_FREE(path);
		}
        }

	sys_closedir(dir);
	return 0;
}

/****************************************************************************
 mput some files.
****************************************************************************/

static int cmd_mput(void)
{
	pstring buf;
	char *p=buf;
	
	while (next_token_nr(NULL,p,NULL,sizeof(buf))) {
		int ret;
		struct file_list *temp_list;
		char *quest, *lname, *rname;
	
		file_list = NULL;

		ret = file_find(&file_list, ".", p, True);
		if (ret) {
			free_file_list(file_list);
			continue;
		}
		
		quest = NULL;
		lname = NULL;
		rname = NULL;
				
		for (temp_list = file_list; temp_list; 
		     temp_list = temp_list->next) {

			SAFE_FREE(lname);
			if (asprintf(&lname, "%s/", temp_list->file_path) <= 0)
				continue;
			trim_string(lname, "./", "/");
			
			/* check if it's a directory */
			if (temp_list->isdir) {
				/* if (!recurse) continue; */
				
				SAFE_FREE(quest);
				if (asprintf(&quest, "Put directory %s? ", lname) < 0) break;
				if (prompt && !yesno(quest)) { /* No */
					/* Skip the directory */
					lname[strlen(lname)-1] = '/';
					if (!seek_list(temp_list, lname))
						break;		    
				} else { /* Yes */
	      				SAFE_FREE(rname);
					if(asprintf(&rname, "%s%s", cur_dir, lname) < 0) break;
					dos_format(rname);
					if (!cli_chkpath(cli, rname) && 
					    !do_mkdir(rname)) {
						DEBUG (0, ("Unable to make dir, skipping..."));
						/* Skip the directory */
						lname[strlen(lname)-1] = '/';
						if (!seek_list(temp_list, lname))
							break;
					}
				}
				continue;
			} else {
				SAFE_FREE(quest);
				if (asprintf(&quest,"Put file %s? ", lname) < 0) break;
				if (prompt && !yesno(quest)) /* No */
					continue;
				
				/* Yes */
				SAFE_FREE(rname);
				if (asprintf(&rname, "%s%s", cur_dir, lname) < 0) break;
			}

			dos_format(rname);

			do_put(rname, lname, False);
		}
		free_file_list(file_list);
		SAFE_FREE(quest);
		SAFE_FREE(lname);
		SAFE_FREE(rname);
	}

	return 0;
}

/****************************************************************************
 Cancel a print job.
****************************************************************************/

static int do_cancel(int job)
{
	if (cli_printjob_del(cli, job)) {
		d_printf("Job %d cancelled\n",job);
		return 0;
	} else {
		d_printf("Error cancelling job %d : %s\n",job,cli_errstr(cli));
		return 1;
	}
}

/****************************************************************************
 Cancel a print job.
****************************************************************************/

static int cmd_cancel(void)
{
	pstring buf;
	int job; 

	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("cancel <jobid> ...\n");
		return 1;
	}
	do {
		job = atoi(buf);
		do_cancel(job);
	} while (next_token_nr(NULL,buf,NULL,sizeof(buf)));
	
	return 0;
}

/****************************************************************************
 Print a file.
****************************************************************************/

static int cmd_print(void)
{
	pstring lname;
	pstring rname;
	char *p;

	if (!next_token_nr(NULL,lname,NULL, sizeof(lname))) {
		d_printf("print <filename>\n");
		return 1;
	}

	pstrcpy(rname,lname);
	p = strrchr_m(rname,'/');
	if (p) {
		slprintf(rname, sizeof(rname)-1, "%s-%d", p+1, (int)sys_getpid());
	}

	if (strequal(lname,"-")) {
		slprintf(rname, sizeof(rname)-1, "stdin-%d", (int)sys_getpid());
	}

	return do_put(rname, lname, False);
}

/****************************************************************************
 Show a print queue entry.
****************************************************************************/

static void queue_fn(struct print_job_info *p)
{
	d_printf("%-6d   %-9d    %s\n", (int)p->id, (int)p->size, p->name);
}

/****************************************************************************
 Show a print queue.
****************************************************************************/

static int cmd_queue(void)
{
	cli_print_queue(cli, queue_fn);
	
	return 0;
}

/****************************************************************************
 Delete some files.
****************************************************************************/

static void do_del(file_info *finfo)
{
	pstring mask;

	pstr_sprintf( mask, "%s%c%s", finfo->dir, CLI_DIRSEP_CHAR, finfo->name );

	if (finfo->mode & aDIR) 
		return;

	if (!cli_unlink(finfo->cli, mask)) {
		d_printf("%s deleting remote file %s\n",cli_errstr(finfo->cli),mask);
	}
}

/****************************************************************************
 Delete some files.
****************************************************************************/

static int cmd_del(void)
{
	pstring mask;
	pstring buf;
	uint16 attribute = aSYSTEM | aHIDDEN;

	if (recurse)
		attribute |= aDIR;
	
	pstrcpy(mask,cur_dir);
	
	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("del <filename>\n");
		return 1;
	}
	pstrcat(mask,buf);

	do_list(mask, attribute,do_del,False,False);
	
	return 0;
}

/****************************************************************************
 Wildcard delete some files.
****************************************************************************/

static int cmd_wdel(void)
{
	pstring mask;
	pstring buf;
	uint16 attribute;
	struct cli_state *targetcli;
	pstring targetname;

	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("wdel 0x<attrib> <wcard>\n");
		return 1;
	}

	attribute = (uint16)strtol(buf, (char **)NULL, 16);

	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("wdel 0x<attrib> <wcard>\n");
		return 1;
	}

	pstrcpy(mask,cur_dir);
	pstrcat(mask,buf);

	if ( !cli_resolve_path( "", cli, mask, &targetcli, targetname ) ) {
		d_printf("cmd_wdel %s: %s\n", mask, cli_errstr(cli));
		return 1;
	}
	
	if (!cli_unlink_full(targetcli, targetname, attribute)) {
		d_printf("%s deleting remote files %s\n",cli_errstr(targetcli),targetname);
	}
	return 0;
}

/****************************************************************************
****************************************************************************/

static int cmd_open(void)
{
	pstring mask;
	pstring buf;
	struct cli_state *targetcli;
	pstring targetname;
	int fnum;

	pstrcpy(mask,cur_dir);
	
	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("open <filename>\n");
		return 1;
	}
	pstrcat(mask,buf);

	if ( !cli_resolve_path( "", cli, mask, &targetcli, targetname ) ) {
		d_printf("open %s: %s\n", mask, cli_errstr(cli));
		return 1;
	}
	
	fnum = cli_nt_create(targetcli, targetname, FILE_READ_DATA|FILE_WRITE_DATA);
	if (fnum == -1) {
		fnum = cli_nt_create(targetcli, targetname, FILE_READ_DATA);
		if (fnum != -1) {
			d_printf("open file %s: for read/write fnum %d\n", targetname, fnum);
		} else {
			d_printf("Failed to open file %s. %s\n", targetname, cli_errstr(cli));
		}
	} else {
		d_printf("open file %s: for read/write fnum %d\n", targetname, fnum);
	}

	return 0;
}

/****************************************************************************
****************************************************************************/

static int cmd_posix_open(void)
{
	pstring mask;
	pstring buf;
	struct cli_state *targetcli;
	pstring targetname;
	mode_t mode;
	int fnum;

	pstrcpy(mask,cur_dir);
	
	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("posix_open <filename> 0<mode>\n");
		return 1;
	}
	pstrcat(mask,buf);
	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("posix_open <filename> 0<mode>\n");
		return 1;
	}
	mode = (mode_t)strtol(buf, (char **)NULL, 8);

	if (!cli_resolve_path( "", cli, mask, &targetcli, targetname )) {
		d_printf("posix_open %s: %s\n", mask, cli_errstr(cli));
		return 1;
	}
	
	fnum = cli_posix_open(targetcli, targetname, O_CREAT|O_RDWR, mode);
	if (fnum == -1) {
		fnum = cli_posix_open(targetcli, targetname, O_CREAT|O_RDONLY, mode);
		if (fnum != -1) {
			d_printf("posix_open file %s: for read/write fnum %d\n", targetname, fnum);
		} else {
			d_printf("Failed to open file %s. %s\n", targetname, cli_errstr(cli));
		}
	} else {
		d_printf("posix_open file %s: for read/write fnum %d\n", targetname, fnum);
	}

	return 0;
}

static int cmd_posix_mkdir(void)
{
	pstring mask;
	pstring buf;
	struct cli_state *targetcli;
	pstring targetname;
	mode_t mode;
	int fnum;

	pstrcpy(mask,cur_dir);
	
	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("posix_mkdir <filename> 0<mode>\n");
		return 1;
	}
	pstrcat(mask,buf);
	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("posix_mkdir <filename> 0<mode>\n");
		return 1;
	}
	mode = (mode_t)strtol(buf, (char **)NULL, 8);

	if (!cli_resolve_path( "", cli, mask, &targetcli, targetname )) {
		d_printf("posix_mkdir %s: %s\n", mask, cli_errstr(cli));
		return 1;
	}

	fnum = cli_posix_mkdir(targetcli, targetname, mode);
	if (fnum == -1) {
		d_printf("Failed to open file %s. %s\n", targetname, cli_errstr(cli));
	} else {
		d_printf("posix_mkdir created directory %s\n", targetname);
	}

	return 0;
}

static int cmd_posix_unlink(void)
{
	pstring mask;
	pstring buf;
	struct cli_state *targetcli;
	pstring targetname;

	pstrcpy(mask,cur_dir);
	
	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("posix_unlink <filename>\n");
		return 1;
	}
	pstrcat(mask,buf);

	if (!cli_resolve_path( "", cli, mask, &targetcli, targetname )) {
		d_printf("posix_unlink %s: %s\n", mask, cli_errstr(cli));
		return 1;
	}
	
	if (!cli_posix_unlink(targetcli, targetname)) {
		d_printf("Failed to unlink file %s. %s\n", targetname, cli_errstr(cli));
	} else {
		d_printf("posix_unlink deleted file %s\n", targetname);
	}

	return 0;
}

static int cmd_posix_rmdir(void)
{
	pstring mask;
	pstring buf;
	struct cli_state *targetcli;
	pstring targetname;

	pstrcpy(mask,cur_dir);
	
	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("posix_rmdir <filename>\n");
		return 1;
	}
	pstrcat(mask,buf);

	if (!cli_resolve_path( "", cli, mask, &targetcli, targetname)) {
		d_printf("posix_rmdir %s: %s\n", mask, cli_errstr(cli));
		return 1;
	}
	
	if (!cli_posix_rmdir(targetcli, targetname)) {
		d_printf("Failed to unlink directory %s. %s\n", targetname, cli_errstr(cli));
	} else {
		d_printf("posix_rmdir deleted directory %s\n", targetname);
	}

	return 0;
}

static int cmd_close(void)
{
	fstring buf;
	int fnum;

	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("close <fnum>\n");
		return 1;
	}

	fnum = atoi(buf);
	/* We really should use the targetcli here.... */
	if (!cli_close(cli, fnum)) {
		d_printf("close %d: %s\n", fnum, cli_errstr(cli));
		return 1;
	}
	return 0;
}

static int cmd_posix(void)
{
	uint16 major, minor;
	uint32 caplow, caphigh;
	pstring caps;

	if (!SERVER_HAS_UNIX_CIFS(cli)) {
		d_printf("Server doesn't support UNIX CIFS extensions.\n");
		return 1;
	}

	if (!cli_unix_extensions_version(cli, &major, &minor, &caplow, &caphigh)) {
		d_printf("Can't get UNIX CIFS extensions version from server.\n");
		return 1;
	}

	d_printf("Server supports CIFS extensions %u.%u\n", (unsigned int)major, (unsigned int)minor);

	*caps = '\0';
        if (caplow & CIFS_UNIX_FCNTL_LOCKS_CAP) {
		pstrcat(caps, "locks ");
	}
        if (caplow & CIFS_UNIX_POSIX_ACLS_CAP) {
		pstrcat(caps, "acls ");
	}
        if (caplow & CIFS_UNIX_XATTTR_CAP) {
		pstrcat(caps, "eas ");
	}
        if (caplow & CIFS_UNIX_POSIX_PATHNAMES_CAP) {
		pstrcat(caps, "pathnames ");
	}
        if (caplow & CIFS_UNIX_POSIX_PATH_OPERATIONS_CAP) {
		pstrcat(caps, "posix_path_operations ");
	}

	if (strlen(caps) > 0 && caps[strlen(caps)-1] == ' ') {
		caps[strlen(caps)-1] = '\0';
	}

	if (!cli_set_unix_extensions_capabilities(cli, major, minor, caplow, caphigh)) {
		d_printf("Can't set UNIX CIFS extensions capabilities. %s.\n", cli_errstr(cli));
		return 1;
	}

	d_printf("Selecting server supported CIFS capabilities %s\n", caps);

	if (caplow & CIFS_UNIX_POSIX_PATHNAMES_CAP) {
		CLI_DIRSEP_CHAR = '/';
		*CLI_DIRSEP_STR = '/';
		pstrcpy(cur_dir, CLI_DIRSEP_STR);
	}

	return 0;
}

static int cmd_lock(void)
{
	fstring buf;
	SMB_BIG_UINT start, len;
	enum brl_type lock_type;
	int fnum;

	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("lock <fnum> [r|w] <hex-start> <hex-len>\n");
		return 1;
	}
	fnum = atoi(buf);

	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("lock <fnum> [r|w] <hex-start> <hex-len>\n");
		return 1;
	}

	if (*buf == 'r' || *buf == 'R') {
		lock_type = READ_LOCK;
	} else if (*buf == 'w' || *buf == 'W') {
		lock_type = WRITE_LOCK;
	} else {
		d_printf("lock <fnum> [r|w] <hex-start> <hex-len>\n");
		return 1;
	}

	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("lock <fnum> [r|w] <hex-start> <hex-len>\n");
		return 1;
	}

	start = (SMB_BIG_UINT)strtol(buf, (char **)NULL, 16);

	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("lock <fnum> [r|w] <hex-start> <hex-len>\n");
		return 1;
	}

	len = (SMB_BIG_UINT)strtol(buf, (char **)NULL, 16);

	if (!cli_posix_lock(cli, fnum, start, len, True, lock_type)) {
		d_printf("lock failed %d: %s\n", fnum, cli_errstr(cli));
	}

	return 0;
}

static int cmd_unlock(void)
{
	fstring buf;
	SMB_BIG_UINT start, len;
	int fnum;

	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("unlock <fnum> <hex-start> <hex-len>\n");
		return 1;
	}
	fnum = atoi(buf);

	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("unlock <fnum> <hex-start> <hex-len>\n");
		return 1;
	}

	start = (SMB_BIG_UINT)strtol(buf, (char **)NULL, 16);

	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("unlock <fnum> <hex-start> <hex-len>\n");
		return 1;
	}

	len = (SMB_BIG_UINT)strtol(buf, (char **)NULL, 16);

	if (!cli_posix_unlock(cli, fnum, start, len)) {
		d_printf("unlock failed %d: %s\n", fnum, cli_errstr(cli));
	}

	return 0;
}


/****************************************************************************
 Remove a directory.
****************************************************************************/

static int cmd_rmdir(void)
{
	pstring mask;
	pstring buf;
	struct cli_state *targetcli;
	pstring targetname;
  
	pstrcpy(mask,cur_dir);
	
	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("rmdir <dirname>\n");
		return 1;
	}
	pstrcat(mask,buf);

	if ( !cli_resolve_path( "", cli, mask, &targetcli, targetname ) ) {
		d_printf("rmdir %s: %s\n", mask, cli_errstr(cli));
		return 1;
	}
	
	if (!cli_rmdir(targetcli, targetname)) {
		d_printf("%s removing remote directory file %s\n",
			 cli_errstr(targetcli),mask);
	}
	
	return 0;
}

/****************************************************************************
 UNIX hardlink.
****************************************************************************/

static int cmd_link(void)
{
	pstring oldname,newname;
	pstring buf,buf2;
	struct cli_state *targetcli;
	pstring targetname;
  
	pstrcpy(oldname,cur_dir);
	pstrcpy(newname,cur_dir);
  
	if (!next_token_nr(NULL,buf,NULL,sizeof(buf)) || 
	    !next_token_nr(NULL,buf2,NULL, sizeof(buf2))) {
		d_printf("link <oldname> <newname>\n");
		return 1;
	}

	pstrcat(oldname,buf);
	pstrcat(newname,buf2);

	if ( !cli_resolve_path( "", cli, oldname, &targetcli, targetname ) ) {
		d_printf("link %s: %s\n", oldname, cli_errstr(cli));
		return 1;
	}
	
	if (!SERVER_HAS_UNIX_CIFS(targetcli)) {
		d_printf("Server doesn't support UNIX CIFS calls.\n");
		return 1;
	}
	
	if (!cli_unix_hardlink(targetcli, targetname, newname)) {
		d_printf("%s linking files (%s -> %s)\n", cli_errstr(targetcli), newname, oldname);
		return 1;
	}  

	return 0;
}

/****************************************************************************
 UNIX symlink.
****************************************************************************/

static int cmd_symlink(void)
{
	pstring oldname,newname;
	pstring buf,buf2;
	struct cli_state *targetcli;
	pstring targetname;
  
	if (!SERVER_HAS_UNIX_CIFS(cli)) {
		d_printf("Server doesn't support UNIX CIFS calls.\n");
		return 1;
	}

	pstrcpy(newname,cur_dir);
	
	if (!next_token_nr(NULL,buf,NULL,sizeof(buf)) || 
	    !next_token_nr(NULL,buf2,NULL, sizeof(buf2))) {
		d_printf("symlink <oldname> <newname>\n");
		return 1;
	}

	pstrcpy(oldname,buf);
	pstrcat(newname,buf2);

	if ( !cli_resolve_path( "", cli, oldname, &targetcli, targetname ) ) {
		d_printf("link %s: %s\n", oldname, cli_errstr(cli));
		return 1;
	}

	if (!cli_unix_symlink(targetcli, targetname, newname)) {
		d_printf("%s symlinking files (%s -> %s)\n",
			cli_errstr(targetcli), newname, targetname);
		return 1;
	} 

	return 0;
}

/****************************************************************************
 UNIX chmod.
****************************************************************************/

static int cmd_chmod(void)
{
	pstring src;
	mode_t mode;
	pstring buf, buf2;
	struct cli_state *targetcli;
	pstring targetname;
  
	pstrcpy(src,cur_dir);
	
	if (!next_token_nr(NULL,buf,NULL,sizeof(buf)) || 
	    !next_token_nr(NULL,buf2,NULL, sizeof(buf2))) {
		d_printf("chmod mode file\n");
		return 1;
	}

	mode = (mode_t)strtol(buf, NULL, 8);
	pstrcat(src,buf2);

	if ( !cli_resolve_path( "", cli, src, &targetcli, targetname ) ) {
		d_printf("chmod %s: %s\n", src, cli_errstr(cli));
		return 1;
	}
	
	if (!SERVER_HAS_UNIX_CIFS(targetcli)) {
		d_printf("Server doesn't support UNIX CIFS calls.\n");
		return 1;
	}
	
	if (!cli_unix_chmod(targetcli, targetname, mode)) {
		d_printf("%s chmod file %s 0%o\n",
			cli_errstr(targetcli), src, (unsigned int)mode);
		return 1;
	} 

	return 0;
}

static const char *filetype_to_str(mode_t mode)
{
	if (S_ISREG(mode)) {
		return "regular file";
	} else if (S_ISDIR(mode)) {
		return "directory";
	} else 
#ifdef S_ISCHR
	if (S_ISCHR(mode)) {
		return "character device";
	} else
#endif
#ifdef S_ISBLK
	if (S_ISBLK(mode)) {
		return "block device";
	} else
#endif
#ifdef S_ISFIFO
	if (S_ISFIFO(mode)) {
		return "fifo";
	} else
#endif
#ifdef S_ISLNK
	if (S_ISLNK(mode)) {
		return "symbolic link";
	} else
#endif
#ifdef S_ISSOCK
	if (S_ISSOCK(mode)) {
		return "socket";
	} else
#endif
	return "";
}

static char rwx_to_str(mode_t m, mode_t bt, char ret)
{
	if (m & bt) {
		return ret;
	} else {
		return '-';
	}
}

static char *unix_mode_to_str(char *s, mode_t m)
{
	char *p = s;
	const char *str = filetype_to_str(m);

	switch(str[0]) {
		case 'd':
			*p++ = 'd';
			break;
		case 'c':
			*p++ = 'c';
			break;
		case 'b':
			*p++ = 'b';
			break;
		case 'f':
			*p++ = 'p';
			break;
		case 's':
			*p++ = str[1] == 'y' ? 'l' : 's';
			break;
		case 'r':
		default:
			*p++ = '-';
			break;
	}
	*p++ = rwx_to_str(m, S_IRUSR, 'r');
	*p++ = rwx_to_str(m, S_IWUSR, 'w');
	*p++ = rwx_to_str(m, S_IXUSR, 'x');
	*p++ = rwx_to_str(m, S_IRGRP, 'r');
	*p++ = rwx_to_str(m, S_IWGRP, 'w');
	*p++ = rwx_to_str(m, S_IXGRP, 'x');
	*p++ = rwx_to_str(m, S_IROTH, 'r');
	*p++ = rwx_to_str(m, S_IWOTH, 'w');
	*p++ = rwx_to_str(m, S_IXOTH, 'x');
	*p++ = '\0';
	return s;
}

/****************************************************************************
 Utility function for UNIX getfacl.
****************************************************************************/

static char *perms_to_string(fstring permstr, unsigned char perms)
{
	fstrcpy(permstr, "---");
	if (perms & SMB_POSIX_ACL_READ) {
		permstr[0] = 'r';
	}
	if (perms & SMB_POSIX_ACL_WRITE) {
		permstr[1] = 'w';
	}
	if (perms & SMB_POSIX_ACL_EXECUTE) {
		permstr[2] = 'x';
	}
	return permstr;
}

/****************************************************************************
 UNIX getfacl.
****************************************************************************/

static int cmd_getfacl(void)
{
	pstring src, name;
	uint16 major, minor;
	uint32 caplow, caphigh;
	char *retbuf = NULL;
	size_t rb_size = 0;
	SMB_STRUCT_STAT sbuf;
	uint16 num_file_acls = 0;
	uint16 num_dir_acls = 0;
	uint16 i;
	struct cli_state *targetcli;
	pstring targetname;
 
	pstrcpy(src,cur_dir);
	
	if (!next_token_nr(NULL,name,NULL,sizeof(name))) {
		d_printf("stat file\n");
		return 1;
	}

	pstrcat(src,name);
	
	if ( !cli_resolve_path( "", cli, src, &targetcli, targetname ) ) {
		d_printf("stat %s: %s\n", src, cli_errstr(cli));
		return 1;
	}
	
	if (!SERVER_HAS_UNIX_CIFS(targetcli)) {
		d_printf("Server doesn't support UNIX CIFS calls.\n");
		return 1;
	}
	
	if (!cli_unix_extensions_version(targetcli, &major, &minor, &caplow, &caphigh)) {
		d_printf("Can't get UNIX CIFS version from server.\n");
		return 1;
	}

	if (!(caplow & CIFS_UNIX_POSIX_ACLS_CAP)) {
		d_printf("This server supports UNIX extensions but doesn't support POSIX ACLs.\n");
		return 1;
	}

	if (!cli_unix_stat(targetcli, targetname, &sbuf)) {
		d_printf("%s getfacl doing a stat on file %s\n",
			cli_errstr(targetcli), src);
		return 1;
	} 

	if (!cli_unix_getfacl(targetcli, targetname, &rb_size, &retbuf)) {
		d_printf("%s getfacl file %s\n",
			cli_errstr(targetcli), src);
		return 1;
	} 

	/* ToDo : Print out the ACL values. */
	if (SVAL(retbuf,0) != SMB_POSIX_ACL_VERSION || rb_size < 6) {
		d_printf("getfacl file %s, unknown POSIX acl version %u.\n",
			src, (unsigned int)CVAL(retbuf,0) );
		SAFE_FREE(retbuf);
		return 1;
	}

	num_file_acls = SVAL(retbuf,2);
	num_dir_acls = SVAL(retbuf,4);
	if (rb_size != SMB_POSIX_ACL_HEADER_SIZE + SMB_POSIX_ACL_ENTRY_SIZE*(num_file_acls+num_dir_acls)) {
		d_printf("getfacl file %s, incorrect POSIX acl buffer size (should be %u, was %u).\n",
			src,
			(unsigned int)(SMB_POSIX_ACL_HEADER_SIZE + SMB_POSIX_ACL_ENTRY_SIZE*(num_file_acls+num_dir_acls)),
			(unsigned int)rb_size);

		SAFE_FREE(retbuf);
		return 1;
	}

	d_printf("# file: %s\n", src);
	d_printf("# owner: %u\n# group: %u\n", (unsigned int)sbuf.st_uid, (unsigned int)sbuf.st_gid);

	if (num_file_acls == 0 && num_dir_acls == 0) {
		d_printf("No acls found.\n");
	}

	for (i = 0; i < num_file_acls; i++) {
		uint32 uorg;
		fstring permstring;
		unsigned char tagtype = CVAL(retbuf, SMB_POSIX_ACL_HEADER_SIZE+(i*SMB_POSIX_ACL_ENTRY_SIZE));
		unsigned char perms = CVAL(retbuf, SMB_POSIX_ACL_HEADER_SIZE+(i*SMB_POSIX_ACL_ENTRY_SIZE)+1);

		switch(tagtype) {
			case SMB_POSIX_ACL_USER_OBJ:
				d_printf("user::");
				break;
			case SMB_POSIX_ACL_USER:
				uorg = IVAL(retbuf,SMB_POSIX_ACL_HEADER_SIZE+(i*SMB_POSIX_ACL_ENTRY_SIZE)+2);
				d_printf("user:%u:", uorg);
				break;
			case SMB_POSIX_ACL_GROUP_OBJ:
				d_printf("group::");
				break;
			case SMB_POSIX_ACL_GROUP:
				uorg = IVAL(retbuf,SMB_POSIX_ACL_HEADER_SIZE+(i*SMB_POSIX_ACL_ENTRY_SIZE)+2);
				d_printf("group:%u", uorg);
				break;
			case SMB_POSIX_ACL_MASK:
				d_printf("mask::");
				break;
			case SMB_POSIX_ACL_OTHER:
				d_printf("other::");
				break;
			default:
				d_printf("getfacl file %s, incorrect POSIX acl tagtype (%u).\n",
					src, (unsigned int)tagtype );
				SAFE_FREE(retbuf);
				return 1;
		}

		d_printf("%s\n", perms_to_string(permstring, perms));
	}

	for (i = 0; i < num_dir_acls; i++) {
		uint32 uorg;
		fstring permstring;
		unsigned char tagtype = CVAL(retbuf, SMB_POSIX_ACL_HEADER_SIZE+((i+num_file_acls)*SMB_POSIX_ACL_ENTRY_SIZE));
		unsigned char perms = CVAL(retbuf, SMB_POSIX_ACL_HEADER_SIZE+((i+num_file_acls)*SMB_POSIX_ACL_ENTRY_SIZE)+1);

		switch(tagtype) {
			case SMB_POSIX_ACL_USER_OBJ:
				d_printf("default:user::");
				break;
			case SMB_POSIX_ACL_USER:
				uorg = IVAL(retbuf,SMB_POSIX_ACL_HEADER_SIZE+((i+num_file_acls)*SMB_POSIX_ACL_ENTRY_SIZE)+2);
				d_printf("default:user:%u:", uorg);
				break;
			case SMB_POSIX_ACL_GROUP_OBJ:
				d_printf("default:group::");
				break;
			case SMB_POSIX_ACL_GROUP:
				uorg = IVAL(retbuf,SMB_POSIX_ACL_HEADER_SIZE+((i+num_file_acls)*SMB_POSIX_ACL_ENTRY_SIZE)+2);
				d_printf("default:group:%u", uorg);
				break;
			case SMB_POSIX_ACL_MASK:
				d_printf("default:mask::");
				break;
			case SMB_POSIX_ACL_OTHER:
				d_printf("default:other::");
				break;
			default:
				d_printf("getfacl file %s, incorrect POSIX acl tagtype (%u).\n",
					src, (unsigned int)tagtype );
				SAFE_FREE(retbuf);
				return 1;
		}

		d_printf("%s\n", perms_to_string(permstring, perms));
	}

	SAFE_FREE(retbuf);
	return 0;
}

/****************************************************************************
 UNIX stat.
****************************************************************************/

static int cmd_stat(void)
{
	pstring src, name;
	fstring mode_str;
	SMB_STRUCT_STAT sbuf;
	struct cli_state *targetcli;
	struct tm *lt;
	pstring targetname;
 
	if (!SERVER_HAS_UNIX_CIFS(cli)) {
		d_printf("Server doesn't support UNIX CIFS calls.\n");
		return 1;
	}

	pstrcpy(src,cur_dir);
	
	if (!next_token_nr(NULL,name,NULL,sizeof(name))) {
		d_printf("stat file\n");
		return 1;
	}

	pstrcat(src,name);

	
	if ( !cli_resolve_path( "", cli, src, &targetcli, targetname ) ) {
		d_printf("stat %s: %s\n", src, cli_errstr(cli));
		return 1;
	}
	
	if (!cli_unix_stat(targetcli, targetname, &sbuf)) {
		d_printf("%s stat file %s\n",
			cli_errstr(targetcli), src);
		return 1;
	} 

	/* Print out the stat values. */
	d_printf("File: %s\n", src);
	d_printf("Size: %-12.0f\tBlocks: %u\t%s\n",
		(double)sbuf.st_size,
		(unsigned int)sbuf.st_blocks,
		filetype_to_str(sbuf.st_mode));

#if defined(S_ISCHR) && defined(S_ISBLK)
	if (S_ISCHR(sbuf.st_mode) || S_ISBLK(sbuf.st_mode)) {
		d_printf("Inode: %.0f\tLinks: %u\tDevice type: %u,%u\n",
			(double)sbuf.st_ino,
			(unsigned int)sbuf.st_nlink,
			unix_dev_major(sbuf.st_rdev),
			unix_dev_minor(sbuf.st_rdev));
	} else 
#endif
		d_printf("Inode: %.0f\tLinks: %u\n",
			(double)sbuf.st_ino,
			(unsigned int)sbuf.st_nlink);

	d_printf("Access: (0%03o/%s)\tUid: %u\tGid: %u\n",
		((int)sbuf.st_mode & 0777),
		unix_mode_to_str(mode_str, sbuf.st_mode),
		(unsigned int)sbuf.st_uid, 
		(unsigned int)sbuf.st_gid);

	lt = localtime(&sbuf.st_atime);
	if (lt) {
		strftime(mode_str, sizeof(mode_str), "%Y-%m-%d %T %z", lt);
	} else {
		fstrcpy(mode_str, "unknown");
	}
	d_printf("Access: %s\n", mode_str);

	lt = localtime(&sbuf.st_mtime);
	if (lt) {
		strftime(mode_str, sizeof(mode_str), "%Y-%m-%d %T %z", lt);
	} else {
		fstrcpy(mode_str, "unknown");
	}
	d_printf("Modify: %s\n", mode_str);

	lt = localtime(&sbuf.st_ctime);
	if (lt) {
		strftime(mode_str, sizeof(mode_str), "%Y-%m-%d %T %z", lt);
	} else {
		fstrcpy(mode_str, "unknown");
	}
	d_printf("Change: %s\n", mode_str);
	
	return 0;
}


/****************************************************************************
 UNIX chown.
****************************************************************************/

static int cmd_chown(void)
{
	pstring src;
	uid_t uid;
	gid_t gid;
	pstring buf, buf2, buf3;
	struct cli_state *targetcli;
	pstring targetname;
  
	pstrcpy(src,cur_dir);
	
	if (!next_token_nr(NULL,buf,NULL,sizeof(buf)) || 
	    !next_token_nr(NULL,buf2,NULL, sizeof(buf2)) ||
	    !next_token_nr(NULL,buf3,NULL, sizeof(buf3))) {
		d_printf("chown uid gid file\n");
		return 1;
	}

	uid = (uid_t)atoi(buf);
	gid = (gid_t)atoi(buf2);
	pstrcat(src,buf3);

	if ( !cli_resolve_path( "", cli, src, &targetcli, targetname ) ) {
		d_printf("chown %s: %s\n", src, cli_errstr(cli));
		return 1;
	}

	if (!SERVER_HAS_UNIX_CIFS(targetcli)) {
		d_printf("Server doesn't support UNIX CIFS calls.\n");
		return 1;
	}
	
	if (!cli_unix_chown(targetcli, targetname, uid, gid)) {
		d_printf("%s chown file %s uid=%d, gid=%d\n",
			cli_errstr(targetcli), src, (int)uid, (int)gid);
		return 1;
	} 

	return 0;
}

/****************************************************************************
 Rename some file.
****************************************************************************/

static int cmd_rename(void)
{
	pstring src,dest;
	pstring buf,buf2;
	struct cli_state *targetcli;
	pstring targetname;
  
	pstrcpy(src,cur_dir);
	pstrcpy(dest,cur_dir);
	
	if (!next_token_nr(NULL,buf,NULL,sizeof(buf)) || 
	    !next_token_nr(NULL,buf2,NULL, sizeof(buf2))) {
		d_printf("rename <src> <dest>\n");
		return 1;
	}

	pstrcat(src,buf);
	pstrcat(dest,buf2);

	if ( !cli_resolve_path( "", cli, src, &targetcli, targetname ) ) {
		d_printf("chown %s: %s\n", src, cli_errstr(cli));
		return 1;
	}

	if (!cli_rename(targetcli, targetname, dest)) {
		d_printf("%s renaming files\n",cli_errstr(targetcli));
		return 1;
	}
	
	return 0;
}

/****************************************************************************
 Print the volume name.
****************************************************************************/

static int cmd_volume(void)
{
	fstring volname;
	uint32 serial_num;
	time_t create_date;
  
	if (!cli_get_fs_volume_info(cli, volname, &serial_num, &create_date)) {
		d_printf("Errr %s getting volume info\n",cli_errstr(cli));
		return 1;
	}
	
	d_printf("Volume: |%s| serial number 0x%x\n", volname, (unsigned int)serial_num);
	return 0;
}

/****************************************************************************
 Hard link files using the NT call.
****************************************************************************/

static int cmd_hardlink(void)
{
	pstring src,dest;
	pstring buf,buf2;
	struct cli_state *targetcli;
	pstring targetname;
  
	pstrcpy(src,cur_dir);
	pstrcpy(dest,cur_dir);
	
	if (!next_token_nr(NULL,buf,NULL,sizeof(buf)) || 
	    !next_token_nr(NULL,buf2,NULL, sizeof(buf2))) {
		d_printf("hardlink <src> <dest>\n");
		return 1;
	}

	pstrcat(src,buf);
	pstrcat(dest,buf2);

	if ( !cli_resolve_path( "", cli, src, &targetcli, targetname ) ) {
		d_printf("hardlink %s: %s\n", src, cli_errstr(cli));
		return 1;
	}
	
	if (!SERVER_HAS_UNIX_CIFS(targetcli)) {
		d_printf("Server doesn't support UNIX CIFS calls.\n");
		return 1;
	}
	
	if (!cli_nt_hardlink(targetcli, targetname, dest)) {
		d_printf("%s doing an NT hard link of files\n",cli_errstr(targetcli));
		return 1;
	}
	
	return 0;
}

/****************************************************************************
 Toggle the prompt flag.
****************************************************************************/

static int cmd_prompt(void)
{
	prompt = !prompt;
	DEBUG(2,("prompting is now %s\n",prompt?"on":"off"));
	
	return 1;
}

/****************************************************************************
 Set the newer than time.
****************************************************************************/

static int cmd_newer(void)
{
	pstring buf;
	BOOL ok;
	SMB_STRUCT_STAT sbuf;

	ok = next_token_nr(NULL,buf,NULL,sizeof(buf));
	if (ok && (sys_stat(buf,&sbuf) == 0)) {
		newer_than = sbuf.st_mtime;
		DEBUG(1,("Getting files newer than %s",
			 time_to_asc(newer_than)));
	} else {
		newer_than = 0;
	}

	if (ok && newer_than == 0) {
		d_printf("Error setting newer-than time\n");
		return 1;
	}

	return 0;
}

/****************************************************************************
 Set the archive level.
****************************************************************************/

static int cmd_archive(void)
{
	pstring buf;

	if (next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		archive_level = atoi(buf);
	} else
		d_printf("Archive level is %d\n",archive_level);

	return 0;
}

/****************************************************************************
 Toggle the lowercaseflag.
****************************************************************************/

static int cmd_lowercase(void)
{
	lowercase = !lowercase;
	DEBUG(2,("filename lowercasing is now %s\n",lowercase?"on":"off"));

	return 0;
}

/****************************************************************************
 Toggle the case sensitive flag.
****************************************************************************/

static int cmd_setcase(void)
{
	BOOL orig_case_sensitive = cli_set_case_sensitive(cli, False);

	cli_set_case_sensitive(cli, !orig_case_sensitive);
	DEBUG(2,("filename case sensitivity is now %s\n",!orig_case_sensitive ?
		"on":"off"));

	return 0;
}

/****************************************************************************
 Toggle the showacls flag.
****************************************************************************/

static int cmd_showacls(void)
{
	showacls = !showacls;
	DEBUG(2,("showacls is now %s\n",showacls?"on":"off"));

	if (!ctx && showacls)
		ctx = talloc_init("smbclient:showacls");
		if (!ctx) {
			DEBUG( 0, ("cmd_showacls() out of memory.  talloc_init() failed.\n"));
	}

	return 0;
}


/****************************************************************************
 Toggle the recurse flag.
****************************************************************************/

static int cmd_recurse(void)
{
	recurse = !recurse;
	DEBUG(2,("directory recursion is now %s\n",recurse?"on":"off"));

	return 0;
}

/****************************************************************************
 Toggle the translate flag.
****************************************************************************/

static int cmd_translate(void)
{
	translation = !translation;
	DEBUG(2,("CR/LF<->LF and print text translation now %s\n",
		 translation?"on":"off"));

	return 0;
}

/****************************************************************************
 Do the lcd command.
 ****************************************************************************/

static int cmd_lcd(void)
{
	pstring buf;
	pstring d;
	
	if (next_token_nr(NULL,buf,NULL,sizeof(buf)))
		chdir(buf);
	DEBUG(2,("the local directory is now %s\n",sys_getwd(d)));

	return 0;
}

/****************************************************************************
 Get a file restarting at end of local file.
 ****************************************************************************/

static int cmd_reget(void)
{
	pstring local_name;
	pstring remote_name;
	char *p;

	pstrcpy(remote_name, cur_dir);
	pstrcat(remote_name, CLI_DIRSEP_STR);
	
	p = remote_name + strlen(remote_name);
	
	if (!next_token_nr(NULL, p, NULL, sizeof(remote_name) - strlen(remote_name))) {
		d_printf("reget <filename>\n");
		return 1;
	}
	pstrcpy(local_name, p);
	clean_name(remote_name);
	
	next_token_nr(NULL, local_name, NULL, sizeof(local_name));
	
	return do_get(remote_name, local_name, True);
}

/****************************************************************************
 Put a file restarting at end of local file.
 ****************************************************************************/

static int cmd_reput(void)
{
	pstring local_name;
	pstring remote_name;
	pstring buf;
	char *p = buf;
	SMB_STRUCT_STAT st;
	
	pstrcpy(remote_name, cur_dir);
	pstrcat(remote_name, CLI_DIRSEP_STR);
  
	if (!next_token_nr(NULL, p, NULL, sizeof(buf))) {
		d_printf("reput <filename>\n");
		return 1;
	}
	pstrcpy(local_name, p);
  
	if (!file_exist(local_name, &st)) {
		d_printf("%s does not exist\n", local_name);
		return 1;
	}

	if (next_token_nr(NULL, p, NULL, sizeof(buf)))
		pstrcat(remote_name, p);
	else
		pstrcat(remote_name, local_name);
	
	clean_name(remote_name);

	return do_put(remote_name, local_name, True);
}

/****************************************************************************
 List a share name.
 ****************************************************************************/

static void browse_fn(const char *name, uint32 m, 
                      const char *comment, void *state)
{
        fstring typestr;

        *typestr=0;

        switch (m & 7)
        {
          case STYPE_DISKTREE:
            fstrcpy(typestr,"Disk"); break;
          case STYPE_PRINTQ:
            fstrcpy(typestr,"Printer"); break;
          case STYPE_DEVICE:
            fstrcpy(typestr,"Device"); break;
          case STYPE_IPC:
            fstrcpy(typestr,"IPC"); break;
        }
	/* FIXME: If the remote machine returns non-ascii characters
	   in any of these fields, they can corrupt the output.  We
	   should remove them. */
	if (!grepable) {
		d_printf("\t%-15s %-10.10s%s\n",
               		name,typestr,comment);
	} else {
		d_printf ("%s|%s|%s\n",typestr,name,comment);
	}
}

static BOOL browse_host_rpc(BOOL sort)
{
	NTSTATUS status;
	struct rpc_pipe_client *pipe_hnd;
	TALLOC_CTX *mem_ctx;
	ENUM_HND enum_hnd;
	WERROR werr;
	SRV_SHARE_INFO_CTR ctr;
	int i;

	mem_ctx = talloc_new(NULL);
	if (mem_ctx == NULL) {
		DEBUG(0, ("talloc_new failed\n"));
		return False;
	}

	init_enum_hnd(&enum_hnd, 0);

	pipe_hnd = cli_rpc_pipe_open_noauth(cli, PI_SRVSVC, &status);

	if (pipe_hnd == NULL) {
		DEBUG(10, ("Could not connect to srvsvc pipe: %s\n",
			   nt_errstr(status)));
		TALLOC_FREE(mem_ctx);
		return False;
	}

	werr = rpccli_srvsvc_net_share_enum(pipe_hnd, mem_ctx, 1, &ctr,
					    0xffffffff, &enum_hnd);

	if (!W_ERROR_IS_OK(werr)) {
		TALLOC_FREE(mem_ctx);
		cli_rpc_pipe_close(pipe_hnd);
		return False;
	}

	for (i=0; i<ctr.num_entries; i++) {
		SRV_SHARE_INFO_1 *info = &ctr.share.info1[i];
		char *name, *comment;
		name = rpcstr_pull_unistr2_talloc(
			mem_ctx, &info->info_1_str.uni_netname);
		comment = rpcstr_pull_unistr2_talloc(
			mem_ctx, &info->info_1_str.uni_remark);
		browse_fn(name, info->info_1.type, comment, NULL);
	}

	TALLOC_FREE(mem_ctx);
	cli_rpc_pipe_close(pipe_hnd);
	return True;
}

/****************************************************************************
 Try and browse available connections on a host.
****************************************************************************/

static BOOL browse_host(BOOL sort)
{
	int ret;
	if (!grepable) {
	        d_printf("\n\tSharename       Type      Comment\n");
	        d_printf("\t---------       ----      -------\n");
	}

	if (browse_host_rpc(sort)) {
		return True;
	}

	if((ret = cli_RNetShareEnum(cli, browse_fn, NULL)) == -1)
		d_printf("Error returning browse list: %s\n", cli_errstr(cli));

	return (ret != -1);
}

/****************************************************************************
 List a server name.
****************************************************************************/

static void server_fn(const char *name, uint32 m, 
                      const char *comment, void *state)
{
	
	if (!grepable){
		d_printf("\t%-16s     %s\n", name, comment);
	} else {
		d_printf("%s|%s|%s\n",(char *)state, name, comment);
	}
}

/****************************************************************************
 Try and browse available connections on a host.
****************************************************************************/

static BOOL list_servers(const char *wk_grp)
{
	fstring state;

	if (!cli->server_domain)
		return False;

	if (!grepable) {
        	d_printf("\n\tServer               Comment\n");
        	d_printf("\t---------            -------\n");
	};
	fstrcpy( state, "Server" );
	cli_NetServerEnum(cli, cli->server_domain, SV_TYPE_ALL, server_fn,
			  state);

	if (!grepable) {
	        d_printf("\n\tWorkgroup            Master\n");
	        d_printf("\t---------            -------\n");
	}; 

	fstrcpy( state, "Workgroup" );
	cli_NetServerEnum(cli, cli->server_domain, SV_TYPE_DOMAIN_ENUM,
			  server_fn, state);
	return True;
}

/****************************************************************************
 Print or set current VUID
****************************************************************************/

static int cmd_vuid(void)
{
	fstring buf;
	
	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("Current VUID is %d\n", cli->vuid);
		return 0;
	}

	cli->vuid = atoi(buf);
	return 0;
}

/****************************************************************************
 Setup a new VUID, by issuing a session setup
****************************************************************************/

static int cmd_logon(void)
{
	pstring l_username, l_password;
	pstring buf,buf2;
  
	if (!next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		d_printf("logon <username> [<password>]\n");
		return 0;
	}

	pstrcpy(l_username, buf);

	if (!next_token_nr(NULL,buf2,NULL,sizeof(buf))) 
	{
		char *pass = getpass("Password: ");
		if (pass) 
			pstrcpy(l_password, pass);
	} 
	else
		pstrcpy(l_password, buf2);

	if (!NT_STATUS_IS_OK(cli_session_setup(cli, l_username, 
					       l_password, strlen(l_password),
					       l_password, strlen(l_password),
					       lp_workgroup()))) {
		d_printf("session setup failed: %s\n", cli_errstr(cli));
		return -1;
	}

	d_printf("Current VUID is %d\n", cli->vuid);
	return 0;
}


/****************************************************************************
 list active connections
****************************************************************************/

static int cmd_list_connect(void)
{
	cli_cm_display();

	return 0;
}

/****************************************************************************
 display the current active client connection
****************************************************************************/

static int cmd_show_connect( void )
{
	struct cli_state *targetcli;
	pstring targetpath;
	
	if ( !cli_resolve_path( "", cli, cur_dir, &targetcli, targetpath ) ) {
		d_printf("showconnect %s: %s\n", cur_dir, cli_errstr(cli));
		return 1;
	}
	
	d_printf("//%s/%s\n", targetcli->desthost, targetcli->share);
	return 0;
}

/* Some constants for completing filename arguments */

#define COMPL_NONE        0          /* No completions */
#define COMPL_REMOTE      1          /* Complete remote filename */
#define COMPL_LOCAL       2          /* Complete local filename */

/* This defines the commands supported by this client.
 * NOTE: The "!" must be the last one in the list because it's fn pointer
 *       field is NULL, and NULL in that field is used in process_tok()
 *       (below) to indicate the end of the list.  crh
 */
static struct
{
  const char *name;
  int (*fn)(void);
  const char *description;
  char compl_args[2];      /* Completion argument info */
} commands[] = {
  {"?",cmd_help,"[command] give help on a command",{COMPL_NONE,COMPL_NONE}},
  {"altname",cmd_altname,"<file> show alt name",{COMPL_NONE,COMPL_NONE}},
  {"archive",cmd_archive,"<level>\n0=ignore archive bit\n1=only get archive files\n2=only get archive files and reset archive bit\n3=get all files and reset archive bit",{COMPL_NONE,COMPL_NONE}},
  {"blocksize",cmd_block,"blocksize <number> (default 20)",{COMPL_NONE,COMPL_NONE}},
  {"cancel",cmd_cancel,"<jobid> cancel a print queue entry",{COMPL_NONE,COMPL_NONE}},
  {"case_sensitive",cmd_setcase,"toggle the case sensitive flag to server",{COMPL_NONE,COMPL_NONE}},
  {"cd",cmd_cd,"[directory] change/report the remote directory",{COMPL_REMOTE,COMPL_NONE}},
  {"chmod",cmd_chmod,"<src> <mode> chmod a file using UNIX permission",{COMPL_REMOTE,COMPL_REMOTE}},
  {"chown",cmd_chown,"<src> <uid> <gid> chown a file using UNIX uids and gids",{COMPL_REMOTE,COMPL_REMOTE}},
  {"close",cmd_close,"<fid> close a file given a fid",{COMPL_REMOTE,COMPL_REMOTE}},
  {"del",cmd_del,"<mask> delete all matching files",{COMPL_REMOTE,COMPL_NONE}},
  {"dir",cmd_dir,"<mask> list the contents of the current directory",{COMPL_REMOTE,COMPL_NONE}},
  {"du",cmd_du,"<mask> computes the total size of the current directory",{COMPL_REMOTE,COMPL_NONE}},
  {"exit",cmd_quit,"logoff the server",{COMPL_NONE,COMPL_NONE}},
  {"get",cmd_get,"<remote name> [local name] get a file",{COMPL_REMOTE,COMPL_LOCAL}},
  {"getfacl",cmd_getfacl,"<file name> get the POSIX ACL on a file (UNIX extensions only)",{COMPL_REMOTE,COMPL_LOCAL}},
  {"hardlink",cmd_hardlink,"<src> <dest> create a Windows hard link",{COMPL_REMOTE,COMPL_REMOTE}},
  {"help",cmd_help,"[command] give help on a command",{COMPL_NONE,COMPL_NONE}},
  {"history",cmd_history,"displays the command history",{COMPL_NONE,COMPL_NONE}},
  {"lcd",cmd_lcd,"[directory] change/report the local current working directory",{COMPL_LOCAL,COMPL_NONE}},
  {"link",cmd_link,"<oldname> <newname> create a UNIX hard link",{COMPL_REMOTE,COMPL_REMOTE}},
  {"lock",cmd_lock,"lock <fnum> [r|w] <hex-start> <hex-len> : set a POSIX lock",{COMPL_REMOTE,COMPL_REMOTE}},
  {"lowercase",cmd_lowercase,"toggle lowercasing of filenames for get",{COMPL_NONE,COMPL_NONE}},  
  {"ls",cmd_dir,"<mask> list the contents of the current directory",{COMPL_REMOTE,COMPL_NONE}},
  {"mask",cmd_select,"<mask> mask all filenames against this",{COMPL_REMOTE,COMPL_NONE}},
  {"md",cmd_mkdir,"<directory> make a directory",{COMPL_NONE,COMPL_NONE}},
  {"mget",cmd_mget,"<mask> get all the matching files",{COMPL_REMOTE,COMPL_NONE}},
  {"mkdir",cmd_mkdir,"<directory> make a directory",{COMPL_NONE,COMPL_NONE}},
  {"more",cmd_more,"<remote name> view a remote file with your pager",{COMPL_REMOTE,COMPL_NONE}},  
  {"mput",cmd_mput,"<mask> put all matching files",{COMPL_REMOTE,COMPL_NONE}},
  {"newer",cmd_newer,"<file> only mget files newer than the specified local file",{COMPL_LOCAL,COMPL_NONE}},
  {"open",cmd_open,"<mask> open a file",{COMPL_REMOTE,COMPL_NONE}},
  {"posix", cmd_posix, "turn on all POSIX capabilities", {COMPL_REMOTE,COMPL_NONE}},
  {"posix_open",cmd_posix_open,"<name> 0<mode> open_flags mode open a file using POSIX interface",{COMPL_REMOTE,COMPL_NONE}},
  {"posix_mkdir",cmd_posix_mkdir,"<name> 0<mode> creates a directory using POSIX interface",{COMPL_REMOTE,COMPL_NONE}},
  {"posix_rmdir",cmd_posix_rmdir,"<name> removes a directory using POSIX interface",{COMPL_REMOTE,COMPL_NONE}},
  {"posix_unlink",cmd_posix_unlink,"<name> removes a file using POSIX interface",{COMPL_REMOTE,COMPL_NONE}},
  {"print",cmd_print,"<file name> print a file",{COMPL_NONE,COMPL_NONE}},
  {"prompt",cmd_prompt,"toggle prompting for filenames for mget and mput",{COMPL_NONE,COMPL_NONE}},  
  {"put",cmd_put,"<local name> [remote name] put a file",{COMPL_LOCAL,COMPL_REMOTE}},
  {"pwd",cmd_pwd,"show current remote directory (same as 'cd' with no args)",{COMPL_NONE,COMPL_NONE}},
  {"q",cmd_quit,"logoff the server",{COMPL_NONE,COMPL_NONE}},
  {"queue",cmd_queue,"show the print queue",{COMPL_NONE,COMPL_NONE}},
  {"quit",cmd_quit,"logoff the server",{COMPL_NONE,COMPL_NONE}},
  {"rd",cmd_rmdir,"<directory> remove a directory",{COMPL_NONE,COMPL_NONE}},
  {"recurse",cmd_recurse,"toggle directory recursion for mget and mput",{COMPL_NONE,COMPL_NONE}},  
  {"reget",cmd_reget,"<remote name> [local name] get a file restarting at end of local file",{COMPL_REMOTE,COMPL_LOCAL}},
  {"rename",cmd_rename,"<src> <dest> rename some files",{COMPL_REMOTE,COMPL_REMOTE}},
  {"reput",cmd_reput,"<local name> [remote name] put a file restarting at end of remote file",{COMPL_LOCAL,COMPL_REMOTE}},
  {"rm",cmd_del,"<mask> delete all matching files",{COMPL_REMOTE,COMPL_NONE}},
  {"rmdir",cmd_rmdir,"<directory> remove a directory",{COMPL_NONE,COMPL_NONE}},
  {"showacls",cmd_showacls,"toggle if ACLs are shown or not",{COMPL_NONE,COMPL_NONE}},  
  {"setmode",cmd_setmode,"filename <setmode string> change modes of file",{COMPL_REMOTE,COMPL_NONE}},
  {"stat",cmd_stat,"filename Do a UNIX extensions stat call on a file",{COMPL_REMOTE,COMPL_REMOTE}},
  {"symlink",cmd_symlink,"<oldname> <newname> create a UNIX symlink",{COMPL_REMOTE,COMPL_REMOTE}},
  {"tar",cmd_tar,"tar <c|x>[IXFqbgNan] current directory to/from <file name>",{COMPL_NONE,COMPL_NONE}},
  {"tarmode",cmd_tarmode,"<full|inc|reset|noreset> tar's behaviour towards archive bits",{COMPL_NONE,COMPL_NONE}},
  {"translate",cmd_translate,"toggle text translation for printing",{COMPL_NONE,COMPL_NONE}},
  {"unlock",cmd_unlock,"unlock <fnum> <hex-start> <hex-len> : remove a POSIX lock",{COMPL_REMOTE,COMPL_REMOTE}},
  {"volume",cmd_volume,"print the volume name",{COMPL_NONE,COMPL_NONE}},
  {"vuid",cmd_vuid,"change current vuid",{COMPL_NONE,COMPL_NONE}},
  {"wdel",cmd_wdel,"<attrib> <mask> wildcard delete all matching files",{COMPL_REMOTE,COMPL_NONE}},
  {"logon",cmd_logon,"establish new logon",{COMPL_NONE,COMPL_NONE}},
  {"listconnect",cmd_list_connect,"list open connections",{COMPL_NONE,COMPL_NONE}},
  {"showconnect",cmd_show_connect,"display the current active connection",{COMPL_NONE,COMPL_NONE}},
  
  /* Yes, this must be here, see crh's comment above. */
  {"!",NULL,"run a shell command on the local system",{COMPL_NONE,COMPL_NONE}},
  {NULL,NULL,NULL,{COMPL_NONE,COMPL_NONE}}
};

/*******************************************************************
 Lookup a command string in the list of commands, including 
 abbreviations.
******************************************************************/

static int process_tok(pstring tok)
{
	int i = 0, matches = 0;
	int cmd=0;
	int tok_len = strlen(tok);
	
	while (commands[i].fn != NULL) {
		if (strequal(commands[i].name,tok)) {
			matches = 1;
			cmd = i;
			break;
		} else if (strnequal(commands[i].name, tok, tok_len)) {
			matches++;
			cmd = i;
		}
		i++;
	}
  
	if (matches == 0)
		return(-1);
	else if (matches == 1)
		return(cmd);
	else
		return(-2);
}

/****************************************************************************
 Help.
****************************************************************************/

static int cmd_help(void)
{
	int i=0,j;
	pstring buf;
	
	if (next_token_nr(NULL,buf,NULL,sizeof(buf))) {
		if ((i = process_tok(buf)) >= 0)
			d_printf("HELP %s:\n\t%s\n\n",commands[i].name,commands[i].description);
	} else {
		while (commands[i].description) {
			for (j=0; commands[i].description && (j<5); j++) {
				d_printf("%-15s",commands[i].name);
				i++;
			}
			d_printf("\n");
		}
	}
	return 0;
}

/****************************************************************************
 Process a -c command string.
****************************************************************************/

static int process_command_string(char *cmd)
{
	pstring line;
	const char *ptr;
	int rc = 0;

	/* establish the connection if not already */
	
	if (!cli) {
		cli = cli_cm_open(desthost, service, True);
		if (!cli)
			return 0;
	}
	
	while (cmd[0] != '\0')    {
		char *p;
		pstring tok;
		int i;
		
		if ((p = strchr_m(cmd, ';')) == 0) {
			strncpy(line, cmd, 999);
			line[1000] = '\0';
			cmd += strlen(cmd);
		} else {
			if (p - cmd > 999)
				p = cmd + 999;
			strncpy(line, cmd, p - cmd);
			line[p - cmd] = '\0';
			cmd = p + 1;
		}
		
		/* and get the first part of the command */
		ptr = line;
		if (!next_token_nr(&ptr,tok,NULL,sizeof(tok))) continue;
		
		if ((i = process_tok(tok)) >= 0) {
			rc = commands[i].fn();
		} else if (i == -2) {
			d_printf("%s: command abbreviation ambiguous\n",tok);
		} else {
			d_printf("%s: command not found\n",tok);
		}
	}
	
	return rc;
}	

#define MAX_COMPLETIONS 100

typedef struct {
	pstring dirmask;
	char **matches;
	int count, samelen;
	const char *text;
	int len;
} completion_remote_t;

static void completion_remote_filter(const char *mnt, file_info *f, const char *mask, void *state)
{
	completion_remote_t *info = (completion_remote_t *)state;

	if ((info->count < MAX_COMPLETIONS - 1) && (strncmp(info->text, f->name, info->len) == 0) && (strcmp(f->name, ".") != 0) && (strcmp(f->name, "..") != 0)) {
		if ((info->dirmask[0] == 0) && !(f->mode & aDIR))
			info->matches[info->count] = SMB_STRDUP(f->name);
		else {
			pstring tmp;

			if (info->dirmask[0] != 0)
				pstrcpy(tmp, info->dirmask);
			else
				tmp[0] = 0;
			pstrcat(tmp, f->name);
			if (f->mode & aDIR)
				pstrcat(tmp, "/");
			info->matches[info->count] = SMB_STRDUP(tmp);
		}
		if (info->matches[info->count] == NULL)
			return;
		if (f->mode & aDIR)
			smb_readline_ca_char(0);

		if (info->count == 1)
			info->samelen = strlen(info->matches[info->count]);
		else
			while (strncmp(info->matches[info->count], info->matches[info->count-1], info->samelen) != 0)
				info->samelen--;
		info->count++;
	}
}

static char **remote_completion(const char *text, int len)
{
	pstring dirmask;
	int i;
	completion_remote_t info = { "", NULL, 1, 0, NULL, 0 };

	/* can't have non-static intialisation on Sun CC, so do it
	   at run time here */
	info.samelen = len;
	info.text = text;
	info.len = len;
		
	if (len >= MIN(PATH_MAX,sizeof(pstring))) {
		return(NULL);
	}

	info.matches = SMB_MALLOC_ARRAY(char *,MAX_COMPLETIONS);
	if (!info.matches) {
		return NULL;
	}

	/*
	 * We're leaving matches[0] free to fill it later with the text to
	 * display: Either the one single match or the longest common subset
	 * of the matches.
	 */
	info.matches[0] = NULL;
	info.count = 1;

	for (i = len-1; i >= 0; i--) {
		if ((text[i] == '/') || (text[i] == CLI_DIRSEP_CHAR)) {
			break;
		}
	}

	info.text = text+i+1;
	info.samelen = info.len = len-i-1;

	if (i > 0) {
		strncpy(info.dirmask, text, i+1);
		info.dirmask[i+1] = 0;
		pstr_sprintf(dirmask, "%s%*s*", cur_dir, i-1, text);
	} else {
		pstr_sprintf(dirmask, "%s*", cur_dir);
	}

	if (cli_list(cli, dirmask, aDIR | aSYSTEM | aHIDDEN, completion_remote_filter, &info) < 0)
		goto cleanup;

	if (info.count == 1) {

		/*
		 * No matches at all, NULL indicates there is nothing
		 */

		SAFE_FREE(info.matches[0]);
		SAFE_FREE(info.matches);
		return NULL;
	}

	if (info.count == 2) {

		/*
		 * Exactly one match in matches[1], indicate this is the one
		 * in matches[0].
		 */

		info.matches[0] = info.matches[1];
		info.matches[1] = NULL;
		info.count -= 1;
		return info.matches;
	}

	/*
	 * We got more than one possible match, set the result to the maximum
	 * common subset
	 */

	info.matches[0] = SMB_STRNDUP(info.matches[1], info.samelen);
	info.matches[info.count] = NULL;
	return info.matches;

cleanup:
	for (i = 0; i < info.count; i++)
		free(info.matches[i]);
	free(info.matches);
	return NULL;
}

static char **completion_fn(const char *text, int start, int end)
{
	smb_readline_ca_char(' ');

	if (start) {
		const char *buf, *sp;
		int i;
		char compl_type;

		buf = smb_readline_get_line_buffer();
		if (buf == NULL)
			return NULL;
		
		sp = strchr(buf, ' ');
		if (sp == NULL)
			return NULL;

		for (i = 0; commands[i].name; i++) {
			if ((strncmp(commands[i].name, buf, sp - buf) == 0) &&
			    (commands[i].name[sp - buf] == 0)) {
				break;
			}
		}
		if (commands[i].name == NULL)
			return NULL;

		while (*sp == ' ')
			sp++;

		if (sp == (buf + start))
			compl_type = commands[i].compl_args[0];
		else
			compl_type = commands[i].compl_args[1];

		if (compl_type == COMPL_REMOTE)
			return remote_completion(text, end - start);
		else /* fall back to local filename completion */
			return NULL;
	} else {
		char **matches;
		int i, len, samelen = 0, count=1;

		matches = SMB_MALLOC_ARRAY(char *, MAX_COMPLETIONS);
		if (!matches) {
			return NULL;
		}
		matches[0] = NULL;

		len = strlen(text);
		for (i=0;commands[i].fn && count < MAX_COMPLETIONS-1;i++) {
			if (strncmp(text, commands[i].name, len) == 0) {
				matches[count] = SMB_STRDUP(commands[i].name);
				if (!matches[count])
					goto cleanup;
				if (count == 1)
					samelen = strlen(matches[count]);
				else
					while (strncmp(matches[count], matches[count-1], samelen) != 0)
						samelen--;
				count++;
			}
		}

		switch (count) {
		case 0:	/* should never happen */
		case 1:
			goto cleanup;
		case 2:
			matches[0] = SMB_STRDUP(matches[1]);
			break;
		default:
			matches[0] = (char *)SMB_MALLOC(samelen+1);
			if (!matches[0])
				goto cleanup;
			strncpy(matches[0], matches[1], samelen);
			matches[0][samelen] = 0;
		}
		matches[count] = NULL;
		return matches;

cleanup:
		for (i = 0; i < count; i++)
			free(matches[i]);

		free(matches);
		return NULL;
	}
}

/****************************************************************************
 Make sure we swallow keepalives during idle time.
****************************************************************************/

static void readline_callback(void)
{
	fd_set fds;
	struct timeval timeout;
	static time_t last_t;
	time_t t;

	t = time(NULL);

	if (t - last_t < 5)
		return;

	last_t = t;

 again:

	if (cli->fd == -1)
		return;

	FD_ZERO(&fds);
	FD_SET(cli->fd,&fds);

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	sys_select_intr(cli->fd+1,&fds,NULL,NULL,&timeout);
      		
	/* We deliberately use receive_smb instead of
	   client_receive_smb as we want to receive
	   session keepalives and then drop them here.
	*/
	if (FD_ISSET(cli->fd,&fds)) {
		if (!receive_smb(cli->fd,cli->inbuf,0)) {
			DEBUG(0, ("Read from server failed, maybe it closed the "
				"connection\n"));
			return;
		}
		goto again;
	}
      
	/* Ping the server to keep the connection alive using SMBecho. */
	{
		unsigned char garbage[16];
		memset(garbage, 0xf0, sizeof(garbage));
		cli_echo(cli, garbage, sizeof(garbage));
	}
}

/****************************************************************************
 Process commands on stdin.
****************************************************************************/

static int process_stdin(void)
{
	const char *ptr;
	int rc = 0;

	while (1) {
		pstring tok;
		pstring the_prompt;
		char *cline;
		pstring line;
		int i;
		
		/* display a prompt */
		slprintf(the_prompt, sizeof(the_prompt)-1, "smb: %s> ", cur_dir);
		cline = smb_readline(the_prompt, readline_callback, completion_fn);
			
		if (!cline) break;
		
		pstrcpy(line, cline);

		/* special case - first char is ! */
		if (*line == '!') {
			system(line + 1);
			continue;
		}
      
		/* and get the first part of the command */
		ptr = line;
		if (!next_token_nr(&ptr,tok,NULL,sizeof(tok))) continue;

		if ((i = process_tok(tok)) >= 0) {
			rc = commands[i].fn();
		} else if (i == -2) {
			d_printf("%s: command abbreviation ambiguous\n",tok);
		} else {
			d_printf("%s: command not found\n",tok);
		}
	}
	return rc;
}

/****************************************************************************
 Process commands from the client.
****************************************************************************/

static int process(char *base_directory)
{
	int rc = 0;

	cli = cli_cm_open(desthost, service, True);
	if (!cli) {
		return 1;
	}

	if (*base_directory) {
		rc = do_cd(base_directory);
		if (rc) {
			cli_cm_shutdown();
			return rc;
		}
	}
	
	if (cmdstr) {
		rc = process_command_string(cmdstr);
	} else {
		process_stdin();
	}
  
	cli_cm_shutdown();
	return rc;
}

/****************************************************************************
 Handle a -L query.
****************************************************************************/

static int do_host_query(char *query_host)
{
	cli = cli_cm_open(query_host, "IPC$", True);
	if (!cli)
		return 1;

	browse_host(True);

	if (port != 139) {

		/* Workgroups simply don't make sense over anything
		   else but port 139... */

		cli_cm_shutdown();
		cli_cm_set_port( 139 );
		cli = cli_cm_open(query_host, "IPC$", True);
	}

	if (cli == NULL) {
		d_printf("NetBIOS over TCP disabled -- no workgroup available\n");
		return 1;
	}

	list_servers(lp_workgroup());

	cli_cm_shutdown();
	
	return(0);
}

/****************************************************************************
 Handle a tar operation.
****************************************************************************/

static int do_tar_op(char *base_directory)
{
	int ret;

	/* do we already have a connection? */
	if (!cli) {
		cli = cli_cm_open(desthost, service, True);
		if (!cli)
			return 1;
	}

	recurse=True;

	if (*base_directory)  {
		ret = do_cd(base_directory);
		if (ret) {
			cli_cm_shutdown();
			return ret;
		}
	}
	
	ret=process_tar();

	cli_cm_shutdown();

	return(ret);
}

/****************************************************************************
 Handle a message operation.
****************************************************************************/

static int do_message_op(void)
{
	struct in_addr ip;
	struct nmb_name called, calling;
	fstring server_name;
	char name_type_hex[10];
	int msg_port;

	make_nmb_name(&calling, calling_name, 0x0);
	make_nmb_name(&called , desthost, name_type);

	fstrcpy(server_name, desthost);
	snprintf(name_type_hex, sizeof(name_type_hex), "#%X", name_type);
	fstrcat(server_name, name_type_hex);

        zero_ip(&ip);
	if (have_ip) 
		ip = dest_ip;

	/* we can only do messages over port 139 (to windows clients at least) */

	msg_port = port ? port : 139;

	if (!(cli=cli_initialise()) || (cli_set_port(cli, msg_port) != msg_port) ||
	    !cli_connect(cli, server_name, &ip)) {
		d_printf("Connection to %s failed\n", desthost);
		return 1;
	}

	if (!cli_session_request(cli, &calling, &called)) {
		d_printf("session request failed\n");
		cli_cm_shutdown();
		return 1;
	}

	send_message();
	cli_cm_shutdown();

	return 0;
}


/****************************************************************************
  main program
****************************************************************************/

 int main(int argc,char *argv[])
{
	pstring base_directory;
	int opt;
	pstring query_host;
	BOOL message = False;
	pstring term_code;
	static const char *new_name_resolve_order = NULL;
	poptContext pc;
	char *p;
	int rc = 0;
	fstring new_workgroup;
	BOOL tar_opt = False;
	BOOL service_opt = False;
	struct poptOption long_options[] = {
		POPT_AUTOHELP

		{ "name-resolve", 'R', POPT_ARG_STRING, &new_name_resolve_order, 'R', "Use these name resolution services only", "NAME-RESOLVE-ORDER" },
		{ "message", 'M', POPT_ARG_STRING, NULL, 'M', "Send message", "HOST" },
		{ "ip-address", 'I', POPT_ARG_STRING, NULL, 'I', "Use this IP to connect to", "IP" },
		{ "stderr", 'E', POPT_ARG_NONE, NULL, 'E', "Write messages to stderr instead of stdout" },
		{ "list", 'L', POPT_ARG_STRING, NULL, 'L', "Get a list of shares available on a host", "HOST" },
		{ "terminal", 't', POPT_ARG_STRING, NULL, 't', "Terminal I/O code {sjis|euc|jis7|jis8|junet|hex}", "CODE" },
		{ "max-protocol", 'm', POPT_ARG_STRING, NULL, 'm', "Set the max protocol level", "LEVEL" },
		{ "tar", 'T', POPT_ARG_STRING, NULL, 'T', "Command line tar", "<c|x>IXFqgbNan" },
		{ "directory", 'D', POPT_ARG_STRING, NULL, 'D', "Start from directory", "DIR" },
		{ "command", 'c', POPT_ARG_STRING, &cmdstr, 'c', "Execute semicolon separated commands" }, 
		{ "send-buffer", 'b', POPT_ARG_INT, &io_bufsize, 'b', "Changes the transmit/send buffer", "BYTES" },
		{ "port", 'p', POPT_ARG_INT, &port, 'p', "Port to connect to", "PORT" },
		{ "grepable", 'g', POPT_ARG_NONE, NULL, 'g', "Produce grepable output" },
		POPT_COMMON_SAMBA
		POPT_COMMON_CONNECTION
		POPT_COMMON_CREDENTIALS
		POPT_TABLEEND
	};
	
	load_case_tables();

#ifdef KANJI
	pstrcpy(term_code, KANJI);
#else /* KANJI */
	*term_code = 0;
#endif /* KANJI */

	*query_host = 0;
	*base_directory = 0;
	
	/* initialize the workgroup name so we can determine whether or 
	   not it was set by a command line option */
	   
	set_global_myworkgroup( "" );
	set_global_myname( "" );

        /* set default debug level to 0 regardless of what smb.conf sets */
	setup_logging( "smbclient", True );
	DEBUGLEVEL_CLASS[DBGC_ALL] = 1;
	if ((dbf = x_fdup(x_stderr))) {
		x_setbuf( dbf, NULL );
	}

	/* skip argv(0) */
	pc = poptGetContext("smbclient", argc, (const char **) argv, long_options, 0);
	poptSetOtherOptionHelp(pc, "service <password>");

	in_client = True;   /* Make sure that we tell lp_load we are */

	while ((opt = poptGetNextOpt(pc)) != -1) {

		/* if the tar option has been called previouslt, now we need to eat out the leftovers */
		/* I see no other way to keep things sane --SSS */
		if (tar_opt == True) {
			while (poptPeekArg(pc)) {
				poptGetArg(pc);
			}
			tar_opt = False;
		}

		/* if the service has not yet been specified lets see if it is available in the popt stack */
		if (!service_opt && poptPeekArg(pc)) {
			pstrcpy(service, poptGetArg(pc));
			/* Convert any '/' characters in the service name to '\' characters */
			string_replace(service, '/','\\');

			if (count_chars(service,'\\') < 3) {
				d_printf("\n%s: Not enough '\\' characters in service\n",service);
				poptPrintUsage(pc, stderr, 0);
				exit(1);
			}
			service_opt = True;
		}

		/* if the service has already been retrieved then check if we have also a password */
		if (service_opt && (!cmdline_auth_info.got_pass) && poptPeekArg(pc)) {
			pstrcpy(cmdline_auth_info.password, poptGetArg(pc));
			cmdline_auth_info.got_pass = True;
		}
	
		switch (opt) {
		case 'M':
			/* Messages are sent to NetBIOS name type 0x3
			 * (Messenger Service).  Make sure we default
			 * to port 139 instead of port 445. srl,crh
			 */
			name_type = 0x03; 
			cli_cm_set_dest_name_type( name_type );
			pstrcpy(desthost,poptGetOptArg(pc));
			if( !port )
				cli_cm_set_port( 139 );
 			message = True;
 			break;
		case 'I':
			{
				dest_ip = *interpret_addr2(poptGetOptArg(pc));
				if (is_zero_ip(dest_ip))
					exit(1);
				have_ip = True;

				cli_cm_set_dest_ip( dest_ip );
			}
			break;
		case 'E':
			if (dbf) {
				x_fclose(dbf);
			}
			dbf = x_stderr;
			display_set_stderr();
			break;

		case 'L':
			pstrcpy(query_host, poptGetOptArg(pc));
			break;
		case 't':
			pstrcpy(term_code, poptGetOptArg(pc));
			break;
		case 'm':
			max_protocol = interpret_protocol(poptGetOptArg(pc), max_protocol);
			break;
		case 'T':
			/* We must use old option processing for this. Find the
			 * position of the -T option in the raw argv[]. */
			{
				int i;
				for (i = 1; i < argc; i++) {
					if (strncmp("-T", argv[i],2)==0)
						break;
				}
				i++;
				if (!tar_parseargs(argc, argv, poptGetOptArg(pc), i)) {
					poptPrintUsage(pc, stderr, 0);
					exit(1);
				}
			}
			/* this must be the last option, mark we have parsed it so that we know we have */
			tar_opt = True;
			break;
		case 'D':
			pstrcpy(base_directory,poptGetOptArg(pc));
			break;
		case 'g':
			grepable=True;
			break;
		}
	}

	/* We may still have some leftovers after the last popt option has been called */
	if (tar_opt == True) {
		while (poptPeekArg(pc)) {
			poptGetArg(pc);
		}
		tar_opt = False;
	}

	/* if the service has not yet been specified lets see if it is available in the popt stack */
	if (!service_opt && poptPeekArg(pc)) {
		pstrcpy(service, poptGetArg(pc));
		/* Convert any '/' characters in the service name to '\' characters */
		string_replace(service, '/','\\');

		if (count_chars(service,'\\') < 3) {
			d_printf("\n%s: Not enough '\\' characters in service\n",service);
			poptPrintUsage(pc, stderr, 0);
			exit(1);
		}
		service_opt = True;
	}

	/* if the service has already been retrieved then check if we have also a password */
	if (service_opt && (!cmdline_auth_info.got_pass) && poptPeekArg(pc)) {
		pstrcpy(cmdline_auth_info.password, poptGetArg(pc));
		cmdline_auth_info.got_pass = True;
	}
	
	/* check for the -P option */

	if ( port != 0 )
		cli_cm_set_port( port );

	/*
	 * Don't load debug level from smb.conf. It should be
	 * set by cmdline arg or remain default (0)
	 */
	AllowDebugChange = False;
	
	/* save the workgroup...
	
	   FIXME!! do we need to do this for other options as well 
	   (or maybe a generic way to keep lp_load() from overwriting 
	   everything)?  */
	
	fstrcpy( new_workgroup, lp_workgroup() );
	pstrcpy( calling_name, global_myname() );
	
	if ( override_logfile )
		setup_logging( lp_logfile(), False );
	
	if (!lp_load(dyn_CONFIGFILE,True,False,False,True)) {
		fprintf(stderr, "%s: Can't load %s - run testparm to debug it\n",
			argv[0], dyn_CONFIGFILE);
	}
	
	load_interfaces();
	
	if ( strlen(new_workgroup) != 0 )
		set_global_myworkgroup( new_workgroup );

	if ( strlen(calling_name) != 0 )
		set_global_myname( calling_name );
	else
		pstrcpy( calling_name, global_myname() );

	init_names();

	if(new_name_resolve_order)
		lp_set_name_resolve_order(new_name_resolve_order);

	if (!tar_type && !*query_host && !*service && !message) {
		poptPrintUsage(pc, stderr, 0);
		exit(1);
	}

	poptFreeContext(pc);

	/* store the username an password for dfs support */

	cli_cm_set_credentials( &cmdline_auth_info );
	pstrcpy(username, cmdline_auth_info.username);

	DEBUG(3,("Client started (version %s).\n", SAMBA_VERSION_STRING));

	if (tar_type) {
		if (cmdstr)
			process_command_string(cmdstr);
		return do_tar_op(base_directory);
	}

	if (*query_host) {
		char *qhost = query_host;
		char *slash;

		while (*qhost == '\\' || *qhost == '/')
			qhost++;

		if ((slash = strchr_m(qhost, '/'))
		    || (slash = strchr_m(qhost, '\\'))) {
			*slash = 0;
		}

		if ((p=strchr_m(qhost, '#'))) {
			*p = 0;
			p++;
			sscanf(p, "%x", &name_type);
			cli_cm_set_dest_name_type( name_type );
		}

		return do_host_query(qhost);
	}

	if (message) {
		return do_message_op();
	}
	
	if (process(base_directory)) {
		return 1;
	}

	talloc_destroy( ctx);
	return rc;
}
