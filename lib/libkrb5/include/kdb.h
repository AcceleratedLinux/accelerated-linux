/*
 * include/krb5/kdb.h
 *
 * Copyright 1990,1991 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 *
 * KDC Database interface definitions.
 */

/*
 * Copyright (C) 1998 by the FundsXpress, INC.
 * 
 * All rights reserved.
 * 
 * Export of this software from the United States of America may require
 * a specific license from the United States Government.  It is the
 * responsibility of any person or organization contemplating export to
 * obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of FundsXpress. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  FundsXpress makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/* This API is not considered as stable as the main krb5 API.
 *
 * - We may make arbitrary incompatible changes between feature
 *   releases (e.g. from 1.7 to 1.8).
 * - We will make some effort to avoid making incompatible changes for
 *   bugfix releases, but will make them if necessary.
 */

#ifndef KRB5_KDB5__
#define KRB5_KDB5__

#include <krb5.h>

/* Salt types */
#define KRB5_KDB_SALTTYPE_NORMAL	0
#define KRB5_KDB_SALTTYPE_V4		1
#define KRB5_KDB_SALTTYPE_NOREALM	2
#define KRB5_KDB_SALTTYPE_ONLYREALM	3
#define KRB5_KDB_SALTTYPE_SPECIAL	4
#define KRB5_KDB_SALTTYPE_AFS3		5
#define KRB5_KDB_SALTTYPE_CERTHASH	6

/* Attributes */
#define	KRB5_KDB_DISALLOW_POSTDATED	0x00000001
#define	KRB5_KDB_DISALLOW_FORWARDABLE	0x00000002
#define	KRB5_KDB_DISALLOW_TGT_BASED	0x00000004
#define	KRB5_KDB_DISALLOW_RENEWABLE	0x00000008
#define	KRB5_KDB_DISALLOW_PROXIABLE	0x00000010
#define	KRB5_KDB_DISALLOW_DUP_SKEY	0x00000020
#define	KRB5_KDB_DISALLOW_ALL_TIX	0x00000040
#define	KRB5_KDB_REQUIRES_PRE_AUTH	0x00000080
#define KRB5_KDB_REQUIRES_HW_AUTH	0x00000100
#define	KRB5_KDB_REQUIRES_PWCHANGE	0x00000200
#define KRB5_KDB_DISALLOW_SVR		0x00001000
#define KRB5_KDB_PWCHANGE_SERVICE	0x00002000
#define KRB5_KDB_SUPPORT_DESMD5         0x00004000
#define	KRB5_KDB_NEW_PRINC		0x00008000
#define KRB5_KDB_OK_AS_DELEGATE		0x00100000

/* Creation flags */
#define KRB5_KDB_CREATE_BTREE		0x00000001
#define KRB5_KDB_CREATE_HASH		0x00000002

#if !defined(_WIN32)

/*
 * Note --- these structures cannot be modified without changing the
 * database version number in libkdb.a, but should be expandable by
 * adding new tl_data types.
 */
typedef struct _krb5_tl_data {
    struct _krb5_tl_data* tl_data_next;		/* NOT saved */
    krb5_int16 		  tl_data_type;		
    krb5_ui_2		  tl_data_length;	
    krb5_octet 	        * tl_data_contents;	
} krb5_tl_data;

/* 
 * If this ever changes up the version number and make the arrays be as
 * big as necessary.
 *
 * Currently the first type is the enctype and the second is the salt type.
 */
typedef struct _krb5_key_data {
    krb5_int16 		  key_data_ver;		/* Version */
    krb5_int16		  key_data_kvno;	/* Key Version */
    krb5_int16		  key_data_type[2];	/* Array of types */
    krb5_ui_2		  key_data_length[2];	/* Array of lengths */
    krb5_octet 	        * key_data_contents[2];	/* Array of pointers */
} krb5_key_data;

#define KRB5_KDB_V1_KEY_DATA_ARRAY	2	/* # of array elements */

typedef struct _krb5_keysalt {
    krb5_int16		  type;	
    krb5_data		  data;			/* Length, data */
} krb5_keysalt;

typedef struct _krb5_db_entry_new {
    krb5_magic 		  magic;		/* NOT saved */
    krb5_ui_2		  len;			
    krb5_ui_4             mask;                 /* members currently changed/set */	
    krb5_flags 		  attributes;
    krb5_deltat		  max_life;
    krb5_deltat		  max_renewable_life;
    krb5_timestamp 	  expiration;	  	/* When the client expires */
    krb5_timestamp 	  pw_expiration;  	/* When its passwd expires */
    krb5_timestamp 	  last_success;		/* Last successful passwd */
    krb5_timestamp 	  last_failed;		/* Last failed passwd attempt */
    krb5_kvno 	 	  fail_auth_count; 	/* # of failed passwd attempt */
    krb5_int16 		  n_tl_data;
    krb5_int16 		  n_key_data;
    krb5_ui_2		  e_length;		/* Length of extra data */
    krb5_octet		* e_data;		/* Extra data to be saved */

    krb5_principal 	  princ;		/* Length, data */	
    krb5_tl_data	* tl_data;		/* Linked list */
    krb5_key_data       * key_data;		/* Array */
} krb5_db_entry;

typedef struct _osa_policy_ent_t {
    int               version;
    char      *name;
    krb5_ui_4       pw_min_life;
    krb5_ui_4       pw_max_life;
    krb5_ui_4       pw_min_length;
    krb5_ui_4       pw_min_classes;
    krb5_ui_4       pw_history_num;
    krb5_ui_4       policy_refcnt;
} osa_policy_ent_rec, *osa_policy_ent_t;

typedef       void    (*osa_adb_iter_policy_func) (void *, osa_policy_ent_t);

typedef struct __krb5_key_salt_tuple {
    krb5_enctype	ks_enctype;
    krb5_int32		ks_salttype;
} krb5_key_salt_tuple;

#define	KRB5_KDB_MAGIC_NUMBER		0xdbdbdbdb
#define KRB5_KDB_V1_BASE_LENGTH		38
  
#define KRB5_TL_LAST_PWD_CHANGE		0x0001
#define KRB5_TL_MOD_PRINC		0x0002
#define KRB5_TL_KADM_DATA		0x0003
#define KRB5_TL_KADM5_E_DATA		0x0004
#define KRB5_TL_RB1_CHALLENGE		0x0005
#ifdef SECURID
#define KRB5_TL_SECURID_STATE           0x0006
#define KRB5_TL_DB_ARGS                 0x7fff
#endif /* SECURID */
#define KRB5_TL_USER_CERTIFICATE        0x0007
#define KRB5_TL_MKVNO                   0x0008
#define KRB5_TL_ACTKVNO                 0x0009
#define KRB5_TL_MKEY_AUX                0x000a

/* version number for KRB5_TL_ACTKVNO data */
#define KRB5_TL_ACTKVNO_VER     1

/* version number for KRB5_TL_MKEY_AUX data */
#define KRB5_TL_MKEY_AUX_VER    1

typedef struct _krb5_actkvno_node {
    struct _krb5_actkvno_node *next;
    krb5_kvno      act_kvno;
    krb5_timestamp act_time;
} krb5_actkvno_node;

typedef struct _krb5_mkey_aux_node {
    struct _krb5_mkey_aux_node *next;
    krb5_kvno        mkey_kvno; /* kvno of mkey protecting the latest_mkey */
    krb5_key_data    latest_mkey; /* most recent mkey */
} krb5_mkey_aux_node;

typedef struct _krb5_keylist_node {
    krb5_keyblock keyblock;
    krb5_kvno     kvno;
    struct _krb5_keylist_node *next;
} krb5_keylist_node;

/*
 * Determines the number of failed KDC requests before DISALLOW_ALL_TIX is set
 * on the principal.
 */
#define KRB5_MAX_FAIL_COUNT		5

/* XXX depends on knowledge of krb5_parse_name() formats */
#define KRB5_KDB_M_NAME		"K/M"	/* Kerberos/Master */

/* prompts used by default when reading the KDC password from the keyboard. */
#define KRB5_KDC_MKEY_1	"Enter KDC database master key"
#define KRB5_KDC_MKEY_2	"Re-enter KDC database master key to verify"


extern char *krb5_mkey_pwd_prompt1;
extern char *krb5_mkey_pwd_prompt2;

/*
 * These macros specify the encoding of data within the database.
 *
 * Data encoding is little-endian.
 */
#ifdef _KRB5_INT_H
#include "k5-platform.h"
#define	krb5_kdb_decode_int16(cp, i16)	\
	*((krb5_int16 *) &(i16)) = load_16_le(cp)
#define	krb5_kdb_decode_int32(cp, i32)	\
	*((krb5_int32 *) &(i32)) = load_32_le(cp)
#define krb5_kdb_encode_int16(i16, cp)	store_16_le(i16, cp)
#define	krb5_kdb_encode_int32(i32, cp)	store_32_le(i32, cp)
#endif /* _KRB5_INT_H */

#define KRB5_KDB_OPEN_RW                0
#define KRB5_KDB_OPEN_RO                1

#ifndef KRB5_KDB_SRV_TYPE_KDC
#define KRB5_KDB_SRV_TYPE_KDC           0x0100        
#endif

#ifndef KRB5_KDB_SRV_TYPE_ADMIN
#define KRB5_KDB_SRV_TYPE_ADMIN         0x0200  
#endif

#ifndef KRB5_KDB_SRV_TYPE_PASSWD
#define KRB5_KDB_SRV_TYPE_PASSWD        0x0300
#endif

#ifndef KRB5_KDB_SRV_TYPE_OTHER
#define KRB5_KDB_SRV_TYPE_OTHER         0x0400  
#endif

#define KRB5_KDB_OPT_SET_DB_NAME        0
#define KRB5_KDB_OPT_SET_LOCK_MODE      1

#define KRB5_DB_LOCKMODE_SHARED       0x0001
#define KRB5_DB_LOCKMODE_EXCLUSIVE    0x0002
#define KRB5_DB_LOCKMODE_DONTBLOCK    0x0004
#define KRB5_DB_LOCKMODE_PERMANENT    0x0008

/* libkdb.spec */
krb5_error_code krb5_db_setup_lib_handle(krb5_context kcontext);
krb5_error_code krb5_db_open( krb5_context kcontext, char **db_args, int mode );
krb5_error_code krb5_db_init  ( krb5_context kcontext );
krb5_error_code krb5_db_create ( krb5_context kcontext, char **db_args );
krb5_error_code krb5_db_inited  ( krb5_context kcontext );
krb5_error_code kdb5_db_create ( krb5_context kcontext, char **db_args );
krb5_error_code krb5_db_fini ( krb5_context kcontext );
const char * krb5_db_errcode2string ( krb5_context kcontext, long err_code );
krb5_error_code krb5_db_destroy ( krb5_context kcontext, char **db_args );
krb5_error_code krb5_db_promote ( krb5_context kcontext, char **db_args );
krb5_error_code krb5_db_get_age ( krb5_context kcontext, char *db_name, time_t *t );
krb5_error_code krb5_db_set_option ( krb5_context kcontext, int option, void *value );
krb5_error_code krb5_db_lock ( krb5_context kcontext, int lock_mode );
krb5_error_code krb5_db_unlock ( krb5_context kcontext );
krb5_error_code krb5_db_get_principal ( krb5_context kcontext,
					krb5_const_principal search_for,
					krb5_db_entry *entries,
					int *nentries,
					krb5_boolean *more );
krb5_error_code krb5_db_free_principal ( krb5_context kcontext,
					 krb5_db_entry *entry,
					 int count );
krb5_error_code krb5_db_put_principal ( krb5_context kcontext,
					krb5_db_entry *entries,
					int *nentries);
krb5_error_code krb5_db_delete_principal ( krb5_context kcontext,
					   krb5_principal search_for,
					   int *nentries );
krb5_error_code krb5_db_iterate ( krb5_context kcontext,
				  char *match_entry,
				  int (*func) (krb5_pointer, krb5_db_entry *),
				  krb5_pointer func_arg );
krb5_error_code krb5_supported_realms ( krb5_context kcontext,
					char **realms );
krb5_error_code krb5_free_supported_realms ( krb5_context kcontext,
					     char **realms );
krb5_error_code krb5_db_set_master_key_ext ( krb5_context kcontext,
					     char *pwd,
					     krb5_keyblock *key );
krb5_error_code krb5_db_set_mkey ( krb5_context context, 
				   krb5_keyblock *key);
krb5_error_code krb5_db_get_mkey ( krb5_context kcontext,
				   krb5_keyblock **key );

krb5_error_code krb5_db_set_mkey_list( krb5_context context,
                                       krb5_keylist_node * keylist);

krb5_error_code krb5_db_get_mkey_list( krb5_context kcontext,
                                       krb5_keylist_node ** keylist);

krb5_error_code krb5_db_free_master_key ( krb5_context kcontext,
					  krb5_keyblock *key );
krb5_error_code krb5_db_store_master_key  ( krb5_context kcontext, 
					    char *keyfile, 
					    krb5_principal mname,
					    krb5_kvno kvno,
					    krb5_keyblock *key,
					    char *master_pwd);
krb5_error_code krb5_db_store_master_key_list  ( krb5_context kcontext, 
						 char *keyfile, 
						 krb5_principal mname,
						 krb5_keylist_node *keylist,
						 char *master_pwd);
krb5_error_code krb5_db_fetch_mkey  ( krb5_context   context,
				      krb5_principal mname,
				      krb5_enctype   etype,
				      krb5_boolean   fromkeyboard,
				      krb5_boolean   twice,
				      char          *db_args,
                                      krb5_kvno     *kvno,
				      krb5_data     *salt,
				      krb5_keyblock *key);
krb5_error_code krb5_db_verify_master_key ( krb5_context   kcontext,
					    krb5_principal mprinc,
                                            krb5_kvno      kvno,
					    krb5_keyblock  *mkey );
krb5_error_code
krb5_db_fetch_mkey_list( krb5_context    context,
		         krb5_principal  mname,
		         const krb5_keyblock * mkey,
		         krb5_kvno             mkvno,
		         krb5_keylist_node  **mkeys_list );

krb5_error_code
krb5_db_free_mkey_list( krb5_context         context,
		        krb5_keylist_node  *mkey_list );

krb5_error_code
krb5_dbe_find_enctype( krb5_context	kcontext,
		       krb5_db_entry	*dbentp,
		       krb5_int32		ktype,
		       krb5_int32		stype,
		       krb5_int32		kvno,
		       krb5_key_data	**kdatap);


krb5_error_code krb5_dbe_search_enctype ( krb5_context kcontext, 
					  krb5_db_entry *dbentp, 
					  krb5_int32 *start, 
					  krb5_int32 ktype, 
					  krb5_int32 stype, 
					  krb5_int32 kvno, 
					  krb5_key_data **kdatap);

krb5_error_code
krb5_db_setup_mkey_name ( krb5_context context,
			  const char *keyname,
			  const char *realm,
			  char **fullname,
			  krb5_principal *principal);

krb5_error_code
krb5_dbekd_decrypt_key_data( krb5_context 	  context,
			     const krb5_keyblock	* mkey,
			     const krb5_key_data	* key_data,
			     krb5_keyblock 	* dbkey,
			     krb5_keysalt 	* keysalt);

krb5_error_code
krb5_dbekd_encrypt_key_data( krb5_context 		  context,
			     const krb5_keyblock	* mkey,
			     const krb5_keyblock 	* dbkey,
			     const krb5_keysalt		* keysalt,
			     int			  keyver,
			     krb5_key_data	        * key_data);

krb5_error_code
krb5_dbe_fetch_act_key_list(krb5_context          context,
			     krb5_principal       princ,
			     krb5_actkvno_node  **act_key_list);

krb5_error_code
krb5_dbe_find_act_mkey( krb5_context          context,
                        krb5_keylist_node   * mkey_list,
                        krb5_actkvno_node   * act_mkey_list,
                        krb5_kvno           * act_kvno,
                        krb5_keyblock      ** act_mkey);

krb5_error_code
krb5_dbe_find_mkey( krb5_context	 context,
                    krb5_keylist_node * mkey_list,
                    krb5_db_entry      * entry,
                    krb5_keyblock      ** mkey);

krb5_error_code
krb5_dbe_lookup_mkvno( krb5_context    context,
		       krb5_db_entry * entry,
		       krb5_kvno     * mkvno);

krb5_error_code
krb5_dbe_lookup_mod_princ_data( krb5_context          context,
				krb5_db_entry       * entry,
				krb5_timestamp      * mod_time,
				krb5_principal      * mod_princ);
 
krb5_error_code
krb5_dbe_lookup_mkey_aux( krb5_context         context,
		          krb5_db_entry      * entry,
		          krb5_mkey_aux_node ** mkey_aux_data_list);
krb5_error_code
krb5_dbe_update_mkvno( krb5_context    context,
		       krb5_db_entry * entry,
		       krb5_kvno       mkvno);

krb5_error_code
krb5_dbe_lookup_actkvno( krb5_context         context,
		         krb5_db_entry      * entry,
		         krb5_actkvno_node ** actkvno_list);

krb5_error_code
krb5_dbe_update_mkey_aux( krb5_context          context,
		          krb5_db_entry       * entry,
		          krb5_mkey_aux_node  * mkey_aux_data_list);

krb5_error_code
krb5_dbe_update_actkvno(krb5_context    context,
			krb5_db_entry * entry,
			const krb5_actkvno_node *actkvno_list);

krb5_error_code
krb5_dbe_update_last_pwd_change( krb5_context     context,
				 krb5_db_entry  * entry,
				 krb5_timestamp	  stamp);

krb5_error_code
krb5_dbe_lookup_tl_data( krb5_context          context,
			 krb5_db_entry       * entry,
			 krb5_tl_data        * ret_tl_data);

krb5_error_code
krb5_dbe_create_key_data( krb5_context          context,
			  krb5_db_entry       * entry);


krb5_error_code
krb5_dbe_update_mod_princ_data( krb5_context          context,
				krb5_db_entry       * entry,
				krb5_timestamp        mod_date,
				krb5_const_principal  mod_princ);

krb5_error_code
krb5_dbe_update_last_pwd_change( krb5_context          context,
				 krb5_db_entry       * entry,
				 krb5_timestamp	  stamp);

void *krb5_db_alloc( krb5_context kcontext,
		     void *ptr,
		     size_t size );

void krb5_db_free( krb5_context kcontext,
		   void *ptr);


krb5_error_code
krb5_dbe_lookup_last_pwd_change( krb5_context          context,
				 krb5_db_entry       * entry,
				 krb5_timestamp      * stamp);

krb5_error_code
krb5_dbe_delete_tl_data( krb5_context    context,
                         krb5_db_entry * entry,
                         krb5_int16      tl_data_type);

krb5_error_code
krb5_dbe_update_tl_data( krb5_context          context,
			 krb5_db_entry       * entry,
			 krb5_tl_data        * new_tl_data);

krb5_error_code
krb5_dbe_cpw( krb5_context	  kcontext,
	      krb5_keyblock       * master_key,
	      krb5_key_salt_tuple	* ks_tuple,
	      int			  ks_tuple_count,
	      char 		* passwd,
	      int			  new_kvno,
	      krb5_boolean	  keepold,
	      krb5_db_entry	* db_entry);


krb5_error_code
krb5_dbe_ark( krb5_context	  context,
	      krb5_keyblock       * master_key,
	      krb5_key_salt_tuple	* ks_tuple,
	      int			  ks_tuple_count,
	      krb5_db_entry	* db_entry);

krb5_error_code
krb5_dbe_crk( krb5_context	  context,
	      krb5_keyblock       * master_key,
	      krb5_key_salt_tuple	* ks_tuple,
	      int			  ks_tuple_count,
	      krb5_boolean	  keepold,
	      krb5_db_entry	* db_entry);

krb5_error_code
krb5_dbe_apw( krb5_context	  context,
	      krb5_keyblock       * master_key,
	      krb5_key_salt_tuple	* ks_tuple,
	      int			  ks_tuple_count,
	      char 		* passwd,
	      krb5_db_entry	* db_entry);

int
krb5_db_get_key_data_kvno( krb5_context	   context,
			   int		   count,
			   krb5_key_data * data);


/* default functions. Should not be directly called */
/*
 *   Default functions prototype
 */

krb5_error_code
krb5_dbe_def_search_enctype( krb5_context kcontext, 
			     krb5_db_entry *dbentp, 
			     krb5_int32 *start, 
			     krb5_int32 ktype, 
			     krb5_int32 stype, 
			     krb5_int32 kvno, 
			     krb5_key_data **kdatap);

krb5_error_code
krb5_def_store_mkey( krb5_context context,
		     char *keyfile,
		     krb5_principal mname,
		     krb5_kvno kvno,
		     krb5_keyblock *key,
		     char *master_pwd);

krb5_error_code
krb5_def_store_mkey_list( krb5_context context,
			  char *keyfile,
			  krb5_principal mname,
			  krb5_keylist_node *keylist,
			  char *master_pwd);

krb5_error_code
krb5_db_def_fetch_mkey( krb5_context   context,
			krb5_principal mname,
			krb5_keyblock *key,
			krb5_kvno     *kvno,
			char          *db_args);

krb5_error_code
krb5_def_verify_master_key( krb5_context   context,
			    krb5_principal mprinc,
			    krb5_kvno      kvno,
			    krb5_keyblock *mkey);

krb5_error_code
krb5_def_fetch_mkey_list( krb5_context            context,
			    krb5_principal        mprinc,
			    const krb5_keyblock  *mkey,
			    krb5_kvno             mkvno,
			    krb5_keylist_node  **mkeys_list);

krb5_error_code kdb_def_set_mkey ( krb5_context kcontext,
				   char *pwd,
				   krb5_keyblock *key );

krb5_error_code kdb_def_set_mkey_list ( krb5_context kcontext,
				        krb5_keylist_node *keylist );

krb5_error_code kdb_def_get_mkey ( krb5_context kcontext,
				   krb5_keyblock **key );

krb5_error_code kdb_def_get_mkey_list ( krb5_context kcontext,
				        krb5_keylist_node **keylist );

krb5_error_code
krb5_dbe_def_cpw( krb5_context	  context,
		  krb5_keyblock       * master_key,
		  krb5_key_salt_tuple	* ks_tuple,
		  int			  ks_tuple_count,
		  char 		* passwd,
		  int			  new_kvno,
		  krb5_boolean	  keepold,
		  krb5_db_entry	* db_entry);

krb5_error_code
krb5_def_promote_db(krb5_context, char *, char **);

krb5_error_code
krb5_dbekd_def_decrypt_key_data( krb5_context		  context,
				 const krb5_keyblock	* mkey,
				 const krb5_key_data	* key_data,
				 krb5_keyblock		* dbkey,
				 krb5_keysalt		* keysalt);

krb5_error_code
krb5_dbekd_def_encrypt_key_data( krb5_context		  context,
				 const krb5_keyblock	* mkey,
				 const krb5_keyblock	* dbkey,
				 const krb5_keysalt	* keysalt,
				 int			  keyver,
				 krb5_key_data		* key_data);

krb5_error_code
krb5_dbekd_def_decrypt_key_data( krb5_context 	  context,
				 const krb5_keyblock	* mkey,
				 const krb5_key_data	* key_data,
				 krb5_keyblock 	* dbkey,
				 krb5_keysalt 	* keysalt);

krb5_error_code
krb5_dbekd_def_encrypt_key_data( krb5_context 		  context,
				 const krb5_keyblock	* mkey,
				 const krb5_keyblock 	* dbkey,
				 const krb5_keysalt	* keysalt,
				 int			  keyver,
				 krb5_key_data	        * key_data);

krb5_error_code 
krb5_db_create_policy( krb5_context kcontext, 
		       osa_policy_ent_t policy);

krb5_error_code 
krb5_db_get_policy ( krb5_context kcontext, 
		     char *name, 
		     osa_policy_ent_t *policy,
		     int *nentries);

krb5_error_code 
krb5_db_put_policy( krb5_context kcontext, 
		    osa_policy_ent_t policy);

krb5_error_code 
krb5_db_iter_policy( krb5_context kcontext,
		     char *match_entry,
		     osa_adb_iter_policy_func func,
		     void *data);

krb5_error_code 
krb5_db_delete_policy( krb5_context kcontext, 
		       char *policy);

void 
krb5_db_free_policy( krb5_context kcontext, 
		     osa_policy_ent_t policy);


krb5_error_code
krb5_db_set_context
	(krb5_context, void *db_context);

krb5_error_code
krb5_db_get_context
	(krb5_context, void **db_context);

void
krb5_dbe_free_key_data_contents(krb5_context, krb5_key_data *);

void
krb5_dbe_free_key_list(krb5_context, krb5_keylist_node *);

void
krb5_dbe_free_actkvno_list(krb5_context, krb5_actkvno_node *);

void
krb5_dbe_free_mkey_aux_list(krb5_context, krb5_mkey_aux_node *);

void
krb5_dbe_free_tl_data(krb5_context, krb5_tl_data *);

#define KRB5_KDB_DEF_FLAGS	0

#define KDB_MAX_DB_NAME			128
#define KDB_REALM_SECTION		"realms"
#define KDB_MODULE_POINTER		"database_module"
#define KDB_MODULE_DEF_SECTION		"dbdefaults"
#define KDB_MODULE_SECTION		"dbmodules"
#define KDB_LIB_POINTER			"db_library"
#define KDB_DATABASE_CONF_FILE		DEFAULT_SECURE_PROFILE_PATH
#define KDB_DATABASE_ENV_PROF		KDC_PROFILE_ENV

#define KRB5_KDB_OPEN_RW		0
#define KRB5_KDB_OPEN_RO		1

#define KRB5_KDB_OPT_SET_DB_NAME	0
#define KRB5_KDB_OPT_SET_LOCK_MODE	1

typedef struct _kdb_vftabl {
    short int maj_ver;
    short int min_ver;

    krb5_error_code (*init_library)();
    krb5_error_code (*fini_library)();
    krb5_error_code (*init_module) ( krb5_context kcontext,
				     char * conf_section,
				     char ** db_args,
				     int mode );

    krb5_error_code (*fini_module) ( krb5_context kcontext );

    krb5_error_code (*db_create) ( krb5_context kcontext,
				   char * conf_section,
				   char ** db_args );

    krb5_error_code (*db_destroy) ( krb5_context kcontext,
				    char *conf_section,
				    char ** db_args );

    krb5_error_code (*db_get_age) ( krb5_context kcontext, 
				    char *db_name, 
				    time_t *age );

    krb5_error_code (*db_set_option) ( krb5_context kcontext,
				       int option,
				       void *value );

    krb5_error_code (*db_lock) ( krb5_context kcontext,
				 int mode );

    krb5_error_code (*db_unlock) ( krb5_context kcontext);

    krb5_error_code (*db_get_principal) ( krb5_context kcontext,
					  krb5_const_principal search_for,
					  unsigned int flags,
					  krb5_db_entry *entries,
					  int *nentries,
					  krb5_boolean *more );

    krb5_error_code (*db_free_principal) ( krb5_context kcontext,
					   krb5_db_entry *entry,
					   int count );

    krb5_error_code (*db_put_principal) ( krb5_context kcontext,
					  krb5_db_entry *entries,
					  int *nentries,
					  char **db_args);

    krb5_error_code (*db_delete_principal) ( krb5_context kcontext,
					     krb5_const_principal search_for,
					     int *nentries );

    krb5_error_code (*db_iterate) ( krb5_context kcontext,
				    char *match_entry,
				    int (*func) (krb5_pointer, krb5_db_entry *),
				    krb5_pointer func_arg );

    krb5_error_code (*db_create_policy) ( krb5_context kcontext,
					  osa_policy_ent_t policy );

    krb5_error_code (*db_get_policy) ( krb5_context kcontext,
				       char *name,
				       osa_policy_ent_t *policy,
				       int *cnt);

    krb5_error_code (*db_put_policy) ( krb5_context kcontext,
				       osa_policy_ent_t policy );

    krb5_error_code (*db_iter_policy) ( krb5_context kcontext,
					char *match_entry,
					osa_adb_iter_policy_func func,
					void *data );


    krb5_error_code (*db_delete_policy) ( krb5_context kcontext,
					  char *policy );

    void (*db_free_policy) ( krb5_context kcontext,
			     osa_policy_ent_t val );

    krb5_error_code (*db_supported_realms) ( krb5_context kcontext,
					    char **realms );

    krb5_error_code (*db_free_supported_realms) ( krb5_context kcontext,
						  char **realms );


    const char * (*errcode_2_string) ( krb5_context kcontext,
				       long err_code );

    void (*release_errcode_string) (krb5_context kcontext, const char *msg);

    void * (*db_alloc) (krb5_context kcontext, void *ptr, size_t size);
    void   (*db_free)  (krb5_context kcontext, void *ptr);



    /* optional functions */
    krb5_error_code (*set_master_key)    ( krb5_context kcontext, 
					   char *pwd, 
					   krb5_keyblock *key);

    krb5_error_code (*get_master_key)    ( krb5_context kcontext,
					   krb5_keyblock **key);

    krb5_error_code (*set_master_key_list) ( krb5_context kcontext,
				             krb5_keylist_node *keylist);

    krb5_error_code (*get_master_key_list) ( krb5_context kcontext,
				             krb5_keylist_node **keylist);

    krb5_error_code (*setup_master_key_name) ( krb5_context kcontext,
					       char *keyname,
					       char *realm, 
					       char **fullname, 
					       krb5_principal  *principal);

    krb5_error_code (*store_master_key)  ( krb5_context kcontext, 
					   char *db_arg, 
					   krb5_principal mname,
					   krb5_kvno kvno,
					   krb5_keyblock *key,
					   char *master_pwd);

    krb5_error_code (*fetch_master_key)  ( krb5_context kcontext,
					   krb5_principal mname,
					   krb5_keyblock *key,
					   krb5_kvno *kvno,
					   char *db_args);

    krb5_error_code (*verify_master_key) ( krb5_context kcontext,
					   krb5_principal mprinc,
					   krb5_kvno kvno,
					   krb5_keyblock *mkey );

    krb5_error_code (*fetch_master_key_list) (krb5_context kcontext,
					      krb5_principal mname,
					      const krb5_keyblock *key,
					      krb5_kvno            kvno,
					      krb5_keylist_node  **mkeys_list);

    krb5_error_code (*store_master_key_list)  ( krb5_context kcontext, 
						char *db_arg, 
						krb5_principal mname,
						krb5_keylist_node *keylist,
						char *master_pwd);

    krb5_error_code (*dbe_search_enctype) ( krb5_context kcontext, 
					    krb5_db_entry *dbentp, 
					    krb5_int32 *start, 
					    krb5_int32 ktype, 
					    krb5_int32 stype, 
					    krb5_int32 kvno, 
					    krb5_key_data **kdatap);
    

    krb5_error_code
    (*db_change_pwd) ( krb5_context	  context,
		       krb5_keyblock       * master_key,
		       krb5_key_salt_tuple	* ks_tuple,
		       int			  ks_tuple_count,
		       char 		* passwd,
		       int			  new_kvno,
		       krb5_boolean	  keepold,
		       krb5_db_entry	* db_entry);

    /* Promote a temporary database to be the live one.  */
    krb5_error_code (*promote_db) (krb5_context context,
				   char *conf_section,
				   char **db_args);

    krb5_error_code (*dbekd_decrypt_key_data) ( krb5_context kcontext,
					        const krb5_keyblock *mkey,
						const krb5_key_data *key_data,
						krb5_keyblock *dbkey,
						krb5_keysalt *keysalt );

    krb5_error_code (*dbekd_encrypt_key_data) ( krb5_context kcontext,
						const krb5_keyblock *mkey,
						const krb5_keyblock *dbkey,
						const krb5_keysalt *keyselt,
						int keyver,
						krb5_key_data *key_data );

    krb5_error_code
    (*db_invoke) ( krb5_context context,
		   unsigned int method,
		   const krb5_data *req,
		   krb5_data *rep );
} kdb_vftabl;
#endif /* !defined(_WIN32) */

#endif /* KRB5_KDB5__ */
