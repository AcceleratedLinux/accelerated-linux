/* 
   Unix SMB/CIFS implementation.
   tdb utility functions
   Copyright (C) Andrew Tridgell   1992-1998
   Copyright (C) Rafal Szczesniak  2002
   
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
#undef malloc
#undef realloc
#undef calloc
#undef strdup

/* these are little tdb utility functions that are meant to make
   dealing with a tdb database a little less cumbersome in Samba */

static SIG_ATOMIC_T gotalarm;

/***************************************************************
 Signal function to tell us we timed out.
****************************************************************/

static void gotalarm_sig(void)
{
	gotalarm = 1;
}

/***************************************************************
 Make a TDB_DATA and keep the const warning in one place
****************************************************************/

TDB_DATA make_tdb_data(const char *dptr, size_t dsize)
{
	TDB_DATA ret;
	ret.dptr = CONST_DISCARD(char *, dptr);
	ret.dsize = dsize;
	return ret;
}

TDB_DATA string_tdb_data(const char *string)
{
	return make_tdb_data(string, strlen(string));
}

/****************************************************************************
 Lock a chain with timeout (in seconds).
****************************************************************************/

static int tdb_chainlock_with_timeout_internal( TDB_CONTEXT *tdb, TDB_DATA key, unsigned int timeout, int rw_type)
{
	/* Allow tdb_chainlock to be interrupted by an alarm. */
	int ret;
	gotalarm = 0;

	if (timeout) {
		CatchSignal(SIGALRM, SIGNAL_CAST gotalarm_sig);
		alarm(timeout);
	}

	if (rw_type == F_RDLCK)
		ret = tdb_chainlock_read(tdb, key);
	else
		ret = tdb_chainlock(tdb, key);

	if (timeout) {
		alarm(0);
		CatchSignal(SIGALRM, SIGNAL_CAST SIG_IGN);
		if (gotalarm) {
			DEBUG(0,("tdb_chainlock_with_timeout_internal: alarm (%u) timed out for key %s in tdb %s\n",
				timeout, key.dptr, tdb_name(tdb)));
			/* TODO: If we time out waiting for a lock, it might
			 * be nice to use F_GETLK to get the pid of the
			 * process currently holding the lock and print that
			 * as part of the debugging message. -- mbp */
			return -1;
		}
	}

	return ret;
}

/****************************************************************************
 Write lock a chain. Return -1 if timeout or lock failed.
****************************************************************************/

int tdb_chainlock_with_timeout( TDB_CONTEXT *tdb, TDB_DATA key, unsigned int timeout)
{
	return tdb_chainlock_with_timeout_internal(tdb, key, timeout, F_WRLCK);
}

/****************************************************************************
 Lock a chain by string. Return -1 if timeout or lock failed.
****************************************************************************/

int tdb_lock_bystring(TDB_CONTEXT *tdb, const char *keyval)
{
	TDB_DATA key = make_tdb_data(keyval, strlen(keyval)+1);
	
	return tdb_chainlock(tdb, key);
}

int tdb_lock_bystring_with_timeout(TDB_CONTEXT *tdb, const char *keyval,
				   int timeout)
{
	TDB_DATA key = make_tdb_data(keyval, strlen(keyval)+1);
	
	return tdb_chainlock_with_timeout(tdb, key, timeout);
}

/****************************************************************************
 Unlock a chain by string.
****************************************************************************/

void tdb_unlock_bystring(TDB_CONTEXT *tdb, const char *keyval)
{
	TDB_DATA key = make_tdb_data(keyval, strlen(keyval)+1);

	tdb_chainunlock(tdb, key);
}

/****************************************************************************
 Read lock a chain by string. Return -1 if timeout or lock failed.
****************************************************************************/

int tdb_read_lock_bystring_with_timeout(TDB_CONTEXT *tdb, const char *keyval, unsigned int timeout)
{
	TDB_DATA key = make_tdb_data(keyval, strlen(keyval)+1);
	
	return tdb_chainlock_with_timeout_internal(tdb, key, timeout, F_RDLCK);
}

/****************************************************************************
 Read unlock a chain by string.
****************************************************************************/

void tdb_read_unlock_bystring(TDB_CONTEXT *tdb, const char *keyval)
{
	TDB_DATA key = make_tdb_data(keyval, strlen(keyval)+1);
	
	tdb_chainunlock_read(tdb, key);
}


/****************************************************************************
 Fetch a int32 value by a arbitrary blob key, return -1 if not found.
 Output is int32 in native byte order.
****************************************************************************/

int32 tdb_fetch_int32_byblob(TDB_CONTEXT *tdb, const char *keyval, size_t len)
{
	TDB_DATA key = make_tdb_data(keyval, len);
	TDB_DATA data;
	int32 ret;

	data = tdb_fetch(tdb, key);
	if (!data.dptr || data.dsize != sizeof(int32)) {
		SAFE_FREE(data.dptr);
		return -1;
	}

	ret = IVAL(data.dptr,0);
	SAFE_FREE(data.dptr);
	return ret;
}

/****************************************************************************
 Fetch a int32 value by string key, return -1 if not found.
 Output is int32 in native byte order.
****************************************************************************/

int32 tdb_fetch_int32(TDB_CONTEXT *tdb, const char *keystr)
{
	return tdb_fetch_int32_byblob(tdb, keystr, strlen(keystr) + 1);
}

/****************************************************************************
 Store a int32 value by an arbitary blob key, return 0 on success, -1 on failure.
 Input is int32 in native byte order. Output in tdb is in little-endian.
****************************************************************************/

int tdb_store_int32_byblob(TDB_CONTEXT *tdb, const char *keystr, size_t len, int32 v)
{
	TDB_DATA key = make_tdb_data(keystr, len);
	TDB_DATA data;
	int32 v_store;

	SIVAL(&v_store,0,v);
	data.dptr = (char *)&v_store;
	data.dsize = sizeof(int32);

	return tdb_store(tdb, key, data, TDB_REPLACE);
}

/****************************************************************************
 Store a int32 value by string key, return 0 on success, -1 on failure.
 Input is int32 in native byte order. Output in tdb is in little-endian.
****************************************************************************/

int tdb_store_int32(TDB_CONTEXT *tdb, const char *keystr, int32 v)
{
	return tdb_store_int32_byblob(tdb, keystr, strlen(keystr) + 1, v);
}

/****************************************************************************
 Fetch a uint32 value by a arbitrary blob key, return -1 if not found.
 Output is uint32 in native byte order.
****************************************************************************/

BOOL tdb_fetch_uint32_byblob(TDB_CONTEXT *tdb, const char *keyval, size_t len, uint32 *value)
{
	TDB_DATA key = make_tdb_data(keyval, len);
	TDB_DATA data;

	data = tdb_fetch(tdb, key);
	if (!data.dptr || data.dsize != sizeof(uint32)) {
		SAFE_FREE(data.dptr);
		return False;
	}

	*value = IVAL(data.dptr,0);
	SAFE_FREE(data.dptr);
	return True;
}

/****************************************************************************
 Fetch a uint32 value by string key, return -1 if not found.
 Output is uint32 in native byte order.
****************************************************************************/

BOOL tdb_fetch_uint32(TDB_CONTEXT *tdb, const char *keystr, uint32 *value)
{
	return tdb_fetch_uint32_byblob(tdb, keystr, strlen(keystr) + 1, value);
}

/****************************************************************************
 Store a uint32 value by an arbitary blob key, return 0 on success, -1 on failure.
 Input is uint32 in native byte order. Output in tdb is in little-endian.
****************************************************************************/

BOOL tdb_store_uint32_byblob(TDB_CONTEXT *tdb, const char *keystr, size_t len, uint32 value)
{
	TDB_DATA key = make_tdb_data(keystr, len);
	TDB_DATA data;
	uint32 v_store;
	BOOL ret = True;

	SIVAL(&v_store, 0, value);
	data.dptr = (char *)&v_store;
	data.dsize = sizeof(uint32);

	if (tdb_store(tdb, key, data, TDB_REPLACE) == -1)
		ret = False;

	return ret;
}

/****************************************************************************
 Store a uint32 value by string key, return 0 on success, -1 on failure.
 Input is uint32 in native byte order. Output in tdb is in little-endian.
****************************************************************************/

BOOL tdb_store_uint32(TDB_CONTEXT *tdb, const char *keystr, uint32 value)
{
	return tdb_store_uint32_byblob(tdb, keystr, strlen(keystr) + 1, value);
}
/****************************************************************************
 Store a buffer by a null terminated string key.  Return 0 on success, -1
 on failure.
****************************************************************************/

int tdb_store_bystring(TDB_CONTEXT *tdb, const char *keystr, TDB_DATA data, int flags)
{
	TDB_DATA key = make_tdb_data(keystr, strlen(keystr)+1);
	
	return tdb_store(tdb, key, data, flags);
}

/****************************************************************************
 Fetch a buffer using a null terminated string key.  Don't forget to call
 free() on the result dptr.
****************************************************************************/

TDB_DATA tdb_fetch_bystring(TDB_CONTEXT *tdb, const char *keystr)
{
	TDB_DATA key = make_tdb_data(keystr, strlen(keystr)+1);

	return tdb_fetch(tdb, key);
}

/****************************************************************************
 Delete an entry using a null terminated string key. 
****************************************************************************/

int tdb_delete_bystring(TDB_CONTEXT *tdb, const char *keystr)
{
	TDB_DATA key = make_tdb_data(keystr, strlen(keystr)+1);

	return tdb_delete(tdb, key);
}

/****************************************************************************
 Atomic integer change. Returns old value. To create, set initial value in *oldval. 
****************************************************************************/

int32 tdb_change_int32_atomic(TDB_CONTEXT *tdb, const char *keystr, int32 *oldval, int32 change_val)
{
	int32 val;
	int32 ret = -1;

	if (tdb_lock_bystring(tdb, keystr) == -1)
		return -1;

	if ((val = tdb_fetch_int32(tdb, keystr)) == -1) {
		/* The lookup failed */
		if (tdb_error(tdb) != TDB_ERR_NOEXIST) {
			/* but not because it didn't exist */
			goto err_out;
		}
		
		/* Start with 'old' value */
		val = *oldval;

	} else {
		/* It worked, set return value (oldval) to tdb data */
		*oldval = val;
	}

	/* Increment value for storage and return next time */
	val += change_val;
		
	if (tdb_store_int32(tdb, keystr, val) == -1)
		goto err_out;

	ret = 0;

  err_out:

	tdb_unlock_bystring(tdb, keystr);
	return ret;
}

/****************************************************************************
 Atomic unsigned integer change. Returns old value. To create, set initial value in *oldval. 
****************************************************************************/

BOOL tdb_change_uint32_atomic(TDB_CONTEXT *tdb, const char *keystr, uint32 *oldval, uint32 change_val)
{
	uint32 val;
	BOOL ret = False;

	if (tdb_lock_bystring(tdb, keystr) == -1)
		return False;

	if (!tdb_fetch_uint32(tdb, keystr, &val)) {
		/* It failed */
		if (tdb_error(tdb) != TDB_ERR_NOEXIST) { 
			/* and not because it didn't exist */
			goto err_out;
		}

		/* Start with 'old' value */
		val = *oldval;

	} else {
		/* it worked, set return value (oldval) to tdb data */
		*oldval = val;

	}

	/* get a new value to store */
	val += change_val;
		
	if (!tdb_store_uint32(tdb, keystr, val))
		goto err_out;

	ret = True;

  err_out:

	tdb_unlock_bystring(tdb, keystr);
	return ret;
}

/****************************************************************************
 Useful pair of routines for packing/unpacking data consisting of
 integers and strings.
****************************************************************************/

size_t tdb_pack_va(char *buf, int bufsize, const char *fmt, va_list ap)
{
	uint8 bt;
	uint16 w;
	uint32 d;
	int i;
	void *p;
	int len;
	char *s;
	char c;
	char *buf0 = buf;
	const char *fmt0 = fmt;
	int bufsize0 = bufsize;

	while (*fmt) {
		switch ((c = *fmt++)) {
		case 'b': /* unsigned 8-bit integer */
			len = 1;
			bt = (uint8)va_arg(ap, int);
			if (bufsize && bufsize >= len)
				SSVAL(buf, 0, bt);
			break;
		case 'w': /* unsigned 16-bit integer */
			len = 2;
			w = (uint16)va_arg(ap, int);
			if (bufsize && bufsize >= len)
				SSVAL(buf, 0, w);
			break;
		case 'd': /* signed 32-bit integer (standard int in most systems) */
			len = 4;
			d = va_arg(ap, uint32);
			if (bufsize && bufsize >= len)
				SIVAL(buf, 0, d);
			break;
		case 'p': /* pointer */
			len = 4;
			p = va_arg(ap, void *);
			d = p?1:0;
			if (bufsize && bufsize >= len)
				SIVAL(buf, 0, d);
			break;
		case 'P': /* null-terminated string */
			s = va_arg(ap,char *);
			w = strlen(s);
			len = w + 1;
			if (bufsize && bufsize >= len)
				memcpy(buf, s, len);
			break;
		case 'f': /* null-terminated string */
			s = va_arg(ap,char *);
			w = strlen(s);
			len = w + 1;
			if (bufsize && bufsize >= len)
				memcpy(buf, s, len);
			break;
		case 'B': /* fixed-length string */
			i = va_arg(ap, int);
			s = va_arg(ap, char *);
			len = 4+i;
			if (bufsize && bufsize >= len) {
				SIVAL(buf, 0, i);
				memcpy(buf+4, s, i);
			}
			break;
		default:
			DEBUG(0,("Unknown tdb_pack format %c in %s\n", 
				 c, fmt));
			len = 0;
			break;
		}

		buf += len;
		if (bufsize)
			bufsize -= len;
		if (bufsize < 0)
			bufsize = 0;
	}

	DEBUG(18,("tdb_pack_va(%s, %d) -> %d\n", 
		 fmt0, bufsize0, (int)PTR_DIFF(buf, buf0)));
	
	return PTR_DIFF(buf, buf0);
}

size_t tdb_pack(char *buf, int bufsize, const char *fmt, ...)
{
	va_list ap;
	size_t result;

	va_start(ap, fmt);
	result = tdb_pack_va(buf, bufsize, fmt, ap);
	va_end(ap);
	return result;
}

BOOL tdb_pack_append(TALLOC_CTX *mem_ctx, uint8 **buf, size_t *len,
		     const char *fmt, ...)
{
	va_list ap;
	size_t len1, len2;

	va_start(ap, fmt);
	len1 = tdb_pack_va(NULL, 0, fmt, ap);
	va_end(ap);

	if (mem_ctx != NULL) {
		*buf = TALLOC_REALLOC_ARRAY(mem_ctx, *buf, uint8,
					    (*len) + len1);
	} else {
		*buf = SMB_REALLOC_ARRAY(*buf, uint8, (*len) + len1);
	}

	if (*buf == NULL) {
		return False;
	}

	va_start(ap, fmt);
	len2 = tdb_pack_va((char *)(*buf)+(*len), len1, fmt, ap);
	va_end(ap);

	if (len1 != len2) {
		return False;
	}

	*len += len2;

	return True;
}

/****************************************************************************
 Useful pair of routines for packing/unpacking data consisting of
 integers and strings.
****************************************************************************/

int tdb_unpack(char *buf, int bufsize, const char *fmt, ...)
{
	va_list ap;
	uint8 *bt;
	uint16 *w;
	uint32 *d;
	int len;
	int *i;
	void **p;
	char *s, **b;
	char c;
	char *buf0 = buf;
	const char *fmt0 = fmt;
	int bufsize0 = bufsize;

	va_start(ap, fmt);
	
	while (*fmt) {
		switch ((c=*fmt++)) {
		case 'b':
			len = 1;
			bt = va_arg(ap, uint8 *);
			if (bufsize < len)
				goto no_space;
			*bt = SVAL(buf, 0);
			break;
		case 'w':
			len = 2;
			w = va_arg(ap, uint16 *);
			if (bufsize < len)
				goto no_space;
			*w = SVAL(buf, 0);
			break;
		case 'd':
			len = 4;
			d = va_arg(ap, uint32 *);
			if (bufsize < len)
				goto no_space;
			*d = IVAL(buf, 0);
			break;
		case 'p':
			len = 4;
			p = va_arg(ap, void **);
			if (bufsize < len)
				goto no_space;
			/* 
			 * This isn't a real pointer - only a token (1 or 0)
			 * to mark the fact a pointer is present.
			 */

			*p = (void *)(IVAL(buf, 0) ? (void *)1 : NULL);
			break;
		case 'P':
			s = va_arg(ap,char *);
			len = strlen(buf) + 1;
			if (bufsize < len || len > sizeof(pstring))
				goto no_space;
			memcpy(s, buf, len);
			break;
		case 'f':
			s = va_arg(ap,char *);
			len = strlen(buf) + 1;
			if (bufsize < len || len > sizeof(fstring))
				goto no_space;
			memcpy(s, buf, len);
			break;
		case 'B':
			i = va_arg(ap, int *);
			b = va_arg(ap, char **);
			len = 4;
			if (bufsize < len)
				goto no_space;
			*i = IVAL(buf, 0);
			if (! *i) {
				*b = NULL;
				break;
			}
			len += *i;
			if (bufsize < len)
				goto no_space;
			*b = (char *)SMB_MALLOC(*i);
			if (! *b)
				goto no_space;
			memcpy(*b, buf+4, *i);
			break;
		default:
			DEBUG(0,("Unknown tdb_unpack format %c in %s\n", 
				 c, fmt));

			len = 0;
			break;
		}

		buf += len;
		bufsize -= len;
	}

	va_end(ap);

	DEBUG(18,("tdb_unpack(%s, %d) -> %d\n", 
		 fmt0, bufsize0, (int)PTR_DIFF(buf, buf0)));

	return PTR_DIFF(buf, buf0);

 no_space:
	return -1;
}


/****************************************************************************
 Log tdb messages via DEBUG().
****************************************************************************/

static void tdb_log(TDB_CONTEXT *tdb, enum tdb_debug_level level, const char *format, ...)
{
	va_list ap;
	char *ptr = NULL;

	va_start(ap, format);
	vasprintf(&ptr, format, ap);
	va_end(ap);
	
	if (!ptr || !*ptr)
		return;

	DEBUG((int)level, ("tdb(%s): %s", tdb_name(tdb) ? tdb_name(tdb) : "unnamed", ptr));
	SAFE_FREE(ptr);
}

/****************************************************************************
 Like tdb_open() but also setup a logging function that redirects to
 the samba DEBUG() system.
****************************************************************************/

TDB_CONTEXT *tdb_open_log(const char *name, int hash_size, int tdb_flags,
			  int open_flags, mode_t mode)
{
	TDB_CONTEXT *tdb;
	struct tdb_logging_context log_ctx;

	if (!lp_use_mmap())
		tdb_flags |= TDB_NOMMAP;

	log_ctx.log_fn = tdb_log;
	log_ctx.log_private = NULL;

	tdb = tdb_open_ex(name, hash_size, tdb_flags, 
			  open_flags, mode, &log_ctx, NULL);
	if (!tdb)
		return NULL;

	return tdb;
}

/****************************************************************************
 Allow tdb_delete to be used as a tdb_traversal_fn.
****************************************************************************/

int tdb_traverse_delete_fn(TDB_CONTEXT *the_tdb, TDB_DATA key, TDB_DATA dbuf,
                     void *state)
{
    return tdb_delete(the_tdb, key);
}



/**
 * Search across the whole tdb for keys that match the given pattern
 * return the result as a list of keys
 *
 * @param tdb pointer to opened tdb file context
 * @param pattern searching pattern used by fnmatch(3) functions
 *
 * @return list of keys found by looking up with given pattern
 **/
TDB_LIST_NODE *tdb_search_keys(TDB_CONTEXT *tdb, const char* pattern)
{
	TDB_DATA key, next;
	TDB_LIST_NODE *list = NULL;
	TDB_LIST_NODE *rec = NULL;
	
	for (key = tdb_firstkey(tdb); key.dptr; key = next) {
		/* duplicate key string to ensure null-termination */
		char *key_str = (char*) SMB_STRNDUP(key.dptr, key.dsize);
		if (!key_str) {
			DEBUG(0, ("tdb_search_keys: strndup() failed!\n"));
			smb_panic("strndup failed!\n");
		}
		
		DEBUG(18, ("checking %s for match to pattern %s\n", key_str, pattern));
		
		next = tdb_nextkey(tdb, key);

		/* do the pattern checking */
		if (fnmatch(pattern, key_str, 0) == 0) {
			rec = SMB_MALLOC_P(TDB_LIST_NODE);
			ZERO_STRUCTP(rec);

			rec->node_key = key;
	
			DLIST_ADD_END(list, rec, TDB_LIST_NODE *);
		
			DEBUG(18, ("checking %s matched pattern %s\n", key_str, pattern));
		} else {
			free(key.dptr);
		}
		
		/* free duplicated key string */
		free(key_str);
	}
	
	return list;

}


/**
 * Free the list returned by tdb_search_keys
 *
 * @param node list of results found by tdb_search_keys
 **/
void tdb_search_list_free(TDB_LIST_NODE* node)
{
	TDB_LIST_NODE *next_node;
	
	while (node) {
		next_node = node->next;
		SAFE_FREE(node->node_key.dptr);
		SAFE_FREE(node);
		node = next_node;
	};
}

/****************************************************************************
 tdb_store, wrapped in a transaction. This way we make sure that a process
 that dies within writing does not leave a corrupt tdb behind.
****************************************************************************/

int tdb_trans_store(struct tdb_context *tdb, TDB_DATA key, TDB_DATA dbuf,
		    int flag)
{
	int res;

	if ((res = tdb_transaction_start(tdb)) != 0) {
		DEBUG(5, ("tdb_transaction_start failed\n"));
		return res;
	}

	if ((res = tdb_store(tdb, key, dbuf, flag)) != 0) {
		DEBUG(10, ("tdb_store failed\n"));
		if (tdb_transaction_cancel(tdb) != 0) {
			smb_panic("Cancelling transaction failed\n");
		}
		return res;
	}

	if ((res = tdb_transaction_commit(tdb)) != 0) {
		DEBUG(5, ("tdb_transaction_commit failed\n"));
	}

	return res;
}

/****************************************************************************
 tdb_delete, wrapped in a transaction. This way we make sure that a process
 that dies within deleting does not leave a corrupt tdb behind.
****************************************************************************/

int tdb_trans_delete(struct tdb_context *tdb, TDB_DATA key)
{
	int res;

	if ((res = tdb_transaction_start(tdb)) != 0) {
		DEBUG(5, ("tdb_transaction_start failed\n"));
		return res;
	}

	if ((res = tdb_delete(tdb, key)) != 0) {
		DEBUG(10, ("tdb_delete failed\n"));
		if (tdb_transaction_cancel(tdb) != 0) {
			smb_panic("Cancelling transaction failed\n");
		}
		return res;
	}

	if ((res = tdb_transaction_commit(tdb)) != 0) {
		DEBUG(5, ("tdb_transaction_commit failed\n"));
	}

	return res;
}

/*
 Log tdb messages via DEBUG().
*/
static void tdb_wrap_log(TDB_CONTEXT *tdb, enum tdb_debug_level level, 
			 const char *format, ...) PRINTF_ATTRIBUTE(3,4);

static void tdb_wrap_log(TDB_CONTEXT *tdb, enum tdb_debug_level level, 
			 const char *format, ...)
{
	va_list ap;
	char *ptr = NULL;
	int debuglevel = 0;

	va_start(ap, format);
	vasprintf(&ptr, format, ap);
	va_end(ap);
	
	switch (level) {
	case TDB_DEBUG_FATAL:
		debug_level = 0;
		break;
	case TDB_DEBUG_ERROR:
		debuglevel = 1;
		break;
	case TDB_DEBUG_WARNING:
		debuglevel = 2;
		break;
	case TDB_DEBUG_TRACE:
		debuglevel = 5;
		break;
	default:
		debuglevel = 0;
	}		

	if (ptr != NULL) {
		const char *name = tdb_name(tdb);
		DEBUG(debuglevel, ("tdb(%s): %s", name ? name : "unnamed", ptr));
		free(ptr);
	}
}

static struct tdb_wrap *tdb_list;

/* destroy the last connection to a tdb */
static int tdb_wrap_destructor(struct tdb_wrap *w)
{
	tdb_close(w->tdb);
	DLIST_REMOVE(tdb_list, w);
	return 0;
}				 

/*
  wrapped connection to a tdb database
  to close just talloc_free() the tdb_wrap pointer
 */
struct tdb_wrap *tdb_wrap_open(TALLOC_CTX *mem_ctx,
			       const char *name, int hash_size, int tdb_flags,
			       int open_flags, mode_t mode)
{
	struct tdb_wrap *w;
	struct tdb_logging_context log_ctx;
	log_ctx.log_fn = tdb_wrap_log;

	for (w=tdb_list;w;w=w->next) {
		if (strcmp(name, w->name) == 0) {
			/*
			 * Yes, talloc_reference is exactly what we want
			 * here. Otherwise we would have to implement our own
			 * reference counting.
			 */
			return talloc_reference(mem_ctx, w);
		}
	}

	w = talloc(mem_ctx, struct tdb_wrap);
	if (w == NULL) {
		return NULL;
	}

	if (!(w->name = talloc_strdup(w, name))) {
		talloc_free(w);
		return NULL;
	}

	w->tdb = tdb_open_ex(name, hash_size, tdb_flags, 
			     open_flags, mode, &log_ctx, NULL);
	if (w->tdb == NULL) {
		talloc_free(w);
		return NULL;
	}

	talloc_set_destructor(w, tdb_wrap_destructor);

	DLIST_ADD(tdb_list, w);

	return w;
}
