/*
 * lib/krb5/ccache/cc_memory.c
 *
 * Copyright 1990,1991,2000,2004,2008 by the Massachusetts Institute of Technology.
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
 * implementation of memory-based credentials cache
 */
#include "cc-int.h"
#include <errno.h>

static krb5_error_code KRB5_CALLCONV krb5_mcc_close
	(krb5_context, krb5_ccache id );

static krb5_error_code KRB5_CALLCONV krb5_mcc_destroy 
	(krb5_context, krb5_ccache id );

static krb5_error_code KRB5_CALLCONV krb5_mcc_end_seq_get 
	(krb5_context, krb5_ccache id , krb5_cc_cursor *cursor );

static krb5_error_code KRB5_CALLCONV krb5_mcc_generate_new 
	(krb5_context, krb5_ccache *id );

static const char * KRB5_CALLCONV krb5_mcc_get_name 
	(krb5_context, krb5_ccache id );

static krb5_error_code KRB5_CALLCONV krb5_mcc_get_principal 
	(krb5_context, krb5_ccache id , krb5_principal *princ );

static krb5_error_code KRB5_CALLCONV krb5_mcc_initialize 
	(krb5_context, krb5_ccache id , krb5_principal princ );

static krb5_error_code KRB5_CALLCONV krb5_mcc_next_cred 
	(krb5_context, 
		   krb5_ccache id , 
		   krb5_cc_cursor *cursor , 
		   krb5_creds *creds );

static krb5_error_code KRB5_CALLCONV krb5_mcc_resolve 
	(krb5_context, krb5_ccache *id , const char *residual );

static krb5_error_code KRB5_CALLCONV krb5_mcc_retrieve 
	(krb5_context, 
		   krb5_ccache id , 
		   krb5_flags whichfields , 
		   krb5_creds *mcreds , 
		   krb5_creds *creds );

static krb5_error_code KRB5_CALLCONV krb5_mcc_start_seq_get 
	(krb5_context, krb5_ccache id , krb5_cc_cursor *cursor );

static krb5_error_code KRB5_CALLCONV krb5_mcc_store 
	(krb5_context, krb5_ccache id , krb5_creds *creds );

static krb5_error_code KRB5_CALLCONV krb5_mcc_set_flags 
	(krb5_context, krb5_ccache id , krb5_flags flags );

static krb5_error_code KRB5_CALLCONV krb5_mcc_ptcursor_new
	(krb5_context, krb5_cc_ptcursor *);

static krb5_error_code KRB5_CALLCONV krb5_mcc_ptcursor_next
	(krb5_context, krb5_cc_ptcursor, krb5_ccache *);

static krb5_error_code KRB5_CALLCONV krb5_mcc_ptcursor_free
	(krb5_context, krb5_cc_ptcursor *);

static krb5_error_code KRB5_CALLCONV krb5_mcc_last_change_time
	(krb5_context, krb5_ccache, krb5_timestamp *);

static krb5_error_code KRB5_CALLCONV krb5_mcc_lock
	(krb5_context context, krb5_ccache id);

static krb5_error_code KRB5_CALLCONV krb5_mcc_unlock
	(krb5_context context, krb5_ccache id);


extern const krb5_cc_ops krb5_mcc_ops;
extern krb5_error_code krb5_change_cache (void);

#define KRB5_OK 0

/* Individual credentials within a cache, in a linked list.  */
typedef struct _krb5_mcc_link {
    struct _krb5_mcc_link *next;
    krb5_creds *creds;
} krb5_mcc_link, *krb5_mcc_cursor;

/* Per-cache data header.  */
typedef struct _krb5_mcc_data {
    char *name;
    k5_cc_mutex lock;
    krb5_principal prin;
    krb5_mcc_cursor link;
    krb5_timestamp changetime;
} krb5_mcc_data;

/* List of memory caches.  */
typedef struct krb5_mcc_list_node {
    struct krb5_mcc_list_node *next;
    krb5_mcc_data *cache;
} krb5_mcc_list_node;

/* Iterator over memory caches.  */
struct krb5_mcc_ptcursor_data {
    struct krb5_mcc_list_node *cur;
};

k5_cc_mutex krb5int_mcc_mutex = K5_CC_MUTEX_PARTIAL_INITIALIZER;
static krb5_mcc_list_node *mcc_head = 0;

static void update_mcc_change_time(krb5_mcc_data *);

/*
 * Modifies:
 * id
 *
 * Effects:
 * Creates/refreshes the file cred cache id.  If the cache exists, its
 * contents are destroyed.
 *
 * Errors:
 * system errors
 * permission errors
 */
static void krb5_mcc_free (krb5_context context, krb5_ccache id);

krb5_error_code KRB5_CALLCONV
krb5_mcc_initialize(krb5_context context, krb5_ccache id, krb5_principal princ)
{
    krb5_error_code ret; 
    krb5_mcc_data *d;

    d = (krb5_mcc_data *)id->data;
    ret = k5_cc_mutex_lock(context, &d->lock);
    if (ret)
        return ret;

    krb5_mcc_free(context, id);
	
    d = (krb5_mcc_data *)id->data;
    ret = krb5_copy_principal(context, princ,
			      &d->prin);
    update_mcc_change_time(d);

    k5_cc_mutex_unlock(context, &d->lock);
    if (ret == KRB5_OK)
        krb5_change_cache();
    return ret;
}

/*
 * Modifies:
 * id
 *
 * Effects:
 * Closes the file cache, invalidates the id, and frees any resources
 * associated with the cache.
 */
krb5_error_code KRB5_CALLCONV
krb5_mcc_close(krb5_context context, krb5_ccache id)
{
     free(id);
     return KRB5_OK;
}

void
krb5_mcc_free(krb5_context context, krb5_ccache id)
{
    krb5_mcc_cursor curr,next;
    krb5_mcc_data *d;

    d = (krb5_mcc_data *) id->data;
    for (curr = d->link; curr;) {
	krb5_free_creds(context, curr->creds);
	next = curr->next;
	free(curr);
	curr = next;
    }
    d->link = NULL;
    krb5_free_principal(context, d->prin);
}

/*
 * Effects:
 * Destroys the contents of id.
 *
 * Errors:
 * none
 */
krb5_error_code KRB5_CALLCONV
krb5_mcc_destroy(krb5_context context, krb5_ccache id)
{
    krb5_mcc_list_node **curr, *node;
    krb5_mcc_data *d;
    krb5_error_code err;

    err = k5_cc_mutex_lock(context, &krb5int_mcc_mutex);
    if (err)
	return err;

    d = (krb5_mcc_data *)id->data;
    for (curr = &mcc_head; *curr; curr = &(*curr)->next) {
	if ((*curr)->cache == d) {
	    node = *curr;
	    *curr = node->next;
	    free(node);
	    break;
	}
    }
    k5_cc_mutex_unlock(context, &krb5int_mcc_mutex);

    err = k5_cc_mutex_lock(context, &d->lock);
    if (err)
        return err;

    krb5_mcc_free(context, id);
    free(d->name);
    k5_cc_mutex_unlock(context, &d->lock);
    k5_cc_mutex_destroy(&d->lock);
    free(d); 
    free(id);

    krb5_change_cache ();
    return KRB5_OK;
}

/*
 * Requires:
 * residual is a legal path name, and a null-terminated string
 *
 * Modifies:
 * id
 * 
 * Effects:
 * creates a file-based cred cache that will reside in the file
 * residual.  The cache is not opened, but the filename is reserved.
 * 
 * Returns:
 * A filled in krb5_ccache structure "id".
 *
 * Errors:
 * KRB5_CC_NOMEM - there was insufficient memory to allocate the
 *              krb5_ccache.  id is undefined.
 * permission errors
 */
static krb5_error_code new_mcc_data (const char *, krb5_mcc_data **);

krb5_error_code KRB5_CALLCONV
krb5_mcc_resolve (krb5_context context, krb5_ccache *id, const char *residual)
{
    krb5_ccache lid;
    krb5_mcc_list_node *ptr;
    krb5_error_code err;
    krb5_mcc_data *d;

    err = k5_cc_mutex_lock(context, &krb5int_mcc_mutex);
    if (err)
	return err;
    for (ptr = mcc_head; ptr; ptr=ptr->next)
	if (!strcmp(ptr->cache->name, residual))
	    break;
    if (ptr)
	d = ptr->cache;
    else {
	err = new_mcc_data(residual, &d);
	if (err) {
	    k5_cc_mutex_unlock(context, &krb5int_mcc_mutex);
	    return err;
	}
    }
    k5_cc_mutex_unlock(context, &krb5int_mcc_mutex);

    lid = (krb5_ccache) malloc(sizeof(struct _krb5_ccache));
    if (lid == NULL)
	return KRB5_CC_NOMEM;
    
    lid->ops = &krb5_mcc_ops;
    lid->data = d;
    *id = lid; 
    return KRB5_OK;
}

/*
 * Effects:
 * Prepares for a sequential search of the credentials cache.
 * Returns a krb5_cc_cursor to be used with krb5_mcc_next_cred and
 * krb5_mcc_end_seq_get.
 *
 * If the cache is modified between the time of this call and the time
 * of the final krb5_mcc_end_seq_get, the results are undefined.
 *
 * Errors:
 * KRB5_CC_NOMEM
 * system errors
 */
krb5_error_code KRB5_CALLCONV
krb5_mcc_start_seq_get(krb5_context context, krb5_ccache id,
		       krb5_cc_cursor *cursor)
{
     krb5_mcc_cursor mcursor;
     krb5_error_code err;
     krb5_mcc_data *d;

     d = id->data;
     err = k5_cc_mutex_lock(context, &d->lock);
     if (err)
	 return err;
     mcursor = d->link;
     k5_cc_mutex_unlock(context, &d->lock);
     *cursor = (krb5_cc_cursor) mcursor;
     return KRB5_OK;
}

/*
 * Requires:
 * cursor is a krb5_cc_cursor originally obtained from
 * krb5_mcc_start_seq_get.
 *
 * Modifes:
 * cursor, creds
 * 
 * Effects:
 * Fills in creds with the "next" credentals structure from the cache
 * id.  The actual order the creds are returned in is arbitrary.
 * Space is allocated for the variable length fields in the
 * credentials structure, so the object returned must be passed to
 * krb5_destroy_credential.
 *
 * The cursor is updated for the next call to krb5_mcc_next_cred.
 *
 * Errors:
 * system errors
 */
krb5_error_code KRB5_CALLCONV
krb5_mcc_next_cred(krb5_context context, krb5_ccache id,
		   krb5_cc_cursor *cursor, krb5_creds *creds)
{
     krb5_mcc_cursor mcursor;
     krb5_error_code retval;

     /* Once the node in the linked list is created, it's never
	modified, so we don't need to worry about locking here.  (Note
	that we don't support _remove_cred.)  */
     mcursor = (krb5_mcc_cursor) *cursor;
     if (mcursor == NULL)
	return KRB5_CC_END;
     memset(creds, 0, sizeof(krb5_creds));     
     if (mcursor->creds) {
	 retval = krb5int_copy_creds_contents(context, mcursor->creds, creds);
	 if (retval)
	     return retval;
     }
     *cursor = (krb5_cc_cursor)mcursor->next;
     return KRB5_OK;
}

/*
 * Requires:
 * cursor is a krb5_cc_cursor originally obtained from
 * krb5_mcc_start_seq_get.
 *
 * Modifies:
 * id, cursor
 *
 * Effects:
 * Finishes sequential processing of the file credentials ccache id,
 * and invalidates the cursor (it must never be used after this call).
 */
/* ARGSUSED */
krb5_error_code KRB5_CALLCONV
krb5_mcc_end_seq_get(krb5_context context, krb5_ccache id, krb5_cc_cursor *cursor)
{
     *cursor = 0L;
     return KRB5_OK;
}

/* Utility routine: Creates the back-end data for a memory cache, and
   threads it into the global linked list.

   Call with the global list lock held.  */
static krb5_error_code
new_mcc_data (const char *name, krb5_mcc_data **dataptr)
{
    krb5_error_code err;
    krb5_mcc_data *d;
    krb5_mcc_list_node *n;

    d = malloc(sizeof(krb5_mcc_data));
    if (d == NULL)
	return KRB5_CC_NOMEM;
        
    err = k5_cc_mutex_init(&d->lock);
    if (err) {
	free(d);
	return err;
    }

    d->name = strdup(name);
    if (d->name == NULL) {
	k5_cc_mutex_destroy(&d->lock);
	free(d);
	return KRB5_CC_NOMEM;
    }
    d->link = NULL;
    d->prin = NULL;
    d->changetime = 0;
    update_mcc_change_time(d);

    n = malloc(sizeof(krb5_mcc_list_node));
    if (n == NULL) {
	free(d->name);
	k5_cc_mutex_destroy(&d->lock);
	free(d);
	return KRB5_CC_NOMEM;
    }

    n->cache = d;
    n->next = mcc_head;
    mcc_head = n;

    *dataptr = d;
    return 0;
}

/*
 * Effects:
 * Creates a new file cred cache whose name is guaranteed to be
 * unique.  The name begins with the string TKT_ROOT (from mcc.h).
 * The cache is not opened, but the new filename is reserved.
 *  
 * Returns:
 * The filled in krb5_ccache id.
 *
 * Errors:
 * KRB5_CC_NOMEM - there was insufficient memory to allocate the
 *              krb5_ccache.  id is undefined.
 * system errors (from open)
 */

krb5_error_code KRB5_CALLCONV
krb5_mcc_generate_new (krb5_context context, krb5_ccache *id)
{
    krb5_ccache lid;
    char uniquename[8];
    krb5_error_code err;
    krb5_mcc_data *d;

    /* Allocate memory */
    lid = (krb5_ccache) malloc(sizeof(struct _krb5_ccache));
    if (lid == NULL)
	return KRB5_CC_NOMEM;

    lid->ops = &krb5_mcc_ops;
    
    err = k5_cc_mutex_lock(context, &krb5int_mcc_mutex);
    if (err) {
	free(lid);
	return err;
    }
    
    /* Check for uniqueness with mutex locked to avoid race conditions */
    while (1) {
        krb5_mcc_list_node *ptr;

        err = krb5int_random_string (context, uniquename, sizeof (uniquename));
        if (err) {
	    k5_cc_mutex_unlock(context, &krb5int_mcc_mutex);
	    free(lid);
	    return err;
        }
        
	for (ptr = mcc_head; ptr; ptr=ptr->next) {
            if (!strcmp(ptr->cache->name, uniquename)) {
		break;  /* got a match, loop again */
            }
	}
        if (!ptr) break; /* got to the end without finding a match */
    }
    
    err = new_mcc_data(uniquename, &d);

    k5_cc_mutex_unlock(context, &krb5int_mcc_mutex);
    if (err) {
	free(lid);
	return err;
    }
    lid->data = d;
    *id = lid;
    krb5_change_cache ();
    return KRB5_OK;
}

/* Utility routine: Creates a random memory ccache name. 
 * This algorithm was selected because it creates readable 
 * random ccache names in a fixed size buffer.  */

krb5_error_code
krb5int_random_string (krb5_context context, char *string, unsigned int length)
{
    static const unsigned char charlist[] =
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    krb5_error_code err = 0;
    unsigned char *bytes = NULL;
    unsigned int bytecount = length - 1;
    
    if (!err) {
        bytes = malloc (bytecount);
        if (bytes == NULL) { err = ENOMEM; }
    }
    
    if (!err) {
        krb5_data data;
        data.length = bytecount;
        data.data = (char *) bytes;
        err = krb5_c_random_make_octets (context, &data);
    }
    
    if (!err) {
        unsigned int i;
        for (i = 0; i < bytecount; i++) {
            string [i] = charlist[bytes[i] % (sizeof (charlist) - 1)];
        }
        string[length - 1] = '\0';
    }
    
    if (bytes != NULL) { free (bytes); }
    
    return err;
}

/*
 * Requires:
 * id is a file credential cache
 * 
 * Returns:
 * The name of the file cred cache id.
 */
const char * KRB5_CALLCONV
krb5_mcc_get_name (krb5_context context, krb5_ccache id)
{
     return (char *) ((krb5_mcc_data *) id->data)->name;
}

/*
 * Modifies:
 * id, princ
 *
 * Effects:
 * Retrieves the primary principal from id, as set with
 * krb5_mcc_initialize.  The principal is returned is allocated
 * storage that must be freed by the caller via krb5_free_principal.
 *
 * Errors:
 * system errors
 * KRB5_CC_NOMEM
 */
krb5_error_code KRB5_CALLCONV
krb5_mcc_get_principal(krb5_context context, krb5_ccache id, krb5_principal *princ)
{
     krb5_mcc_data *ptr = (krb5_mcc_data *)id->data;
     if (!ptr->prin) {
        *princ = 0L;
        return KRB5_FCC_NOFILE;
     }
     return krb5_copy_principal(context, ptr->prin, princ);
}

krb5_error_code KRB5_CALLCONV
krb5_mcc_retrieve(krb5_context context, krb5_ccache id, krb5_flags whichfields,
		  krb5_creds *mcreds, krb5_creds *creds)
{
    return krb5_cc_retrieve_cred_default (context, id, whichfields,
					  mcreds, creds);
}

/* 
 * Non-functional stub implementation for krb5_mcc_remove
 * 
 * Errors:
 *    KRB5_CC_NOSUPP - not implemented
 */
static krb5_error_code KRB5_CALLCONV
krb5_mcc_remove_cred(krb5_context context, krb5_ccache cache, krb5_flags flags,
                     krb5_creds *creds)
{
    return KRB5_CC_NOSUPP;
}


/*
 * Requires:
 * id is a cred cache returned by krb5_mcc_resolve or
 * krb5_mcc_generate_new, but has not been opened by krb5_mcc_initialize.
 *
 * Modifies:
 * id
 * 
 * Effects:
 * Sets the operational flags of id to flags.
 */
krb5_error_code KRB5_CALLCONV
krb5_mcc_set_flags(krb5_context context, krb5_ccache id, krb5_flags flags)
{
    return KRB5_OK;
}

static krb5_error_code KRB5_CALLCONV
krb5_mcc_get_flags(krb5_context context, krb5_ccache id, krb5_flags *flags)
{
    *flags = 0;
    return KRB5_OK;
}

/* store: Save away creds in the ccache.  */
krb5_error_code KRB5_CALLCONV
krb5_mcc_store(krb5_context ctx, krb5_ccache id, krb5_creds *creds)
{
    krb5_error_code err;
    krb5_mcc_link *new_node;
    krb5_mcc_data *mptr = (krb5_mcc_data *)id->data;

    new_node = malloc(sizeof(krb5_mcc_link));
    if (new_node == NULL)
	return ENOMEM;
    err = krb5_copy_creds(ctx, creds, &new_node->creds);
    if (err) {
	free(new_node);
	return err;
    }
    err = k5_cc_mutex_lock(ctx, &mptr->lock);
    if (err)
	return err;
    new_node->next = mptr->link;
    mptr->link = new_node;
    update_mcc_change_time(mptr);
    k5_cc_mutex_unlock(ctx, &mptr->lock);
    return 0;
}

static krb5_error_code KRB5_CALLCONV
krb5_mcc_ptcursor_new(
    krb5_context context,
    krb5_cc_ptcursor *cursor)
{
    krb5_error_code ret = 0;
    krb5_cc_ptcursor n = NULL;
    struct krb5_mcc_ptcursor_data *cdata = NULL;

    *cursor = NULL;

    n = malloc(sizeof(*n));
    if (n == NULL)
	return ENOMEM;
    n->ops = &krb5_mcc_ops;
    cdata = malloc(sizeof(struct krb5_mcc_ptcursor_data));
    if (cdata == NULL) {
	ret = ENOMEM;
	goto errout;
    }
    n->data = cdata;
    ret = k5_cc_mutex_lock(context, &krb5int_mcc_mutex);
    if (ret)
	goto errout;
    cdata->cur = mcc_head;
    ret = k5_cc_mutex_unlock(context, &krb5int_mcc_mutex);
    if (ret)
	goto errout;

errout:
    if (ret) {
	krb5_mcc_ptcursor_free(context, &n);
    }
    *cursor = n;
    return ret;
}

static krb5_error_code KRB5_CALLCONV
krb5_mcc_ptcursor_next(
    krb5_context context,
    krb5_cc_ptcursor cursor,
    krb5_ccache *ccache)
{
    krb5_error_code ret = 0;
    struct krb5_mcc_ptcursor_data *cdata = NULL;

    *ccache = NULL;
    cdata = cursor->data;
    if (cdata->cur == NULL)
	return 0;

    *ccache = malloc(sizeof(**ccache));
    if (*ccache == NULL)
	return ENOMEM;

    (*ccache)->ops = &krb5_mcc_ops;
    (*ccache)->data = cdata->cur->cache;
    ret = k5_cc_mutex_lock(context, &krb5int_mcc_mutex);
    if (ret)
	goto errout;
    cdata->cur = cdata->cur->next;
    ret = k5_cc_mutex_unlock(context, &krb5int_mcc_mutex);
    if (ret)
	goto errout;
errout:
    if (ret && *ccache != NULL) {
	free(*ccache);
	*ccache = NULL;
    }
    return ret;
}

static krb5_error_code KRB5_CALLCONV
krb5_mcc_ptcursor_free(
    krb5_context context,
    krb5_cc_ptcursor *cursor)
{
    if (*cursor == NULL)
	return 0;
    if ((*cursor)->data != NULL)
	free((*cursor)->data);
    free(*cursor);
    *cursor = NULL;
    return 0;
}

static krb5_error_code KRB5_CALLCONV 
krb5_mcc_last_change_time(
    krb5_context context,
    krb5_ccache id, 
    krb5_timestamp *change_time)
{
    krb5_error_code ret = 0;
    krb5_mcc_data *data = (krb5_mcc_data *) id->data;
    
    *change_time = 0;
    
    ret = k5_cc_mutex_lock(context, &data->lock);
    if (!ret) {
        *change_time = data->changetime;
        k5_cc_mutex_unlock(context, &data->lock);
    }

    return ret;
}

/*
 Utility routine: called by krb5_mcc_* functions to keep 
 result of krb5_mcc_last_change_time up to date
 */

static void
update_mcc_change_time(krb5_mcc_data *d)
{
    krb5_timestamp now_time = time(NULL);
    d->changetime = (d->changetime >= now_time) ? 
	d->changetime + 1 : now_time;
}

static krb5_error_code KRB5_CALLCONV 
krb5_mcc_lock(krb5_context context, krb5_ccache id)
{
    krb5_error_code ret = 0;
    krb5_mcc_data *data = (krb5_mcc_data *) id->data;
    ret = k5_cc_mutex_lock(context, &data->lock);
    return ret;
}

static krb5_error_code KRB5_CALLCONV 
krb5_mcc_unlock(krb5_context context, krb5_ccache id)
{
    krb5_error_code ret = 0;
    krb5_mcc_data *data = (krb5_mcc_data *) id->data;
    ret = k5_cc_mutex_unlock(context, &data->lock);
    return ret;
}

const krb5_cc_ops krb5_mcc_ops = {
     0,
     "MEMORY",
     krb5_mcc_get_name,
     krb5_mcc_resolve,
     krb5_mcc_generate_new,
     krb5_mcc_initialize,
     krb5_mcc_destroy,
     krb5_mcc_close,
     krb5_mcc_store,
     krb5_mcc_retrieve,
     krb5_mcc_get_principal,
     krb5_mcc_start_seq_get,
     krb5_mcc_next_cred,
     krb5_mcc_end_seq_get,
     krb5_mcc_remove_cred,
     krb5_mcc_set_flags,
     krb5_mcc_get_flags,
     krb5_mcc_ptcursor_new,
     krb5_mcc_ptcursor_next,
     krb5_mcc_ptcursor_free,
     NULL, /* move */
     krb5_mcc_last_change_time,
     NULL, /* wasdefault */
     krb5_mcc_lock,
     krb5_mcc_unlock,
};
