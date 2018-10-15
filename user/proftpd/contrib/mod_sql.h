/*
 * ProFTPD: mod_sql.h -- header file for mod_sql and backends
 * Time-stamp: <1999-10-04 03:21:21 root>
 * Copyright (c) 1998-1999 Johnie Ingram.
 * Copyright (c) 2001 Andrew Houghton
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
 * As a special exemption, Andrew Houghton and other respective copyright
 * holders give permission to link this program with OpenSSL, and distribute
 * the resulting executable, without including the source code for OpenSSL in
 * the source distribution.
 *
 * $Id: mod_sql.h,v 1.6 2004/09/26 18:09:11 castaglia Exp $
 */

#ifndef MOD_SQL_H
#define MOD_SQL_H

/* mod_sql helper functions */
int sql_log(int, const char *, ...);
cmd_rec *_sql_make_cmd(pool * cp, int argc, ...);
int sql_register_backend(const char *, cmdtable *);
int sql_unregister_backend(const char *);

/* data passing structure */
struct sql_data_struct {
  unsigned long rnum;     /* number of rows of data    */
  unsigned long fnum;     /* number of fields per row  */
  char **data;            /* data[][]                  */
};

typedef struct sql_data_struct sql_data_t;

/* on the assumption that logging will turn into a bitmask later */
#define DEBUG_FUNC DEBUG5
#define DEBUG_AUTH DEBUG4
#define DEBUG_INFO DEBUG3
#define DEBUG_WARN DEBUG2

#define SQL_FREE_CMD(c)       destroy_pool((c)->pool)

/* 
 * These macros are for backends to create basic internal error messages
 */

#define PR_ERR_SQL_REDEF(cmd)        mod_create_ret((cmd), 1, _MOD_VERSION, \
                                     "named connection already exists")
#define PR_ERR_SQL_UNDEF(cmd)        mod_create_ret((cmd), 1, _MOD_VERSION, \
                                     "unknown named connection")
#define PR_ERR_SQL_UNKNOWN(cmd)      mod_create_ret((cmd), 1, _MOD_VERSION, \
                                     "unknown backend error")
#define PR_ERR_SQL_BADCMD(cmd)       mod_create_ret((cmd), 1, _MOD_VERSION, \
                                     "badly formed request")

/* API versions */

/* MOD_SQL_API_V2: guarantees to correctly implement cmd_open, cmd_close,
 *  cmd_defineconnection, cmd_select, cmd_insert, cmd_update, cmd_escapestring,
 *  cmd_query, cmd_checkauth, and cmd_identify.  Also guarantees to
 *  perform proper registration of the cmdtable.
 */

#define MOD_SQL_API_V1 "mod_sql_api_v2"

/* MOD_SQL_API_V2: MOD_SQL_API_V1 && guarantees to correctly implement 
 *  cmd_procedure.
 */

#define MOD_SQL_API_V2 "mod_sql_api_v2"
                
#endif /* MOD_SQL_H */


