/*
 * ProFTPD: mod_ratio -- Support upload/download ratios.
 * Copyright (c) 2002 James Dogopoulos.
 * Portions Copyright (c) 1998-1999 Johnie Ingram.
 *  
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 */

#define MOD_RATIO_VERSION "mod_ratio/3.3"

/* This is mod_ratio, contrib software for proftpd 1.2.0 and above.
   For more information contact James Dogopoulos <jd@dynw.com> or
   Johnie Ingram <johnie@netgod.net>.

   History Log:

   * 2002-05-24: v3.3: Fixed numerous bugs and compatibility issues with
     changing code within ProFTPD. In other words, it /works/ again!
     Added default AnonRatio support (AnonRatio * ...).

   * 2000-08-01: v3.2: Fixed CwdRatio directive. Ratios are no longer part
     of CWD or NOOP commands. Fixed file byte ratio printing. It is now
     UL:DL - the way it should be. Updated README.ratio file. - jD

   * 2000-02-15: v3.1: Added support to save user stats to plain text file.
     (See README.ratio) Changed display to MBytes rather than Bytes. Tracks
     to the nearest Kilobyte rather than byte. - jD

   * 1999-10-03: v3.0: Uses generic API to access SQL data at runtime.
     Supports negative ratios (upload X to get 1) by popular demand.
     Added proper SITE command and help.  Various presentation
     idiosyncracies fixed.

   * 1999-06-13: v2.2: fixed ratio display, it was printing ratios in
     reverse order.

   * 1999-05-03: v2.1: mod_mysql bugfix; rearranged CWD reply so
     Netscape shows ratios; fixed recalculation in XRATIO.  Added
     CwdRatioMsg directive for showing equivalent URLs (always
     enabled).
   
   * 1999-04-08: v2.0: Reformat and rewrite.  Add FileRatioErrMsg,
     ByteRatioErrMsg, and LeechRatioMsg directives and support for
     proftpd mod_mysql.

   * 1998-09-08: v1.0: Accepted into CVS as a contrib module.

*/

#include "conf.h"

/* *INDENT-OFF* */

/* Maximum username field to expect, etc. */
#define ARBITRARY_MAX                   128

int gotratuser,fileerr;

static struct
{
  int fstor,fretr,bstor,bretr;
  int frate,fcred,brate,bcred;
  int files,bytes;

  char ftext [64],btext [64];

} stats;

static struct
{
  int enable;
  int save;
  char user [ARBITRARY_MAX];

  const char *rtype;          /* The ratio type currently in effect. */

  const char *filemsg;
  const char *bytemsg;
  const char *leechmsg;
  const char *ratiofile;
  const char *ratiotmp;
} g;

#define RATIO_ENFORCE (stats.frate || stats.brate)

#define RATIO_STUFFS "-%i/%i +%i/%i (%i %i %i %i) = %i/%i%s%s", \
	    stats.fretr, (stats.bretr / 1024), stats.fstor, \
	    (stats.bstor / 1024), \
            stats.frate, stats.fcred, stats.brate, stats.bcred, \
            stats.files, (stats.bytes / 1024), \
	    (stats.frate && stats.files < 1) ? " [NO F]" : "", \
	    (stats.brate && (stats.bytes / 1024) < 5) ? " [LO B]" : ""
#define SHORT_RATIO_STUFFS "-%i/%i +%i/%i = %i/%i%s%s", \
	    stats.fretr, (stats.bretr / 1024), stats.fstor, \
	    (stats.bstor / 1024), \
            stats.files, (stats.bytes / 1024), \
	    (stats.frate && stats.files < 1) ? " [NO F]" : "", \
	    (stats.brate && (stats.bytes / 1024) < 5) ? " [LO B]" : ""

/* *INDENT-ON* */

static cmd_rec *
_make_cmd (pool * cp, int argc, ...)
{
  va_list args;
  cmd_rec *c;
  int i;

  pool *newpool = NULL;
  newpool = make_sub_pool( cp );
  c = pcalloc(newpool, sizeof(cmd_rec));

  c->pool = newpool;
  c->argv = pcalloc(newpool, sizeof(void *) * (argc + 1));

  c->argc = argc;
  c->stash_index = -1;

  c->argv[0] = MOD_RATIO_VERSION;
  va_start (args, argc);
  for (i = 0; i < argc; i++)
    c->argv[i + 1] = (void *) va_arg (args, char *);
  va_end (args);

  return c;
}

static modret_t *
_dispatch_ratio (cmd_rec * cmd, char *match)
{
  authtable *m;
  modret_t *mr = NULL;

  m = pr_stash_get_symbol (PR_SYM_AUTH, match, NULL, &cmd->stash_index);
  while (m)
    {
      mr = call_module (m->m, m->handler, cmd);
      if (MODRET_ISHANDLED (mr) || MODRET_ISERROR (mr))
	break;
      m = pr_stash_get_symbol (PR_SYM_AUTH, match, m, &cmd->stash_index);
    }
  if (MODRET_ISERROR (mr))
    pr_log_debug (DEBUG0, "Aiee! mod_ratio internal!  %s", MODRET_ERRMSG (mr));
  return mr;
}

static modret_t *
_dispatch (cmd_rec * cmd, char *match)
{
  cmd_rec *cr;
  modret_t *mr = 0;

  cr = _make_cmd (cmd->tmp_pool, 0);
  mr = _dispatch_ratio (cr, match);
  if (cr->tmp_pool)
    destroy_pool (cr->tmp_pool);
  return mr;
}

static void
_set_stats (char *fstor, char *fretr, char *bstor, char *bretr)
{
  if (fstor)
    stats.fstor = atoi (fstor);
  if (fretr)
    stats.fretr = atoi (fretr);
  if (bstor)
    stats.bstor = atoi (bstor);
  if (bretr)
    stats.bretr = atoi (bretr);
}


static void
_set_ratios (char *frate, char *fcred, char *brate, char *bcred)
{
stats.frate = stats.fcred = stats.brate = stats.bcred = 0;
  if (frate)
    stats.frate = atoi (frate);
  if (fcred)
    stats.fcred = atoi (fcred);
  if (brate)
    stats.brate = atoi (brate);
  if (bcred)
    stats.bcred = atoi (bcred);

  if (stats.frate >= 0)
    {
      stats.files = (stats.frate * stats.fstor) + stats.fcred - stats.fretr;
      snprintf (stats.ftext, sizeof(stats.ftext), "1:%iF", stats.frate);
    }
  else
    {
      stats.files = (stats.fstor / (stats.frate * -1)) + stats.fcred - stats.fretr;
      snprintf (stats.ftext, sizeof(stats.ftext), "%i:1F", stats.frate * -1);
    }

  if (stats.brate >= 0)
    {
      stats.bytes = (stats.brate * stats.bstor) + stats.bcred - stats.bretr;
      snprintf (stats.btext, sizeof(stats.btext), "1:%iB", stats.brate);
    }
  else
    {
      stats.bytes = (stats.bstor / (stats.brate * -1)) + stats.bcred - stats.bretr;
      snprintf (stats.btext, sizeof(stats.btext), "%i:1B", stats.brate * -1);
    }
}


MODRET _calc_ratios (cmd_rec * cmd)
{
  modret_t *mr = 0;
  config_rec *c;
  char buf[1024] = {'\0'};
  char *mask;
  char **data;

  if (!(g.enable = get_param_int (main_server->conf, "Ratios", FALSE) == TRUE))
    return DECLINED (cmd);

  mr = _dispatch (cmd, "getstats");
  if (MODRET_HASDATA (mr))
    {
      data = mr->data;
      if (data[4])
	pr_log_debug (DEBUG4, "ratio: warning: getstats on %s not unique",
		   g.user);
      _set_stats (data[0], data[1], data[2], data[3]);
    }

  mr = _dispatch (cmd, "getratio");
  if (MODRET_HASDATA (mr))
    {
      data = mr->data;
      if (data[4])
	pr_log_debug (DEBUG4, "ratio: warning: getratio on %s not unique",
		   g.user);
      _set_ratios (data[0], data[1], data[2], data[3]);
      g.rtype = "U";
      return DECLINED (cmd);
    }

  c = find_config (main_server->conf, CONF_PARAM, "HostRatio", TRUE);
  while (c)
    {
     mask = buf;
      if (*(char *) c->argv[0] == '.')
	{
	  *mask++ = '*';
	  sstrncpy (mask, c->argv[0], sizeof (buf));
	}
      else if (*(char *) ((char *) c->argv[0] + (strlen (c->argv[0]) - 1)) == '.')
	{
	  sstrncpy (mask, c->argv[0], sizeof(buf) - 2);
	  sstrcat(buf, "*", sizeof(buf));
	}
      else
	sstrncpy (mask, c->argv[0], sizeof (buf));

      if (!pr_fnmatch (buf, session.c->remote_name, PR_FNM_NOESCAPE | PR_FNM_CASEFOLD) ||
	  !pr_fnmatch (buf, pr_netaddr_get_ipstr (session.c->remote_addr),
		       PR_FNM_NOESCAPE | PR_FNM_CASEFOLD))
	{
	  _set_ratios (c->argv[1], c->argv[2], c->argv[3], c->argv[4]);
	  g.rtype = "h";
	  return DECLINED (cmd);
	}
      c = find_config_next (c, c->next, CONF_PARAM, "HostRatio", FALSE);
    }

  c = find_config (main_server->conf, CONF_PARAM, "AnonRatio", TRUE);
  while (c)
    {
      if ((session.anon_user && !strcmp (c->argv[0], session.anon_user)) ||
		*(char *) c->argv[0] == '*')
	{
	  _set_ratios (c->argv[1], c->argv[2], c->argv[3], c->argv[4]);
	  g.rtype = "a";
	  return DECLINED (cmd);
	}
      c = find_config_next (c, c->next, CONF_PARAM, "AnonRatio", FALSE);
    }

  c = find_config (main_server->conf, CONF_PARAM, "UserRatio", TRUE);
  while (c)
    {
      if (*(char *) c->argv[0] == '*' || !strcmp (c->argv[0], g.user))
	{
	  _set_ratios (c->argv[1], c->argv[2], c->argv[3], c->argv[4]);
	  g.rtype = "u";
	  return DECLINED (cmd);
	}
      c = find_config_next (c, c->next, CONF_PARAM, "UserRatio", FALSE);
    }

  c = find_config (main_server->conf, CONF_PARAM, "GroupRatio", TRUE);
  while (c)
    {
      if (!strcmp (c->argv[0], session.group))
	{
	  _set_ratios (c->argv[1], c->argv[2], c->argv[3], c->argv[4]);
	  g.rtype = "g";
	  return DECLINED (cmd);
	}
      c = find_config_next (c, c->next, CONF_PARAM, "GroupRatio", FALSE);
    }

  return DECLINED (cmd);
}

static void
_log_ratios (cmd_rec * cmd)
{
  char buf[1024] = {'\0'};

  snprintf (buf, sizeof(buf), SHORT_RATIO_STUFFS);
  pr_log_debug (DEBUG0, "%s in %s: %s %s%s%s", g.user,
	     session.cwd, cmd->argv[0], cmd->arg,
	     RATIO_ENFORCE ? " :" : "", RATIO_ENFORCE ? buf : "");
}

static void
_update_stats ()
{
    FILE *usrfile,*newfile;
    char usrstr[256] = {'\0'}, *ratname;
    int ulfiles,ulbytes,dlfiles,dlbytes,cpc;

     	if (!fileerr) {
        newfile=fopen(g.ratiotmp,"w");
          if (newfile == NULL)
          {
          pr_log_pri (PR_LOG_ERR, "Error opening temporary ratios file.");
          gotratuser=1;
          fileerr=1;
     	  }
				}

	if (!fileerr) {
	   usrfile=fopen(g.ratiofile,"r");
           while (fgets(usrstr,sizeof(usrstr),usrfile)!=NULL) {
                ratname = strtok(usrstr, "|");
                ulfiles = atoi (strtok(NULL, "|"));
                ulbytes = atoi (strtok(NULL, "|"));
                dlfiles = atoi (strtok(NULL, "|"));
                dlbytes = atoi (strtok(NULL, "|"));
                  if (strcmp(ratname, g.user) == 0) {
                  fprintf(newfile,"%s|%i|%i|%i|%i\n",g.user,stats.fstor,\
		  stats.bstor,stats.fretr,stats.bretr);
                  }
                else {
                fprintf(newfile,"%s|%i|%i|%i|%i\n",ratname,ulfiles,ulbytes,\
		dlfiles,dlbytes);
                     }
                }
        fclose(usrfile);
        fclose(newfile);

	newfile=fopen(g.ratiotmp,"rb");
        usrfile=fopen(g.ratiofile,"wb");
        while ((cpc=getc(newfile))!=EOF) {
        putc(cpc, usrfile); }
        fclose(usrfile);
        fclose(newfile);
		}
}

MODRET
pre_cmd_retr (cmd_rec * cmd)
{
  char *path;
  int fsize = 0;
  struct stat sbuf;

  _calc_ratios (cmd);
  if (!g.enable)
    return DECLINED (cmd);
  _log_ratios (cmd);

  if (!RATIO_ENFORCE)
    return DECLINED (cmd);

  if (stats.frate && stats.files < 1)
    {
      pr_response_add_err (R_550, "%s", g.filemsg);
      pr_response_add_err (R_550,
			"%s: FILE RATIO: %s  Down: %i  Up: only %i!",
			cmd->arg, stats.ftext, stats.fretr, stats.fstor);
      return ERROR (cmd);
    }

  if (stats.brate)
    {
      path = dir_realpath (cmd->tmp_pool, cmd->arg);
      if (path
	  && dir_check (cmd->tmp_pool, cmd->argv[0], cmd->group, path, NULL)
	  && pr_fsio_stat (path, &sbuf) > -1)
	fsize = sbuf.st_size;

      if ((stats.bytes - (fsize / 1024)) < 0)
	{
	  pr_response_add_err (R_550, "%s", g.bytemsg);
	  pr_response_add_err (R_550,
			    "%s: BYTE RATIO: %s  Down: %imb  Up: only %imb!",
	cmd->arg, stats.btext, (stats.bretr / 1024), (stats.bstor / 1024));
	  return ERROR (cmd);
	}
    }

  return DECLINED (cmd);
}

MODRET
log_cmd_pass (cmd_rec * cmd)
{
  char buf[120] = {'\0'};

  if (session.anon_user)
    sstrncpy (g.user, session.anon_user, sizeof(g.user));
  _calc_ratios (cmd);
  if (g.enable)
    {
      snprintf (buf, sizeof(buf), RATIO_STUFFS);
      pr_log_pri (PR_LOG_NOTICE, "Ratio: %s/%s %s[%s]: %s.", g.user,
	       session.group, session.c->remote_name,
	       pr_netaddr_get_ipstr (session.c->remote_addr), buf);
    }
  return DECLINED (cmd);
}

MODRET
pre_cmd (cmd_rec * cmd)
{
  if (g.enable)
    {
    /*  if (!strcasecmp (cmd->argv[0], "STOR")) */
      if (strcasecmp (cmd->argv[0], "STOR") || strcasecmp(cmd->argv[0], "RETR"))
	_calc_ratios (cmd);
      _log_ratios (cmd);
    }
  return DECLINED (cmd);
}

MODRET
cmd_cwd (cmd_rec * cmd)
{
  char *dir;
  config_rec *c = find_config (main_server->conf, CONF_PARAM, "CwdRatioMsg", TRUE);
  if (c)
    {
      dir = dir_realpath (cmd->tmp_pool, cmd->argv[1]);
      while (dir && c)
	{
	  if (!*((char *) c->argv[0]))
	    return DECLINED (cmd);
	  pr_response_add (R_250, "%s", (char *)c->argv[0]);
	  c = find_config_next (c, c->next, CONF_PARAM, "CwdRatioMsg", FALSE);
	}
    }
  return DECLINED (cmd);
}

MODRET
ratio_cmd (cmd_rec * cmd)
{
  FILE *usrfile,*newfile;
  char sbuf1[128] = {'\0'},sbuf2[128] = {'\0'},
       sbuf3[128] = {'\0'},usrstr[256] = {'\0'};
  char *ratname;
  int ulfiles,ulbytes,dlfiles,dlbytes,cpc;

  if (!gotratuser && g.save) {
	usrfile=fopen(g.ratiofile,"r");
	if (usrfile == NULL)
	 {
	 pr_log_pri (PR_LOG_ERR, "Error opening ratios file.");
	 gotratuser=1;
	 fileerr=1;
	 }
		   	     }

   if (session.anon_user)
     sstrncpy(g.user, session.anon_user, sizeof(g.user));
   if (strlen(g.user) == 0)
     strcpy(g.user, "NOBODY\0");

   if (!gotratuser && !fileerr && g.save) {
	usrfile=fopen(g.ratiofile,"r");
        while (fgets(usrstr,sizeof(usrstr),usrfile)!=NULL) {
                ratname = strtok(usrstr, "|");
                ulfiles = atoi (strtok(NULL, "|"));
                ulbytes = atoi (strtok(NULL, "|"));
                dlfiles = atoi (strtok(NULL, "|"));
                dlbytes = atoi (strtok(NULL, "|"));

                if (strcmp(ratname, g.user) == 0)
                {
                stats.fretr += dlfiles;
                stats.bretr += dlbytes;
		stats.fstor += ulfiles;
		stats.bstor += ulbytes;
		gotratuser = 1;
                }   				}
        fclose(usrfile);

	/* Entry for user must not exist, create... */

	if (!gotratuser && !fileerr) {
	newfile=fopen(g.ratiotmp,"w");
	  if (newfile == NULL)
	  {
	  pr_log_pri (PR_LOG_ERR, "Error opening temporary ratios file.");
	  gotratuser=1;
	  fileerr=1;
          }
				    }

		if (!gotratuser && !fileerr) {
		usrfile=fopen(g.ratiofile,"r");
        	newfile=fopen(g.ratiotmp,"w");
		  if (newfile == NULL)
         	  {
         	  pr_log_pri (PR_LOG_ERR, "Error opening temporary ratios file.");
         	  gotratuser=1;
         	  fileerr=1;
         	  }
                while (fgets(usrstr,sizeof(usrstr),usrfile)!=NULL) {
                fprintf(newfile,"%s",usrstr); }
		fprintf(newfile,"%s|%i|%i|%i|%i\n",g.user,stats.fstor,\
		stats.bstor,stats.fretr,stats.bretr);
        	fclose(usrfile);
        	fclose(newfile);
			/* Copy temp to actual ratio file.. */
			newfile=fopen(g.ratiotmp,"rb");
        		usrfile=fopen(g.ratiofile,"wb");
        		while ((cpc=getc(newfile))!=EOF) {
        		putc(cpc, usrfile); }
        		fclose(usrfile);
        		fclose(newfile);
				}

	}

  if (g.enable)
    {
      int cwding = !strcasecmp (cmd->argv[0], "CWD");
    char *r = (cwding) ? R_250 : R_DUP;
      sbuf1[0] = sbuf2[0] = sbuf3[0] = 0;
      if (cwding || !strcasecmp (cmd->argv[0], "PASS"))
	_calc_ratios (cmd);
	
      snprintf (sbuf1, sizeof(sbuf1), "Down: %i Files (%imb)  Up: %i Files (%imb)",
	stats.fretr, (stats.bretr / 1024), stats.fstor, (stats.bstor / 1024));
      if (stats.frate)
	snprintf (sbuf2, sizeof(sbuf2),
		  "   %s CR: %i", stats.ftext, stats.files);
      if (stats.brate)
	snprintf (sbuf3, sizeof(sbuf3),
		  "   %s CR: %i", stats.btext, (stats.bytes / 1024));

      if (RATIO_ENFORCE)
	{
	  pr_response_add (r, "%s%s%s", sbuf1, sbuf2, sbuf3);
	  if (stats.frate && stats.files < 0)
	    pr_response_add (r, "%s", g.filemsg);
	  if (stats.brate && stats.bytes < 0)
	    pr_response_add (r, "%s", g.bytemsg);
	}
      else
	pr_response_add (r, "%s%s%s", sbuf1, g.leechmsg ? "  " : "", g.leechmsg);
    }
  return DECLINED (cmd);
}

MODRET
cmd_site (cmd_rec * cmd)
{
  char buf[128] = {'\0'};
  
  if(cmd->argc < 2)
    return DECLINED(cmd);
  
  if(!strcasecmp(cmd->argv[1], "RATIO")) {
    _calc_ratios(cmd);
    snprintf(buf, sizeof(buf), RATIO_STUFFS);
    pr_response_add(R_214, "Current Ratio: ( %s )", buf);
    if(stats.frate)
      pr_response_add(R_214,
		   "Files: %s  Down: %i  Up: %i  CR: %i file%s",
		   stats.ftext, stats.fretr, stats.fstor,
		   stats.files, (stats.files != 1) ? "s" : "");
    if(stats.brate)
      pr_response_add(R_214,
		   "Bytes: %s  Down: %imb  Up: %imb  CR: %i Mbytes",
		   stats.btext, (stats.bretr / 1024), (stats.bstor / 1024),
		   (stats.bytes / 1024));
    return HANDLED(cmd);
  }
  
  if(!strcasecmp (cmd->argv[1], "HELP")) {
    pr_response_add(R_214,
		 "The following SITE extensions are recognized:");
    pr_response_add(R_214, "RATIO " "-- show all ratios in effect");
  }
  
  return DECLINED (cmd);
}

/* FIXME: because of how ratio and sql interact, the status sent after
   STOR and RETR commands is always out-of-date.  Reorder module loading?  */

MODRET
post_cmd_stor (cmd_rec * cmd)
{
  stats.fstor++;
  stats.bstor += (session.xfer.total_bytes / 1024);
  _calc_ratios (cmd);
  if (!fileerr && g.save) { _update_stats(); }
  return ratio_cmd (cmd);
}

MODRET
post_cmd_retr (cmd_rec * cmd)
{
  stats.fretr++;
  stats.bretr += (session.xfer.total_bytes / 1024);
  _calc_ratios (cmd);
  if (!fileerr && g.save) { _update_stats(); }
  return ratio_cmd (cmd);
}

MODRET
cmd_user (cmd_rec * cmd)
{
  if (!g.user[0])
    sstrncpy (g.user, cmd->argv[1], ARBITRARY_MAX);
  return DECLINED (cmd);
}

static cmdtable ratio_cmdtab[] = {
/* *INDENT-OFF* */

  { LOG_CMD,  C_PASS,	G_NONE, log_cmd_pass, 	FALSE, FALSE },

  { PRE_CMD,  C_RETR,   G_NONE, pre_cmd_retr,	FALSE, FALSE },
  { PRE_CMD,  C_CWD,	G_NONE, pre_cmd, 	FALSE, FALSE },
  { PRE_CMD,  C_STOR,	G_NONE, pre_cmd, 	FALSE, FALSE },
  { PRE_CMD,  C_LIST,	G_NONE, pre_cmd, 	FALSE, FALSE },
  { PRE_CMD,  C_NLST,	G_NONE, pre_cmd, 	FALSE, FALSE },

  { CMD,      C_USER,	G_NONE, cmd_user, 	FALSE, FALSE },
  { CMD,      C_SITE,	G_NONE, cmd_site, 	FALSE, FALSE },
  { CMD,      C_CWD,	G_NONE, cmd_cwd, 	FALSE, FALSE },

  { POST_CMD, C_LIST,	G_NONE, ratio_cmd, 	FALSE, FALSE },
  { POST_CMD, C_NLST,	G_NONE, ratio_cmd, 	FALSE, FALSE },
  { POST_CMD, C_PASS,	G_NONE, ratio_cmd, 	FALSE, FALSE },

  { POST_CMD, C_STOR,	G_NONE, post_cmd_stor,	FALSE, FALSE },
  { POST_CMD, C_RETR,   G_NONE, post_cmd_retr,	FALSE, FALSE },

  { 0, NULL }

/* *INDENT-ON* */

};

/* **************************************************************** */

MODRET
add_ratiodata (cmd_rec * cmd)
{
  CHECK_ARGS (cmd, 5);
  CHECK_CONF (cmd,
	      CONF_ROOT | CONF_VIRTUAL | CONF_ANON | CONF_DIR | CONF_GLOBAL);
  add_config_param_str (cmd->argv[0], 5, (void *) cmd->argv[1],
			(void *) cmd->argv[2], (void *) cmd->argv[3],
			(void *) cmd->argv[4], (void *) cmd->argv[5]);
  return HANDLED (cmd);
}

MODRET
add_ratios (cmd_rec * cmd)
{
  int b;
  config_rec *c;

  CHECK_ARGS (cmd, 1);
  CHECK_CONF (cmd, CONF_ROOT | CONF_VIRTUAL
	      | CONF_ANON | CONF_DIR | CONF_GLOBAL);
  b = get_boolean (cmd, 1);
  if (b == -1)
    CONF_ERROR (cmd, "requires a boolean value");
  c = add_config_param ("Ratios", 1, (void *) b);
  c->flags |= CF_MERGEDOWN;
  return HANDLED (cmd);
}

MODRET
add_saveratios (cmd_rec * cmd)
{
  int b;
  config_rec *c;

  CHECK_ARGS (cmd, 1);
  CHECK_CONF (cmd, CONF_ROOT | CONF_VIRTUAL
              | CONF_ANON | CONF_DIR | CONF_GLOBAL);
  b = get_boolean (cmd, 1);
  if (b == -1)
    CONF_ERROR (cmd, "requires a boolean value");
  c = add_config_param ("SaveRatios", 1, (void *) b);
  c->flags |= CF_MERGEDOWN;
  return HANDLED (cmd);
}

MODRET
add_str (cmd_rec * cmd)
{
  CHECK_ARGS (cmd, 1);
  CHECK_CONF (cmd, CONF_ROOT | CONF_VIRTUAL
	      | CONF_ANON | CONF_DIR | CONF_GLOBAL);
  add_config_param_str (cmd->argv[0], 1, (void *) cmd->argv[1]);
  return HANDLED (cmd);
}

static conftable ratio_conftab[] = {
/* *INDENT-OFF* */

  { "UserRatio",	add_ratiodata,       NULL },
  { "GroupRatio",	add_ratiodata,       NULL },
  { "AnonRatio",	add_ratiodata,       NULL },
  { "HostRatio",	add_ratiodata,       NULL },
  { "Ratios",	        add_ratios,          NULL },

  { "FileRatioErrMsg",	add_str,             NULL },
  { "ByteRatioErrMsg",	add_str,             NULL },
  { "LeechRatioMsg",	add_str,             NULL },
  { "CwdRatioMsg",	add_str,             NULL },
  { "SaveRatios",	add_saveratios,	     NULL },
  { "RatioFile",	add_str,	     NULL },
  { "RatioTempFile",	add_str,	     NULL },

  { NULL, NULL, NULL }

/* *INDENT-ON* */

};

/* **************************************************************** */

static int
ratio_child_init ()
{
  memset (&g, 0, sizeof (g));
  g.enable = get_param_int (TOPLEVEL_CONF, "Ratios", FALSE) == TRUE;
  g.save = get_param_int (TOPLEVEL_CONF, "SaveRatios", FALSE) == TRUE;

  if (!(g.filemsg = get_param_ptr (TOPLEVEL_CONF, "FileRatioErrMsg", FALSE)))
    g.filemsg = "Too few files uploaded to earn file -- please upload more.";

  if (!(g.ratiofile = get_param_ptr (TOPLEVEL_CONF, "RatioFile", FALSE)))
    g.ratiofile = "";

  if (!(g.ratiotmp = get_param_ptr (TOPLEVEL_CONF, "RatioTempFile", FALSE)))
    g.ratiotmp = "";

  if (!(g.bytemsg = get_param_ptr (TOPLEVEL_CONF, "ByteRatioErrMsg", FALSE)))
    g.bytemsg = "Too few bytes uploaded to earn more data -- please upload.";

  if (!(g.leechmsg = get_param_ptr (TOPLEVEL_CONF, "LeechRatioMsg", FALSE)))
    g.leechmsg = "10,000,000:1  CR: LEECH";

  return 0;
}

module ratio_module = {
  NULL, NULL,			/* Always NULL */
  0x20,				/* API Version 2.0 */
  "ratio",
  ratio_conftab,		/* Ratio configuration handler table */
  ratio_cmdtab,			/* Ratio command handler table */
  NULL,				/* No authentication handler table */
  NULL,				/* No initial initialization needed */
  ratio_child_init		/* Post-fork "child mode" init */
};
