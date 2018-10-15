/* Copyright (C) 2000-2003 MySQL AB

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

/* sql_yacc.yy */

%{
#define MYSQL_YACC
#define YYINITDEPTH 100
#define YYMAXDEPTH 3200				/* Because of 64K stack */
#define Lex current_lex
#define Select Lex->select
#include "mysql_priv.h"
#include "slave.h"
#include "sql_acl.h"
#include "lex_symbol.h"
#include <myisam.h>
#include <myisammrg.h>

extern void yyerror(const char*);
int yylex(void *yylval);

#define yyoverflow(A,B,C,D,E,F) if (my_yyoverflow((B),(D),(int*) (F))) { yyerror((char*) (A)); return 2; }

inline Item *or_or_concat(Item* A, Item* B)
{
  return (current_thd->sql_mode & MODE_PIPES_AS_CONCAT ?
          (Item*) new Item_func_concat(A,B) : (Item*) new Item_cond_or(A,B));
}

%}
%union {
  int  num;
  ulong ulong_num;
  ulonglong ulonglong_number;
  LEX_STRING lex_str;
  LEX_STRING *lex_str_ptr;
  LEX_SYMBOL symbol;
  Table_ident *table;
  char *simple_string;
  Item *item;
  List<Item> *item_list;
  List<String> *string_list;
  String *string;
  key_part_spec *key_part;
  TABLE_LIST *table_list;
  udf_func *udf;
  LEX_USER *lex_user;
  sys_var *variable;
  Key::Keytype key_type;
  enum db_type db_type;
  enum row_type row_type;
  enum ha_rkey_function ha_rkey_mode;
  enum enum_tx_isolation tx_isolation;
  enum Item_cast cast_type;
  enum Item_udftype udf_type;
  thr_lock_type lock_type;
  interval_type interval;
}

%{
bool my_yyoverflow(short **a, YYSTYPE **b,int *yystacksize);
%}

%pure_parser					/* We have threads */

%token	END_OF_INPUT

%token CLOSE_SYM
%token HANDLER_SYM
%token LAST_SYM
%token NEXT_SYM
%token PREV_SYM

%token	EQ
%token	EQUAL_SYM
%token	GE
%token	GT_SYM
%token	LE
%token	LT
%token	NE
%token	IS
%token	SHIFT_LEFT
%token	SHIFT_RIGHT
%token  SET_VAR

%token	ABORT_SYM
%token	ADD
%token	AFTER_SYM
%token	ALTER
%token	ANALYZE_SYM
%token	AVG_SYM
%token	BEGIN_SYM
%token	BINLOG_SYM
%token	CHANGE
%token	CLIENT_SYM
%token	COMMENT_SYM
%token	COMMIT_SYM
%token	COUNT_SYM
%token	CREATE
%token	CROSS
%token  CUBE_SYM
%token	DELETE_SYM
%token	DO_SYM
%token	DROP
%token	EVENTS_SYM
%token	EXECUTE_SYM
%token	FLUSH_SYM
%token	INSERT
%token	IO_THREAD
%token	KILL_SYM
%token	LOAD
%token	LOCKS_SYM
%token	LOCK_SYM
%token	MASTER_SYM
%token	MAX_SYM
%token	MIN_SYM
%token	NONE_SYM
%token	OPTIMIZE
%token	PURGE
%token	REPAIR
%token	REPLICATION
%token	RESET_SYM
%token	ROLLBACK_SYM
%token  ROLLUP_SYM
%token	SAVEPOINT_SYM
%token	SELECT_SYM
%token	SHOW
%token	SLAVE
%token	SQL_THREAD
%token	START_SYM
%token	STD_SYM
%token	STOP_SYM
%token	SUM_SYM
%token	SUPER_SYM
%token	TRUNCATE_SYM
%token	UNLOCK_SYM
%token	UPDATE_SYM

%token	ACTION
%token	AGGREGATE_SYM
%token	ALL
%token	AND
%token	AS
%token	ASC
%token	AUTO_INC
%token	AVG_ROW_LENGTH
%token	BACKUP_SYM
%token	BERKELEY_DB_SYM
%token	BINARY
%token	BIT_SYM
%token	BOOL_SYM
%token	BOOLEAN_SYM
%token	BOTH
%token	BY
%token	CACHE_SYM
%token	CASCADE
%token	CAST_SYM
%token	CHARSET
%token	CHECKSUM_SYM
%token	CHECK_SYM
%token	COMMITTED_SYM
%token	COLUMNS
%token	COLUMN_SYM
%token	CONCURRENT
%token	CONSTRAINT
%token	CONVERT_SYM
%token	DATABASES
%token	DATA_SYM
%token	DEFAULT
%token	DELAYED_SYM
%token	DELAY_KEY_WRITE_SYM
%token	DESC
%token	DESCRIBE
%token	DES_KEY_FILE
%token	DISABLE_SYM
%token	DISTINCT
%token	DYNAMIC_SYM
%token	ENABLE_SYM
%token	ENCLOSED
%token	ESCAPED
%token	DIRECTORY_SYM
%token	ESCAPE_SYM
%token	EXISTS
%token	EXTENDED_SYM
%token	FILE_SYM
%token	FIRST_SYM
%token	FIXED_SYM
%token	FLOAT_NUM
%token	FORCE_SYM
%token	FOREIGN
%token	FROM
%token	FULL
%token	FULLTEXT_SYM
%token	GLOBAL_SYM
%token	GRANT
%token	GRANTS
%token	GREATEST_SYM
%token	GROUP
%token	HAVING
%token	HEAP_SYM
%token	HEX_NUM
%token	HIGH_PRIORITY
%token	HOSTS_SYM
%token	IDENT
%token	IGNORE_SYM
%token	INDEX
%token	INDEXES
%token	INFILE
%token	INNER_SYM
%token	INNOBASE_SYM
%token	INTO
%token	IN_SYM
%token	ISOLATION
%token	ISAM_SYM
%token	JOIN_SYM
%token	KEYS
%token	KEY_SYM
%token	LEADING
%token	LEAST_SYM
%token	LEVEL_SYM
%token	LEX_HOSTNAME
%token	LIKE
%token	LINES
%token	LOCAL_SYM
%token	LOG_SYM
%token	LOGS_SYM
%token	LONG_NUM
%token	LONG_SYM
%token	LOW_PRIORITY
%token	MASTER_HOST_SYM
%token	MASTER_USER_SYM
%token	MASTER_LOG_FILE_SYM
%token	MASTER_LOG_POS_SYM
%token	MASTER_PASSWORD_SYM
%token	MASTER_PORT_SYM
%token	MASTER_CONNECT_RETRY_SYM
%token	MASTER_SERVER_ID_SYM
%token	RELAY_LOG_FILE_SYM
%token	RELAY_LOG_POS_SYM
%token	MATCH
%token	MAX_ROWS
%token	MAX_CONNECTIONS_PER_HOUR
%token	MAX_QUERIES_PER_HOUR
%token	MAX_UPDATES_PER_HOUR
%token	MEDIUM_SYM
%token	MERGE_SYM
%token	MEMORY_SYM
%token	MIN_ROWS
%token	MYISAM_SYM
%token	NATIONAL_SYM
%token	NATURAL
%token	NEW_SYM
%token	NCHAR_SYM
%token	NOT
%token	NO_SYM
%token	NULL_SYM
%token	NUM
%token	OFFSET_SYM
%token	ON
%token	OPEN_SYM
%token	OPTION
%token	OPTIONALLY
%token	OR
%token	OR_OR_CONCAT
%token	ORDER_SYM
%token	OUTER
%token	OUTFILE
%token	DUMPFILE
%token	PACK_KEYS_SYM
%token	PARTIAL
%token	PRIMARY_SYM
%token	PRIVILEGES
%token	PROCESS
%token	PROCESSLIST_SYM
%token	QUERY_SYM
%token	RAID_0_SYM
%token	RAID_STRIPED_SYM
%token	RAID_TYPE
%token	RAID_CHUNKS
%token	RAID_CHUNKSIZE
%token	READ_SYM
%token	REAL_NUM
%token	REFERENCES
%token	REGEXP
%token	RELOAD
%token	RENAME
%token	REPEATABLE_SYM
%token	REQUIRE_SYM
%token	RESOURCES
%token	RESTORE_SYM
%token	RESTRICT
%token	REVOKE
%token	ROWS_SYM
%token	ROW_FORMAT_SYM
%token	ROW_SYM
%token	SET
%token	SERIALIZABLE_SYM
%token	SESSION_SYM
%token	SHUTDOWN
%token	SSL_SYM
%token	STARTING
%token	STATUS_SYM
%token	STRAIGHT_JOIN
%token	SUBJECT_SYM
%token	TABLES
%token	TABLE_SYM
%token	TEMPORARY
%token	TERMINATED
%token	TEXT_STRING
%token	TO_SYM
%token	TRAILING
%token	TRANSACTION_SYM
%token	TYPE_SYM
%token	FUNC_ARG0
%token	FUNC_ARG1
%token	FUNC_ARG2
%token	FUNC_ARG3
%token	UDF_RETURNS_SYM
%token	UDF_SONAME_SYM
%token	UDF_SYM
%token	UNCOMMITTED_SYM
%token	UNION_SYM
%token	UNIQUE_SYM
%token	USAGE
%token	USE_FRM
%token	USE_SYM
%token	USING
%token	VALUES
%token	VARIABLES
%token	WHERE
%token	WITH
%token	WRITE_SYM
%token	X509_SYM
%token  XOR
%token	COMPRESSED_SYM

%token	BIGINT
%token	BLOB_SYM
%token	CHAR_SYM
%token	CHANGED
%token	COALESCE
%token	DATETIME
%token	DATE_SYM
%token	DECIMAL_SYM
%token	DOUBLE_SYM
%token	ENUM
%token	FAST_SYM
%token	FLOAT_SYM
%token	INT_SYM
%token	LIMIT
%token	LONGBLOB
%token	LONGTEXT
%token	MEDIUMBLOB
%token	MEDIUMINT
%token	MEDIUMTEXT
%token	NUMERIC_SYM
%token	PRECISION
%token	QUICK
%token	REAL
%token	SIGNED_SYM
%token	SMALLINT
%token	STRING_SYM
%token	TEXT_SYM
%token	TIMESTAMP
%token	TIME_SYM
%token	TINYBLOB
%token	TINYINT
%token	TINYTEXT
%token	ULONGLONG_NUM
%token	UNSIGNED
%token	VARBINARY
%token	VARCHAR
%token	VARYING
%token	ZEROFILL

%token	AGAINST
%token	ATAN
%token	BETWEEN_SYM
%token	BIT_AND
%token	BIT_OR
%token	CASE_SYM
%token	CONCAT
%token	CONCAT_WS
%token	CURDATE
%token	CURTIME
%token	DATABASE
%token	DATE_ADD_INTERVAL
%token	DATE_SUB_INTERVAL
%token	DAY_HOUR_SYM
%token	DAY_MINUTE_SYM
%token	DAY_SECOND_SYM
%token	DAY_SYM
%token	DECODE_SYM
%token	DES_ENCRYPT_SYM
%token	DES_DECRYPT_SYM
%token	ELSE
%token	ELT_FUNC
%token	ENCODE_SYM
%token	ENCRYPT
%token	EXPORT_SET
%token	EXTRACT_SYM
%token	FIELD_FUNC
%token	FORMAT_SYM
%token	FOR_SYM
%token	FROM_UNIXTIME
%token	GROUP_UNIQUE_USERS
%token	HOUR_MINUTE_SYM
%token	HOUR_SECOND_SYM
%token	HOUR_SYM
%token	IDENTIFIED_SYM
%token	IF
%token	INSERT_METHOD
%token	INTERVAL_SYM
%token	LAST_INSERT_ID
%token	LEFT
%token	LOCATE
%token	MAKE_SET_SYM
%token	MASTER_POS_WAIT
%token	MINUTE_SECOND_SYM
%token	MINUTE_SYM
%token	MODE_SYM
%token	MODIFY_SYM
%token	MONTH_SYM
%token	NOW_SYM
%token	PASSWORD
%token	POSITION_SYM
%token	PROCEDURE
%token	RAND
%token	REPLACE
%token	RIGHT
%token	ROUND
%token	SECOND_SYM
%token	SHARE_SYM
%token	SUBSTRING
%token	SUBSTRING_INDEX
%token	TRIM
%token	UDA_CHAR_SUM
%token	UDA_FLOAT_SUM
%token	UDA_INT_SUM
%token	UDF_CHAR_FUNC
%token	UDF_FLOAT_FUNC
%token	UDF_INT_FUNC
%token	UNIQUE_USERS
%token	UNIX_TIMESTAMP
%token	USER
%token	WEEK_SYM
%token	WHEN_SYM
%token	WORK_SYM
%token	YEAR_MONTH_SYM
%token	YEAR_SYM
%token	YEARWEEK
%token	BENCHMARK_SYM
%token	END
%token	THEN_SYM

%token	SQL_BIG_RESULT
%token	SQL_CACHE_SYM
%token	SQL_CALC_FOUND_ROWS
%token	SQL_NO_CACHE_SYM
%token	SQL_SMALL_RESULT
%token  SQL_BUFFER_RESULT

%token  ISSUER_SYM
%token  SUBJECT_SYM
%token  CIPHER_SYM

%left   SET_VAR
%left	OR_OR_CONCAT OR
%left	AND
%left	BETWEEN_SYM CASE_SYM WHEN_SYM THEN_SYM ELSE
%left	EQ EQUAL_SYM GE GT_SYM LE LT NE IS LIKE REGEXP IN_SYM
%left	'|'
%left	'&'
%left	SHIFT_LEFT SHIFT_RIGHT
%left	'-' '+'
%left	'*' '/' '%'
%left	NEG '~'
%left   XOR
%left   '^'
%right	NOT
%right	BINARY

%type <lex_str>
	IDENT TEXT_STRING REAL_NUM FLOAT_NUM NUM LONG_NUM HEX_NUM LEX_HOSTNAME
	ULONGLONG_NUM field_ident select_alias ident ident_or_text

%type <lex_str_ptr>
	opt_table_alias

%type <table>
	table_ident

%type <simple_string>
	remember_name remember_end opt_len opt_ident opt_db text_or_password
	opt_escape

%type <string>
	text_string 

%type <num>
	type int_type real_type order_dir opt_field_spec lock_option
	udf_type if_exists opt_local opt_table_options table_options
	table_option opt_if_not_exists opt_var_type opt_var_ident_type
	opt_temporary 

%type <ulong_num>
	ULONG_NUM raid_types merge_insert_types

%type <ulonglong_number>
	ulonglong_num

%type <lock_type>
	replace_lock_option opt_low_priority insert_lock_option load_data_lock

%type <item>
	literal text_literal insert_ident order_ident
	simple_ident select_item2 expr opt_expr opt_else sum_expr in_sum_expr
	table_wild opt_pad no_in_expr expr_expr simple_expr no_and_expr
	using_list expr_or_default set_expr_or_default

%type <item_list>
	expr_list udf_expr_list when_list ident_list ident_list_arg

%type <key_type>
	key_type opt_unique_or_fulltext

%type <string_list>
	key_usage_list

%type <key_part>
	key_part

%type <table_list>
	join_table_list join_table

%type <udf>
	UDF_CHAR_FUNC UDF_FLOAT_FUNC UDF_INT_FUNC
	UDA_CHAR_SUM UDA_FLOAT_SUM UDA_INT_SUM

%type <interval> interval

%type <db_type> table_types

%type <row_type> row_types

%type <tx_isolation> isolation_types

%type <ha_rkey_mode> handler_rkey_mode

%type <cast_type> cast_type

%type <udf_type> udf_func_type

%type <symbol> FUNC_ARG0 FUNC_ARG1 FUNC_ARG2 FUNC_ARG3 keyword

%type <lex_user> user grant_user

%type <variable> internal_variable_name

%type <NONE>
	query verb_clause create change select do drop insert replace insert2
	insert_values update delete truncate rename
	show describe load alter optimize flush
	reset purge begin commit rollback savepoint
	slave master_def master_defs
	repair restore backup analyze check start
	field_list field_list_item field_spec kill column_def key_def
	select_item_list select_item values_list no_braces
	limit_clause delete_limit_clause fields opt_values values
	procedure_list procedure_list2 procedure_item
	when_list2 expr_list2  handler
	opt_precision opt_ignore opt_column opt_restrict
	grant revoke set lock unlock string_list field_options field_option
	field_opt_list opt_binary table_lock_list table_lock varchar
	references opt_on_delete opt_on_delete_list opt_on_delete_item use
	opt_delete_options opt_delete_option
	opt_outer table_list table_name opt_option opt_place
	opt_attribute opt_attribute_list attribute column_list column_list_id
	opt_column_list grant_privileges opt_table user_list grant_option
	grant_privilege grant_privilege_list
	flush_options flush_option
	equal optional_braces opt_key_definition key_usage_list2
	opt_mi_check_type opt_to mi_check_types normal_join
	table_to_table_list table_to_table opt_table_list opt_as
	handler_rkey_function handler_read_or_scan
	single_multi table_wild_list table_wild_one opt_wild opt_union union_list
	precision union_option opt_and
END_OF_INPUT

%type <NONE>
	'-' '+' '*' '/' '%' '(' ')'
	',' '!' '{' '}' '&' '|' AND OR OR_OR_CONCAT BETWEEN_SYM CASE_SYM THEN_SYM WHEN_SYM
%%


query:
	END_OF_INPUT
	{
	   THD *thd=current_thd;
	   if (!thd->bootstrap &&
	      (!(thd->lex.select_lex.options & OPTION_FOUND_COMMENT)))
	   {
	     send_error(&current_thd->net,ER_EMPTY_QUERY);
	     YYABORT;
 	   }
	   else
	   {
	     thd->lex.sql_command = SQLCOM_EMPTY_QUERY;
	   }
	}
	| verb_clause END_OF_INPUT {};

verb_clause:
	  alter
	| analyze
	| backup
	| begin
	| change
	| check
	| commit
	| create
	| delete
	| describe
	| do
	| drop
	| grant
	| insert
	| flush
	| load
	| lock
	| kill
	| optimize
	| purge  
	| rename
        | repair
	| replace
	| reset
	| restore
	| revoke
	| rollback
	| savepoint
	| select
	| set
	| slave
	| start
	| show
	| truncate
	| handler
	| unlock
	| update
	| use;

/* change master */

change:
       CHANGE MASTER_SYM TO_SYM
        {
	  LEX *lex = Lex;
	  lex->sql_command = SQLCOM_CHANGE_MASTER;
	  bzero((char*) &lex->mi, sizeof(lex->mi));
        }
       master_defs
	{}
       ;

master_defs:
       master_def
       | master_defs ',' master_def;

master_def:
       MASTER_HOST_SYM EQ TEXT_STRING
       {
	 Lex->mi.host = $3.str;
       }
       |
       MASTER_USER_SYM EQ TEXT_STRING
       {
	 Lex->mi.user = $3.str;
       }
       |
       MASTER_PASSWORD_SYM EQ TEXT_STRING
       {
	 Lex->mi.password = $3.str;
       }
       |
       MASTER_LOG_FILE_SYM EQ TEXT_STRING
       {
	 Lex->mi.log_file_name = $3.str;
       }
       |
       MASTER_PORT_SYM EQ ULONG_NUM
       {
	 Lex->mi.port = $3;
       }
       |
       MASTER_LOG_POS_SYM EQ ulonglong_num
       {
	 Lex->mi.pos = $3;
         /* 
            If the user specified a value < BIN_LOG_HEADER_SIZE, adjust it
            instead of causing subsequent errors. 
            We need to do it in this file, because only there we know that 
            MASTER_LOG_POS has been explicitely specified. On the contrary
            in change_master() (sql_repl.cc) we cannot distinguish between 0
            (MASTER_LOG_POS explicitely specified as 0) and 0 (unspecified),
            whereas we want to distinguish (specified 0 means "read the binlog
            from 0" (4 in fact), unspecified means "don't change the position
            (keep the preceding value)").
         */
         Lex->mi.pos = max(BIN_LOG_HEADER_SIZE, Lex->mi.pos);
       }
       |
       MASTER_CONNECT_RETRY_SYM EQ ULONG_NUM
       {
	 Lex->mi.connect_retry = $3;
       }
       |
       RELAY_LOG_FILE_SYM EQ TEXT_STRING
       {
	 Lex->mi.relay_log_name = $3.str;
       }
       |
       RELAY_LOG_POS_SYM EQ ULONG_NUM
       {
	 Lex->mi.relay_log_pos = $3;
         /* Adjust if < BIN_LOG_HEADER_SIZE (same comment as Lex->mi.pos) */
         Lex->mi.relay_log_pos = max(BIN_LOG_HEADER_SIZE, Lex->mi.relay_log_pos);
       };


/* create a table */

create:
	CREATE opt_table_options TABLE_SYM opt_if_not_exists table_ident
	{
	  LEX *lex=Lex;
	  lex->sql_command= SQLCOM_CREATE_TABLE;
	  if (!add_table_to_list($5,
				 ($2 & HA_LEX_CREATE_TMP_TABLE ?
				   &tmp_table_alias : (LEX_STRING*) 0),
				 TL_OPTION_UPDATING))
	    YYABORT;
	  lex->create_list.empty();
	  lex->key_list.empty();
	  lex->col_list.empty();
	  lex->change=NullS;
	  bzero((char*) &lex->create_info,sizeof(lex->create_info));
	  lex->create_info.options=$2 | $4;
	  lex->create_info.db_type= (enum db_type) lex->thd->variables.table_type;
	}
	create2
	  {Lex->select= &Lex->select_lex;}
	| CREATE opt_unique_or_fulltext INDEX ident ON table_ident
	  {
	    LEX *lex=Lex;
	    lex->sql_command= SQLCOM_CREATE_INDEX;
	    if (!add_table_to_list($6, NULL, TL_OPTION_UPDATING))
	      YYABORT;
	    lex->create_list.empty();
	    lex->key_list.empty();
	    lex->col_list.empty();
	    lex->change=NullS;
	  }
	  '(' key_list ')'
	  {
	    LEX *lex=Lex;
	    lex->key_list.push_back(new Key($2,$4.str,lex->col_list));
	    lex->col_list.empty();
	  }
	| CREATE DATABASE opt_if_not_exists ident
	  {
	    LEX *lex=Lex;
	    lex->sql_command=SQLCOM_CREATE_DB;
	    lex->name=$4.str;
            lex->create_info.options=$3;
	  }
	| CREATE udf_func_type UDF_SYM ident
	  {
	    LEX *lex=Lex;
	    lex->sql_command = SQLCOM_CREATE_FUNCTION;
	    lex->udf.name=$4.str;
	    lex->udf.name_length=$4.length;
	    lex->udf.type= $2;
	  }
	  UDF_RETURNS_SYM udf_type UDF_SONAME_SYM TEXT_STRING
	  {
	    LEX *lex=Lex;
	    lex->udf.returns=(Item_result) $7;
	    lex->udf.dl=$9.str;
	  };

create2:
	'(' create2a {}
	| opt_create_table_options create3 {};

create2a:
        field_list ')' opt_create_table_options create3 {}
	|  create_select ')' { Select->braces= 1;} union_opt {}
        ;

create3:
	/* empty */ {}
	| opt_duplicate opt_as     create_select
          { Select->braces= 0;} opt_union {}
	| opt_duplicate opt_as '(' create_select ')'
          { Select->braces= 1;} union_opt {}
        ;

create_select:
          SELECT_SYM
          {
	    LEX *lex=Lex;
	    lex->lock_option= (using_update_log) ? TL_READ_NO_INSERT : TL_READ;
	    if (lex->sql_command == SQLCOM_INSERT)
	      lex->sql_command= SQLCOM_INSERT_SELECT;
	    else if (lex->sql_command == SQLCOM_REPLACE)
	      lex->sql_command= SQLCOM_REPLACE_SELECT;
	    lex->select->table_list.save_and_clear(&lex->save_list);
	    mysql_init_select(lex);
          }
          select_options select_item_list opt_select_from
	  { Lex->select->table_list.push_front(&Lex->save_list); }
          ;

opt_as:
	/* empty */ {}
	| AS	    {};

opt_table_options:
	/* empty */	 { $$= 0; }
	| table_options  { $$= $1;};

table_options:
	table_option	{ $$=$1; }
	| table_option table_options { $$= $1 | $2; };

table_option:
	TEMPORARY	{ $$=HA_LEX_CREATE_TMP_TABLE; };

opt_if_not_exists:
	/* empty */	 { $$= 0; }
	| IF NOT EXISTS	 { $$=HA_LEX_CREATE_IF_NOT_EXISTS; };

opt_create_table_options:
	/* empty */
	| create_table_options;

create_table_options:
	create_table_option
	| create_table_option create_table_options;

create_table_option:
	TYPE_SYM EQ table_types		{ Lex->create_info.db_type= $3; }
	| MAX_ROWS EQ ulonglong_num	{ Lex->create_info.max_rows= $3; Lex->create_info.used_fields|= HA_CREATE_USED_MAX_ROWS;}
	| MIN_ROWS EQ ulonglong_num	{ Lex->create_info.min_rows= $3; Lex->create_info.used_fields|= HA_CREATE_USED_MIN_ROWS;}
	| AVG_ROW_LENGTH EQ ULONG_NUM	{ Lex->create_info.avg_row_length=$3; Lex->create_info.used_fields|= HA_CREATE_USED_AVG_ROW_LENGTH;}
	| PASSWORD EQ TEXT_STRING	{ Lex->create_info.password=$3.str; }
	| COMMENT_SYM EQ TEXT_STRING	{ Lex->create_info.comment=$3.str; }
	| AUTO_INC EQ ulonglong_num	{ Lex->create_info.auto_increment_value=$3; Lex->create_info.used_fields|= HA_CREATE_USED_AUTO;}
	| PACK_KEYS_SYM EQ ULONG_NUM	{ Lex->create_info.table_options|= $3 ? HA_OPTION_PACK_KEYS : HA_OPTION_NO_PACK_KEYS; Lex->create_info.used_fields|= HA_CREATE_USED_PACK_KEYS;}
	| PACK_KEYS_SYM EQ DEFAULT	{ Lex->create_info.table_options&= ~(HA_OPTION_PACK_KEYS | HA_OPTION_NO_PACK_KEYS); Lex->create_info.used_fields|= HA_CREATE_USED_PACK_KEYS;}
	| CHECKSUM_SYM EQ ULONG_NUM	{ Lex->create_info.table_options|= $3 ? HA_OPTION_CHECKSUM : HA_OPTION_NO_CHECKSUM; }
	| DELAY_KEY_WRITE_SYM EQ ULONG_NUM { Lex->create_info.table_options|= $3 ? HA_OPTION_DELAY_KEY_WRITE : HA_OPTION_NO_DELAY_KEY_WRITE; }
	| ROW_FORMAT_SYM EQ row_types	{ Lex->create_info.row_type= $3; }
	| RAID_TYPE EQ raid_types	{ Lex->create_info.raid_type= $3; Lex->create_info.used_fields|= HA_CREATE_USED_RAID;}
	| RAID_CHUNKS EQ ULONG_NUM	{ Lex->create_info.raid_chunks= $3; Lex->create_info.used_fields|= HA_CREATE_USED_RAID;}
	| RAID_CHUNKSIZE EQ ULONG_NUM	{ Lex->create_info.raid_chunksize= $3*RAID_BLOCK_SIZE; Lex->create_info.used_fields|= HA_CREATE_USED_RAID;}
	| UNION_SYM EQ '(' table_list ')'
	  {
	    /* Move the union list to the merge_list */
	    LEX *lex=Lex;
	    TABLE_LIST *table_list= (TABLE_LIST*) lex->select->table_list.first;
	    lex->create_info.merge_list= lex->select->table_list;
	    lex->create_info.merge_list.elements--;
	    lex->create_info.merge_list.first= (byte*) (table_list->next);
	    lex->select->table_list.elements=1;
	    lex->select->table_list.next= (byte**) &(table_list->next);
	    table_list->next=0;
	    lex->create_info.used_fields|= HA_CREATE_USED_UNION;
	  }
	| CHARSET opt_equal ident {}
	| CHAR_SYM SET opt_equal ident {}
	| INSERT_METHOD EQ merge_insert_types   { Lex->create_info.merge_insert_method= $3; Lex->create_info.used_fields|= HA_CREATE_USED_INSERT_METHOD;}
	| DATA_SYM DIRECTORY_SYM EQ TEXT_STRING	{ Lex->create_info.data_file_name= $4.str; }
	| INDEX DIRECTORY_SYM EQ TEXT_STRING	{ Lex->create_info.index_file_name= $4.str; };

table_types:
	ISAM_SYM	{ $$= DB_TYPE_ISAM; }
	| MYISAM_SYM	{ $$= DB_TYPE_MYISAM; }
	| MERGE_SYM	{ $$= DB_TYPE_MRG_MYISAM; }
	| HEAP_SYM	{ $$= DB_TYPE_HEAP; }
	| MEMORY_SYM	{ $$= DB_TYPE_HEAP; }
	| BERKELEY_DB_SYM { $$= DB_TYPE_BERKELEY_DB; }
	| INNOBASE_SYM  { $$= DB_TYPE_INNODB; };

row_types:
	DEFAULT		{ $$= ROW_TYPE_DEFAULT; }
	| FIXED_SYM	{ $$= ROW_TYPE_FIXED; }
	| DYNAMIC_SYM	{ $$= ROW_TYPE_DYNAMIC; }
	| COMPRESSED_SYM { $$= ROW_TYPE_COMPRESSED; };

raid_types:
	RAID_STRIPED_SYM { $$= RAID_TYPE_0; }
	| RAID_0_SYM	 { $$= RAID_TYPE_0; }
	| ULONG_NUM	 { $$=$1;};

merge_insert_types:
       NO_SYM            { $$= MERGE_INSERT_DISABLED; }
       | FIRST_SYM       { $$= MERGE_INSERT_TO_FIRST; }
       | LAST_SYM        { $$= MERGE_INSERT_TO_LAST; };

opt_select_from:
	/* empty */
	| select_from select_lock_type;

udf_func_type:
	/* empty */ 	{ $$ = UDFTYPE_FUNCTION; }
	| AGGREGATE_SYM { $$ = UDFTYPE_AGGREGATE; };

udf_type:
	STRING_SYM {$$ = (int) STRING_RESULT; }
	| REAL {$$ = (int) REAL_RESULT; }
	| INT_SYM {$$ = (int) INT_RESULT; };

field_list:
	  field_list_item
	| field_list ',' field_list_item;


field_list_item:
           column_def
         | key_def
         ;

column_def:
	  field_spec check_constraint
	| field_spec references
	  {
	    Lex->col_list.empty();		/* Alloced by sql_alloc */
	  }
        ;

key_def:
	  key_type opt_ident '(' key_list ')'
	  {
	    LEX *lex=Lex;
	    lex->key_list.push_back(new Key($1,$2,lex->col_list));
	    lex->col_list.empty();		/* Alloced by sql_alloc */
	  }
	| opt_constraint FOREIGN KEY_SYM opt_ident '(' key_list ')' references
	  {
	    Lex->col_list.empty();		/* Alloced by sql_alloc */
	  }
	| opt_constraint check_constraint
	  {
	    Lex->col_list.empty();		/* Alloced by sql_alloc */
	  }
	;

check_constraint:
	/* empty */
	| CHECK_SYM expr
	;

opt_constraint:
	/* empty */
	| CONSTRAINT opt_ident;

field_spec:
	field_ident
	 {
	   LEX *lex=Lex;
	   lex->length=lex->dec=0; lex->type=0; lex->interval=0;
	   lex->default_value=0;
	 }
	type opt_attribute
	{
	  LEX *lex=Lex;
	  if (add_field_to_list($1.str,
				(enum enum_field_types) $3,
				lex->length,lex->dec,lex->type,
				lex->default_value,lex->change,
				lex->interval))
	    YYABORT;
	};

type:
	int_type opt_len field_options	{ Lex->length=$2; $$=$1; }
	| real_type opt_precision field_options { $$=$1; }
	| FLOAT_SYM float_options field_options { $$=FIELD_TYPE_FLOAT; }
	| BIT_SYM opt_len		{ Lex->length=(char*) "1";
					  $$=FIELD_TYPE_TINY; }
	| BOOL_SYM			{ Lex->length=(char*) "1";
					  $$=FIELD_TYPE_TINY; }
	| char '(' NUM ')' opt_binary { Lex->length=$3.str;
					  $$=FIELD_TYPE_STRING; }
	| char opt_binary		{ Lex->length=(char*) "1";
					  $$=FIELD_TYPE_STRING; }
	| BINARY '(' NUM ')' 		{ Lex->length=$3.str;
					  Lex->type|=BINARY_FLAG;
					  $$=FIELD_TYPE_STRING; }
	| varchar '(' NUM ')' opt_binary { Lex->length=$3.str;
					  $$=FIELD_TYPE_VAR_STRING; }
	| VARBINARY '(' NUM ')' 	{ Lex->length=$3.str;
					  Lex->type|=BINARY_FLAG;
					  $$=FIELD_TYPE_VAR_STRING; }
	| YEAR_SYM opt_len field_options { $$=FIELD_TYPE_YEAR; Lex->length=$2; }
	| DATE_SYM			{ $$=FIELD_TYPE_DATE; }
	| TIME_SYM			{ $$=FIELD_TYPE_TIME; }
	| TIMESTAMP			{ $$=FIELD_TYPE_TIMESTAMP; }
	| TIMESTAMP '(' NUM ')'		{ Lex->length=$3.str;
					  $$=FIELD_TYPE_TIMESTAMP; }
	| DATETIME			{ $$=FIELD_TYPE_DATETIME; }
	| TINYBLOB			{ Lex->type|=BINARY_FLAG;
					  $$=FIELD_TYPE_TINY_BLOB; }
	| BLOB_SYM			{ Lex->type|=BINARY_FLAG;
					  $$=FIELD_TYPE_BLOB; }
	| MEDIUMBLOB			{ Lex->type|=BINARY_FLAG;
					  $$=FIELD_TYPE_MEDIUM_BLOB; }
	| LONGBLOB			{ Lex->type|=BINARY_FLAG;
					  $$=FIELD_TYPE_LONG_BLOB; }
	| LONG_SYM VARBINARY		{ Lex->type|=BINARY_FLAG;
					  $$=FIELD_TYPE_MEDIUM_BLOB; }
	| LONG_SYM varchar		{ $$=FIELD_TYPE_MEDIUM_BLOB; }
	| TINYTEXT			{ $$=FIELD_TYPE_TINY_BLOB; }
	| TEXT_SYM			{ $$=FIELD_TYPE_BLOB; }
	| MEDIUMTEXT			{ $$=FIELD_TYPE_MEDIUM_BLOB; }
	| LONGTEXT			{ $$=FIELD_TYPE_LONG_BLOB; }
	| DECIMAL_SYM float_options field_options
					{ $$=FIELD_TYPE_DECIMAL;}
	| NUMERIC_SYM float_options field_options
					{ $$=FIELD_TYPE_DECIMAL;}
	| ENUM {Lex->interval_list.empty();} '(' string_list ')'
	  {
	    LEX *lex=Lex;
	    lex->interval=typelib(lex->interval_list);
	    $$=FIELD_TYPE_ENUM;
	  }
	| SET { Lex->interval_list.empty();} '(' string_list ')'
	  {
	    LEX *lex=Lex;
	    lex->interval=typelib(lex->interval_list);
	    $$=FIELD_TYPE_SET;
	  };

char:
	CHAR_SYM {}
	| NCHAR_SYM {}
	| NATIONAL_SYM CHAR_SYM {};

varchar:
	char VARYING {}
	| VARCHAR {}
	| NATIONAL_SYM VARCHAR {}
	| NCHAR_SYM VARCHAR {};

int_type:
	INT_SYM		{ $$=FIELD_TYPE_LONG; }
	| TINYINT	{ $$=FIELD_TYPE_TINY; }
	| SMALLINT	{ $$=FIELD_TYPE_SHORT; }
	| MEDIUMINT	{ $$=FIELD_TYPE_INT24; }
	| BIGINT	{ $$=FIELD_TYPE_LONGLONG; };

real_type:
	REAL		{ $$= current_thd->sql_mode & MODE_REAL_AS_FLOAT ?
			      FIELD_TYPE_FLOAT : FIELD_TYPE_DOUBLE; }
	| DOUBLE_SYM	{ $$=FIELD_TYPE_DOUBLE; }
	| DOUBLE_SYM PRECISION { $$=FIELD_TYPE_DOUBLE; };


float_options:
	/* empty */		{}
	| '(' NUM ')'		{ Lex->length=$2.str; }
	| precision		{};

precision:
	'(' NUM ',' NUM ')'
	{
	  LEX *lex=Lex;
	  lex->length=$2.str; lex->dec=$4.str;
	};

field_options:
	/* empty */		{}
	| field_opt_list	{};

field_opt_list:
	field_opt_list field_option {}
	| field_option {};

field_option:
	SIGNED_SYM	{}
	| UNSIGNED	{ Lex->type|= UNSIGNED_FLAG;}
	| ZEROFILL	{ Lex->type|= UNSIGNED_FLAG | ZEROFILL_FLAG; };

opt_len:
	/* empty */	{ $$=(char*) 0; }	/* use default length */
	| '(' NUM ')'	{ $$=$2.str; };

opt_precision:
	/* empty */	{}
	| precision	{};

opt_attribute:
	/* empty */ {}
	| opt_attribute_list {};

opt_attribute_list:
	opt_attribute_list attribute {}
	| attribute;

attribute:
	NULL_SYM	  { Lex->type&= ~ NOT_NULL_FLAG; }
	| NOT NULL_SYM	  { Lex->type|= NOT_NULL_FLAG; }
	| DEFAULT literal { Lex->default_value=$2; }
	| AUTO_INC	  { Lex->type|= AUTO_INCREMENT_FLAG | NOT_NULL_FLAG; }
	| PRIMARY_SYM KEY_SYM { Lex->type|= PRI_KEY_FLAG | NOT_NULL_FLAG; }
	| UNIQUE_SYM	  { Lex->type|= UNIQUE_FLAG; }
	| UNIQUE_SYM KEY_SYM { Lex->type|= UNIQUE_KEY_FLAG; }
	| COMMENT_SYM text_literal {};

opt_binary:
	/* empty */	{}
	| BINARY	{ Lex->type|=BINARY_FLAG; }
	| CHAR_SYM SET opt_equal ident {}
	;

references:
	REFERENCES table_ident opt_on_delete {}
	| REFERENCES table_ident '(' key_list ')' opt_on_delete
	  {
	    Lex->col_list.empty();		/* Alloced by sql_alloc */
	  };

opt_on_delete:
	/* empty */ {}
	| opt_on_delete_list {};

opt_on_delete_list:
	opt_on_delete_list opt_on_delete_item {}
	| opt_on_delete_item {};


opt_on_delete_item:
	ON DELETE_SYM delete_option {}
	| ON UPDATE_SYM delete_option {}
	| MATCH FULL	{}
	| MATCH PARTIAL {};

delete_option:
	RESTRICT	 {}
	| CASCADE	 {}
	| SET NULL_SYM {}
	| NO_SYM ACTION {}
	| SET DEFAULT {};

key_type:
	opt_constraint PRIMARY_SYM KEY_SYM  { $$= Key::PRIMARY; }
	| key_or_index			    { $$= Key::MULTIPLE; }
	| FULLTEXT_SYM			    { $$= Key::FULLTEXT; }
	| FULLTEXT_SYM key_or_index	    { $$= Key::FULLTEXT; }
	| opt_constraint UNIQUE_SYM	    { $$= Key::UNIQUE; }
	| opt_constraint UNIQUE_SYM key_or_index { $$= Key::UNIQUE; };

key_or_index:
	KEY_SYM {}
	| INDEX {};

keys_or_index:
	KEYS {}
	| INDEX {}
	| INDEXES {};

opt_unique_or_fulltext:
	/* empty */	{ $$= Key::MULTIPLE; }
	| UNIQUE_SYM	{ $$= Key::UNIQUE; }
	| FULLTEXT_SYM	{ $$= Key::FULLTEXT; };

key_list:
	key_list ',' key_part order_dir { Lex->col_list.push_back($3); }
	| key_part order_dir		{ Lex->col_list.push_back($1); };

key_part:
	ident			{ $$=new key_part_spec($1.str); }
	| ident '(' NUM ')'	{ $$=new key_part_spec($1.str,(uint) atoi($3.str)); };

opt_ident:
	/* empty */	{ $$=(char*) 0; }	/* Defaultlength */
	| field_ident	{ $$=$1.str; };

string_list:
	text_string			{ Lex->interval_list.push_back($1); }
	| string_list ',' text_string	{ Lex->interval_list.push_back($3); };

/*
** Alter table
*/

alter:
	ALTER opt_ignore TABLE_SYM table_ident
	{
	  LEX *lex=Lex;
	  lex->sql_command = SQLCOM_ALTER_TABLE;
	  lex->name=0;
	  if (!add_table_to_list($4, NULL, TL_OPTION_UPDATING))
	    YYABORT;
	  lex->drop_primary=0;
	  lex->create_list.empty();
	  lex->key_list.empty();
	  lex->col_list.empty();
	  lex->drop_list.empty();
	  lex->alter_list.empty();
          lex->select->order_list.elements=0;
          lex->select->order_list.first=0;
          lex->select->order_list.next= (byte**) &lex->select->order_list.first;
	  lex->select->db=lex->name=0;
    	  bzero((char*) &lex->create_info,sizeof(lex->create_info));
	  lex->create_info.db_type= DB_TYPE_DEFAULT;
	  lex->create_info.row_type= ROW_TYPE_NOT_USED;
          lex->alter_keys_onoff=LEAVE_AS_IS;
          lex->simple_alter=1;
	}
	alter_list
	{}
	;
 
alter_list:
        | alter_list_item
	| alter_list ',' alter_list_item;

add_column:
	ADD opt_column { Lex->change=0; };

alter_list_item:
	add_column column_def opt_place { Lex->simple_alter=0; }
	| ADD key_def { Lex->simple_alter=0; }
	| add_column '(' field_list ')'      { Lex->simple_alter=0; }
	| CHANGE opt_column field_ident
	  {
	     LEX *lex=Lex;
	     lex->change= $3.str; lex->simple_alter=0;
	  }
          field_spec opt_place
	| MODIFY_SYM opt_column field_ident
	  {
	    LEX *lex=Lex;
	    lex->length=lex->dec=0; lex->type=0; lex->interval=0;
	    lex->default_value=0;
            lex->simple_alter=0;
	  }
	  type opt_attribute
	  {
	    LEX *lex=Lex;
	    if (add_field_to_list($3.str,
				  (enum enum_field_types) $5,
				  lex->length,lex->dec,lex->type,
				  lex->default_value, $3.str,
				  lex->interval))
	      YYABORT;
	  }
	  opt_place
	| DROP opt_column field_ident opt_restrict
	  {
	    LEX *lex=Lex;
	    lex->drop_list.push_back(new Alter_drop(Alter_drop::COLUMN,
					    $3.str)); lex->simple_alter=0;
	  }
	| DROP PRIMARY_SYM KEY_SYM
	  {
	    LEX *lex=Lex;
	    lex->drop_primary=1; lex->simple_alter=0;
	  }
	| DROP FOREIGN KEY_SYM opt_ident { Lex->simple_alter=0; }
	| DROP key_or_index field_ident
	  {
	    LEX *lex=Lex;
	    lex->drop_list.push_back(new Alter_drop(Alter_drop::KEY,
						    $3.str));
	    lex->simple_alter=0;
	  }
	| DISABLE_SYM KEYS { Lex->alter_keys_onoff=DISABLE; }
	| ENABLE_SYM KEYS  { Lex->alter_keys_onoff=ENABLE; }
	| ALTER opt_column field_ident SET DEFAULT literal
	  {
	    LEX *lex=Lex;
	    lex->alter_list.push_back(new Alter_column($3.str,$6));
	    lex->simple_alter=0;
	  }
	| ALTER opt_column field_ident DROP DEFAULT
	  {
	    LEX *lex=Lex;
	    lex->alter_list.push_back(new Alter_column($3.str,(Item*) 0));
	    lex->simple_alter=0;
	  }
	| RENAME opt_to table_ident
	  { 
	    LEX *lex=Lex;
	    lex->select->db=$3->db.str;
	    lex->name= $3->table.str;
	  }
        | create_table_options { Lex->simple_alter=0; }
	| order_clause         { Lex->simple_alter=0; };

opt_column:
	/* empty */	{}
	| COLUMN_SYM	{};

opt_ignore:
	/* empty */	{ Lex->duplicates=DUP_ERROR; }
	| IGNORE_SYM	{ Lex->duplicates=DUP_IGNORE; };

opt_restrict:
	/* empty */	{}
	| RESTRICT	{}
	| CASCADE	{};

opt_place:
	/* empty */	{}
	| AFTER_SYM ident { store_position_for_column($2.str); }
	| FIRST_SYM	  { store_position_for_column(first_keyword); };

opt_to:
	/* empty */	{}
	| TO_SYM	{}
	| EQ		{}
	| AS		{};

/*
 * The first two deprecate the last two--delete the last two for 4.1 release
 */
slave:
	START_SYM SLAVE slave_thread_opts
         {
	   LEX *lex=Lex;
           lex->sql_command = SQLCOM_SLAVE_START;
	   lex->type = 0;
         }
         |
	STOP_SYM SLAVE slave_thread_opts
         {
	   LEX *lex=Lex;
           lex->sql_command = SQLCOM_SLAVE_STOP;
	   lex->type = 0;
         }
         |
	SLAVE START_SYM slave_thread_opts
         {
	   LEX *lex=Lex;
           lex->sql_command = SQLCOM_SLAVE_START;
	   lex->type = 0;
         }
         |
	SLAVE STOP_SYM slave_thread_opts
         {
	   LEX *lex=Lex;
           lex->sql_command = SQLCOM_SLAVE_STOP;
	   lex->type = 0;
         };

start:
	START_SYM TRANSACTION_SYM { Lex->sql_command = SQLCOM_BEGIN;}
	{}
	;

slave_thread_opts:
	slave_thread_opt
	| slave_thread_opts ',' slave_thread_opt;

slave_thread_opt:
	/*empty*/	{} 
	| SQL_THREAD	{ Lex->slave_thd_opt|=SLAVE_SQL; }
	| IO_THREAD   	{ Lex->slave_thd_opt|=SLAVE_IO; }
	;
  
restore:
	RESTORE_SYM table_or_tables
	{
	   Lex->sql_command = SQLCOM_RESTORE_TABLE;
	}
	table_list FROM TEXT_STRING
        {
	  Lex->backup_dir = $6.str;
        };

backup:
	BACKUP_SYM table_or_tables
	{
	   Lex->sql_command = SQLCOM_BACKUP_TABLE;
	}
	table_list TO_SYM TEXT_STRING
        {
	  Lex->backup_dir = $6.str;
        };

repair:
	REPAIR table_or_tables
	{
	   LEX *lex=Lex;
	   lex->sql_command = SQLCOM_REPAIR;
	   lex->check_opt.init();
	}
	table_list opt_mi_repair_type
	{}
	;

opt_mi_repair_type:
	/* empty */ { Lex->check_opt.flags = T_MEDIUM; }
	| mi_repair_types {};

mi_repair_types:
	mi_repair_type {}
	| mi_repair_type mi_repair_types {};

mi_repair_type:
	QUICK          { Lex->check_opt.flags|= T_QUICK; }
	| EXTENDED_SYM { Lex->check_opt.flags|= T_EXTEND; }
        | USE_FRM      { Lex->check_opt.sql_flags|= TT_USEFRM; };

analyze:
	ANALYZE_SYM table_or_tables
	{
	   LEX *lex=Lex;
	   lex->sql_command = SQLCOM_ANALYZE;
	   lex->check_opt.init();
	}
	table_list opt_mi_check_type
	{}
	;

check:
	CHECK_SYM table_or_tables
	{
	   LEX *lex=Lex;
	   lex->sql_command = SQLCOM_CHECK;
	   lex->check_opt.init();
	}
	table_list opt_mi_check_type
	{}
	;

opt_mi_check_type:
	/* empty */ { Lex->check_opt.flags = T_MEDIUM; }
	| mi_check_types {};

mi_check_types:
	mi_check_type {}
	| mi_check_type mi_check_types {};

mi_check_type:
	QUICK      { Lex->check_opt.flags|= T_QUICK; }
	| FAST_SYM { Lex->check_opt.flags|= T_FAST; }
	| MEDIUM_SYM { Lex->check_opt.flags|= T_MEDIUM; }
	| EXTENDED_SYM { Lex->check_opt.flags|= T_EXTEND; }
	| CHANGED  { Lex->check_opt.flags|= T_CHECK_ONLY_CHANGED; };

optimize:
	OPTIMIZE table_or_tables
	{
	   LEX *lex=Lex;
	   lex->sql_command = SQLCOM_OPTIMIZE;
	   lex->check_opt.init();
	}
	table_list opt_mi_check_type
	{}
	;

rename:
	RENAME table_or_tables
	{
	   Lex->sql_command=SQLCOM_RENAME_TABLE;
	}
	table_to_table_list
	{}
	;

table_to_table_list:
	table_to_table
	| table_to_table_list ',' table_to_table;

table_to_table:
	table_ident TO_SYM table_ident
	{
	   if (!add_table_to_list($1, NULL, TL_OPTION_UPDATING, TL_IGNORE) ||
	       !add_table_to_list($3, NULL, TL_OPTION_UPDATING, TL_IGNORE))
	     YYABORT;
 	};

/*
  Select : retrieve data from table
*/


select:
	select_init { Lex->sql_command=SQLCOM_SELECT; };

select_init:
	SELECT_SYM select_part2 { Select->braces= 0;	} opt_union
	|
	'(' SELECT_SYM select_part2 ')' { Select->braces= 1;} union_opt;


select_part2:
	{
	  LEX *lex=Lex;
	  lex->lock_option=TL_READ;
	   mysql_init_select(lex);
	}
	select_options select_item_list select_into select_lock_type;

select_into:
	limit_clause {}
	| select_from
	| opt_into select_from
	| select_from opt_into;

select_from:
	FROM join_table_list where_clause group_clause having_clause opt_order_clause limit_clause procedure_clause;


select_options:
	/* empty*/
	| select_option_list;

select_option_list:
	select_option_list select_option
	| select_option;

select_option:
	STRAIGHT_JOIN { Select->options|= SELECT_STRAIGHT_JOIN; }
	| HIGH_PRIORITY
	  {
	    if (check_simple_select())
	      YYABORT;
	    Lex->lock_option= TL_READ_HIGH_PRIORITY;
	  }
	| DISTINCT	{ Select->options|= SELECT_DISTINCT; }
	| SQL_SMALL_RESULT { Select->options|= SELECT_SMALL_RESULT; }
	| SQL_BIG_RESULT { Select->options|= SELECT_BIG_RESULT; }
	| SQL_BUFFER_RESULT
	  {
	    if (check_simple_select())
	      YYABORT;
	    Select->options|= OPTION_BUFFER_RESULT;
	  }
	| SQL_CALC_FOUND_ROWS
	  {
	    if (check_simple_select())
	      YYABORT;
	    Select->options|= OPTION_FOUND_ROWS;
	  }
	| SQL_NO_CACHE_SYM { current_thd->safe_to_cache_query=0; }
	| SQL_CACHE_SYM
	  {
	    Lex->select_lex.options|= OPTION_TO_QUERY_CACHE;
	  }
	| ALL		{}
	;

select_lock_type:
	/* empty */
	| FOR_SYM UPDATE_SYM
	  {
	    LEX *lex=Lex;
	    if (check_simple_select())
	      YYABORT;	
	    lex->lock_option= TL_WRITE;
	    lex->thd->safe_to_cache_query=0;
	  }
	| LOCK_SYM IN_SYM SHARE_SYM MODE_SYM
	  {
	    LEX *lex=Lex;
	    if (check_simple_select())
	      YYABORT;	
	    lex->lock_option= TL_READ_WITH_SHARED_LOCKS;
	    lex->thd->safe_to_cache_query=0;
	  }
	;

select_item_list:
	  select_item_list ',' select_item
	| select_item
	| '*'
	  {
	    if (add_item_to_list(new Item_field(NULL,NULL,"*")))
	      YYABORT;
	  };


select_item:
	  remember_name select_item2 remember_end select_alias
	  {
	    if (add_item_to_list($2))
	      YYABORT;
	    if ($4.str)
	      $2->set_name($4.str);
	    else if (!$2->name)
	      $2->set_name($1,(uint) ($3 - $1));
	  };

remember_name:
	{ $$=(char*) Lex->tok_start; };

remember_end:
	{ $$=(char*) Lex->tok_end; };

select_item2:
	table_wild	{ $$=$1; } /* table.* */
	| expr		{ $$=$1; };

select_alias:
	{ $$.str=0;}
	| AS ident { $$=$2; }
	| AS TEXT_STRING  { $$=$2; }
	| ident { $$=$1; }
	| TEXT_STRING  { $$=$1; };

optional_braces:
	/* empty */ {}
	| '(' ')' {};

/* all possible expressions */
expr:	expr_expr	{$$ = $1; }
	| simple_expr	{$$ = $1; };

/* expressions that begin with 'expr' */
expr_expr:
	expr IN_SYM '(' expr_list ')'
	  { $$= new Item_func_in($1,*$4); }
	| expr NOT IN_SYM '(' expr_list ')'
	  { $$= new Item_func_not(new Item_func_in($1,*$5)); }
	| expr BETWEEN_SYM no_and_expr AND expr
	  { $$= new Item_func_between($1,$3,$5); }
	| expr NOT BETWEEN_SYM no_and_expr AND expr
	  { $$= new Item_func_not(new Item_func_between($1,$4,$6)); }
	| expr OR_OR_CONCAT expr { $$= or_or_concat($1,$3); }
	| expr OR expr		{ $$= new Item_cond_or($1,$3); }
        | expr XOR expr		{ $$= new Item_cond_xor($1,$3); }
	| expr AND expr		{ $$= new Item_cond_and($1,$3); }
	| expr LIKE simple_expr opt_escape { $$= new Item_func_like($1,$3,$4); }
	| expr NOT LIKE simple_expr opt_escape	{ $$= new Item_func_not(new Item_func_like($1,$4,$5));}
	| expr REGEXP expr { $$= new Item_func_regex($1,$3); }
	| expr NOT REGEXP expr { $$= new Item_func_not(new Item_func_regex($1,$4)); }
	| expr IS NULL_SYM	{ $$= new Item_func_isnull($1); }
	| expr IS NOT NULL_SYM { $$= new Item_func_isnotnull($1); }
	| expr EQ expr		{ $$= new Item_func_eq($1,$3); }
	| expr EQUAL_SYM expr	{ $$= new Item_func_equal($1,$3); }
	| expr GE expr		{ $$= new Item_func_ge($1,$3); }
	| expr GT_SYM expr	{ $$= new Item_func_gt($1,$3); }
	| expr LE expr		{ $$= new Item_func_le($1,$3); }
	| expr LT expr		{ $$= new Item_func_lt($1,$3); }
	| expr NE expr		{ $$= new Item_func_ne($1,$3); }
	| expr SHIFT_LEFT expr	{ $$= new Item_func_shift_left($1,$3); }
	| expr SHIFT_RIGHT expr { $$= new Item_func_shift_right($1,$3); }
	| expr '+' expr		{ $$= new Item_func_plus($1,$3); }
	| expr '-' expr		{ $$= new Item_func_minus($1,$3); }
	| expr '*' expr		{ $$= new Item_func_mul($1,$3); }
	| expr '/' expr		{ $$= new Item_func_div($1,$3); }
	| expr '|' expr		{ $$= new Item_func_bit_or($1,$3); }
        | expr '^' expr		{ $$= new Item_func_bit_xor($1,$3); }	
	| expr '&' expr		{ $$= new Item_func_bit_and($1,$3); }
	| expr '%' expr		{ $$= new Item_func_mod($1,$3); }
	| expr '+' INTERVAL_SYM expr interval
	  { $$= new Item_date_add_interval($1,$4,$5,0); }
	| expr '-' INTERVAL_SYM expr interval
	  { $$= new Item_date_add_interval($1,$4,$5,1); };

/* expressions that begin with 'expr' that do NOT follow IN_SYM */
no_in_expr:
	no_in_expr BETWEEN_SYM no_and_expr AND expr
	  { $$= new Item_func_between($1,$3,$5); }
	| no_in_expr NOT BETWEEN_SYM no_and_expr AND expr
	  { $$= new Item_func_not(new Item_func_between($1,$4,$6)); }
	| no_in_expr OR_OR_CONCAT expr	{ $$= or_or_concat($1,$3); }
	| no_in_expr OR expr		{ $$= new Item_cond_or($1,$3); }
        | no_in_expr XOR expr		{ $$= new Item_cond_xor($1,$3); }
	| no_in_expr AND expr		{ $$= new Item_cond_and($1,$3); }
	| no_in_expr LIKE simple_expr opt_escape { $$= new Item_func_like($1,$3,$4); }
	| no_in_expr NOT LIKE simple_expr opt_escape { $$= new Item_func_not(new Item_func_like($1,$4,$5)); }
	| no_in_expr REGEXP expr { $$= new Item_func_regex($1,$3); }
	| no_in_expr NOT REGEXP expr { $$= new Item_func_not(new Item_func_regex($1,$4)); }
	| no_in_expr IS NULL_SYM	{ $$= new Item_func_isnull($1); }
	| no_in_expr IS NOT NULL_SYM { $$= new Item_func_isnotnull($1); }
	| no_in_expr EQ expr		{ $$= new Item_func_eq($1,$3); }
	| no_in_expr EQUAL_SYM expr	{ $$= new Item_func_equal($1,$3); }
	| no_in_expr GE expr		{ $$= new Item_func_ge($1,$3); }
	| no_in_expr GT_SYM expr	{ $$= new Item_func_gt($1,$3); }
	| no_in_expr LE expr		{ $$= new Item_func_le($1,$3); }
	| no_in_expr LT expr		{ $$= new Item_func_lt($1,$3); }
	| no_in_expr NE expr		{ $$= new Item_func_ne($1,$3); }
	| no_in_expr SHIFT_LEFT expr  { $$= new Item_func_shift_left($1,$3); }
	| no_in_expr SHIFT_RIGHT expr { $$= new Item_func_shift_right($1,$3); }
	| no_in_expr '+' expr		{ $$= new Item_func_plus($1,$3); }
	| no_in_expr '-' expr		{ $$= new Item_func_minus($1,$3); }
	| no_in_expr '*' expr		{ $$= new Item_func_mul($1,$3); }
	| no_in_expr '/' expr		{ $$= new Item_func_div($1,$3); }
	| no_in_expr '|' expr		{ $$= new Item_func_bit_or($1,$3); }
        | no_in_expr '^' expr		{ $$= new Item_func_bit_xor($1,$3); }
	| no_in_expr '&' expr		{ $$= new Item_func_bit_and($1,$3); }
	| no_in_expr '%' expr		{ $$= new Item_func_mod($1,$3); }
	| no_in_expr '+' INTERVAL_SYM expr interval
	  { $$= new Item_date_add_interval($1,$4,$5,0); }
	| no_in_expr '-' INTERVAL_SYM expr interval
	  { $$= new Item_date_add_interval($1,$4,$5,1); }
	| simple_expr;

/* expressions that begin with 'expr' that does NOT follow AND */
no_and_expr:
	no_and_expr IN_SYM '(' expr_list ')'
	{ $$= new Item_func_in($1,*$4); }
	| no_and_expr NOT IN_SYM '(' expr_list ')'
	  { $$= new Item_func_not(new Item_func_in($1,*$5)); }
	| no_and_expr BETWEEN_SYM no_and_expr AND expr
	  { $$= new Item_func_between($1,$3,$5); }
	| no_and_expr NOT BETWEEN_SYM no_and_expr AND expr
	  { $$= new Item_func_not(new Item_func_between($1,$4,$6)); }
	| no_and_expr OR_OR_CONCAT expr	{ $$= or_or_concat($1,$3); }
	| no_and_expr OR expr		{ $$= new Item_cond_or($1,$3); }
        | no_and_expr XOR expr		{ $$= new Item_cond_xor($1,$3); }
	| no_and_expr LIKE simple_expr opt_escape { $$= new Item_func_like($1,$3,$4); }
	| no_and_expr NOT LIKE simple_expr opt_escape	{ $$= new Item_func_not(new Item_func_like($1,$4,$5)); }
	| no_and_expr REGEXP expr { $$= new Item_func_regex($1,$3); }
	| no_and_expr NOT REGEXP expr { $$= new Item_func_not(new Item_func_regex($1,$4)); }
	| no_and_expr IS NULL_SYM	{ $$= new Item_func_isnull($1); }
	| no_and_expr IS NOT NULL_SYM { $$= new Item_func_isnotnull($1); }
	| no_and_expr EQ expr		{ $$= new Item_func_eq($1,$3); }
	| no_and_expr EQUAL_SYM expr	{ $$= new Item_func_equal($1,$3); }
	| no_and_expr GE expr		{ $$= new Item_func_ge($1,$3); }
	| no_and_expr GT_SYM expr	{ $$= new Item_func_gt($1,$3); }
	| no_and_expr LE expr		{ $$= new Item_func_le($1,$3); }
	| no_and_expr LT expr		{ $$= new Item_func_lt($1,$3); }
	| no_and_expr NE expr		{ $$= new Item_func_ne($1,$3); }
	| no_and_expr SHIFT_LEFT expr  { $$= new Item_func_shift_left($1,$3); }
	| no_and_expr SHIFT_RIGHT expr { $$= new Item_func_shift_right($1,$3); }
	| no_and_expr '+' expr		{ $$= new Item_func_plus($1,$3); }
	| no_and_expr '-' expr		{ $$= new Item_func_minus($1,$3); }
	| no_and_expr '*' expr		{ $$= new Item_func_mul($1,$3); }
	| no_and_expr '/' expr		{ $$= new Item_func_div($1,$3); }
	| no_and_expr '|' expr		{ $$= new Item_func_bit_or($1,$3); }
        | no_and_expr '^' expr		{ $$= new Item_func_bit_xor($1,$3); }
	| no_and_expr '&' expr		{ $$= new Item_func_bit_and($1,$3); }
	| no_and_expr '%' expr		{ $$= new Item_func_mod($1,$3); }
	| no_and_expr '+' INTERVAL_SYM expr interval
	  { $$= new Item_date_add_interval($1,$4,$5,0); }
	| no_and_expr '-' INTERVAL_SYM expr interval
	  { $$= new Item_date_add_interval($1,$4,$5,1); }
	| simple_expr;

simple_expr:
	simple_ident
	| literal
	| '@' ident_or_text SET_VAR expr
	  {
	    $$= new Item_func_set_user_var($2,$4);
	    current_thd->safe_to_cache_query=0;
	  }
	| '@' ident_or_text	 
	  {
	    $$= new Item_func_get_user_var($2);
	    current_thd->safe_to_cache_query=0;
	  }
	| '@' '@' opt_var_ident_type ident_or_text
	  {
	    if (!($$= get_system_var((enum_var_type) $3, $4)))
	      YYABORT;
	  }
	| sum_expr
	| '-' expr %prec NEG	{ $$= new Item_func_neg($2); }
	| '~' expr %prec NEG	{ $$= new Item_func_bit_neg($2); }
	| NOT expr %prec NEG	{ $$= new Item_func_not($2); }
	| '!' expr %prec NEG	{ $$= new Item_func_not($2); }
	| '(' expr ')'		{ $$= $2; }
	| '{' ident expr '}'	{ $$= $3; }
        | MATCH ident_list_arg AGAINST '(' expr ')'
          { Select->ftfunc_list.push_back((Item_func_match *)
                   ($$=new Item_func_match_nl(*$2,$5))); }
        | MATCH ident_list_arg AGAINST '(' expr IN_SYM BOOLEAN_SYM MODE_SYM ')'
          { Select->ftfunc_list.push_back((Item_func_match *)
                   ($$=new Item_func_match_bool(*$2,$5))); }
	| BINARY expr %prec NEG	{ $$= new Item_func_binary($2); }
	| CAST_SYM '(' expr AS cast_type ')'  { $$= create_func_cast($3, $5); }
	| CASE_SYM opt_expr WHEN_SYM when_list opt_else END
	  { $$= new Item_func_case(* $4, $2, $5 ); }
	| CONVERT_SYM '(' expr ',' cast_type ')'  { $$= create_func_cast($3, $5); }
	| FUNC_ARG0 '(' ')'
	  { $$= ((Item*(*)(void))($1.symbol->create_func))();}
	| FUNC_ARG1 '(' expr ')'
	  { $$= ((Item*(*)(Item*))($1.symbol->create_func))($3);}
	| FUNC_ARG2 '(' expr ',' expr ')'
	  { $$= ((Item*(*)(Item*,Item*))($1.symbol->create_func))($3,$5);}
	| FUNC_ARG3 '(' expr ',' expr ',' expr ')'
	  { $$= ((Item*(*)(Item*,Item*,Item*))($1.symbol->create_func))($3,$5,$7);}
	| ATAN	'(' expr ')'
	  { $$= new Item_func_atan($3); }
	| ATAN	'(' expr ',' expr ')'
	  { $$= new Item_func_atan($3,$5); }
	| CHAR_SYM '(' expr_list ')'
	  { $$= new Item_func_char(*$3); }
	| COALESCE '(' expr_list ')'
	  { $$= new Item_func_coalesce(* $3); }
	| CONCAT '(' expr_list ')'
	  { $$= new Item_func_concat(* $3); }
	| CONCAT_WS '(' expr ',' expr_list ')'
	  { $$= new Item_func_concat_ws($3, *$5); }
	| CURDATE optional_braces
	  { $$= new Item_func_curdate(); current_thd->safe_to_cache_query=0; }
	| CURTIME optional_braces
	  { $$= new Item_func_curtime(); current_thd->safe_to_cache_query=0; }
	| CURTIME '(' expr ')'
	  { 
	    $$= new Item_func_curtime($3); 
	    current_thd->safe_to_cache_query=0;
	  }
	| DATE_ADD_INTERVAL '(' expr ',' INTERVAL_SYM expr interval ')'
	  { $$= new Item_date_add_interval($3,$6,$7,0); }
	| DATE_SUB_INTERVAL '(' expr ',' INTERVAL_SYM expr interval ')'
	  { $$= new Item_date_add_interval($3,$6,$7,1); }
	| DATABASE '(' ')'
	  { 
	    $$= new Item_func_database();
            current_thd->safe_to_cache_query=0; 
	  }
	| ELT_FUNC '(' expr ',' expr_list ')'
	  { $$= new Item_func_elt($3, *$5); }
	| MAKE_SET_SYM '(' expr ',' expr_list ')'
	  { $$= new Item_func_make_set($3, *$5); }
	| ENCRYPT '(' expr ')'
	  {
	    $$= new Item_func_encrypt($3);
	    current_thd->safe_to_cache_query=0; 
	  }
	| ENCRYPT '(' expr ',' expr ')'   { $$= new Item_func_encrypt($3,$5); }
	| DECODE_SYM '(' expr ',' TEXT_STRING ')'
	  { $$= new Item_func_decode($3,$5.str); }
	| ENCODE_SYM '(' expr ',' TEXT_STRING ')'
	 { $$= new Item_func_encode($3,$5.str); }
	| DES_DECRYPT_SYM '(' expr ')'
        { $$= new Item_func_des_decrypt($3); }
	| DES_DECRYPT_SYM '(' expr ',' expr ')'
        { $$= new Item_func_des_decrypt($3,$5); }
	| DES_ENCRYPT_SYM '(' expr ')'
        { $$= new Item_func_des_encrypt($3); }
	| DES_ENCRYPT_SYM '(' expr ',' expr ')'
        { $$= new Item_func_des_encrypt($3,$5); }
	| EXPORT_SET '(' expr ',' expr ',' expr ')'
		{ $$= new Item_func_export_set($3, $5, $7); }
	| EXPORT_SET '(' expr ',' expr ',' expr ',' expr ')'
		{ $$= new Item_func_export_set($3, $5, $7, $9); }
	| EXPORT_SET '(' expr ',' expr ',' expr ',' expr ',' expr ')'
		{ $$= new Item_func_export_set($3, $5, $7, $9, $11); }
	| FORMAT_SYM '(' expr ',' NUM ')'
	  { $$= new Item_func_format($3,atoi($5.str)); }
	| FROM_UNIXTIME '(' expr ')'
	  { $$= new Item_func_from_unixtime($3); }
	| FROM_UNIXTIME '(' expr ',' expr ')'
	  {
	    $$= new Item_func_date_format (new Item_func_from_unixtime($3),$5,0);
	  }
	| FIELD_FUNC '(' expr ',' expr_list ')'
	  { $$= new Item_func_field($3, *$5); }
	| HOUR_SYM '(' expr ')'
	  { $$= new Item_func_hour($3); }
	| IF '(' expr ',' expr ',' expr ')'
	  { $$= new Item_func_if($3,$5,$7); }
	| INSERT '(' expr ',' expr ',' expr ',' expr ')'
	  { $$= new Item_func_insert($3,$5,$7,$9); }
	| INTERVAL_SYM expr interval '+' expr
	  /* we cannot put interval before - */
	  { $$= new Item_date_add_interval($5,$2,$3,0); }
	| INTERVAL_SYM '(' expr ',' expr_list ')'
	  { $$= new Item_func_interval($3,* $5); }
	| LAST_INSERT_ID '(' ')'
	  {
	    $$= new Item_int((char*) "last_insert_id()",
			     current_thd->insert_id(),21);
	    current_thd->safe_to_cache_query=0;
	  }
	| LAST_INSERT_ID '(' expr ')'
	  {
	    $$= new Item_func_set_last_insert_id($3);
	    current_thd->safe_to_cache_query=0;
	  }
	| LEFT '(' expr ',' expr ')'
	  { $$= new Item_func_left($3,$5); }
	| LOCATE '(' expr ',' expr ')'
	  { $$= new Item_func_locate($5,$3); }
	| LOCATE '(' expr ',' expr ',' expr ')'
	  { $$= new Item_func_locate($5,$3,$7); }
 	| GREATEST_SYM '(' expr ',' expr_list ')'
	  { $5->push_front($3); $$= new Item_func_max(*$5); }
	| LEAST_SYM '(' expr ',' expr_list ')'
	  { $5->push_front($3); $$= new Item_func_min(*$5); }
	| LOG_SYM '(' expr ')'
		{ $$= new Item_func_log($3); }
	| LOG_SYM '(' expr ',' expr ')'
		{ $$= new Item_func_log($3, $5); }
	| MASTER_POS_WAIT '(' expr ',' expr ')'
	  { 
	    $$= new Item_master_pos_wait($3, $5);
	    current_thd->safe_to_cache_query=0; 
	  }
	| MASTER_POS_WAIT '(' expr ',' expr ',' expr ')'
	  { 
	    $$= new Item_master_pos_wait($3, $5, $7);
	    current_thd->safe_to_cache_query=0; 
	  }
	| MINUTE_SYM '(' expr ')'
	  { $$= new Item_func_minute($3); }
	| MONTH_SYM '(' expr ')'
	  { $$= new Item_func_month($3); }
	| NOW_SYM optional_braces
	  { $$= new Item_func_now(); current_thd->safe_to_cache_query=0;}
	| NOW_SYM '(' expr ')'
	  { $$= new Item_func_now($3); current_thd->safe_to_cache_query=0;}
	| PASSWORD '(' expr ')'
	  {
	    $$= new Item_func_password($3);
	   }
	| POSITION_SYM '(' no_in_expr IN_SYM expr ')'
	  { $$ = new Item_func_locate($5,$3); }
	| RAND '(' expr ')'
	  { $$= new Item_func_rand($3); current_thd->safe_to_cache_query=0;}
	| RAND '(' ')'
	  { $$= new Item_func_rand(); current_thd->safe_to_cache_query=0;}
	| REPLACE '(' expr ',' expr ',' expr ')'
	  { $$= new Item_func_replace($3,$5,$7); }
	| RIGHT '(' expr ',' expr ')'
	  { $$= new Item_func_right($3,$5); }
	| ROUND '(' expr ')'
	  { $$= new Item_func_round($3, new Item_int((char*)"0",0,1),0); }
	| ROUND '(' expr ',' expr ')' { $$= new Item_func_round($3,$5,0); }
	| SECOND_SYM '(' expr ')'
	  { $$= new Item_func_second($3); }
	| SUBSTRING '(' expr ',' expr ',' expr ')'
	  { $$= new Item_func_substr($3,$5,$7); }
	| SUBSTRING '(' expr ',' expr ')'
	  { $$= new Item_func_substr($3,$5); }
	| SUBSTRING '(' expr FROM expr FOR_SYM expr ')'
	  { $$= new Item_func_substr($3,$5,$7); }
	| SUBSTRING '(' expr FROM expr ')'
	  { $$= new Item_func_substr($3,$5); }
	| SUBSTRING_INDEX '(' expr ',' expr ',' expr ')'
	  { $$= new Item_func_substr_index($3,$5,$7); }
	| TRIM '(' expr ')'
	  { $$= new Item_func_trim($3,new Item_string(" ",1)); }
	| TRIM '(' LEADING opt_pad FROM expr ')'
	  { $$= new Item_func_ltrim($6,$4); }
	| TRIM '(' TRAILING opt_pad FROM expr ')'
	  { $$= new Item_func_rtrim($6,$4); }
	| TRIM '(' BOTH opt_pad FROM expr ')'
	  { $$= new Item_func_trim($6,$4); }
	| TRIM '(' expr FROM expr ')'
	  { $$= new Item_func_trim($5,$3); }
	| TRUNCATE_SYM '(' expr ',' expr ')'
	  { $$= new Item_func_round($3,$5,1); }
	| UDA_CHAR_SUM '(' udf_expr_list ')'
	  {
	    if ($3 != NULL)
	      $$ = new Item_sum_udf_str($1, *$3);
	    else
	      $$ = new Item_sum_udf_str($1);
	  }
	| UDA_FLOAT_SUM '(' udf_expr_list ')'
	  {
	    if ($3 != NULL)
	      $$ = new Item_sum_udf_float($1, *$3);
	    else
	      $$ = new Item_sum_udf_float($1);
	  }
	| UDA_INT_SUM '(' udf_expr_list ')'
	  {
	    if ($3 != NULL)
	      $$ = new Item_sum_udf_int($1, *$3);
	    else
	      $$ = new Item_sum_udf_int($1);
	  }
	| UDF_CHAR_FUNC '(' udf_expr_list ')'
	  {
	    if ($3 != NULL)
	      $$ = new Item_func_udf_str($1, *$3);
	    else
	      $$ = new Item_func_udf_str($1);
	  }
	| UDF_FLOAT_FUNC '(' udf_expr_list ')'
	  {
	    if ($3 != NULL)
	      $$ = new Item_func_udf_float($1, *$3);
	    else
	      $$ = new Item_func_udf_float($1);
	  }
	| UDF_INT_FUNC '(' udf_expr_list ')'
	  {
	    if ($3 != NULL)
	      $$ = new Item_func_udf_int($1, *$3);
	    else
	      $$ = new Item_func_udf_int($1);
	  }
	| UNIQUE_USERS '(' text_literal ',' NUM ',' NUM ',' expr_list ')'
	  { 
            $$= new Item_func_unique_users($3,atoi($5.str),atoi($7.str), * $9);
	  }
	| UNIX_TIMESTAMP '(' ')'
	  {
	    $$= new Item_func_unix_timestamp();
	    current_thd->safe_to_cache_query=0;
	  }
	| UNIX_TIMESTAMP '(' expr ')'
	  { $$= new Item_func_unix_timestamp($3); }
	| USER '(' ')'
	  { $$= new Item_func_user(); current_thd->safe_to_cache_query=0; }
	| WEEK_SYM '(' expr ')'
	  { 
	    LEX *lex=Lex;
	    $$= new Item_func_week($3,new Item_int((char*) "0", 
                                   lex->thd->variables.default_week_format,1));
	  }
	| WEEK_SYM '(' expr ',' expr ')'
	  { $$= new Item_func_week($3,$5); }
	| YEAR_SYM '(' expr ')'
	  { $$= new Item_func_year($3); }
	| YEARWEEK '(' expr ')'
	  { $$= new Item_func_yearweek($3,new Item_int((char*) "0",0,1)); }
	| YEARWEEK '(' expr ',' expr ')'
	  { $$= new Item_func_yearweek($3, $5); }
	| BENCHMARK_SYM '(' ULONG_NUM ',' expr ')'
	  { 
	    $$=new Item_func_benchmark($3,$5);
	    current_thd->safe_to_cache_query=0;
	  }
	| EXTRACT_SYM '(' interval FROM expr ')'
	{ $$=new Item_extract( $3, $5); };

udf_expr_list:
	/* empty */	{ $$= NULL; }
	| expr_list	{ $$= $1;};

sum_expr:
	AVG_SYM '(' in_sum_expr ')'
	  { $$=new Item_sum_avg($3); }
	| BIT_AND  '(' in_sum_expr ')'
	  { $$=new Item_sum_and($3); }
	| BIT_OR  '(' in_sum_expr ')'
	  { $$=new Item_sum_or($3); }
	| COUNT_SYM '(' opt_all '*' ')'
	  { $$=new Item_sum_count(new Item_int((int32) 0L,1)); }
	| COUNT_SYM '(' in_sum_expr ')'
	  { $$=new Item_sum_count($3); }
	| COUNT_SYM '(' DISTINCT
	  { Select->in_sum_expr++; }
	   expr_list
	  { Select->in_sum_expr--; }
	  ')'
	  { $$=new Item_sum_count_distinct(* $5); }
	| GROUP_UNIQUE_USERS '(' text_literal ',' NUM ',' NUM ',' in_sum_expr ')'
	  { $$= new Item_sum_unique_users($3,atoi($5.str),atoi($7.str),$9); }
	| MIN_SYM '(' in_sum_expr ')'
	  { $$=new Item_sum_min($3); }
	| MAX_SYM '(' in_sum_expr ')'
	  { $$=new Item_sum_max($3); }
	| STD_SYM '(' in_sum_expr ')'
	  { $$=new Item_sum_std($3); }
	| SUM_SYM '(' in_sum_expr ')'
	  { $$=new Item_sum_sum($3); };

in_sum_expr:
	opt_all
	{ Select->in_sum_expr++; }
	expr
	{
	  Select->in_sum_expr--;
	  $$=$3;
	};

cast_type:
	BINARY 			{ $$=ITEM_CAST_BINARY; }
	| CHAR_SYM		{ $$=ITEM_CAST_CHAR; }
	| SIGNED_SYM		{ $$=ITEM_CAST_SIGNED_INT; }
	| SIGNED_SYM INT_SYM	{ $$=ITEM_CAST_SIGNED_INT; }
	| UNSIGNED		{ $$=ITEM_CAST_UNSIGNED_INT; }
	| UNSIGNED INT_SYM	{ $$=ITEM_CAST_UNSIGNED_INT; }
	| DATE_SYM		{ $$=ITEM_CAST_DATE; }
	| TIME_SYM		{ $$=ITEM_CAST_TIME; }
	| DATETIME		{ $$=ITEM_CAST_DATETIME; }
	;

expr_list:
	{ Select->expr_list.push_front(new List<Item>); }
	expr_list2
	{ $$= Select->expr_list.pop(); };

expr_list2:
	expr { Select->expr_list.head()->push_back($1); }
	| expr_list2 ',' expr { Select->expr_list.head()->push_back($3); };

ident_list_arg:
          ident_list          { $$= $1; }
        | '(' ident_list ')'  { $$= $2; };

ident_list:
        { Select->expr_list.push_front(new List<Item>); }
        ident_list2
        { $$= Select->expr_list.pop(); };

ident_list2:
        simple_ident { Select->expr_list.head()->push_back($1); }
        | ident_list2 ',' simple_ident { Select->expr_list.head()->push_back($3); };

opt_expr:
	/* empty */      { $$= NULL; }
	| expr         	 { $$= $1; };

opt_else:
	/* empty */    { $$= NULL; }
	| ELSE expr    { $$= $2; };

when_list:
        { Select->when_list.push_front(new List<Item>); }
	when_list2
	{ $$= Select->when_list.pop(); };

when_list2:
	expr THEN_SYM expr
	  {
	    SELECT_LEX *sel=Select;	    
	    sel->when_list.head()->push_back($1);
	    sel->when_list.head()->push_back($3);
	}
	| when_list2 WHEN_SYM expr THEN_SYM expr
	  {
	    SELECT_LEX *sel=Select;
	    sel->when_list.head()->push_back($3);
	    sel->when_list.head()->push_back($5);
	  };

opt_pad:
	/* empty */ { $$=new Item_string(" ",1); }
	| expr	    { $$=$1; };

join_table_list:
	'(' join_table_list ')'	{ $$=$2; }
	| join_table		{ $$=$1; }
	| join_table_list ',' join_table_list { $$=$3; }
	| join_table_list normal_join join_table_list { $$=$3; }
	| join_table_list STRAIGHT_JOIN join_table_list
	  { $$=$3 ; $1->next->straight=1; }
	| join_table_list normal_join join_table_list ON expr
	  { add_join_on($3,$5); $$=$3; }
	| join_table_list normal_join join_table_list
	  USING 
	  {
	    SELECT_LEX *sel=Select;
	    sel->db1=$1->db; sel->table1=$1->alias;
	    sel->db2=$3->db; sel->table2=$3->alias;
	  }
	  '(' using_list ')'
	  { add_join_on($3,$7); $$=$3; }

	| join_table_list LEFT opt_outer JOIN_SYM join_table_list ON expr
	  { add_join_on($5,$7); $5->outer_join|=JOIN_TYPE_LEFT; $$=$5; }
	| join_table_list LEFT opt_outer JOIN_SYM join_table_list
	  {
	    SELECT_LEX *sel=Select;
	    sel->db1=$1->db; sel->table1=$1->alias;
	    sel->db2=$5->db; sel->table2=$5->alias;
	  }
	  USING '(' using_list ')'
	  { add_join_on($5,$9); $5->outer_join|=JOIN_TYPE_LEFT; $$=$5; }
	| join_table_list NATURAL LEFT opt_outer JOIN_SYM join_table_list
	  {
	    add_join_natural($1,$1->next);
	    $1->next->outer_join|=JOIN_TYPE_LEFT;
	    $$=$6;
	  }
	| join_table_list RIGHT opt_outer JOIN_SYM join_table_list ON expr
	  { add_join_on($1,$7); $1->outer_join|=JOIN_TYPE_RIGHT; $$=$5; }
	| join_table_list RIGHT opt_outer JOIN_SYM join_table_list
	  {
	    SELECT_LEX *sel=Select;
	    sel->db1=$1->db; sel->table1=$1->alias;
	    sel->db2=$5->db; sel->table2=$5->alias;
	  }
	  USING '(' using_list ')'
	  { add_join_on($1,$9); $1->outer_join|=JOIN_TYPE_RIGHT; $$=$5; }
	| join_table_list NATURAL RIGHT opt_outer JOIN_SYM join_table_list
	  {
	    add_join_natural($1->next,$1);
	    $1->outer_join|=JOIN_TYPE_RIGHT;
	    $$=$6;
	  }
	| join_table_list NATURAL JOIN_SYM join_table_list
	  { add_join_natural($1,$1->next); $$=$4; };

normal_join:
	JOIN_SYM		{}
	| INNER_SYM JOIN_SYM	{}
	| CROSS JOIN_SYM	{}
	;

join_table:
	{
	  SELECT_LEX *sel=Select;
	  sel->use_index_ptr=sel->ignore_index_ptr=0;
	  sel->table_join_options= 0;
	}
        table_ident opt_table_alias opt_key_definition
	{
	  SELECT_LEX *sel=Select;
	  if (!($$=add_table_to_list($2, $3, sel->table_join_options,
				     TL_UNLOCK, sel->use_index_ptr,
	                             sel->ignore_index_ptr)))
	    YYABORT;
	}
	| '{' ident join_table LEFT OUTER JOIN_SYM join_table ON expr '}'
	  { add_join_on($7,$9); $7->outer_join|=JOIN_TYPE_LEFT; $$=$7; };

opt_outer:
	/* empty */	{}
	| OUTER		{};

opt_key_definition:
	/* empty */	{}
	| USE_SYM    key_usage_list
          {
	    SELECT_LEX *sel=Select;
	    sel->use_index= *$2;
	    sel->use_index_ptr= &sel->use_index;
	  }
	| FORCE_SYM key_usage_list
          {
	    SELECT_LEX *sel=Select;
	    sel->use_index= *$2;
	    sel->use_index_ptr= &sel->use_index;
	    sel->table_join_options|= TL_OPTION_FORCE_INDEX;
	  }
	| IGNORE_SYM key_usage_list
	  {
	    SELECT_LEX *sel=Select;
	    sel->ignore_index= *$2;
	    sel->ignore_index_ptr= &sel->ignore_index;
	  }
	;

key_usage_list:
	key_or_index { Select->interval_list.empty(); } '(' key_usage_list2 ')'
        { $$= &Select->interval_list; };

key_usage_list2:
	key_usage_list2 ',' ident
        { Select->interval_list.push_back(new String((const char*) $3.str,$3.length)); }
	| ident
        { Select->interval_list.push_back(new String((const char*) $1.str,$1.length)); }
	| PRIMARY_SYM
        { Select->interval_list.push_back(new String("PRIMARY",7)); };

using_list:
	ident
	  {
	    SELECT_LEX *sel=Select;
	    if (!($$= new Item_func_eq(new Item_field(sel->db1,sel->table1, $1.str), new Item_field(sel->db2,sel->table2,$1.str))))
	      YYABORT;
	  }
	| using_list ',' ident
	  {
	    SELECT_LEX *sel=Select;
	    if (!($$= new Item_cond_and(new Item_func_eq(new Item_field(sel->db1,sel->table1,$3.str), new Item_field(sel->db2,sel->table2,$3.str)), $1)))
	      YYABORT;
	  };

interval:
	 DAY_HOUR_SYM		{ $$=INTERVAL_DAY_HOUR; }
	| DAY_MINUTE_SYM	{ $$=INTERVAL_DAY_MINUTE; }
	| DAY_SECOND_SYM	{ $$=INTERVAL_DAY_SECOND; }
	| DAY_SYM		{ $$=INTERVAL_DAY; }
	| HOUR_MINUTE_SYM	{ $$=INTERVAL_HOUR_MINUTE; }
	| HOUR_SECOND_SYM	{ $$=INTERVAL_HOUR_SECOND; }
	| HOUR_SYM		{ $$=INTERVAL_HOUR; }
	| MINUTE_SECOND_SYM	{ $$=INTERVAL_MINUTE_SECOND; }
	| MINUTE_SYM		{ $$=INTERVAL_MINUTE; }
	| MONTH_SYM		{ $$=INTERVAL_MONTH; }
	| SECOND_SYM		{ $$=INTERVAL_SECOND; }
	| YEAR_MONTH_SYM	{ $$=INTERVAL_YEAR_MONTH; }
	| YEAR_SYM		{ $$=INTERVAL_YEAR; };

table_alias:
	/* empty */
	| AS
	| EQ;

opt_table_alias:
	/* empty */		{ $$=0; }
	| table_alias ident
	  { $$= (LEX_STRING*) sql_memdup(&$2,sizeof(LEX_STRING)); };

opt_all:
	/* empty */
	| ALL
	;

where_clause:
	/* empty */  { Select->where= 0; }
	| WHERE expr
	  {
	    Select->where= $2;
	    if ($2)
	      $2->top_level_item();
	  }
	  ;

having_clause:
	/* empty */
	| HAVING { Select->create_refs=1; } expr
	{
	  SELECT_LEX *sel=Select;
	  sel->having= $3;
	  sel->create_refs=0;
	  if ($3)
	    $3->top_level_item();
	}
	;

opt_escape:
	ESCAPE_SYM TEXT_STRING	{ $$= $2.str; }
	| /* empty */		{ $$= (char*) "\\"; };


/*
   group by statement in select
*/

group_clause:
	/* empty */
	| GROUP BY group_list olap_opt;

group_list:
	group_list ',' order_ident order_dir
	  { if (add_group_to_list($3,(bool) $4)) YYABORT; }
	| order_ident order_dir
	  { if (add_group_to_list($1,(bool) $2)) YYABORT; };

olap_opt:
	/* empty */ {}
	| WITH CUBE_SYM
          {
	    LEX *lex=Lex;
	    net_printf(&lex->thd->net, ER_NOT_SUPPORTED_YET, "CUBE");
	    YYABORT;	/* To be deleted in 4.1 */
	  }
	| WITH ROLLUP_SYM
          {
	    LEX *lex=Lex;
	    net_printf(&lex->thd->net, ER_NOT_SUPPORTED_YET, "ROLLUP");
	    YYABORT;	/* To be deleted in 4.1 */
	  }
	;

/*
   Order by statement in select
*/

opt_order_clause:
	/* empty */
	| order_clause;

order_clause:
	ORDER_SYM BY 
        { 
	  LEX *lex=Lex;
	  if (lex->select->olap != UNSPECIFIED_OLAP_TYPE)
	  {
	    net_printf(&lex->thd->net, ER_WRONG_USAGE,
		       "CUBE/ROLLUP",
		       "ORDER BY");
	    YYABORT;
	  }
	  lex->select->sort_default=1;
	} order_list;

order_list:
	order_list ',' order_ident order_dir
	  { if (add_order_to_list($3,(bool) $4)) YYABORT; }
	| order_ident order_dir
	  { if (add_order_to_list($1,(bool) $2)) YYABORT; };

order_dir:
	/* empty */ { $$ =  1; }
	| ASC  { $$ =1; }
	| DESC { $$ =0; };


limit_clause:
	/* empty */ {}
	| LIMIT 
	  {
	    LEX *lex=Lex;
	    if (lex->select->olap != UNSPECIFIED_OLAP_TYPE)
	    {
	      net_printf(&lex->thd->net, ER_WRONG_USAGE, "CUBE/ROLLUP",
		        "LIMIT");
	      YYABORT;
	    }
	  }
	  limit_options
	  {}
	  ;

limit_options:
	ULONG_NUM
	  {
            SELECT_LEX *sel= Select;
            sel->select_limit= $1;
            sel->offset_limit= 0L;
	  }
	| ULONG_NUM ',' ULONG_NUM
	  {
	    SELECT_LEX *sel= Select;
	    sel->select_limit= $3;
	    sel->offset_limit= $1;
	  }
	| ULONG_NUM OFFSET_SYM ULONG_NUM
	  {
	    SELECT_LEX *sel= Select;
	    sel->select_limit= $1;
	    sel->offset_limit= $3;
	  }
	;

delete_limit_clause:
	/* empty */
	{
	  LEX *lex=Lex;
	  lex->select->select_limit= HA_POS_ERROR;
	}
	| LIMIT ulonglong_num
	{ Select->select_limit= (ha_rows) $2; };

ULONG_NUM:
	NUM	    { $$= strtoul($1.str,NULL,10); }
	| LONG_NUM  { $$= (ulong) strtoll($1.str,NULL,10); }
	| ULONGLONG_NUM { $$= (ulong) strtoull($1.str,NULL,10); }
	| REAL_NUM  { $$= strtoul($1.str,NULL,10); }
	| FLOAT_NUM { $$= strtoul($1.str,NULL,10); };

ulonglong_num:
	NUM	    { $$= (ulonglong) strtoul($1.str,NULL,10); }
	| ULONGLONG_NUM { $$= strtoull($1.str,NULL,10); }
	| LONG_NUM  { $$= (ulonglong) strtoll($1.str,NULL,10); }
	| REAL_NUM  { $$= strtoull($1.str,NULL,10); }
	| FLOAT_NUM { $$= strtoull($1.str,NULL,10); };

procedure_clause:
	/* empty */
	| PROCEDURE ident			/* Procedure name */
	  {
	    LEX *lex=Lex;
	    lex->proc_list.elements=0;
	    lex->proc_list.first=0;
	    lex->proc_list.next= (byte**) &lex->proc_list.first;
	    if (add_proc_to_list(lex->thd, new Item_field(NULL,NULL,$2.str)))
	      YYABORT;
	    current_thd->safe_to_cache_query=0;
	  }
	  '(' procedure_list ')';


procedure_list:
	/* empty */ {}
	| procedure_list2 {};

procedure_list2:
	procedure_list2 ',' procedure_item
	| procedure_item;

procedure_item:
	  remember_name expr
	  {
	    LEX *lex= Lex;
	    if (add_proc_to_list(lex->thd, $2))
	      YYABORT;
	    if (!$2->name)
	      $2->set_name($1,(uint) ((char*) lex->tok_end - $1));
	  };

opt_into:
	INTO OUTFILE TEXT_STRING
	{
	  THD *thd= current_thd;
	  thd->safe_to_cache_query= 0; 
	  if (!(thd->lex.exchange= new sql_exchange($3.str,0)))
	    YYABORT;
	}
	opt_field_term opt_line_term
	| INTO DUMPFILE TEXT_STRING
	{
	  THD *thd= current_thd;
	  thd->safe_to_cache_query= 0;
	  if (!(thd->lex.exchange= new sql_exchange($3.str,1)))
	    YYABORT;
	};

/*
  DO statement
*/

do:	DO_SYM 
	{
	  LEX *lex=Lex;
	  lex->sql_command = SQLCOM_DO;
	  if (!(lex->insert_list = new List_item))
	    YYABORT;
	}
	values
	{}
	;

/*
  Drop : delete tables or index
*/

drop:
	DROP opt_temporary TABLE_SYM if_exists table_list opt_restrict
	{
	  LEX *lex=Lex;
	  lex->sql_command = SQLCOM_DROP_TABLE;
	  lex->drop_temporary= $2;
	  lex->drop_if_exists= $4;
	}
	| DROP INDEX ident ON table_ident {}
	  {
	     LEX *lex=Lex;
	     lex->sql_command= SQLCOM_DROP_INDEX;
	     lex->drop_list.empty();
	     lex->drop_list.push_back(new Alter_drop(Alter_drop::KEY,
						     $3.str));
	     if (!add_table_to_list($5, NULL, TL_OPTION_UPDATING))
	      YYABORT;
	  }
	| DROP DATABASE if_exists ident
	  {
	    LEX *lex=Lex;
	    lex->sql_command= SQLCOM_DROP_DB;
	    lex->drop_if_exists=$3;
	    lex->name=$4.str;
	 }
	| DROP UDF_SYM ident
	  {
	    LEX *lex=Lex;
	    lex->sql_command = SQLCOM_DROP_FUNCTION;
	    lex->udf.name=$3.str;
	  };


table_list:
	table_name
	| table_list ',' table_name;

table_name:
	table_ident
	{ if (!add_table_to_list($1,NULL,TL_OPTION_UPDATING)) YYABORT; };

if_exists:
	/* empty */ { $$= 0; }
	| IF EXISTS { $$= 1; }
	;

opt_temporary:
	/* empty */ { $$= 0; }
	| TEMPORARY { $$= 1; }
	;
/*
** Insert : add new data to table
*/

insert:
	INSERT { Lex->sql_command = SQLCOM_INSERT; } insert_lock_option
	opt_ignore insert2
	{
	  set_lock_for_tables($3);
	  Lex->select= &Lex->select_lex;
	}
	insert_field_spec
	{}
	;

replace:
	REPLACE
	{
	  LEX *lex=Lex;
	  lex->sql_command = SQLCOM_REPLACE;
	  lex->duplicates= DUP_REPLACE;
	}
	replace_lock_option insert2
	{
	  set_lock_for_tables($3);
          Lex->select= &Lex->select_lex;
	}
	insert_field_spec
	{}
	;

insert_lock_option:
	/* empty */	{ $$= TL_WRITE_CONCURRENT_INSERT; }
	| LOW_PRIORITY	{ $$= TL_WRITE_LOW_PRIORITY; }
	| DELAYED_SYM	{ $$= TL_WRITE_DELAYED; }
	| HIGH_PRIORITY { $$= TL_WRITE; }
	;

replace_lock_option:
	opt_low_priority { $$= $1; }
	| DELAYED_SYM	 { $$= TL_WRITE_DELAYED; };

insert2:
	INTO insert_table {}
	| insert_table {};

insert_table:
	table_name
	{
	  LEX *lex=Lex;
	  lex->field_list.empty();
	  lex->many_values.empty();
	  lex->insert_list=0;
	};

insert_field_spec:
	insert_values {}
	| '(' ')' insert_values {}
	| '(' fields ')' insert_values {}
	| SET
	  {
	    LEX *lex=Lex;
	    if (!(lex->insert_list = new List_item) ||
		lex->many_values.push_back(lex->insert_list))
	      YYABORT;
	   }
	   ident_eq_list;

opt_field_spec:
	/* empty */	  { }
	| '(' fields ')'  { }
	| '(' ')'	  { };

fields:
	fields ',' insert_ident { Lex->field_list.push_back($3); }
	| insert_ident		{ Lex->field_list.push_back($1); };

insert_values:
	VALUES	values_list  {}
	|     create_select     { Select->braces= 0;} opt_union {}
	| '(' create_select ')' { Select->braces= 1;} union_opt {}
        ;

values_list:
	values_list ','  no_braces
	| no_braces;

ident_eq_list:
	ident_eq_list ',' ident_eq_value
	|
	ident_eq_value;

ident_eq_value:
	simple_ident equal expr_or_default
	 {
	  LEX *lex=Lex;
	  if (lex->field_list.push_back($1) ||
	      lex->insert_list->push_back($3))
	    YYABORT;
	 };

equal:	EQ		{}
	| SET_VAR	{}
	;

opt_equal:
	/* empty */	{}
	| equal		{}
	;

no_braces:
	 '('
	 {
	    if (!(Lex->insert_list = new List_item))
	      YYABORT;
	 }
	 opt_values ')'
	 {
	  LEX *lex=Lex;
	  if (lex->many_values.push_back(lex->insert_list))
	    YYABORT;
	 };

opt_values:
	/* empty */ {}
	| values;

values:
	values ','  expr_or_default
	{
	  if (Lex->insert_list->push_back($3))
	    YYABORT;
	}
	| expr_or_default
	  {
	    if (Lex->insert_list->push_back($1))
	      YYABORT;
	  }
	;

expr_or_default:
	expr 	  { $$= $1;}
	| DEFAULT {$$= new Item_default(); }
	;

/* Update rows in a table */

update:
        UPDATE_SYM 
	{ 
	  LEX *lex=Lex;
          lex->sql_command = SQLCOM_UPDATE;
          lex->select->order_list.elements=0;
          lex->select->order_list.first=0;
          lex->select->order_list.next= (byte**) &lex->select->order_list.first;
        }
        opt_low_priority opt_ignore join_table_list
	SET update_list where_clause opt_order_clause delete_limit_clause
	{
	  set_lock_for_tables($3);
	}
	;

update_list:
	update_list ',' simple_ident equal expr
	{
	  if (add_item_to_list($3) || add_value_to_list($5))
	    YYABORT;
	}
	| simple_ident equal expr
	  {
	    if (add_item_to_list($1) || add_value_to_list($3))
	      YYABORT;
	  };

opt_low_priority:
	/* empty */	{ $$= current_thd->update_lock_default; }
	| LOW_PRIORITY	{ $$= TL_WRITE_LOW_PRIORITY; };

/* Delete rows from a table */

delete:
	DELETE_SYM
	{ 
	  LEX *lex=Lex;
	  lex->sql_command= SQLCOM_DELETE; lex->select->options=0;
	  lex->lock_option= lex->thd->update_lock_default;
	  lex->select->order_list.elements=0;
	  lex->select->order_list.first=0;
	  lex->select->order_list.next= (byte**) &lex->select->order_list.first;
	}
	opt_delete_options single_multi {}
	;

single_multi:
 	FROM table_ident
	{
	  if (!add_table_to_list($2, NULL, TL_OPTION_UPDATING,
				 Lex->lock_option))
	    YYABORT;
	}
	where_clause opt_order_clause
	delete_limit_clause {}
	| table_wild_list
	  { mysql_init_multi_delete(Lex); }
          FROM join_table_list where_clause
	| FROM table_wild_list
	  { mysql_init_multi_delete(Lex); }
	  USING join_table_list where_clause
	  {}
	;

table_wild_list:
	  table_wild_one {}
	  | table_wild_list ',' table_wild_one {};

table_wild_one:
	 ident opt_wild
	 {
	   if (!add_table_to_list(new Table_ident($1), NULL,
				  TL_OPTION_UPDATING, Lex->lock_option))
	     YYABORT;
         }
	 | ident '.' ident opt_wild
	   {
	     if (!add_table_to_list(new Table_ident($1,$3,0), NULL,
				    TL_OPTION_UPDATING,
				    Lex->lock_option))
	      YYABORT;
	   }
	;

opt_wild:
	/* empty */	{} 
	| '.' '*'	{};


opt_delete_options:
	/* empty */	{}
	| opt_delete_option opt_delete_options {};

opt_delete_option:
	QUICK		{ Select->options|= OPTION_QUICK; }
	| LOW_PRIORITY	{ Lex->lock_option= TL_WRITE_LOW_PRIORITY; };

truncate:
	TRUNCATE_SYM opt_table_sym table_name
	{
	  LEX* lex = Lex;
	  lex->sql_command= SQLCOM_TRUNCATE;
	  lex->select->options=0;
	  lex->select->order_list.elements=0;
          lex->select->order_list.first=0;
          lex->select->order_list.next= (byte**) &lex->select->order_list.first;
	}
	;

opt_table_sym:
	/* empty */
	| TABLE_SYM;
 
/* Show things */

show:	SHOW { Lex->wild=0;} show_param
	{}
	;

show_param:
	DATABASES wild
	  { Lex->sql_command= SQLCOM_SHOW_DATABASES; }
	| TABLES opt_db wild
	  {
	    LEX *lex=Lex;
	    lex->sql_command= SQLCOM_SHOW_TABLES;
	    lex->select->db= $2; lex->select->options=0;
	   }
	| TABLE_SYM STATUS_SYM opt_db wild
	  {
	    LEX *lex=Lex;
	    lex->sql_command= SQLCOM_SHOW_TABLES;
	    lex->select->options|= SELECT_DESCRIBE;
	    lex->select->db= $3;
	  }
	| OPEN_SYM TABLES opt_db wild
	  {
	    LEX *lex=Lex;
	    lex->sql_command= SQLCOM_SHOW_OPEN_TABLES;
	    lex->select->db= $3;
	    lex->select->options=0;
	  }
	| opt_full COLUMNS from_or_in table_ident opt_db wild
	  {
	    Lex->sql_command= SQLCOM_SHOW_FIELDS;
	    if ($5)
	      $4->change_db($5);
	    if (!add_table_to_list($4, NULL, 0))
	      YYABORT;
	  }
        | NEW_SYM MASTER_SYM FOR_SYM SLAVE WITH MASTER_LOG_FILE_SYM EQ 
	  TEXT_STRING AND MASTER_LOG_POS_SYM EQ ulonglong_num
	  AND MASTER_SERVER_ID_SYM EQ
	ULONG_NUM
          {
	    Lex->sql_command = SQLCOM_SHOW_NEW_MASTER;
	    Lex->mi.log_file_name = $8.str;
	    Lex->mi.pos = $12;
	    Lex->mi.server_id = $16;
          }
        | MASTER_SYM LOGS_SYM
          {
	    Lex->sql_command = SQLCOM_SHOW_BINLOGS;
          }
        | SLAVE HOSTS_SYM
          {
	    Lex->sql_command = SQLCOM_SHOW_SLAVE_HOSTS;
          }
        | BINLOG_SYM EVENTS_SYM binlog_in binlog_from
          {
	    LEX *lex=Lex;
	    lex->sql_command = SQLCOM_SHOW_BINLOG_EVENTS;
	    lex->select->select_limit= lex->thd->variables.select_limit;
	    lex->select->offset_limit= 0L;
          } limit_clause
	| keys_or_index FROM table_ident opt_db
	  {
	    Lex->sql_command= SQLCOM_SHOW_KEYS;
	    if ($4)
	      $3->change_db($4);
	    if (!add_table_to_list($3, NULL, 0))
	      YYABORT;
	  }
	| STATUS_SYM wild
	  { Lex->sql_command= SQLCOM_SHOW_STATUS; }
        | INNOBASE_SYM STATUS_SYM
          { Lex->sql_command = SQLCOM_SHOW_INNODB_STATUS;}
	| opt_full PROCESSLIST_SYM
	  { Lex->sql_command= SQLCOM_SHOW_PROCESSLIST;}
	| opt_var_type VARIABLES wild
	{
	    THD *thd= current_thd;
	    thd->lex.sql_command= SQLCOM_SHOW_VARIABLES;
	    thd->lex.option_type= (enum_var_type) $1;
	  }
	| LOGS_SYM
	  { Lex->sql_command= SQLCOM_SHOW_LOGS; }
	| GRANTS FOR_SYM user
	  {
	    LEX *lex=Lex;
	    lex->sql_command= SQLCOM_SHOW_GRANTS;
	    lex->grant_user=$3;
	    lex->grant_user->password.str=NullS;
	  }
        | CREATE TABLE_SYM table_ident
          {
	    Lex->sql_command = SQLCOM_SHOW_CREATE;
	    if(!add_table_to_list($3, NULL, 0))
	      YYABORT;
	  }
        | MASTER_SYM STATUS_SYM
          {
	    Lex->sql_command = SQLCOM_SHOW_MASTER_STAT;
          }
        | SLAVE STATUS_SYM
          {
	    Lex->sql_command = SQLCOM_SHOW_SLAVE_STAT;
          };

opt_db:
	/* empty */  { $$= 0; }
	| from_or_in ident { $$= $2.str; };

wild:
	/* empty */
	| LIKE text_string { Lex->wild= $2; };

opt_full:
	/* empty */ { Lex->verbose=0; }
	| FULL	    { Lex->verbose=1; };

from_or_in:
	FROM
	| IN_SYM;

binlog_in:
	/* empty */ { Lex->mi.log_file_name = 0; }
        | IN_SYM TEXT_STRING { Lex->mi.log_file_name = $2.str; };

binlog_from:
	/* empty */ { Lex->mi.pos = 4; /* skip magic number */ }
        | FROM ulonglong_num { Lex->mi.pos = $2; };


/* A Oracle compatible synonym for show */
describe:
	describe_command table_ident
	{
	  LEX *lex=Lex;
	  lex->wild=0;
	  lex->verbose=0;
	  lex->sql_command=SQLCOM_SHOW_FIELDS;
	  if (!add_table_to_list($2, NULL, 0))
	    YYABORT;
	}
	opt_describe_column {}
	| describe_command select
          { Lex->select_lex.options|= SELECT_DESCRIBE; };


describe_command:
	DESC
	| DESCRIBE;

opt_describe_column:
	/* empty */	{}
	| text_string	{ Lex->wild= $1; }
	| ident
 { Lex->wild= new String((const char*) $1.str,$1.length); };


/* flush things */

flush:
	FLUSH_SYM
	{
	  LEX *lex=Lex;
	  lex->sql_command= SQLCOM_FLUSH; lex->type=0;
	}
	flush_options
	{}
	;

flush_options:
	flush_options ',' flush_option
	| flush_option;

flush_option:
	table_or_tables	{ Lex->type|= REFRESH_TABLES; } opt_table_list {}
	| TABLES WITH READ_SYM LOCK_SYM { Lex->type|= REFRESH_TABLES | REFRESH_READ_LOCK; }
	| QUERY_SYM CACHE_SYM { Lex->type|= REFRESH_QUERY_CACHE_FREE; }
	| HOSTS_SYM	{ Lex->type|= REFRESH_HOSTS; }
	| PRIVILEGES	{ Lex->type|= REFRESH_GRANT; }
	| LOGS_SYM	{ Lex->type|= REFRESH_LOG; }
	| STATUS_SYM	{ Lex->type|= REFRESH_STATUS; }
        | SLAVE         { Lex->type|= REFRESH_SLAVE; }
        | MASTER_SYM    { Lex->type|= REFRESH_MASTER; }
	| DES_KEY_FILE	{ Lex->type|= REFRESH_DES_KEY_FILE; }
 	| RESOURCES     { Lex->type|= REFRESH_USER_RESOURCES; };

opt_table_list:
	/* empty */  {;}
	| table_list {;};

reset:
	RESET_SYM
	{
	  LEX *lex=Lex;
	  lex->sql_command= SQLCOM_RESET; lex->type=0;
	} reset_options
	{}
	;

reset_options:
	reset_options ',' reset_option
	| reset_option;

reset_option:
        SLAVE                 { Lex->type|= REFRESH_SLAVE; }
        | MASTER_SYM          { Lex->type|= REFRESH_MASTER; }
	| QUERY_SYM CACHE_SYM { Lex->type|= REFRESH_QUERY_CACHE;};

purge:
	PURGE
	{
	  LEX *lex=Lex;
	  lex->sql_command = SQLCOM_PURGE;
	  lex->type=0;
	}
        MASTER_SYM LOGS_SYM TO_SYM TEXT_STRING
         {
	   Lex->to_log = $6.str;
         } ;

/* kill threads */

kill:
	KILL_SYM expr
	{
	  LEX *lex=Lex;
	  if ($2->fix_fields(lex->thd,0))
	  { 
	    send_error(&lex->thd->net, ER_SET_CONSTANTS_ONLY);
	    YYABORT;
	  }
          lex->sql_command=SQLCOM_KILL;
	  lex->thread_id= (ulong) $2->val_int();
	};

/* change database */

use:	USE_SYM ident
	{
	  LEX *lex=Lex;
	  lex->sql_command=SQLCOM_CHANGE_DB; lex->select->db= $2.str;
	};

/* import, export of files */

load:	LOAD DATA_SYM load_data_lock opt_local INFILE TEXT_STRING
	{
	  LEX *lex=Lex;
	  lex->sql_command= SQLCOM_LOAD;
	  lex->lock_option= $3;
	  lex->local_file=  $4;
	  if (!(lex->exchange= new sql_exchange($6.str,0)))
	    YYABORT;
	  lex->field_list.empty();
	}
	opt_duplicate INTO TABLE_SYM table_ident opt_field_term opt_line_term
	opt_ignore_lines opt_field_spec
	{
	  if (!add_table_to_list($11, NULL, TL_OPTION_UPDATING))
	    YYABORT;
	}
        |
	LOAD TABLE_SYM table_ident FROM MASTER_SYM
        {
	  Lex->sql_command = SQLCOM_LOAD_MASTER_TABLE;
	  if (!add_table_to_list($3, NULL, TL_OPTION_UPDATING))
	    YYABORT;

        }
        |
	LOAD DATA_SYM FROM MASTER_SYM
        {
	  Lex->sql_command = SQLCOM_LOAD_MASTER_DATA;
        };

opt_local:
	/* empty */	{ $$=0;}
	| LOCAL_SYM	{ $$=1;};

load_data_lock:
	/* empty */	{ $$= current_thd->update_lock_default; }
	| CONCURRENT	{ $$= TL_WRITE_CONCURRENT_INSERT ; }
	| LOW_PRIORITY	{ $$= TL_WRITE_LOW_PRIORITY; };


opt_duplicate:
	/* empty */	{ Lex->duplicates=DUP_ERROR; }
	| REPLACE	{ Lex->duplicates=DUP_REPLACE; }
	| IGNORE_SYM	{ Lex->duplicates=DUP_IGNORE; };

opt_field_term:
	/* empty */
	| COLUMNS field_term_list;

field_term_list:
	field_term_list field_term
	| field_term;

field_term:
	TERMINATED BY text_string { Lex->exchange->field_term= $3;}
	| OPTIONALLY ENCLOSED BY text_string
	  {
	    LEX *lex=Lex;
	    lex->exchange->enclosed= $4;
	    lex->exchange->opt_enclosed=1;
	  }
	| ENCLOSED BY text_string { Lex->exchange->enclosed= $3;}
	| ESCAPED BY text_string  { Lex->exchange->escaped= $3;};

opt_line_term:
	/* empty */
	| LINES line_term_list;

line_term_list:
	line_term_list line_term
	| line_term;

line_term:
	TERMINATED BY text_string { Lex->exchange->line_term= $3;}
	| STARTING BY text_string { Lex->exchange->line_start= $3;};

opt_ignore_lines:
	/* empty */
	| IGNORE_SYM NUM LINES
	  { Lex->exchange->skip_lines=atol($2.str); };

/* Common definitions */

text_literal:
	TEXT_STRING { $$ = new Item_string($1.str,$1.length); }
	| text_literal TEXT_STRING
	{ ((Item_string*) $1)->append($2.str,$2.length); };

text_string:
	TEXT_STRING	{ $$=  new String($1.str,$1.length); }
	| HEX_NUM
	  {
	    Item *tmp = new Item_varbinary($1.str,$1.length);
	    $$= tmp ? tmp->val_str((String*) 0) : (String*) 0;
	  };

literal:
	text_literal	{ $$ =	$1; }
	| NUM		{ $$ =	new Item_int($1.str, (longlong) strtol($1.str, NULL, 10),$1.length); }
	| LONG_NUM	{ $$ =	new Item_int($1.str, (longlong) strtoll($1.str,NULL,10), $1.length); }
	| ULONGLONG_NUM	{ $$ =	new Item_uint($1.str, $1.length); }
	| REAL_NUM	{ $$ =	new Item_real($1.str, $1.length); }
	| FLOAT_NUM	{ $$ =	new Item_float($1.str, $1.length); }
	| NULL_SYM	{ $$ =	new Item_null();
			  Lex->next_state=STATE_OPERATOR_OR_IDENT;}
	| HEX_NUM	{ $$ =	new Item_varbinary($1.str,$1.length);}
	| DATE_SYM text_literal { $$ = $2; }
	| TIME_SYM text_literal { $$ = $2; }
	| TIMESTAMP text_literal { $$ = $2; };

/**********************************************************************
** Createing different items.
**********************************************************************/

insert_ident:
	simple_ident	 { $$=$1; }
	| table_wild	 { $$=$1; };

table_wild:
	ident '.' '*' { $$ = new Item_field(NullS,$1.str,"*"); }
	| ident '.' ident '.' '*'
	{ $$ = new Item_field((current_thd->client_capabilities &
   CLIENT_NO_SCHEMA ? NullS : $1.str),$3.str,"*"); };

order_ident:
	expr { $$=$1; };

simple_ident:
	ident
	{
	  SELECT_LEX *sel=Select;
	  $$ = !sel->create_refs || sel->in_sum_expr > 0 ? (Item*) new Item_field(NullS,NullS,$1.str) : (Item*) new Item_ref(NullS,NullS,$1.str);
	}
	| ident '.' ident
	{
	  SELECT_LEX *sel=Select;
	  $$ = !sel->create_refs || sel->in_sum_expr > 0 ? (Item*) new Item_field(NullS,$1.str,$3.str) : (Item*) new Item_ref(NullS,$1.str,$3.str);
	}
	| '.' ident '.' ident
	{
	  SELECT_LEX *sel=Select;
	  $$ = !sel->create_refs || sel->in_sum_expr > 0 ? (Item*) new Item_field(NullS,$2.str,$4.str) : (Item*) new Item_ref(NullS,$2.str,$4.str);
	}
	| ident '.' ident '.' ident
	{
	  SELECT_LEX *sel=Select;
	  $$ = !sel->create_refs || sel->in_sum_expr > 0 ? (Item*) new Item_field((current_thd->client_capabilities & CLIENT_NO_SCHEMA ? NullS :$1.str),$3.str,$5.str) : (Item*) new Item_ref((current_thd->client_capabilities & CLIENT_NO_SCHEMA ? NullS :$1.str),$3.str,$5.str);
	};


field_ident:
	ident			{ $$=$1;}
	| ident '.' ident	{ $$=$3;}	/* Skipp schema name in create*/
	| '.' ident		{ $$=$2;}	/* For Delphi */;

table_ident:
	ident			{ $$=new Table_ident($1); }
	| ident '.' ident	{ $$=new Table_ident($1,$3,0);}
	| '.' ident		{ $$=new Table_ident($2);}
   /* For Delphi */;

ident:
	IDENT	    { $$=$1; }
	| keyword
	{
	  LEX *lex= Lex;
	  $$.str= lex->thd->strmake($1.str,$1.length);
	  $$.length=$1.length;
	  if (lex->next_state != STATE_END)
	    lex->next_state=STATE_OPERATOR_OR_IDENT;
	}
	;

ident_or_text:
	ident 		{ $$=$1;}
	| TEXT_STRING	{ $$=$1;}
	| LEX_HOSTNAME	{ $$=$1;};

user:
	ident_or_text
	{
	  if (!($$=(LEX_USER*) sql_alloc(sizeof(st_lex_user))))
	    YYABORT;
	  $$->user = $1; $$->host.str=NullS;
	  }
	| ident_or_text '@' ident_or_text
	  {
	  if (!($$=(LEX_USER*) sql_alloc(sizeof(st_lex_user))))
	      YYABORT;
	    $$->user = $1; $$->host=$3;
	  };

/* Keyword that we allow for identifiers */

keyword:
	ACTION			{}
	| AFTER_SYM		{}
	| AGAINST		{}
	| AGGREGATE_SYM		{}
	| AUTO_INC		{}
	| AVG_ROW_LENGTH	{}
	| AVG_SYM		{}
	| BACKUP_SYM		{}
	| BEGIN_SYM		{}
	| BERKELEY_DB_SYM	{}
	| BINLOG_SYM		{}
	| BIT_SYM		{}
	| BOOL_SYM		{}
	| BOOLEAN_SYM		{}
	| CACHE_SYM		{}
	| CHANGED		{}
	| CHARSET		{}
	| CHECKSUM_SYM		{}
	| CIPHER_SYM		{}
	| CLIENT_SYM		{}
	| CLOSE_SYM		{}
	| COMMENT_SYM		{}
	| COMMITTED_SYM		{}
	| COMMIT_SYM		{}
	| COMPRESSED_SYM	{}
	| CONCURRENT		{}
	| CUBE_SYM		{}
	| DATA_SYM		{}
	| DATETIME		{}
	| DATE_SYM		{}
	| DAY_SYM		{}
	| DELAY_KEY_WRITE_SYM	{}
	| DES_KEY_FILE		{}
	| DIRECTORY_SYM		{}
	| DO_SYM		{}
	| DUMPFILE		{}
	| DYNAMIC_SYM		{}
	| END			{}
	| ENUM			{}
	| ESCAPE_SYM		{}
	| EVENTS_SYM		{}
	| EXECUTE_SYM		{}
	| EXTENDED_SYM		{}
	| FAST_SYM		{}
        | DISABLE_SYM           {}
        | ENABLE_SYM            {}
	| FULL			{}
	| FILE_SYM		{}
	| FIRST_SYM		{}
	| FIXED_SYM		{}
	| FLUSH_SYM		{}
	| GRANTS                {}
	| GLOBAL_SYM		{}
	| HEAP_SYM		{}
	| HANDLER_SYM		{}
	| HOSTS_SYM		{}
	| HOUR_SYM		{}
	| IDENTIFIED_SYM	{}
	| INDEXES		{}
	| ISOLATION		{}
	| ISAM_SYM		{}
	| ISSUER_SYM		{}
	| INNOBASE_SYM		{}
	| INSERT_METHOD		{}
        | IO_THREAD		{}
	| LAST_SYM		{}
	| LEVEL_SYM		{}
	| LOCAL_SYM		{}
	| LOCKS_SYM		{}
	| LOGS_SYM		{}
	| MAX_ROWS		{}
	| MASTER_SYM		{}
	| MASTER_HOST_SYM	{}
	| MASTER_PORT_SYM	{}
	| MASTER_LOG_FILE_SYM	{}
	| MASTER_LOG_POS_SYM	{}
	| MASTER_USER_SYM	{}
	| MASTER_PASSWORD_SYM	{}
	| MASTER_CONNECT_RETRY_SYM	{}
	| MAX_CONNECTIONS_PER_HOUR       {}
	| MAX_QUERIES_PER_HOUR  {}
	| MAX_UPDATES_PER_HOUR  {}
	| MEDIUM_SYM		{}
	| MERGE_SYM		{}
	| MEMORY_SYM		{}
	| MINUTE_SYM		{}
	| MIN_ROWS		{}
	| MODIFY_SYM		{}
	| MODE_SYM		{}
	| MONTH_SYM		{}
	| MYISAM_SYM		{}
	| NATIONAL_SYM		{}
	| NCHAR_SYM		{}
	| NEXT_SYM		{}
	| NEW_SYM		{}
	| NO_SYM		{}
	| NONE_SYM		{}
	| OFFSET_SYM		{}
	| OPEN_SYM		{}
	| PACK_KEYS_SYM		{}
	| PASSWORD		{}
	| PREV_SYM		{}
	| PROCESS		{}
	| PROCESSLIST_SYM	{}
	| QUERY_SYM		{}
	| QUICK			{}
	| RAID_0_SYM            {}
	| RAID_CHUNKS		{}
	| RAID_CHUNKSIZE	{}
	| RAID_STRIPED_SYM      {}
	| RAID_TYPE		{}
        | RELAY_LOG_FILE_SYM	{}
        | RELAY_LOG_POS_SYM	{}
	| RELOAD		{}
	| REPAIR		{}
	| REPEATABLE_SYM	{}
	| REPLICATION		{}
	| RESET_SYM		{}
	| RESOURCES		{}
	| RESTORE_SYM		{}
	| ROLLBACK_SYM		{}
	| ROLLUP_SYM		{}
	| ROWS_SYM		{}
	| ROW_FORMAT_SYM	{}
	| ROW_SYM		{}
	| SAVEPOINT_SYM		{}
	| SECOND_SYM		{}
	| SERIALIZABLE_SYM	{}
	| SESSION_SYM		{}
	| SIGNED_SYM		{}
	| SHARE_SYM		{}
	| SHUTDOWN		{}
        | SLAVE		        {}
	| SQL_CACHE_SYM		{}
	| SQL_BUFFER_RESULT	{}
	| SQL_NO_CACHE_SYM	{}
        | SQL_THREAD		{}
	| START_SYM		{}
	| STATUS_SYM		{}
	| STOP_SYM		{}
	| STRING_SYM		{}
	| SUBJECT_SYM		{}
	| SUPER_SYM		{}
	| TEMPORARY		{}
	| TEXT_SYM		{}
	| TRANSACTION_SYM	{}
	| TRUNCATE_SYM		{}
	| TIMESTAMP		{}
	| TIME_SYM		{}
	| TYPE_SYM		{}
	| UDF_SYM		{}
	| UNCOMMITTED_SYM	{}
	| USE_FRM		{}
	| VARIABLES		{}
	| WORK_SYM		{}
	| X509_SYM		{}
	| YEAR_SYM		{};

/* Option functions */

set:
	SET opt_option
	{
	  LEX *lex=Lex;
	  lex->sql_command= SQLCOM_SET_OPTION;
	  lex->option_type=OPT_DEFAULT;
	  lex->var_list.empty();
	}
	option_value_list
	{}
	;

opt_option:
	/* empty */ {}
	| OPTION {};

option_value_list:
	option_type option_value
	| option_value_list ',' option_type option_value;

option_type:
	/* empty */	{}
	| GLOBAL_SYM	{ Lex->option_type= OPT_GLOBAL; }
	| LOCAL_SYM	{ Lex->option_type= OPT_SESSION; }
	| SESSION_SYM	{ Lex->option_type= OPT_SESSION; }
	;

opt_var_type:
	/* empty */	{ $$=OPT_SESSION; }
	| LOCAL_SYM	{ $$=OPT_SESSION; }
	| SESSION_SYM	{ $$=OPT_SESSION; }
	| GLOBAL_SYM	{ $$=OPT_GLOBAL; }
	;

opt_var_ident_type:
	/* empty */		{ $$=OPT_DEFAULT; }
	| LOCAL_SYM '.'		{ $$=OPT_SESSION; }
	| SESSION_SYM '.'	{ $$=OPT_SESSION; }
	| GLOBAL_SYM '.'	{ $$=OPT_GLOBAL; }
	;

option_value:
	'@' ident_or_text equal expr
	{
	  Lex->var_list.push_back(new set_var_user(new Item_func_set_user_var($2,$4)));
	}
	| internal_variable_name equal set_expr_or_default
	  {
	    LEX *lex=Lex;
	    lex->var_list.push_back(new set_var(lex->option_type, $1, $3));
	  }
	| '@' '@' opt_var_ident_type internal_variable_name equal set_expr_or_default
	  {
	    LEX *lex=Lex;
	    lex->var_list.push_back(new set_var((enum_var_type) $3, $4, $6));
	  }
	| TRANSACTION_SYM ISOLATION LEVEL_SYM isolation_types
	  {
	    LEX *lex=Lex;
	    lex->var_list.push_back(new set_var(lex->option_type,
						find_sys_var("tx_isolation"),
						new Item_int((int32) $4)));
	  }
	| CHAR_SYM SET opt_equal set_expr_or_default
	{
	  LEX *lex=Lex;
	  lex->var_list.push_back(new set_var(lex->option_type,
					      find_sys_var("convert_character_set"),
					      $4));
	}
	| PASSWORD equal text_or_password
	  {
	    THD *thd=current_thd;
	    LEX_USER *user;
	    if (!(user=(LEX_USER*) sql_alloc(sizeof(LEX_USER))))
	      YYABORT;
	    user->host.str=0;
	    user->user.str=thd->priv_user;
	    thd->lex.var_list.push_back(new set_var_password(user, $3));
	  }
	| PASSWORD FOR_SYM user equal text_or_password
	  {
	    Lex->var_list.push_back(new set_var_password($3,$5));
	  }
	;

internal_variable_name:
	ident
	{
	  sys_var *tmp=find_sys_var($1.str, $1.length);
	  if (!tmp)
	    YYABORT;
	  $$=tmp;
	}
	;

isolation_types:
	READ_SYM UNCOMMITTED_SYM	{ $$= ISO_READ_UNCOMMITTED; }
	| READ_SYM COMMITTED_SYM	{ $$= ISO_READ_COMMITTED; }
	| REPEATABLE_SYM READ_SYM	{ $$= ISO_REPEATABLE_READ; }
	| SERIALIZABLE_SYM		{ $$= ISO_SERIALIZABLE; }
	;

text_or_password:
	TEXT_STRING { $$=$1.str;}
	| PASSWORD '(' TEXT_STRING ')'
	  {
	    if (!$3.length)
	      $$=$3.str;
	    else
	    {
	      char *buff=(char*) sql_alloc(HASH_PASSWORD_LENGTH+1);
	      make_scrambled_password(buff,$3.str);
	      $$=buff;
	    }
	  };


set_expr_or_default:
	expr      { $$=$1; }
	| DEFAULT { $$=0; }
	| ON	  { $$=new Item_string("ON",2); }
	| ALL	  { $$=new Item_string("ALL",3); }
	;


/* Lock function */

lock:
	LOCK_SYM table_or_tables
	{
	  Lex->sql_command=SQLCOM_LOCK_TABLES;
	}
	table_lock_list
	{}
	;

table_or_tables:
	TABLE_SYM
	| TABLES;

table_lock_list:
	table_lock
	| table_lock_list ',' table_lock;

table_lock:
	table_ident opt_table_alias lock_option
	{ if (!add_table_to_list($1,$2,0,(thr_lock_type) $3)) YYABORT; };

lock_option:
	READ_SYM	{ $$=TL_READ_NO_INSERT; }
	| WRITE_SYM     { $$=current_thd->update_lock_default; }
	| LOW_PRIORITY WRITE_SYM { $$=TL_WRITE_LOW_PRIORITY; }
	| READ_SYM LOCAL_SYM { $$= TL_READ; };

unlock:
	UNLOCK_SYM table_or_tables { Lex->sql_command=SQLCOM_UNLOCK_TABLES; };


/*
** Handler: direct access to ISAM functions
*/

handler:
	HANDLER_SYM table_ident OPEN_SYM opt_table_alias
	{
	  Lex->sql_command = SQLCOM_HA_OPEN;
	  if (!add_table_to_list($2,$4,0))
	    YYABORT;
	}
	| HANDLER_SYM table_ident CLOSE_SYM
	{
	  Lex->sql_command = SQLCOM_HA_CLOSE;
	  if (!add_table_to_list($2,0,0))
	    YYABORT;
	}
	| HANDLER_SYM table_ident READ_SYM
	{
	  LEX *lex=Lex;
	  lex->sql_command = SQLCOM_HA_READ;
	  lex->ha_rkey_mode= HA_READ_KEY_EXACT;	/* Avoid purify warnings */
	  lex->select->select_limit= 1;
	  lex->select->offset_limit= 0L;
	  if (!add_table_to_list($2,0,0))
	    YYABORT;
        }
        handler_read_or_scan where_clause limit_clause { };

handler_read_or_scan:
	handler_scan_function         { Lex->backup_dir= 0; }
        | ident handler_rkey_function { Lex->backup_dir= $1.str; };

handler_scan_function:
	FIRST_SYM  { Lex->ha_read_mode = RFIRST; }
	| NEXT_SYM { Lex->ha_read_mode = RNEXT;  };

handler_rkey_function:
	FIRST_SYM  { Lex->ha_read_mode = RFIRST; }
	| NEXT_SYM { Lex->ha_read_mode = RNEXT;  }
	| PREV_SYM { Lex->ha_read_mode = RPREV;  }
	| LAST_SYM { Lex->ha_read_mode = RLAST;  }
	| handler_rkey_mode
	{
	  LEX *lex=Lex;
	  lex->ha_read_mode = RKEY;
	  lex->ha_rkey_mode=$1;
	  if (!(lex->insert_list = new List_item))
	    YYABORT;
	} '(' values ')' { };

handler_rkey_mode:
	  EQ     { $$=HA_READ_KEY_EXACT;   }
	| GE     { $$=HA_READ_KEY_OR_NEXT; }
	| LE     { $$=HA_READ_KEY_OR_PREV; }
	| GT_SYM { $$=HA_READ_AFTER_KEY;   }
	| LT     { $$=HA_READ_BEFORE_KEY;  };

/* GRANT / REVOKE */

revoke:
	REVOKE
	{
	  LEX *lex=Lex;
	  lex->sql_command = SQLCOM_REVOKE;
	  lex->users_list.empty();
	  lex->columns.empty();
	  lex->grant= lex->grant_tot_col=0;
	  lex->select->db=0;
	  lex->ssl_type= SSL_TYPE_NOT_SPECIFIED;
	  lex->ssl_cipher= lex->x509_subject= lex->x509_issuer= 0;
	  bzero((char*) &lex->mqh, sizeof(lex->mqh));
	}
	grant_privileges ON opt_table FROM user_list
	{}
	;

grant:
	GRANT
	{
	  LEX *lex=Lex;
	  lex->users_list.empty();
	  lex->columns.empty();
	  lex->sql_command = SQLCOM_GRANT;
	  lex->grant= lex->grant_tot_col= 0;
	  lex->select->db= 0;
	  lex->ssl_type= SSL_TYPE_NOT_SPECIFIED;
	  lex->ssl_cipher= lex->x509_subject= lex->x509_issuer= 0;
	  bzero((char *)&(lex->mqh),sizeof(lex->mqh));
	}
	grant_privileges ON opt_table TO_SYM user_list
	require_clause grant_options
	{}
	;

grant_privileges:
	grant_privilege_list {}
	| ALL PRIVILEGES	{ Lex->grant = GLOBAL_ACLS;}
	| ALL			{ Lex->grant = GLOBAL_ACLS;};

grant_privilege_list:
	grant_privilege
	| grant_privilege_list ',' grant_privilege;

grant_privilege:
	SELECT_SYM 	{ Lex->which_columns = SELECT_ACL;} opt_column_list {}
	| INSERT	{ Lex->which_columns = INSERT_ACL;} opt_column_list {}
	| UPDATE_SYM	{ Lex->which_columns = UPDATE_ACL; } opt_column_list {}
	| REFERENCES	{ Lex->which_columns = REFERENCES_ACL;} opt_column_list {}
	| DELETE_SYM	{ Lex->grant |= DELETE_ACL;}
	| USAGE		{}
	| INDEX		{ Lex->grant |= INDEX_ACL;}
	| ALTER		{ Lex->grant |= ALTER_ACL;}
	| CREATE	{ Lex->grant |= CREATE_ACL;}
	| DROP		{ Lex->grant |= DROP_ACL;}
	| EXECUTE_SYM	{ Lex->grant |= EXECUTE_ACL;}
	| RELOAD	{ Lex->grant |= RELOAD_ACL;}
	| SHUTDOWN	{ Lex->grant |= SHUTDOWN_ACL;}
	| PROCESS	{ Lex->grant |= PROCESS_ACL;}
	| FILE_SYM	{ Lex->grant |= FILE_ACL;}
	| GRANT OPTION  { Lex->grant |= GRANT_ACL;}
	| SHOW DATABASES { Lex->grant |= SHOW_DB_ACL;}
	| SUPER_SYM	{ Lex->grant |= SUPER_ACL;}
	| CREATE TEMPORARY TABLES { Lex->grant |= CREATE_TMP_ACL;}
	| LOCK_SYM TABLES   { Lex->grant |= LOCK_TABLES_ACL; }
	| REPLICATION SLAVE  { Lex->grant |= REPL_SLAVE_ACL;}
	| REPLICATION CLIENT_SYM { Lex->grant |= REPL_CLIENT_ACL;}
	;


opt_and:
	/* empty */	{}
	| AND		{}
	;

require_list:
	 require_list_element opt_and require_list
	 | require_list_element
	 ;

require_list_element:
	SUBJECT_SYM TEXT_STRING
	{
	  LEX *lex=Lex;
	  if (lex->x509_subject)
	  {
	    net_printf(&lex->thd->net,ER_DUP_ARGUMENT, "SUBJECT");
	    YYABORT;
	  }
	  lex->x509_subject=$2.str;
	}
	| ISSUER_SYM TEXT_STRING
	{
	  LEX *lex=Lex;
	  if (lex->x509_issuer)
	  {
	    net_printf(&lex->thd->net,ER_DUP_ARGUMENT, "ISSUER");
	    YYABORT;
	  }
	  lex->x509_issuer=$2.str;
	}
	| CIPHER_SYM TEXT_STRING
	{
	  LEX *lex=Lex;
	  if (lex->ssl_cipher)
	  {
	    net_printf(&lex->thd->net,ER_DUP_ARGUMENT, "CIPHER");
	    YYABORT;
	  }
	  lex->ssl_cipher=$2.str;
	}
	;
 
opt_table:
	'*'
	  {
	    LEX *lex=Lex;
	    lex->select->db=lex->thd->db;
	    if (lex->grant == GLOBAL_ACLS)
	      lex->grant = DB_ACLS & ~GRANT_ACL;
	    else if (lex->columns.elements)
	    {
	      send_error(&lex->thd->net,ER_ILLEGAL_GRANT_FOR_TABLE);
	      YYABORT;
	    }
	  }
	| ident '.' '*'
	  {
	    LEX *lex=Lex;
	    lex->select->db = $1.str;
	    if (lex->grant == GLOBAL_ACLS)
	      lex->grant = DB_ACLS & ~GRANT_ACL;
	    else if (lex->columns.elements)
	    {
	      send_error(&lex->thd->net,ER_ILLEGAL_GRANT_FOR_TABLE);
	      YYABORT;
	    }
	  }
	| '*' '.' '*'
	  {
	    LEX *lex=Lex;
	    lex->select->db = NULL;
	    if (lex->grant == GLOBAL_ACLS)
	      lex->grant= GLOBAL_ACLS & ~GRANT_ACL;
	    else if (lex->columns.elements)
	    {
	      send_error(&lex->thd->net,ER_ILLEGAL_GRANT_FOR_TABLE);
	      YYABORT;
	    }
	  }
	| table_ident
	  {
	    LEX *lex=Lex;
	    if (!add_table_to_list($1,NULL,0))
	      YYABORT;
	    if (lex->grant == GLOBAL_ACLS)
	      lex->grant =  TABLE_ACLS & ~GRANT_ACL;
	  };


user_list:
	grant_user  { if (Lex->users_list.push_back($1)) YYABORT;}
	| user_list ',' grant_user
	  {
	    if (Lex->users_list.push_back($3))
	      YYABORT;
	  }
	;


grant_user:
	user IDENTIFIED_SYM BY TEXT_STRING
	{
	   $$=$1; $1->password=$4;
	   if ($4.length)
	   {
	     char *buff=(char*) sql_alloc(HASH_PASSWORD_LENGTH+1);
	     if (buff)
	     {
	       make_scrambled_password(buff,$4.str);
	       $1->password.str=buff;
	       $1->password.length=HASH_PASSWORD_LENGTH;
	     }
	  }
	}
	| user IDENTIFIED_SYM BY PASSWORD TEXT_STRING
	  { $$=$1; $1->password=$5 ; }
	| user
	  { $$=$1; $1->password.str=NullS; };


opt_column_list:
	/* empty */
	{
	  LEX *lex=Lex;
	  lex->grant |= lex->which_columns;
	}
	| '(' column_list ')';

column_list:
	column_list ',' column_list_id
	| column_list_id;

column_list_id:
	ident
	{
	  String *new_str = new String((const char*) $1.str,$1.length);
	  List_iterator <LEX_COLUMN> iter(Lex->columns);
	  class LEX_COLUMN *point;
	  LEX *lex=Lex;
	  while ((point=iter++))
	  {
	    if (!my_strcasecmp(point->column.ptr(),new_str->ptr()))
		break;
	  }
	  lex->grant_tot_col|= lex->which_columns;
	  if (point)
	    point->rights |= lex->which_columns;
	  else
	    lex->columns.push_back(new LEX_COLUMN (*new_str,lex->which_columns));
	};


require_clause: /* empty */
        | REQUIRE_SYM require_list 
          {
            Lex->ssl_type=SSL_TYPE_SPECIFIED;
          }
        | REQUIRE_SYM SSL_SYM
          {
            Lex->ssl_type=SSL_TYPE_ANY;
          }
        | REQUIRE_SYM X509_SYM
          {
            Lex->ssl_type=SSL_TYPE_X509;
          }
	| REQUIRE_SYM NONE_SYM
	  {
	    Lex->ssl_type=SSL_TYPE_NONE;
	  }
	;

grant_options:
	/* empty */ {}
	| WITH grant_option_list;

grant_option_list:
	grant_option_list grant_option {}
	| grant_option {};

grant_option:
	GRANT OPTION { Lex->grant |= GRANT_ACL;}
        | MAX_QUERIES_PER_HOUR ULONG_NUM
        {
	  Lex->mqh.questions=$2;
	  Lex->mqh.bits |= 1;
	}
        | MAX_UPDATES_PER_HOUR ULONG_NUM
        {
	  Lex->mqh.updates=$2;
	  Lex->mqh.bits |= 2;
	}
        | MAX_CONNECTIONS_PER_HOUR ULONG_NUM
        {
	  Lex->mqh.connections=$2;
	  Lex->mqh.bits |= 4;
	};

begin:
	BEGIN_SYM   { Lex->sql_command = SQLCOM_BEGIN;} opt_work {}
	;

opt_work:
	/* empty */ {}
	| WORK_SYM {;};

commit:
	COMMIT_SYM   { Lex->sql_command = SQLCOM_COMMIT;};

rollback:
	ROLLBACK_SYM 
	{
	  Lex->sql_command = SQLCOM_ROLLBACK;
	}
	| ROLLBACK_SYM TO_SYM SAVEPOINT_SYM ident
	{
	  Lex->sql_command = SQLCOM_ROLLBACK_TO_SAVEPOINT;
	  Lex->savepoint_name = $4.str;
	};
savepoint:
	SAVEPOINT_SYM ident
	{
	  Lex->sql_command = SQLCOM_SAVEPOINT;
	  Lex->savepoint_name = $2.str;
	};

/*
** UNIONS : glue selects together
*/


opt_union:	
	/* empty */ {}
	| union_list;

union_list:
	UNION_SYM union_option
	{
	  LEX *lex=Lex;
	  if (lex->exchange)
	  {
	    /* Only the last SELECT can have  INTO...... */
	    net_printf(&lex->thd->net, ER_WRONG_USAGE,"UNION","INTO");
	    YYABORT;
	  }
	  if (lex->select->linkage == NOT_A_SELECT)
	  {
	    send_error(&lex->thd->net, ER_SYNTAX_ERROR);
	    YYABORT;
	  }
	  if (mysql_new_select(lex))
	    YYABORT;
	  lex->select->linkage=UNION_TYPE;
	} 
	select_init {}
	;

union_opt:
	union_list {}
	| optional_order_or_limit {};

optional_order_or_limit:
      	/* empty 
           intentional reduce/reduce conflict here !!!
           { code } below should not be executed
           when neither ORDER BY nor LIMIT are used */ {}
	|
	  {
    	    LEX *lex=Lex;
	    if (!lex->select->braces)
	    {
	      send_error(&lex->thd->net, ER_SYNTAX_ERROR);
	      YYABORT;
	    }
	    if (mysql_new_select(lex))
	      YYABORT;
	    mysql_init_select(lex);
	    lex->select->linkage=NOT_A_SELECT;
	    lex->select->select_limit=lex->thd->variables.select_limit;
	  }
	  opt_order_clause limit_clause
	;

union_option:
	/* empty */ {}
	| ALL { Lex->union_option=1; }
	;
