/* Copyright (C) 2000 MySQL AB & MySQL Finland AB & TCX DataKonsult AB

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
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */


/* YACC and LEX Definitions */

/* These may not be declared yet */
class Table_ident;
class sql_exchange;
class LEX_COLUMN;

/*
  The following hack is needed because mysql_yacc.cc does not define
  YYSTYPE before including this file
*/

#include "set_var.h"

#ifdef MYSQL_YACC
#define LEX_YYSTYPE void *
#else
#include "lex_symbol.h"
#include "sql_yacc.h"
#define LEX_YYSTYPE YYSTYPE *
#endif

enum enum_sql_command {
  SQLCOM_SELECT, SQLCOM_CREATE_TABLE, SQLCOM_CREATE_INDEX, SQLCOM_ALTER_TABLE,
  SQLCOM_UPDATE, SQLCOM_INSERT, SQLCOM_INSERT_SELECT,
  SQLCOM_DELETE, SQLCOM_TRUNCATE, SQLCOM_DROP_TABLE, SQLCOM_DROP_INDEX,

  SQLCOM_SHOW_DATABASES, SQLCOM_SHOW_TABLES, SQLCOM_SHOW_FIELDS,
  SQLCOM_SHOW_KEYS, SQLCOM_SHOW_VARIABLES, SQLCOM_SHOW_LOGS, SQLCOM_SHOW_STATUS,
  SQLCOM_SHOW_INNODB_STATUS,
  SQLCOM_SHOW_PROCESSLIST, SQLCOM_SHOW_MASTER_STAT, SQLCOM_SHOW_SLAVE_STAT,
  SQLCOM_SHOW_GRANTS, SQLCOM_SHOW_CREATE,

  SQLCOM_LOAD,SQLCOM_SET_OPTION,SQLCOM_LOCK_TABLES,SQLCOM_UNLOCK_TABLES,
  SQLCOM_GRANT, SQLCOM_CHANGE_DB, SQLCOM_CREATE_DB, SQLCOM_DROP_DB,
  SQLCOM_REPAIR, SQLCOM_REPLACE, SQLCOM_REPLACE_SELECT, 
  SQLCOM_CREATE_FUNCTION, SQLCOM_DROP_FUNCTION,
  SQLCOM_REVOKE,SQLCOM_OPTIMIZE, SQLCOM_CHECK,
  SQLCOM_FLUSH, SQLCOM_KILL, SQLCOM_ANALYZE,
  SQLCOM_ROLLBACK, SQLCOM_ROLLBACK_TO_SAVEPOINT,
  SQLCOM_COMMIT, SQLCOM_SAVEPOINT,
  SQLCOM_SLAVE_START, SQLCOM_SLAVE_STOP,
  SQLCOM_BEGIN, SQLCOM_LOAD_MASTER_TABLE, SQLCOM_CHANGE_MASTER,
  SQLCOM_RENAME_TABLE, SQLCOM_BACKUP_TABLE, SQLCOM_RESTORE_TABLE,
  SQLCOM_RESET, SQLCOM_PURGE, SQLCOM_SHOW_BINLOGS,
  SQLCOM_SHOW_OPEN_TABLES, SQLCOM_LOAD_MASTER_DATA,
  SQLCOM_HA_OPEN, SQLCOM_HA_CLOSE, SQLCOM_HA_READ,
  SQLCOM_SHOW_SLAVE_HOSTS, SQLCOM_DELETE_MULTI, SQLCOM_MULTI_UPDATE,
  SQLCOM_SHOW_BINLOG_EVENTS, SQLCOM_SHOW_NEW_MASTER, SQLCOM_DO,
  SQLCOM_EMPTY_QUERY,
  SQLCOM_END
};

enum lex_states
{
  STATE_START, STATE_CHAR, STATE_IDENT, STATE_IDENT_SEP, STATE_IDENT_START,
  STATE_FOUND_IDENT, STATE_SIGNED_NUMBER, STATE_REAL, STATE_HEX_NUMBER,
  STATE_CMP_OP, STATE_LONG_CMP_OP, STATE_STRING, STATE_COMMENT, STATE_END,
  STATE_OPERATOR_OR_IDENT, STATE_NUMBER_IDENT, STATE_INT_OR_REAL,
  STATE_REAL_OR_POINT, STATE_BOOL, STATE_EOL, STATE_ESCAPE, STATE_LONG_COMMENT,
  STATE_END_LONG_COMMENT, STATE_COLON, STATE_SET_VAR, STATE_USER_END,
  STATE_HOSTNAME, STATE_SKIP, STATE_USER_VARIABLE_DELIMITER, STATE_SYSTEM_VAR,
  STATE_IDENT_OR_KEYWORD
};

typedef List<Item> List_item;

typedef struct st_lex_master_info
{
  char *host, *user, *password, *log_file_name;
  uint port, connect_retry;
  ulonglong pos;
  ulong server_id;
  char *relay_log_name;
  ulong relay_log_pos;
} LEX_MASTER_INFO;


enum sub_select_type
{
  UNSPECIFIED_TYPE, UNION_TYPE, INTERSECT_TYPE, EXCEPT_TYPE, OLAP_TYPE, NOT_A_SELECT
};

enum olap_type 
{
  UNSPECIFIED_OLAP_TYPE, CUBE_TYPE, ROLLUP_TYPE
};

/* The state of the lex parsing for selects */

typedef struct st_select_lex
{
  enum sub_select_type linkage;
  enum olap_type olap;
  char *db,*db1,*table1,*db2,*table2;		/* For outer join using .. */
  Item *where,*having;
  ha_rows select_limit,offset_limit;
  ulong options, table_join_options;
  List<List_item>     expr_list;
  List<List_item>     when_list;
  SQL_LIST	      order_list,table_list,group_list;
  List<Item>          item_list;
  List<String>        interval_list,use_index, *use_index_ptr,
		      ignore_index, *ignore_index_ptr;
  List<Item_func_match> ftfunc_list;
  uint in_sum_expr, sort_default;
  bool	create_refs, braces;
  st_select_lex *next;
} SELECT_LEX;


/* The state of the lex parsing. This is saved in the THD struct */

typedef struct st_lex
{
  uint	 yylineno,yytoklen;			/* Simulate lex */
  LEX_YYSTYPE yylval;
  SELECT_LEX select_lex, *select, *last_selects;
  uchar *ptr,*tok_start,*tok_end,*end_of_query;
  char *length,*dec,*change,*name;
  char *backup_dir;				/* For RESTORE/BACKUP */
  char* to_log;                                 /* For PURGE MASTER LOGS TO */
  char* x509_subject,*x509_issuer,*ssl_cipher;
  enum SSL_type ssl_type;			/* defined in violite.h */
  String *wild;
  sql_exchange *exchange;

  List<key_part_spec> col_list;
  List<Alter_drop>    drop_list;
  List<Alter_column>  alter_list;
  List<String>	      interval_list;
  List<LEX_USER>      users_list;
  List<LEX_COLUMN>    columns;
  List<Key>	      key_list;
  List<create_field>  create_list;
  List<Item>	      *insert_list,field_list,value_list;
  List<List_item>     many_values;
  List<set_var_base>  var_list;
  SQL_LIST	      proc_list, auxilliary_table_list, save_list;
  TYPELIB	      *interval;
  create_field	      *last_field;
  char*		      savepoint_name;		// Transaction savepoint id
  Item *default_value;
  CONVERT *convert_set;
  CONVERT *thd_convert_set;			// Set with SET CHAR SET
  LEX_USER *grant_user;
  gptr yacc_yyss,yacc_yyvs;
  THD *thd;
  udf_func udf;
  HA_CHECK_OPT   check_opt;			// check/repair options
  HA_CREATE_INFO create_info;
  LEX_MASTER_INFO mi;				// used by CHANGE MASTER
  USER_RESOURCES mqh;
  ulong thread_id,type;
  enum_sql_command sql_command;
  enum lex_states next_state;
  enum enum_duplicates duplicates;
  enum enum_tx_isolation tx_isolation;
  enum enum_ha_read_modes ha_read_mode;
  enum ha_rkey_function ha_rkey_mode;
  enum enum_enable_or_disable alter_keys_onoff;
  enum enum_var_type option_type;
  uint grant,grant_tot_col,which_columns, union_option;
  thr_lock_type lock_option;
  bool	drop_primary, drop_if_exists, drop_temporary, local_file, olap;
  bool  in_comment,ignore_space,verbose,simple_alter;
  uint slave_thd_opt;
} LEX;


void lex_init(void);
void lex_free(void);
LEX *lex_start(THD *thd, uchar *buf,uint length);
void lex_end(LEX *lex);

extern pthread_key(LEX*,THR_LEX);

extern LEX_STRING tmp_table_alias;

#define current_lex (&current_thd->lex)
