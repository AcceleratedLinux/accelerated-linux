/*
 * mod_ldap - LDAP password lookup module for ProFTPD
 * Copyright (c) 1999, 2000-5, John Morrissey <jwm@horde.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * Furthermore, John Morrissey gives permission to link this program with
 * OpenSSL, and distribute the resulting executable, without including the
 * source code for OpenSSL in the source distribution.
 */

/*
 * mod_ldap v2.8.15
 *
 * Thanks for patches go to (in alphabetical order):
 *
 * Peter Fabian (fabian at staff dot matavnet dot hu) - LDAPAuthBinds
 * Marek Gradzki (mgradzki at ost dot net dot pl) - LDAPProtocolVersion
 * Pierrick Hascoet (pierrick at alias dot fr) - OpenSSL password hash support
 * Florian Lohoff (flo at rfc822 dot org) - LDAPForceDefault[UG]ID code
 * Steve Luzynski (steve at uniteone dot net) - HomedirOnDemandPrefix support
 * Gaute Nessan (gaute at kpnqwest dot no) - OpenLDAP 2.0 fixes
 * Marcin Obara (gryzzli at wp-sa dot pl) - User/group caching code, Sun
 *                                          LDAP library portability fixes
 * Phil Oester (phil at theoesters dot com) - Group code memory manip fixes
 * Michael Schout (mschout at gkg dot net) - Full-path HomedirOnDemand and
 *                                           multiple-HomedirOnDemandSuffix
 *                                           support
 * Klaus Steinberger (klaus dot steinberger at physik dot uni-muenchen dot de)
 *                                         - LDAPForceHomedirOnDemand support
 * Andreas Strodl (andreas at strodl dot org) - multiple group support
 * Ross Thomas (ross at grinfinity dot com) - Non-AuthBinds auth fix
 * Ivo Timmermans (ivo at debian dot org) - TLS support
 * Bert Vermeulen (bert at be dot easynet dot net) - LDAPHomedirOnDemand,
 *                                                   LDAPDefaultAuthScheme
 *
 *
 * $Id: mod_ldap.c,v 1.38 2005/03/11 20:44:11 jwm Exp $
 * $Libraries: -lldap -llber$
 */

/* Uncomment this to use LDAP TLS. If enabled, we will try to enable TLS
 * after connecting to the LDAP server. If TLS cannot be enabled, the LDAP
 * connection will fail.
 */
/* #define USE_LDAP_TLS */

/* Uncomment this if you have OpenSSL and wish to verify non-crypt()
 * password hashes locally with OpenSSL. You'll also need to edit
 * ../Make.rules so the compiler will find OpenSSL's include files
 * (-I/path/to/include-dir) and link again OpenSSL's crypto library
 * (-L/path/to/lib-dir -lcrypto).
 */
/* #define HAVE_OPENSSL */

/*
 * If you have to edit anything below this line, it's a bug. Report it
 * at http://bugs.proftpd.org/.
 */

#include "conf.h"
#include "privs.h"

#define MOD_LDAP_VERSION	"mod_ldap/2.8.14"

#if PROFTPD_VERSION_NUMBER < 0x0001021002
# error "mod_ldap " MOD_LDAP_VERSION " requires ProFTPD 1.2.10rc2 or later"
#endif

#if defined(HAVE_CRYPT_H) && !defined(AIX4) && !defined(AIX5)
# include <crypt.h>
#endif

#include <errno.h>
#include <ctype.h>     /* isdigit()   */
#include <stdio.h>     /* snprintf()  */
#include <string.h>    /* various :-) */
#include <sys/types.h> /* seteuid()   */
#include <unistd.h>    /* seteuid()   */

#include <lber.h>
#include <ldap.h>

/* Thanks, Sun. */
#ifndef LDAP_OPT_SUCCESS
# define LDAP_OPT_SUCCESS LDAP_SUCCESS
#endif

#ifdef HAVE_OPENSSL
# include <openssl/evp.h>
#endif

#define HASH_TABLE_SIZE 10

typedef union pr_idauth {
  uid_t uid;
  gid_t gid;
} pr_idauth_t;

typedef struct _idmap {
  struct _idmap *next, *prev;

  /* This is a union because different OSs may give different types/sizes to
   * UIDs and GIDs. This presents a far more portable way to deal with this
   * reality.
   */
  pr_idauth_t id;

  char *name;                  /* user or group name */
  unsigned short int negative; /* have we gotten a negative answer before? */
} pr_idmap_t;

static xaset_t *uid_table[HASH_TABLE_SIZE];
static xaset_t *gid_table[HASH_TABLE_SIZE];

/* Config entries */
static char *ldap_server, *ldap_dn, *ldap_dnpass,
            *ldap_auth_filter, *ldap_uid_filter,
            *ldap_group_gid_filter, *ldap_group_name_filter,
            *ldap_group_member_filter, *ldap_quota_filter,
            *ldap_auth_basedn, *ldap_uid_basedn, *ldap_gid_basedn,
            *ldap_quota_basedn,
            *ldap_defaultauthscheme, *ldap_authbind_dn,
            *ldap_genhdir_prefix, *ldap_default_quota,
            *ldap_attr_uid = "uid",
            *ldap_attr_uidnumber = "uidNumber",
            *ldap_attr_gidnumber = "gidNumber",
            *ldap_attr_homedirectory = "homeDirectory",
            *ldap_attr_userpassword = "userPassword",
            *ldap_attr_loginshell = "loginShell",
            *ldap_attr_cn = "cn",
            *ldap_attr_memberuid = "memberUid",
            *ldap_attr_ftpquota = "ftpQuota";
static int ldap_doauth = 0, ldap_douid = 0, ldap_dogid = 0, ldap_doquota = 0,
           ldap_authbinds = 1, ldap_negcache = 1,
           ldap_querytimeout = 0, ldap_genhdir = 0, ldap_genhdir_prefix_nouname = 0,
           ldap_forcedefaultuid = 0, ldap_forcedefaultgid = 0,
           ldap_forcegenhdir = 0, ldap_protocol_version = 3,
           ldap_search_scope = LDAP_SCOPE_SUBTREE;
static struct timeval ldap_querytimeout_tp;

/* We get these values from get_param_int(), which returns a long. On
 * systems with 4-byte longs (most 32-bit systems in existence), this limits
 * you to a maximum UID/GID of around 2 billion (half the limit of a true
 * 32-bit-UID-enabled system, which tops out at about 4 billion).
 */
static uid_t ldap_defaultuid = -1;
static gid_t ldap_defaultgid = -1;

#ifdef USE_LDAP_TLS
static int ldap_use_tls = 0;
#endif

static LDAP *ld = NULL;
static struct passwd *pw = NULL;
static struct group *gr = NULL;
array_header *cached_quota = NULL;


static int
pr_ldap_module_init(void)
{
  memset(uid_table, 0, sizeof(uid_table));
  memset(gid_table, 0, sizeof(gid_table));
  return 0;
}

static void
pr_ldap_set_sizelimit(LDAP *limit_ld, int limit)
{
#ifdef LDAP_OPT_SIZELIMIT
  int ret;
  if ((ret = ldap_set_option(limit_ld, LDAP_OPT_SIZELIMIT, (void *)&limit)) != LDAP_OPT_SUCCESS)
    pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_set_sizelimit(): ldap_set_option() unable to set query size limit to %d entries: %s", limit, ldap_err2string(ret));
#else
  limit_ld->ld_sizelimit = limit;
#endif
}

static void
pr_ldap_unbind(void)
{
  int ret;

  if (! ld)
    return;

  if ((ret = ldap_unbind_s(ld)) != LDAP_SUCCESS)
    pr_log_pri(PR_LOG_NOTICE, "mod_ldap: pr_ldap_unbind(): ldap_unbind() failed: %s", ldap_err2string(ret));

  ld = NULL;
}

static int
pr_ldap_connect(LDAP **conn_ld, int bind)
{
  int ret, version;

  if ((*conn_ld = ldap_init(ldap_server, LDAP_PORT)) == NULL) {
    pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_connect(): ldap_init() to %s failed: %s", ldap_server, strerror(errno));
    return -1;
  }

  version = -1;
  switch (ldap_protocol_version) {
    case 2:
      version = LDAP_VERSION2;
      break;
    case 3:
    default:
      version = LDAP_VERSION3;
      break;
  }

  if (version != -1) {
    if ((ret = ldap_set_option(*conn_ld, LDAP_OPT_PROTOCOL_VERSION, &version)) != LDAP_OPT_SUCCESS) {
      pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_connect(): Setting LDAP version option failed: %s", ldap_err2string(ret));
      pr_ldap_unbind();
      return -1;
    }
  }

#ifdef USE_LDAP_TLS
  if (ldap_use_tls == 1) {
    pr_log_debug(DEBUG2, "mod_ldap: Starting TLS for this connection.");
    if ((ret = ldap_start_tls_s(*conn_ld, NULL, NULL)) != LDAP_SUCCESS) {
      pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_connect(): Starting TLS failed: %s", ldap_err2string(ret));
      pr_ldap_unbind();
      return -1;
    }
  }
#endif /* USE_LDAP_TLS */

  if (bind == TRUE) {
    if ((ret = ldap_simple_bind_s(*conn_ld, ldap_dn, ldap_dnpass)) != LDAP_SUCCESS) {
      pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_connect(): ldap_simple_bind() as %s failed: %s", ldap_dn, ldap_err2string(ret));
      return -1;
    }
  }

  pr_ldap_set_sizelimit(*conn_ld, 2);

  ldap_querytimeout_tp.tv_sec = (ldap_querytimeout > 0 ? ldap_querytimeout : 5);
  ldap_querytimeout_tp.tv_usec = 0;

  return 1;
}

static char *
pr_ldap_generate_filter(pool *p, char *template, const char *entity)
{
  char *filter, *pos;
  int num_escapes = 0, i = 0, j = 0;

  pos = template;
  while ((pos = strstr(pos + 2, "%v")) != NULL)
    ++num_escapes;

  /* -2 for the %v, +1 for the NULL */
  filter = pcalloc(p, strlen(template) - (num_escapes * 2) + (num_escapes * strlen(entity)) + 1);

  while (template[i] != '\0') {
    /* Replace %u or %v with entity. */
    if (template[i] == '%' && (template[i + 1] == 'u' || template[i + 1] == 'v')) {
      strcat(filter, entity);
      j += strlen(entity);
      i += 2;
    }
    else
      filter[j++] = template[i++];
  }

  return filter;
}

static struct passwd *
pr_ldap_user_lookup(pool *p,
                    char *filter_template, const char *replace,
                    char *basedn, char *ldap_attrs[],
                    char **user_dn)
{
  char *filter, **values, *dn;
  int i = 0, ret;
  LDAPMessage *result, *e;

  if (! basedn) {
    pr_log_pri(PR_LOG_ERR, "mod_ldap: no LDAP base DN specified for auth/UID lookups, declining request.");
    return NULL;
  }

  /* If the LDAP connection has gone away or hasn't been established
   * yet, attempt to establish it now.
   */
  if (ld == NULL) {
    /* If we _still_ can't connect, give up and return NULL. */
    if (pr_ldap_connect(&ld, TRUE) == -1)
      return NULL;
  }

  filter = pr_ldap_generate_filter(p, filter_template, replace);

  if ((ret = ldap_search_st(ld, basedn, ldap_search_scope, filter, ldap_attrs, 0, &ldap_querytimeout_tp, &result)) != LDAP_SUCCESS) {
    if (ret == LDAP_SERVER_DOWN) {
      pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_user_lookup(): LDAP server went away, trying to reconnect");

      if (pr_ldap_connect(&ld, TRUE) == -1) {
        pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_user_lookup(): LDAP server went away, unable to reconnect");
        ld = NULL;
        return NULL;
      }

      pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_user_lookup(): Reconnect to LDAP server successful, resuming normal operations");
      if ((ret = ldap_search_st(ld, basedn, ldap_search_scope, filter, ldap_attrs, 0, &ldap_querytimeout_tp, &result)) != LDAP_SUCCESS) {
        pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_user_lookup(): ldap_search_st() failed: %s", ldap_err2string(ret));
        return NULL;
      }
    }
    else {
      pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_user_lookup(): ldap_search_st() failed: %s", ldap_err2string(ret));
      return NULL;
    }
  }

  if (ldap_count_entries(ld, result) > 1) {
    pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_user_lookup(): LDAP search returned multiple entries, aborting query");
    ldap_msgfree(result);
    return NULL;
  }

  if ((e = ldap_first_entry(ld, result)) == NULL) {
    ldap_msgfree(result);
    return NULL; /* No LDAP entries for this user */
  }

  if (! pw)
    pw = pcalloc(session.pool, sizeof(struct passwd));
  else
    memset(pw, '\0', sizeof(struct passwd));

  while (ldap_attrs[i] != NULL) {
    if ((values = ldap_get_values(ld, e, ldap_attrs[i])) == NULL) {
      /* Try to fill in default values if there's no value for certain attrs. */

      /* If we can't find the [ug]idNumber attrs, just fill the passwd
         struct in with default values from the config file. */
      if (strcasecmp(ldap_attrs[i], ldap_attr_uidnumber) == 0) {
        if (ldap_defaultuid == -1) {
          pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_user_lookup(): no %s attr for DN %s and LDAPDefaultUID was not specified!", (dn = ldap_get_dn(ld, e)), ldap_attr_uidnumber);
          free(dn);
          return NULL;
        }

        pw->pw_uid = ldap_defaultuid;
        ++i;
        continue;
      }
      if (strcasecmp(ldap_attrs[i], ldap_attr_gidnumber) == 0) {
        if (ldap_defaultgid == -1) {
          pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_user_lookup(): no %s attr for DN %s and LDAPDefaultGID was not specified!", (dn = ldap_get_dn(ld, e)), ldap_attr_gidnumber);
          free(dn);
          return NULL;
        }

        pw->pw_gid = ldap_defaultgid;
        ++i;
        continue;
      }

      if (strcasecmp(ldap_attrs[i], ldap_attr_homedirectory) == 0) {
        if (!ldap_genhdir || !ldap_genhdir_prefix || !*ldap_genhdir_prefix) {
          pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_user_lookup(): no %s attr for DN %s and LDAPGenerateHomedirPrefix was not enabled!", (dn = ldap_get_dn(ld, e)), ldap_attr_homedirectory);
          free(dn);
          return NULL;
        }

        if (ldap_genhdir_prefix_nouname)
          pw->pw_dir = pstrcat(session.pool, ldap_genhdir_prefix, NULL);
        else {
          char **canon_username;
          if ((canon_username = ldap_get_values(ld, e, ldap_attr_uid)) == NULL) {
            pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_user_lookup(): couldn't get %s attr for canonical username for %s", (dn = ldap_get_dn(ld, e)), ldap_attr_uid);
            free(dn);
            return NULL;
          }

          pw->pw_dir = pstrcat(session.pool, ldap_genhdir_prefix, "/", canon_username[0], NULL);
          ldap_value_free(canon_username);
        }

        ++i;
        continue;
      }

      /* Don't worry if we don't have a loginShell attr. */
      if (strcasecmp(ldap_attrs[i], ldap_attr_loginshell) == 0) {
        /* Prevent a segfault if no loginShell attr && RequireValidShell on. */
        pw->pw_shell = pstrdup(session.pool, "");
        ++i;
        continue;
      }

      /* We only restart the while loop above if we can fill in alternate
       * values for certain attributes. If something odd has happened, we
       * fall through to here and will complain about not being able to find
       * the attr.
       */

      pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_user_lookup(): ldap_get_values() failed on attr %s for DN %s, ignoring request (perhaps this DN's entry does not have the attr?)", ldap_attrs[i], (dn = ldap_get_dn(ld, e)));
      free(dn);
      ldap_msgfree(result);
      return NULL;
    }

    /* Once we get here, we've already handled the "attribute defaults"
     * situation, so we can just fill in the struct as normal; the if
     * branches below for nonexistant attrs will just never be called.
     */

    if (strcasecmp(ldap_attrs[i], ldap_attr_uid) == 0)
      pw->pw_name = pstrdup(session.pool, values[0]);
    else if (strcasecmp(ldap_attrs[i], ldap_attr_userpassword) == 0)
      pw->pw_passwd = pstrdup(session.pool, values[0]);
    else if (strcasecmp(ldap_attrs[i], ldap_attr_uidnumber) == 0) {
      if (ldap_forcedefaultuid && ldap_defaultuid != -1)
        pw->pw_uid = ldap_defaultuid;
      else
        pw->pw_uid = (uid_t) strtoul(values[0], (char **)NULL, 10);
    }
    else if (strcasecmp(ldap_attrs[i], ldap_attr_gidnumber) == 0) {
      if (ldap_forcedefaultgid && ldap_defaultgid != -1)
        pw->pw_gid = ldap_defaultgid;
      else
        pw->pw_gid = (gid_t) strtoul(values[0], (char **)NULL, 10);
    }
    else if (strcasecmp(ldap_attrs[i], ldap_attr_homedirectory) == 0) {
      if (ldap_forcegenhdir) {
        if (!ldap_genhdir || !ldap_genhdir_prefix || !*ldap_genhdir_prefix) {
          pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_user_lookup(): LDAPForceGeneratedHomedir is enabled, but LDAPGenerateHomedir is not.");
          return NULL;
        }

        if (ldap_genhdir_prefix_nouname)
          pw->pw_dir = pstrcat(session.pool, ldap_genhdir_prefix, NULL);
        else {
          char **canon_username;
          if ((canon_username = ldap_get_values(ld, e, ldap_attr_uid)) == NULL) {
            pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_user_lookup(): couldn't get %s attr for canonical username for %s", (dn = ldap_get_dn(ld, e)), ldap_attr_uid);
            free(dn);
            return NULL;
          }

          pw->pw_dir = pstrcat(session.pool, ldap_genhdir_prefix, "/", canon_username[0], NULL);
          ldap_value_free(canon_username);
        }
      }
      else
        pw->pw_dir = pstrdup(session.pool, values[0]);
    }
    else if (strcasecmp(ldap_attrs[i], ldap_attr_loginshell) == 0)
      pw->pw_shell = pstrdup(session.pool, values[0]);
    else
      pr_log_pri(PR_LOG_WARNING, "mod_ldap: pr_ldap_user_lookup(): ldap_get_values() loop found unknown attr %s", ldap_attrs[i]);

    ldap_value_free(values);
    ++i;
  }

  /* If we're doing auth binds, save the DN of this entry so we can
   * bind to the LDAP server as it later.
   */
  if (user_dn)
    *user_dn = ldap_get_dn(ld, e);

  ldap_msgfree(result);

  return pw;
}

static struct group *
pr_ldap_group_lookup(pool *p,
                     char *filter_template, const char *replace,
                     char *ldap_attrs[])
{
  char *filter, **values, *dn;
  int i = 0, value_count, value_offset, ret;
  LDAPMessage *result, *e;

  if (! ldap_gid_basedn) {
    pr_log_pri(PR_LOG_ERR, "mod_ldap: no LDAP base DN specified for GID lookups");
    return NULL;
  }

  /* If the LDAP connection has gone away or hasn't been established
   * yet, attempt to establish it now.
   */
  if (ld == NULL) {
    /* If we _still_ can't connect, give up and return NULL. */
    if (pr_ldap_connect(&ld, TRUE) == -1)
      return NULL;
  }

  filter = pr_ldap_generate_filter(p, filter_template, replace);

  if ((ret = ldap_search_st(ld, ldap_gid_basedn, ldap_search_scope, filter, ldap_attrs, 0, &ldap_querytimeout_tp, &result)) != LDAP_SUCCESS) {
    if (ret == LDAP_SERVER_DOWN) {
      pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_group_lookup(): LDAP server went away, trying to reconnect");

      if (pr_ldap_connect(&ld, TRUE) != -1) {
        if ((ret = ldap_search_st(ld, ldap_gid_basedn, ldap_search_scope, filter, ldap_attrs, 0, &ldap_querytimeout_tp, &result)) != LDAP_SUCCESS) {
          pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_group_lookup(): ldap_search_st() failed: %s", ldap_err2string(ret));
          return NULL;
        }
      }
      else { /* Still can't connect */
        pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_group_lookup(): LDAP server went away, unable to reconnect");
        return NULL;
      }
    }
    else {
      pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_group_lookup(): ldap_search_st() failed: %s", ldap_err2string(ret));
      return NULL;
    }
  }

  if ((e = ldap_first_entry(ld, result)) == NULL) {
    ldap_msgfree(result);
    return NULL; /* No LDAP entries for this user */
  }

  if (! gr)
    gr = pcalloc(session.pool, sizeof(struct group));
  else
    memset(gr, '\0', sizeof(struct group));

  while (ldap_attrs[i] != NULL) {
    if ((values = ldap_get_values(ld, e, ldap_attrs[i])) == NULL) {
      if (strcasecmp(ldap_attrs[i], ldap_attr_memberuid) == 0) {
        gr->gr_mem = palloc(session.pool, 2 * sizeof(char *));
        gr->gr_mem[0] = pstrdup(session.pool, "");
        gr->gr_mem[1] = NULL;

        ++i;
        continue;
      }

      ldap_msgfree(result);
      pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_group_lookup(): ldap_get_values() failed on attr %s for DN %s, ignoring request (perhaps that DN does not have that attr?)", ldap_attrs[i], (dn = ldap_get_dn(ld, e)));
      free(dn);
      return NULL;
    }

    if (strcasecmp(ldap_attrs[i], ldap_attr_cn) == 0)
      gr->gr_name = pstrdup(session.pool, values[0]);
    else if (strcasecmp(ldap_attrs[i], ldap_attr_gidnumber) == 0)
      gr->gr_gid = strtoul(values[0], (char **)NULL, 10);
    else if (strcasecmp(ldap_attrs[i], ldap_attr_memberuid) == 0) {
      value_count = ldap_count_values(values);
      gr->gr_mem = (char **) palloc(session.pool, value_count * sizeof(char *));

      for (value_offset = 0; value_offset < value_count; ++value_offset)
        gr->gr_mem[value_offset] = pstrdup(session.pool, values[value_offset]);
    }
    else
      pr_log_pri(PR_LOG_WARNING, "mod_ldap: pr_ldap_group_lookup(): ldap_get_values() loop found unknown attr %s", ldap_attrs[i]);

    ldap_value_free(values);
    ++i;
  }

  ldap_msgfree(result);
  return gr;
}

static void
parse_quota(pool *p, const char *replace, char *str)
{
  char **elts, *token;

  if (cached_quota == NULL)
    cached_quota = make_array(p, 9, sizeof(char *));
  elts = (char **)cached_quota->elts;
  elts[0] = pstrdup(session.pool, replace);
  cached_quota->nelts = 1;

  while ((token = strsep(&str, ","))) {
    *((char **)push_array(cached_quota)) = pstrdup(session.pool, token);
  }
}

static unsigned char
pr_ldap_quota_lookup(pool *p, char *filter_template, const char *replace,
                     char *basedn)
{
  char *filter, **values, *attrs[] = {ldap_attr_ftpquota, NULL};
  int ret;
  LDAPMessage *result, *e;

  if (! basedn) {
    pr_log_pri(PR_LOG_ERR, "mod_ldap: no LDAP base DN specified for auth/UID lookups, declining request.");
    return FALSE;
  }

  /* If the LDAP connection has gone away or hasn't been established
   * yet, attempt to establish it now.
   */
  if (ld == NULL) {
    /* If we _still_ can't connect, give up and return NULL. */
    if (pr_ldap_connect(&ld, TRUE) == -1)
      return FALSE;
  }

  filter = pr_ldap_generate_filter(p, filter_template, replace);

  if ((ret = ldap_search_st(ld, basedn, ldap_search_scope, filter, attrs, 0, &ldap_querytimeout_tp, &result)) != LDAP_SUCCESS) {
    if (ret == LDAP_SERVER_DOWN) {
      pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_quota_lookup(): LDAP server went away, trying to reconnect");

      if (pr_ldap_connect(&ld, TRUE) == -1) {
        pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_quota_lookup(): LDAP server went away, unable to reconnect");
        ld = NULL;
        return FALSE;
      }

      pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_quota_lookup(): Reconnect to LDAP server successful, resuming normal operations");
      if ((ret = ldap_search_st(ld, basedn, ldap_search_scope, filter, attrs, 0, &ldap_querytimeout_tp, &result)) != LDAP_SUCCESS) {
        pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_quota_lookup(): ldap_search_st() failed: %s", ldap_err2string(ret));
        return FALSE;
      }
    }
    else {
      pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_quota_lookup(): ldap_search_st() failed: %s", ldap_err2string(ret));
      return FALSE;
    }
  }

  if (ldap_count_entries(ld, result) > 1) {
    pr_log_pri(PR_LOG_ERR, "mod_ldap: pr_ldap_quota_lookup(): LDAP search returned multiple entries, aborting query");
    ldap_msgfree(result);
    if (ldap_default_quota != NULL) {
      parse_quota(p, replace, pstrdup(p, ldap_default_quota));
      return TRUE;
    }
    return FALSE;
  }

  if ((e = ldap_first_entry(ld, result)) == NULL) {
    ldap_msgfree(result);
    if (ldap_default_quota != NULL) {
      parse_quota(p, replace, pstrdup(p, ldap_default_quota));
      return TRUE;
    }
    return FALSE; /* No LDAP entries for this user. */
  }

  if ((values = ldap_get_values(ld, e, attrs[0])) == NULL) {
    ldap_msgfree(result);
    if (ldap_default_quota != NULL) {
      parse_quota(p, replace, pstrdup(p, ldap_default_quota));
      return TRUE;
    }
    return FALSE; /* No quota attr for this user. */
  }

  parse_quota(p, replace, pstrdup(p, values[0]));
  ldap_value_free(values);
  ldap_msgfree(result);

  return TRUE;
}

static struct group *
pr_ldap_getgrnam(pool *p, const char *group_name)
{
  char *group_attrs[] = {ldap_attr_cn, ldap_attr_gidnumber, ldap_attr_memberuid, NULL};

  return pr_ldap_group_lookup(p, ldap_group_name_filter, group_name, group_attrs);
}

static struct group *
pr_ldap_getgrgid(pool *p, gid_t gid)
{
  char gidstr[PR_TUNABLE_BUFFER_SIZE] = {'\0'},
       *group_attrs[] = {ldap_attr_cn, ldap_attr_gidnumber, ldap_attr_memberuid, NULL};

  snprintf(gidstr, sizeof(gidstr), "%u", (unsigned)gid);

  return pr_ldap_group_lookup(p, ldap_group_gid_filter, (const char *)gidstr, group_attrs);
}

static struct passwd *
pr_ldap_getpwnam(pool *p, const char *username)
{
  char *name_attrs[] = {ldap_attr_userpassword, ldap_attr_uid, ldap_attr_uidnumber,
                        ldap_attr_gidnumber, ldap_attr_homedirectory, ldap_attr_loginshell,
                        NULL};

  /* pr_ldap_user_lookup() returns NULL if it doesn't find an entry or
   * encounters an error. If everything goes all right, it returns a
   * struct passwd, so we can just return its result directly.
   *
   * We also do some cute stuff here to work around lameness in LDAP servers
   * like Sun Directory Services (SDS) 1.x and 3.x. If you request an attr
   * that you don't have access to, SDS totally ignores any entries with
   * that attribute. Thank you, Sun; how very smart of you. So if we're
   * doing auth binds, we don't request the userPassword attr.
   *
   * NOTE: if the UserPassword directive is configured, mod_auth will pass
   * a crypted password to handle_ldap_check(), which will NOT do auth binds
   * in order to support UserPassword. (Otherwise, it would try binding to
   * the directory and would ignore UserPassword.)
   *
   * We're reasonably safe in making that assumption as long as we never
   * fetch userPassword from the directory if auth binds are enabled. If we
   * fetched userPassword, auth binds would never be done because
   * handle_ldap_check() would always get a crypted password.
   */
  return pr_ldap_user_lookup(p, ldap_auth_filter, username,
                             pr_ldap_generate_filter(p, ldap_auth_basedn, username),
                             ldap_authbinds ? name_attrs + 1 : name_attrs,
                             ldap_authbinds ? &ldap_authbind_dn : NULL);
}

static struct passwd *
pr_ldap_getpwuid(pool *p, uid_t uid)
{
  char uidstr[PR_TUNABLE_BUFFER_SIZE] = {'\0'},
       *uid_attrs[] = {ldap_attr_uid, ldap_attr_uidnumber, ldap_attr_gidnumber,
                       ldap_attr_homedirectory, ldap_attr_loginshell, NULL};

  snprintf(uidstr, sizeof(uidstr), "%u", (unsigned)uid);

  /* pr_ldap_user_lookup() returns NULL if it doesn't find an entry or
   * encounters an error. If everything goes all right, it returns a
   * struct passwd, so we can just return its result directly.
   */
  return pr_ldap_user_lookup(p, ldap_uid_filter, (const char *)uidstr,
                             ldap_uid_basedn, uid_attrs,
                             ldap_authbinds ? &ldap_authbind_dn : NULL);
}

static int
_compare_uid(pr_idmap_t *m1, pr_idmap_t *m2)
{
  if (m1->id.uid < m2->id.uid)
    return -1;

  if (m1->id.uid > m2->id.uid)
    return 1;

  return 0;
}

static int
_compare_gid(pr_idmap_t *m1, pr_idmap_t *m2)
{
  if (m1->id.gid < m2->id.gid)
    return -1;

  if (m1->id.gid > m2->id.gid)
    return 1;

  return 0;
}

static int
_compare_id(xaset_t **table, pr_idauth_t id, pr_idauth_t idcomp)
{
  if (table == uid_table)
    return id.uid == idcomp.uid;
  else
    return id.gid == idcomp.gid;
}

static pr_idmap_t *
_auth_lookup_id(xaset_t **id_table, pr_idauth_t id)
{
  int hash = ((id_table == uid_table) ? id.uid : id.gid) % HASH_TABLE_SIZE;
  pr_idmap_t *m;

  if (! id_table[hash])
    id_table[hash] = xaset_create(permanent_pool, (id_table == uid_table) ?
                                  (XASET_COMPARE) _compare_uid :
                                  (XASET_COMPARE) _compare_gid);

  for (m = (pr_idmap_t *) id_table[hash]->xas_list; m; m = m->next) {
    if (_compare_id(id_table, m->id, id))
      break;
  }

  if (!m || !_compare_id(id_table, m->id, id)) {
    /* Isn't in the table */
    m = (pr_idmap_t *) pcalloc(id_table[hash]->pool, sizeof(pr_idmap_t));

    if (id_table == uid_table)
      m->id.uid = id.uid;
    else
      m->id.gid = id.gid;

    xaset_insert_sort(id_table[hash], (xasetmember_t *) m, FALSE);
  }

  return m;
}

MODRET
handle_ldap_quota_lookup(cmd_rec *cmd)
{
  char **elts = NULL;
 
  if (cached_quota != NULL)
    elts = (char **)cached_quota->elts;

  if (cached_quota == NULL ||
      strcasecmp(elts[0], cmd->argv[0]) != 0)
  {
    if (pr_ldap_quota_lookup(cmd->tmp_pool, ldap_quota_filter,
                             cmd->argv[0], ldap_quota_basedn) == FALSE)
    {
      return DECLINED(cmd);
    }
  }

  return mod_create_data(cmd, cached_quota);
}

MODRET
handle_ldap_setpwent(cmd_rec *cmd)
{
  if (ldap_doauth || ldap_douid || ldap_dogid) {
    if (ld == NULL)
      (void) pr_ldap_connect(&ld, TRUE);
    return HANDLED(cmd);
  }

  return DECLINED(cmd);
}

MODRET
handle_ldap_endpwent(cmd_rec *cmd)
{
  if (ldap_doauth || ldap_douid || ldap_dogid) {
    pr_ldap_unbind();
    pw = NULL;
    gr = NULL;
    return HANDLED(cmd);
  }

  return DECLINED(cmd);
}

MODRET
handle_ldap_getpwuid(cmd_rec *cmd)
{
  if (! ldap_douid)
    return DECLINED(cmd);

  if ((pw = pr_ldap_getpwuid(cmd->tmp_pool, (uid_t)cmd->argv[0])))
    return mod_create_data(cmd, pw);

  return DECLINED(cmd);
}

MODRET
handle_ldap_getpwnam(cmd_rec *cmd)
{
  if (! ldap_doauth)
    return DECLINED(cmd);

  if (pw && pw->pw_name && strcasecmp(pw->pw_name, cmd->argv[0]) == 0)
    return mod_create_data(cmd, pw);

  if ((pw = pr_ldap_getpwnam(cmd->tmp_pool, cmd->argv[0])))
    return mod_create_data(cmd, pw);

  return DECLINED(cmd);
}

MODRET
handle_ldap_getgrnam(cmd_rec *cmd)
{
  if (! ldap_dogid)
    return DECLINED(cmd);

  if (gr && strcasecmp(gr->gr_name, cmd->argv[0]) == 0)
    return mod_create_data(cmd, gr);

  if ((gr = pr_ldap_getgrnam(cmd->tmp_pool, cmd->argv[0])))
    return mod_create_data(cmd, gr);

  return DECLINED(cmd);
}

MODRET
handle_ldap_getgrgid(cmd_rec *cmd)
{
  if (! ldap_dogid)
    return DECLINED(cmd);

  if (gr && gr->gr_gid == (gid_t)cmd->argv[0])
    return mod_create_data(cmd, gr);

  if ((gr = pr_ldap_getgrgid(cmd->tmp_pool, (gid_t)cmd->argv[0])))
    return mod_create_data(cmd, gr);

  return DECLINED(cmd);
}

MODRET
handle_ldap_getgroups(cmd_rec *cmd)
{
  char *filter, **gidNumber, **cn,
       *w[] = {ldap_attr_gidnumber, ldap_attr_cn, NULL};
  int ret;
  struct passwd *pw;
  struct group *gr;
  LDAPMessage *result = NULL, *e;
  array_header *gids   = (array_header *)cmd->argv[1],
               *groups = (array_header *)cmd->argv[2];

  if (! ldap_dogid)
    return DECLINED(cmd);

  if (!gids || !groups)
    return DECLINED(cmd);

  if ((pw = pr_ldap_getpwnam(cmd->tmp_pool, cmd->argv[0]))) {
    if ((gr = pr_ldap_getgrgid(cmd->tmp_pool, pw->pw_gid))) {
      *((gid_t *) push_array(gids))   = pw->pw_gid;
      *((char **) push_array(groups)) = pstrdup(session.pool, gr->gr_name);
    }
  }

  if (! ldap_gid_basedn) {
    pr_log_pri(PR_LOG_ERR, "mod_ldap: no LDAP base DN specified for GID lookups");
    goto return_groups;
  }

  /* If the LDAP connection has gone away or hasn't been established
   * yet, attempt to establish it now.
   */
  if (ld == NULL) {
    /* If we _still_ can't connect, give up and decline. */
    if (pr_ldap_connect(&ld, TRUE) == -1)
      goto return_groups;
  }

  filter = pr_ldap_generate_filter(cmd->tmp_pool, ldap_group_member_filter, cmd->argv[0]);

  /* Unlimited. */
  pr_ldap_set_sizelimit(ld, 0);
  if ((ret = ldap_search_st(ld, ldap_gid_basedn, ldap_search_scope, filter, w, 0, &ldap_querytimeout_tp, &result)) != LDAP_SUCCESS) {
    if (ret == LDAP_SERVER_DOWN) {
      pr_log_pri(PR_LOG_ERR, "mod_ldap: ldap_handle_getgroups(): LDAP server went away, trying to reconnect");

      if (pr_ldap_connect(&ld, TRUE) != -1) {
        if ((ret = ldap_search_st(ld, ldap_gid_basedn, ldap_search_scope, filter, w, 0, &ldap_querytimeout_tp, &result)) != LDAP_SUCCESS) {
          pr_log_pri(PR_LOG_ERR, "mod_ldap: ldap_handle_getgroups(): ldap_search_st() failed: %s", ldap_err2string(ret));
          goto return_groups;
        }
      }
      else {
        pr_log_pri(PR_LOG_ERR, "mod_ldap: ldap_handle_getgroups(): LDAP server went away, unable to reconnect");
        goto return_groups;
      }
    }
    else {
      pr_log_pri(PR_LOG_ERR, "mod_ldap: ldap_handle_getgroups(): ldap_search_st() failed: %s", ldap_err2string(ret));
      goto return_groups;
    }
  }
  pr_ldap_set_sizelimit(ld, 2);

  if (ldap_count_entries(ld, result) == 0)
    goto return_groups;

  for (e = ldap_first_entry(ld, result); e; e = ldap_next_entry(ld, e)) {
    if (! (gidNumber = ldap_get_values(ld, e, w[0]))) {
      pr_log_pri(PR_LOG_ERR, "mod_ldap: ldap_handle_getgroups(): ldap_get_values() on %s attr failed, skipping current group: %s", ldap_err2string(ret), ldap_attr_gidnumber);
      continue;
    }
    if (! (cn = ldap_get_values(ld, e, w[1]))) {
      pr_log_pri(PR_LOG_ERR, "mod_ldap: ldap_handle_getgroups(): ldap_get_values() on %s attr failed, skipping current group: %s", ldap_err2string(ret), ldap_attr_cn);
      continue;
    }

    if (!pw || strtoul(gidNumber[0], (char **)NULL, 10) != pw->pw_gid) {
      *((gid_t *) push_array(gids))   = strtoul(gidNumber[0], (char **)NULL, 10);
      *((char **) push_array(groups)) = pstrdup(session.pool, cn[0]);
    }

    ldap_value_free(gidNumber);
    ldap_value_free(cn);
  }

return_groups:
  if (result)
    ldap_msgfree(result);

  if (gids->nelts > 0)
    return mod_create_data(cmd, (void *) &gids->nelts);
  return DECLINED(cmd);
}


/****************************
 * High-level auth handlers *
 ****************************/

/* cmd->argv[0] : user name
 * cmd->argv[1] : cleartext password
 */

MODRET
handle_ldap_is_auth(cmd_rec *cmd)
{
  const char *username = cmd->argv[0];
  char *pass_attrs[] = {ldap_attr_userpassword, ldap_attr_homedirectory, NULL};

  if (! ldap_doauth)
    return DECLINED(cmd);

  /* If anything here fails hard (IOW, we've found an LDAP entry for the
   * user, but they appear to have entered the wrong password), boot them.
   * Normally, I'd DECLINE here so other modules could have a shot, but if
   * we've found their LDAP entry, chances are that nothing else is going to
   * be able to auth them. If anyone has a reason that this shouldn't be
   * this way, then by all means, let me know.
   */

  /* If we don't have a cached entry, or if the cached entry isn't for this
   * user, fetch the entry.
   */
  if (!pw || (pw && pw->pw_name && strcasecmp(pw->pw_name, username) != 0))
    if ((pw = pr_ldap_user_lookup(cmd->tmp_pool, ldap_auth_filter, username,
                                  pr_ldap_generate_filter(cmd->tmp_pool, ldap_auth_basedn, username),
                                  ldap_authbinds ? pass_attrs + 1 : pass_attrs,
                                  ldap_authbinds ? &ldap_authbind_dn : NULL)) == NULL)
      return DECLINED(cmd); /* Can't find the user in the LDAP directory. */

  if (!ldap_authbinds && !pw->pw_passwd)
    return ERROR_INT(cmd, PR_AUTH_NOPWD);

  /* FIXME: If we pass a "" or NULL "crypted password" argument to
   * auth_check, the mod_auth_unix auth handler gets called before the
   * mod_ldap auth handler, so mod_auth_unix will allow in any LDAP
   * auth-bind user with an incorrect password. Can we kludge around this by
   * setting the directive to not allow empty passwords? (its name escapes
   * me right now) For now, we'll kludge around this by passing "*", which
   * mod_auth_unix will happily deny auth to.
   */
  if (auth_check(cmd->tmp_pool, ldap_authbinds ? "*" : pw->pw_passwd,
                 username, cmd->argv[1]))
  {
    return ERROR_INT(cmd, PR_AUTH_BADPWD);
  }

  return HANDLED(cmd);
}

/* cmd->argv[0] = hashed password,
 * cmd->argv[1] = user,
 * cmd->argv[2] = cleartext
 */

MODRET
handle_ldap_check(cmd_rec *cmd)
{
  char *pass, *cryptpass, *hash_method;
  int encname_len, ret;
  LDAP *ld_auth;

#ifdef HAVE_OPENSSL
  EVP_MD_CTX EVP_Context;
  const EVP_MD *md;
  int md_len;
  unsigned char md_value[EVP_MAX_MD_SIZE];
  EVP_ENCODE_CTX EVP_Encode;
  char buff[EVP_MAX_KEY_LENGTH];
#endif /* HAVE_OPENSSL */

  if (! ldap_doauth)
    return DECLINED(cmd);

  cryptpass = cmd->argv[0];
  pass      = cmd->argv[2];


  if (ldap_authbinds) {
    /* Don't try to do auth binds with a NULL DN or password.
     *
     * We also need to support the UserPassword directive, so don't do auth
     * binds if we received a crypted password, which seems to indicate the
     * use of that directive. See also the comments in pr_ldap_getpwnam().
     *
     * Note that handle_ldap_is_auth() will pass us "*" for a crypted
     * password to prevent mod_auth_unix from successfully authenticating
     * the user with an "empty" password. If we receive "*" for a crypted
     * password, we will still check authentication. This isn't dangerous,
     * since we bail first if we don't have a DN to authbind with.
     */
    if ( (pass == NULL) || (strlen(pass) == 0) ||
         (ldap_authbind_dn == NULL) || (strlen(ldap_authbind_dn) == 0))
    {
      return DECLINED(cmd);
    }
    if (cryptpass != NULL && strlen(cryptpass) > 0 &&
        strcmp(cryptpass, "*") != 0)
    {
      return DECLINED(cmd);
    }

    if (pr_ldap_connect(&ld_auth, FALSE) == -1) {
      pr_log_pri(PR_LOG_ERR, "mod_ldap: handle_ldap_check(): pr_ldap_connect() failed");
      return DECLINED(cmd);
    }

    if ((ret = ldap_simple_bind_s(ld_auth, ldap_authbind_dn, cmd->argv[2])) != LDAP_SUCCESS) {
      if (ret != LDAP_INVALID_CREDENTIALS)
        pr_log_pri(PR_LOG_ERR, "mod_ldap: handle_ldap_check(): pr_ldap_connect() failed: %s", ldap_err2string(ret));
      ldap_unbind(ld_auth);
      return ERROR(cmd);
    }

    ldap_unbind(ld_auth);
    return HANDLED(cmd);
  }

  /* Get the length of "scheme" in the leading {scheme} so we can skip it
   * in the password comparison.
   */
  encname_len = strcspn(cryptpass + 1, "}");
  hash_method = pstrndup(cmd->tmp_pool, cryptpass + 1, encname_len);

  /* Check to see how the password is encrypted, and check accordingly. */

  if (encname_len == strlen(cryptpass + 1)) { /* No leading {scheme} */
    if (ldap_defaultauthscheme && (strcasecmp(ldap_defaultauthscheme, "clear") == 0)) {
      if (strcmp(pass, cryptpass) != 0)
        return ERROR(cmd);
    }
    else { /* else, assume crypt */
      if (strcmp(crypt(pass, cryptpass), cryptpass) != 0)
        return ERROR(cmd);
    }
  }
  else if (strncasecmp(hash_method, "crypt", strlen(hash_method)) == 0) { /* {crypt} */
    if (strcmp(crypt(pass, cryptpass + encname_len + 2), cryptpass + encname_len + 2) != 0)
      return ERROR(cmd);
  }
  else if (strncasecmp(hash_method, "clear", strlen(hash_method)) == 0) { /* {clear} */
    if (strcmp(pass, cryptpass + encname_len + 2) != 0)
      return ERROR(cmd);
  }
#ifdef HAVE_OPENSSL
  else { /* Try the cipher mode found */
    pr_log_debug(DEBUG5, "mod_ldap: %s-encrypted password found, trying to auth.", hash_method);

    SSLeay_add_all_digests();

    /* This is a kludge. This is only a kludge. OpenLDAP likes {sha}
     * (at least, the OpenLDAP ldappasswd generates {sha}), but OpenSSL
     * likes {sha1} and does not understand {sha}. We translate
     * RMD160 -> RIPEMD160 here, too.
     */
    if (strncasecmp(hash_method, "SHA", 4) == 0)
        md = EVP_get_digestbyname("SHA1");
    else if (strncasecmp(hash_method, "RMD160", 7) == 0)
        md = EVP_get_digestbyname("RIPEMD160");
    else
        md = EVP_get_digestbyname(hash_method);

    if (! md) {
      pr_log_debug(DEBUG5, "mod_ldap: %s not supported by OpenSSL, declining auth request", hash_method);
      return DECLINED(cmd); /* Some other module may support it. */
    }

    /* Make a digest of the user-supplied password. */
    EVP_DigestInit(&EVP_Context, md);
    EVP_DigestUpdate(&EVP_Context, pass, strlen(pass));
    EVP_DigestFinal(&EVP_Context, md_value, &md_len);

    /* Base64 Encoding */
    EVP_EncodeInit(&EVP_Encode);
    EVP_EncodeBlock(buff, md_value, md_len);

    if (strcmp(buff, cryptpass + encname_len + 2) != 0)
      return ERROR(cmd);
  }
#else /* HAVE_OPENSSL */
  else /* Can't find a supported {scheme} */
    return DECLINED(cmd);
#endif /* HAVE_OPENSSL */

  return HANDLED(cmd);
}

MODRET
handle_ldap_uid_name(cmd_rec *cmd)
{
  pr_idmap_t *m;
  pr_idauth_t id;

  if (! ldap_douid)
    return DECLINED(cmd);

  id.uid = *((uid_t *) cmd->argv[0]);
  m = _auth_lookup_id(uid_table, id);

  if (! m->name) {
    if (ldap_negcache) /* If we're doing negative caching as per config... */
      if (m->negative) /* It wasn't in the LDAP db before, don't look again. */
        return DECLINED(cmd);

    /* Wasn't cached and we've haven't seen this one, so perform a lookup.
     * If we don't have a cached entry, or if the cached entry isn't for
     * this user, fetch the entry.
     */
    if (!pw || (pw && pw->pw_uid != id.uid)) {
      if (! (pw = pr_ldap_getpwuid(cmd->tmp_pool, id.uid))) {
        if (ldap_negcache)
          m->negative = 1;
        return DECLINED(cmd); /* Can't find the user in the LDAP directory. */
      }
    }

    m->name = pstrdup(permanent_pool, pw->pw_name);
  }

  return mod_create_data(cmd, m->name);
}

MODRET
handle_ldap_gid_name(cmd_rec *cmd)
{
  pr_idmap_t *m;
  pr_idauth_t id;

  if (! ldap_dogid)
    return DECLINED(cmd);

  id.gid = *((gid_t *) cmd->argv[0]);
  m = _auth_lookup_id(gid_table, id);

  if (! m->name) {
    if (ldap_negcache) /* If we're doing negative caching as per config... */
      if (m->negative) /* It wasn't in the LDAP db before, don't look again. */
        return DECLINED(cmd);

    /* Wasn't cached and we've haven't seen this one, so perform a lookup.
     * If we don't have a cached entry, or if the cached entry isn't for
     * this group, fetch the entry.
     */
    if (!gr || (gr && gr->gr_gid != id.gid)) {
      if (! (gr = pr_ldap_getgrgid(cmd->tmp_pool, id.gid))) {
        if (ldap_negcache)
          m->negative = 1;
        return DECLINED(cmd); /* Can't find the user in the LDAP directory. */
      }
    }

    m->name = pstrdup(permanent_pool, gr->gr_name);
  }

  return mod_create_data(cmd, m->name);
}

MODRET
handle_ldap_name_uid(cmd_rec *cmd)
{
  if (! ldap_doauth)
    return DECLINED(cmd);

  if (pw && pw->pw_name && strcasecmp(pw->pw_name, cmd->argv[0]) == 0)
    return mod_create_data(cmd, (void *) &pw->pw_uid);

  if ((pw = pr_ldap_getpwnam(cmd->tmp_pool, cmd->argv[0])))
    return mod_create_data(cmd, (void *) &pw->pw_uid);

  return DECLINED(cmd);
}

MODRET
handle_ldap_name_gid(cmd_rec *cmd)
{
  if (! ldap_dogid)
    return DECLINED(cmd);

  if (gr && strcasecmp(gr->gr_name, cmd->argv[0]) == 0)
    return mod_create_data(cmd, (void *) &gr->gr_gid);

  if ((gr = pr_ldap_getgrnam(cmd->tmp_pool, cmd->argv[0])))
    return mod_create_data(cmd, (void *) &gr->gr_gid);

  return DECLINED(cmd);
}


/*****************************************
 * Config-file handlers/parsing routines *
 *****************************************/

MODRET
set_ldap_server(cmd_rec *cmd)
{
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str("LDAPServer", 1, cmd->argv[1]);
  return HANDLED(cmd);
}

MODRET
set_ldap_dninfo(cmd_rec *cmd)
{
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str("LDAPDNInfo", 2, cmd->argv[1], cmd->argv[2]);
  return HANDLED(cmd);
}

MODRET
set_ldap_authbinds(cmd_rec *cmd)
{
  int b;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1)
    CONF_ERROR(cmd, "LDAPAuthBinds: expected a boolean value for first argument.");

  add_config_param("LDAPAuthBinds", 1, (void *)b);
  return HANDLED(cmd);
}

MODRET
set_ldap_querytimeout(cmd_rec *cmd)
{
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param("LDAPQueryTimeout", 1, atoi(cmd->argv[1]));
  return HANDLED(cmd);
}

MODRET
set_ldap_searchscope(cmd_rec *cmd)
{
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str("LDAPSearchScope", 1, cmd->argv[1]);
  return HANDLED(cmd);
}

MODRET
set_ldap_doauth(cmd_rec *cmd)
{
  int b;
  config_rec *c;

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1)
    CONF_ERROR(cmd, "LDAPDoAuth: expected a boolean value for first argument.");

  if (b == 1) { CHECK_ARGS(cmd, 2); }
  else        { CHECK_ARGS(cmd, 1); }

  c = add_config_param("LDAPDoAuth", 3, (void *)b);
  c->argv[1] = pstrdup(c->pool, cmd->argv[2]);
  c->argv[2] = pstrdup(c->pool, cmd->argv[3]);

  return HANDLED(cmd);
}

MODRET
set_ldap_douid(cmd_rec *cmd)
{
  int b;
  config_rec *c;

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1)
    CONF_ERROR(cmd, "LDAPDoUIDLookups: expected a boolean value for first argument.");

  if (b == 1) { CHECK_ARGS(cmd, 2); }
  else        { CHECK_ARGS(cmd, 1); }

  c = add_config_param("LDAPDoUIDLookups", 3, (void *)b);
  c->argv[1] = pstrdup(c->pool, cmd->argv[2]);
  c->argv[2] = pstrdup(c->pool, cmd->argv[3]);

  return HANDLED(cmd);
}

MODRET
set_ldap_dogid(cmd_rec *cmd)
{
  int b;
  config_rec *c;

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1)
    CONF_ERROR(cmd, "LDAPDoGIDLookups: expected a boolean value for first argument.");

  if (b == 1) { CHECK_ARGS(cmd, 2); }
  else        { CHECK_ARGS(cmd, 1); }

  c = add_config_param("LDAPDoGIDLookups", cmd->argc - 1, (void *)b);
  if (cmd->argc > 2)
    c->argv[1] = pstrdup(c->pool, cmd->argv[2]);
  if (cmd->argc > 3)
    c->argv[2] = pstrdup(c->pool, cmd->argv[3]);
  if (cmd->argc > 4)
    c->argv[3] = pstrdup(c->pool, cmd->argv[4]);
  if (cmd->argc > 5)
    c->argv[4] = pstrdup(c->pool, cmd->argv[5]);

  return HANDLED(cmd);
}

MODRET
set_ldap_doquota(cmd_rec *cmd)
{
  int b;
  config_rec *c;

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1)
    CONF_ERROR(cmd, "LDAPDoQuotaLookups: expected a boolean value for first argument.");

  if (b == 1) { CHECK_ARGS(cmd, 2); }
  else        { CHECK_ARGS(cmd, 1); }

  c = add_config_param("LDAPDoQuotaLookups", cmd->argc - 1, (void *)b);
  if (cmd->argc > 2)
    c->argv[1] = pstrdup(c->pool, cmd->argv[2]);
  if (cmd->argc > 3)
    c->argv[2] = pstrdup(c->pool, cmd->argv[3]);
  if (cmd->argc > 4)
    c->argv[3] = pstrdup(c->pool, cmd->argv[4]);

  return HANDLED(cmd);
}

MODRET
set_ldap_defaultuid(cmd_rec *cmd)
{
  int i = 0;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  while (cmd->argv[1][i]) {
    if (! isdigit((int) cmd->argv[1][i]))
      CONF_ERROR(cmd, "LDAPDefaultUID: UID argument must be numeric!");
    ++i;
  }

  add_config_param("LDAPDefaultUID", 1, strtoul(cmd->argv[1], (char **)NULL, 10));
  return HANDLED(cmd);
}

MODRET
set_ldap_defaultgid(cmd_rec *cmd)
{
  int i = 0;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  while (cmd->argv[1][i]) {
    if (! isdigit((int) cmd->argv[1][i]))
      CONF_ERROR(cmd, "LDAPDefaultGID: GID argument must be numeric!");
    ++i;
  }

  add_config_param("LDAPDefaultGID", 1, strtoul(cmd->argv[1], (char **)NULL, 10));
  return HANDLED(cmd);
}

MODRET set_ldap_forcedefaultuid(cmd_rec *cmd)
{
  int b;

  CHECK_CONF(cmd,CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1)
    CONF_ERROR(cmd, "LDAPForceDefaultUID: expected boolean argument for first argument.");

  add_config_param("LDAPForceDefaultUID", 1, (void *)b);
  return HANDLED(cmd);
}

MODRET set_ldap_forcedefaultgid(cmd_rec *cmd)
{
  int b;

  CHECK_CONF(cmd,CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1)
    CONF_ERROR(cmd, "LDAPForceDefaultGID: expected boolean argument for first argument.");

  add_config_param("LDAPForceDefaultGID", 1, (void *)b);
  return HANDLED(cmd);
}

MODRET
set_ldap_negcache(cmd_rec *cmd)
{
  int b;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1)
    CONF_ERROR(cmd, "LDAPNegativeCache: expected a boolean value for first argument.");

  add_config_param("LDAPNegativeCache", 1, (void *)b);
  return HANDLED(cmd);
}

MODRET
set_ldap_genhdir(cmd_rec *cmd)
{
  int b;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1)
    CONF_ERROR(cmd, "LDAPGenerateHomedir: expected a boolean value for first argument.");

  c = add_config_param("LDAPGenerateHomedir", 1, (void *)b);
  return HANDLED(cmd);

}

MODRET set_ldap_forcegenhdir(cmd_rec *cmd)
{
  int b;

  CHECK_CONF(cmd,CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1)
    CONF_ERROR(cmd, "LDAPForceGeneratedHomedir: expected boolean argument for first argument.");

  add_config_param("LDAPForceGeneratedHomedir", 1, (void *)b);
  return HANDLED(cmd);
}

MODRET
set_ldap_genhdirprefix(cmd_rec *cmd)
{
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str("LDAPGenerateHomedirPrefix", 1, cmd->argv[1]);
  return HANDLED(cmd);
}

MODRET
set_ldap_genhdirprefixnouname(cmd_rec *cmd)
{
  int b;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1)
    CONF_ERROR(cmd, "LDAPGenerateHomedirPrefixNoUsername: expected a boolean value for first argument.");

  add_config_param("LDAPGenerateHomedirPrefixNoUsername", 1, (void *)b);
  return HANDLED(cmd);
}

MODRET
set_ldap_defaultauthscheme(cmd_rec *cmd)
{
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str("LDAPDefaultAuthScheme", 1, cmd->argv[1]);
  return HANDLED(cmd);
}

MODRET
set_ldap_usetls(cmd_rec *cmd)
{
#ifndef USE_LDAP_TLS
  CONF_ERROR(cmd, "LDAPUseTLS: You must edit mod_ldap.c and recompile with USE_LDAP_TLS enabled in order to use TLS.");
#else /* USE_LDAP_TLS */
  int b;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if ((b = get_boolean(cmd, 1)) == -1)
    CONF_ERROR(cmd, "LDAPUseTLS: expected a boolean value for first argument.");

  add_config_param("LDAPUseTLS", 1, (void *)b);
  return HANDLED(cmd);
#endif /* USE_LDAP_TLS */
}

MODRET
set_ldap_protoversion(cmd_rec *cmd)
{
  int i = 0;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  while (cmd->argv[1][i]) {
    if (! isdigit((int) cmd->argv[1][i]))
      CONF_ERROR(cmd, "LDAPProtocolVersion: argument must be numeric!");
    ++i;
  }

  add_config_param("LDAPProtocolVersion", 1, strtoul(cmd->argv[1], (char **)NULL, 10));
  return HANDLED(cmd);
}

MODRET
set_ldap_attr(cmd_rec *cmd)
{
  CHECK_ARGS(cmd, 2);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (strcasecmp(cmd->argv[1], "uid") != 0 &&
      strcasecmp(cmd->argv[1], "uidNumber") != 0 &&
      strcasecmp(cmd->argv[1], "gidNumber") != 0 &&
      strcasecmp(cmd->argv[1], "homeDirectory") != 0 &&
      strcasecmp(cmd->argv[1], "userPassword") != 0 &&
      strcasecmp(cmd->argv[1], "loginShell") != 0 &&
      strcasecmp(cmd->argv[1], "cn") != 0 &&
      strcasecmp(cmd->argv[1], "memberUid") != 0 &&
      strcasecmp(cmd->argv[1], "ftpQuota") != 0)
  {
    CONF_ERROR(cmd, "LDAPAttr: unknown attribute name.");
  }

  add_config_param_str("LDAPAttr", 2, cmd->argv[1], cmd->argv[2]);
  return HANDLED(cmd);
}

static int
ldap_getconf(void)
{
  char *scope;
  config_rec *c;

  /* If ldap_server is NULL, ldap_init() will connect to your LDAP SDK's
   * default.
   */
  ldap_server = (char *)get_param_ptr(main_server->conf, "LDAPServer", FALSE);

  if ((c = find_config(main_server->conf, CONF_PARAM, "LDAPDNInfo", FALSE)) != NULL) {
    ldap_dn = pstrdup(session.pool, c->argv[0]);
    ldap_dnpass = pstrdup(session.pool, c->argv[1]);
  }

  if (get_param_int(main_server->conf, "LDAPAuthBinds", FALSE) == 0)
    ldap_authbinds = 0;

  ldap_querytimeout = get_param_int(main_server->conf, "LDAPQueryTimeout", FALSE);

  scope = get_param_ptr(main_server->conf, "LDAPSearchScope", FALSE);
  if (scope && *scope)
    if (strcasecmp(scope, "onelevel") == 0)
      ldap_search_scope = LDAP_SCOPE_ONELEVEL;

  if ((c = find_config(main_server->conf, CONF_PARAM, "LDAPDoAuth", FALSE)) != NULL) {
    if ( (int)c->argv[0] > 0) {
      ldap_doauth = 1;
      ldap_auth_basedn = pstrdup(session.pool, c->argv[1]);

      if (c->argv[2])
        ldap_auth_filter = pstrdup(session.pool, c->argv[2]);
      else
        ldap_auth_filter = pstrcat(session.pool, "(&(", ldap_attr_uid, "=%v)(objectclass=posixAccount))", NULL);
    }
  }

  if ((c = find_config(main_server->conf, CONF_PARAM, "LDAPDoUIDLookups", FALSE)) != NULL) {
    if ( (int)c->argv[0] > 0) {
      ldap_douid = 1;
      ldap_uid_basedn = pstrdup(session.pool, c->argv[1]);

      if (c->argv[2])
        ldap_uid_filter = pstrdup(session.pool, c->argv[2]);
      else
        ldap_uid_filter = pstrcat(session.pool, "(&(", ldap_attr_uidnumber, "=%v)(objectclass=posixAccount))", NULL);
    }
  }

  if ((c = find_config(main_server->conf, CONF_PARAM, "LDAPDoGIDLookups", FALSE)) != NULL) {
    if ( (int)c->argv[0] > 0) {
      ldap_dogid = 1;
      ldap_gid_basedn = pstrdup(session.pool, c->argv[1]);

      if (c->argc > 2)
        ldap_group_name_filter = pstrdup(session.pool, c->argv[2]);
      else
        ldap_group_name_filter = pstrcat(session.pool, "(&(", ldap_attr_cn, "=%v)(objectclass=posixGroup))", NULL);

      if (c->argc > 3)
        ldap_group_gid_filter = pstrdup(session.pool, c->argv[3]);
      else
        ldap_group_gid_filter = pstrcat(session.pool, "(&(", ldap_attr_gidnumber, "=%v)(objectclass=posixGroup))", NULL);

      if (c->argc > 4)
        ldap_group_member_filter = pstrdup(session.pool, c->argv[4]);
      else
        ldap_group_member_filter = pstrcat(session.pool, "(&(", ldap_attr_memberuid, "=%v)(objectclass=posixGroup))", NULL);
    }
  }

  if ((c = find_config(main_server->conf, CONF_PARAM, "LDAPDoQuotaLookups", FALSE)) != NULL) {
    if ( (int)c->argv[0] > 0) {
      ldap_doquota = 1;
      ldap_quota_basedn = pstrdup(session.pool, c->argv[1]);

      if (c->argc > 2)
        ldap_quota_filter = pstrdup(session.pool, c->argv[2]);
      else
        ldap_quota_filter = pstrcat(session.pool, "(&(", ldap_attr_uid, "=%v)(objectclass=posixAccount))", NULL);

      if (c->argc > 3)
        ldap_default_quota = pstrdup(session.pool, c->argv[3]);
    }
  }

  ldap_defaultuid = get_param_int(main_server->conf, "LDAPDefaultUID", FALSE);
  ldap_defaultgid = get_param_int(main_server->conf, "LDAPDefaultGID", FALSE);

  if ((c = find_config(main_server->conf, CONF_PARAM, "LDAPForceDefaultUID", FALSE)) != NULL)
    if ( (int)c->argv[0] > 0)
      ldap_forcedefaultuid = 1;

  if ((c = find_config(main_server->conf, CONF_PARAM, "LDAPForceDefaultGID", FALSE)) != NULL)
    if ( (int)c->argv[0] > 0)
      ldap_forcedefaultgid = 1;

  if ((c = find_config(main_server->conf, CONF_PARAM, "LDAPForceGeneratedHomedir", FALSE)) != NULL)
    if ( (int)c->argv[0] > 0)
      ldap_forcegenhdir = 1;

  if (get_param_int(main_server->conf, "LDAPNegativeCache", FALSE) > 0)
    ldap_negcache = 1;

  if ((c = find_config(main_server->conf, CONF_PARAM, "LDAPGenerateHomedir", FALSE)) != NULL)
    if ( (int)c->argv[0] > 0)
      ldap_genhdir = 1;

  ldap_genhdir_prefix = (char *)get_param_ptr(main_server->conf, "LDAPGenerateHomedirPrefix", FALSE);
  if (get_param_int(main_server->conf, "LDAPGenerateHomedirPrefixNoUsername", FALSE) == 1)
    ldap_genhdir_prefix_nouname = 1;

  /* If ldap_defaultauthscheme is NULL, ldap_check() will assume crypt. */
  ldap_defaultauthscheme = (char *)get_param_ptr(main_server->conf, "LDAPDefaultAuthScheme", FALSE);

  ldap_protocol_version = (int)get_param_int(main_server->conf, "LDAPProtocolVersion", TRUE);
#ifdef USE_LDAP_TLS
  ldap_use_tls = (int)get_param_int(main_server->conf, "LDAPUseTLS", FALSE);
#endif

  if ((c = find_config(main_server->conf, CONF_PARAM, "LDAPAttr", FALSE)) != NULL) {
    do {
      if (strcasecmp(c->argv[0], "uid") == 0)
        ldap_attr_uid = pstrdup(session.pool, c->argv[1]);
      else if (strcasecmp(c->argv[0], "uidNumber") == 0)
        ldap_attr_uidnumber = pstrdup(session.pool, c->argv[1]);
      else if (strcasecmp(c->argv[0], "gidNumber") == 0)
        ldap_attr_gidnumber = pstrdup(session.pool, c->argv[1]);
      else if (strcasecmp(c->argv[0], "homeDirectory") == 0)
        ldap_attr_homedirectory = pstrdup(session.pool, c->argv[1]);
      else if (strcasecmp(c->argv[0], "userPassword") == 0)
        ldap_attr_userpassword = pstrdup(session.pool, c->argv[1]);
      else if (strcasecmp(c->argv[0], "loginShell") == 0)
        ldap_attr_loginshell = pstrdup(session.pool, c->argv[1]);
      else if (strcasecmp(c->argv[0], "cn") == 0)
        ldap_attr_cn = pstrdup(session.pool, c->argv[1]);
      else if (strcasecmp(c->argv[0], "memberUid") == 0)
        ldap_attr_memberuid = pstrdup(session.pool, c->argv[1]);
      else if (strcasecmp(c->argv[0], "ftpQuota") == 0)
        ldap_attr_ftpquota = pstrdup(session.pool, c->argv[1]);
    } while ((c = find_config_next(c, c->next, CONF_PARAM, "LDAPAttr", FALSE)));
  }

  return 0;
}

static conftable ldap_config[] = {
  { "LDAPServer",                          set_ldap_server,               NULL },
  { "LDAPDNInfo",                          set_ldap_dninfo,               NULL },
  { "LDAPAuthBinds",                       set_ldap_authbinds,            NULL },
  { "LDAPQueryTimeout",                    set_ldap_querytimeout,         NULL },
  { "LDAPSearchScope",                     set_ldap_searchscope,          NULL },
  { "LDAPNegativeCache",                   set_ldap_negcache,             NULL },
  { "LDAPDoAuth",                          set_ldap_doauth,               NULL },
  { "LDAPDoUIDLookups",                    set_ldap_douid,                NULL },
  { "LDAPDoGIDLookups",                    set_ldap_dogid,                NULL },
  { "LDAPDoQuotaLookups",                  set_ldap_doquota,              NULL },
  { "LDAPDefaultUID",                      set_ldap_defaultuid,           NULL },
  { "LDAPDefaultGID",                      set_ldap_defaultgid,           NULL },
  { "LDAPForceDefaultUID",                 set_ldap_forcedefaultuid,      NULL },
  { "LDAPForceDefaultGID",                 set_ldap_forcedefaultgid,      NULL },
  { "LDAPGenerateHomedir",                 set_ldap_genhdir,              NULL },
  { "LDAPGenerateHomedirPrefix",           set_ldap_genhdirprefix,        NULL },
  { "LDAPGenerateHomedirPrefixNoUsername", set_ldap_genhdirprefixnouname, NULL },
  { "LDAPForceGeneratedHomedir",           set_ldap_forcegenhdir,         NULL },
  { "LDAPDefaultAuthScheme",               set_ldap_defaultauthscheme,    NULL },
  { "LDAPUseTLS",                          set_ldap_usetls,               NULL },
  { "LDAPProtocolVersion",                 set_ldap_protoversion,         NULL },
  { "LDAPAttr",                            set_ldap_attr,                 NULL },
  { NULL,                                  NULL,                          NULL }
};

static cmdtable ldap_cmdtab[] = {
  {HOOK, "ldap_quota_lookup", G_NONE, handle_ldap_quota_lookup, FALSE, FALSE},
  {0, NULL}
};

static authtable ldap_auth[] = {
  { 0, "setpwent",  handle_ldap_setpwent  },
  { 0, "endpwent",  handle_ldap_endpwent  },
  { 0, "setgrent",  handle_ldap_setpwent  },
  { 0, "endgrent",  handle_ldap_endpwent  },
  { 0, "getpwnam",  handle_ldap_getpwnam  },
  { 0, "getpwuid",  handle_ldap_getpwuid  },
  { 0, "getgrnam",  handle_ldap_getgrnam  },
  { 0, "getgrgid",  handle_ldap_getgrgid  },
  { 0, "auth",      handle_ldap_is_auth   },
  { 0, "check",     handle_ldap_check     },
  { 0, "uid2name",  handle_ldap_uid_name  },
  { 0, "gid2name",  handle_ldap_gid_name  },
  { 0, "name2uid",  handle_ldap_name_uid  },
  { 0, "name2gid",  handle_ldap_name_gid  },
  { 0, "getgroups", handle_ldap_getgroups },
  { 0, NULL }
};

module ldap_module = {
  NULL, NULL,                        /* Always NULL */
  0x20,                              /* API Version 2.0 */
  "ldap",
  ldap_config,                       /* Configuration directive table */
  ldap_cmdtab,                       /* Command handlers */
  ldap_auth,                         /* Authentication handlers */
  pr_ldap_module_init, ldap_getconf, /* Initialization functions */
  MOD_LDAP_VERSION
};
