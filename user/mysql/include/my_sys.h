/* Copyright (C) 2000 MySQL AB

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

#ifndef _my_sys_h
#define _my_sys_h
C_MODE_START

#ifdef HAVE_AIOWAIT
#include <sys/asynch.h>			/* Used by record-cache */
typedef struct my_aio_result {
  aio_result_t result;
  int	       pending;
} my_aio_result;
#endif

#ifndef THREAD
extern int NEAR my_errno;		/* Last error in mysys */
#else
#include <my_pthread.h>
#endif

#ifndef _m_ctype_h
#include <m_ctype.h>                    /* for CHARSET_INFO */
#endif

#include <stdarg.h>

#define MYSYS_PROGRAM_USES_CURSES()  { error_handler_hook = my_message_curses;	mysys_uses_curses=1; }
#define MYSYS_PROGRAM_DONT_USE_CURSES()  { error_handler_hook = my_message_no_curses; mysys_uses_curses=0;}
#define MY_INIT(name);		{ my_progname= name; my_init(); }

#define MAXMAPS		(4)	/* Number of error message maps */
#define ERRMOD		(1000)	/* Max number of errors in a map */
#define ERRMSGSIZE	(SC_MAXWIDTH)	/* Max length of a error message */
#define NRERRBUFFS	(2)	/* Buffers for parameters */
#define MY_FILE_ERROR	((uint) ~0)

	/* General bitmaps for my_func's */
#define MY_FFNF		1	/* Fatal if file not found */
#define MY_FNABP	2	/* Fatal if not all bytes read/writen */
#define MY_NABP		4	/* Error if not all bytes read/writen */
#define MY_FAE		8	/* Fatal if any error */
#define MY_WME		16	/* Write message on error */
#define MY_WAIT_IF_FULL 32	/* Wait and try again if disk full error */
#define MY_RAID         64      /* Support for RAID (not the "Johnson&Johnson"-s one ;) */
#define MY_FULL_IO     512      /* For my_read - loop intil I/O
				   is complete
				*/
#define MY_DONT_CHECK_FILESIZE 128	/* Option to init_io_cache() */
#define MY_LINK_WARNING 32	/* my_redel() gives warning if links */
#define MY_COPYTIME	64	/* my_redel() copys time */
#define MY_DELETE_OLD	256	/* my_create_with_symlink() */
#define MY_RESOLVE_LINK 128	/* my_realpath(); Only resolve links */
#define MY_HOLD_ORIGINAL_MODES 128  /* my_copy() holds to file modes */
#define MY_REDEL_MAKE_BACKUP 256
#define MY_SEEK_NOT_DONE 32	/* my_lock may have to do a seek */
#define MY_DONT_WAIT	64	/* my_lock() don't wait if can't lock */
#define MY_ZEROFILL	32	/* my_malloc(), fill array with zero */
#define MY_ALLOW_ZERO_PTR 64	/* my_realloc() ; zero ptr -> malloc */
#define MY_FREE_ON_ERROR 128	/* my_realloc() ; Free old ptr on error */
#define MY_HOLD_ON_ERROR 256	/* my_realloc() ; Return old ptr on error */
#define MY_THREADSAFE	128	/* pread/pwrite:  Don't allow interrupts */
#define MY_DONT_OVERWRITE_FILE 1024	/* my_copy; Don't overwrite file */

#define MY_CHECK_ERROR	1	/* Params to my_end; Check open-close */
#define MY_GIVE_INFO	2	/* Give time info about process*/

#define ME_HIGHBYTE	8	/* Shift for colours */
#define ME_NOCUR	1	/* Don't use curses message */
#define ME_OLDWIN	2	/* Use old window */
#define ME_BELL		4	/* Ring bell then printing message */
#define ME_HOLDTANG	8	/* Don't delete last keys */
#define ME_WAITTOT	16	/* Wait for errtime secs of for a action */
#define ME_WAITTANG	32	/* Wait for a user action  */
#define ME_NOREFRESH	64	/* Dont refresh screen */
#define ME_NOINPUT	128	/* Dont use the input libary */
#define ME_COLOUR1	((1 << ME_HIGHBYTE))	/* Possibly error-colours */
#define ME_COLOUR2	((2 << ME_HIGHBYTE))
#define ME_COLOUR3	((3 << ME_HIGHBYTE))

	/* Bits in last argument to fn_format */
#define MY_REPLACE_DIR		1	/* replace dir in name with 'dir' */
#define MY_REPLACE_EXT		2	/* replace extension with 'ext' */
#define MY_UNPACK_FILENAME	4	/* Unpack name (~ -> home) */
#define MY_PACK_FILENAME	8	/* Pack name (home -> ~) */
#define MY_RESOLVE_SYMLINKS	16	/* Resolve all symbolic links */
#define MY_RETURN_REAL_PATH	32	/* return full path for file */
#define MY_SAFE_PATH		64	/* Return NULL if too long path */
#define MY_RELATIVE_PATH	128	/* name is relative to 'dir' */

	/* My seek flags */
#define MY_SEEK_SET	0
#define MY_SEEK_CUR	1
#define MY_SEEK_END	2

        /* My charsets_list flags */
#define MY_NO_SETS       0
#define MY_COMPILED_SETS 1      /* show compiled-in sets */
#define MY_CONFIG_SETS   2      /* sets that have a *.conf file */
#define MY_INDEX_SETS    4      /* all sets listed in the Index file */
#define MY_LOADED_SETS    8      /* the sets that are currently loaded */

	/* Some constants */
#define MY_WAIT_FOR_USER_TO_FIX_PANIC	60	/* in seconds */
#define MY_WAIT_GIVE_USER_A_MESSAGE	10	/* Every 10 times of prev */
#define MIN_COMPRESS_LENGTH		50	/* Don't compress small bl. */
#define DEFAULT_KEYCACHE_BLOCK_SIZE	1024
#define MAX_KEYCACHE_BLOCK_SIZE		16384

	/* root_alloc flags */
#define MY_KEEP_PREALLOC	1
#define MY_MARK_BLOCKS_FREE     2  /* move used to free list and reuse them */

	/* defines when allocating data */

#ifdef SAFEMALLOC
#define my_malloc(SZ,FLAG) _mymalloc((SZ), __FILE__, __LINE__, FLAG )
#define my_malloc_ci(SZ,FLAG) _mymalloc((SZ), sFile, uLine, FLAG )
#define my_realloc(PTR,SZ,FLAG) _myrealloc((PTR), (SZ), __FILE__, __LINE__, FLAG )
#define my_checkmalloc() _sanity( __FILE__, __LINE__ )
#define my_free(PTR,FLAG) _myfree((PTR), __FILE__, __LINE__,FLAG)
#define my_memdup(A,B,C) _my_memdup((A),(B), __FILE__,__LINE__,C)
#define my_strdup(A,C) _my_strdup((A), __FILE__,__LINE__,C)
#define my_strdup_with_length(A,B,C) _my_strdup_with_length((A),(B),__FILE__,__LINE__,C)
#define QUICK_SAFEMALLOC sf_malloc_quick=1
#define NORMAL_SAFEMALLOC sf_malloc_quick=0
extern uint sf_malloc_prehunc,sf_malloc_endhunc,sf_malloc_quick;
extern ulonglong sf_malloc_mem_limit;

#define CALLER_INFO_PROTO   , const char *sFile, uint uLine
#define CALLER_INFO         , __FILE__, __LINE__
#define ORIG_CALLER_INFO    , sFile, uLine
#else
#define my_checkmalloc()
#undef TERMINATE
#define TERMINATE(A) {}
#define QUICK_SAFEMALLOC
#define NORMAL_SAFEMALLOC
extern gptr my_malloc(uint Size,myf MyFlags);
#define my_malloc_ci(SZ,FLAG) my_malloc( SZ, FLAG )
extern gptr my_realloc(gptr oldpoint,uint Size,myf MyFlags);
extern void my_no_flags_free(gptr ptr);
extern gptr my_memdup(const byte *from,uint length,myf MyFlags);
extern char *my_strdup(const char *from,myf MyFlags);
extern char *my_strdup_with_length(const byte *from, uint length,
				   myf MyFlags);
#define my_free(PTR,FG) my_no_flags_free(PTR)
#define CALLER_INFO_PROTO   /* nothing */
#define CALLER_INFO         /* nothing */
#define ORIG_CALLER_INFO    /* nothing */
#endif

#ifdef HAVE_ALLOCA
#if defined(_AIX) && !defined(__GNUC__)
#pragma alloca
#endif /* _AIX */
#if defined(__GNUC__) && !defined(HAVE_ALLOCA_H)
#define alloca __builtin_alloca
#endif /* GNUC */
#define my_alloca(SZ) alloca((size_t) (SZ))
#define my_afree(PTR) {}
#else
#define my_alloca(SZ) my_malloc(SZ,MYF(0))
#define my_afree(PTR) my_free(PTR,MYF(MY_WME))
#endif /* HAVE_ALLOCA */

#ifdef MSDOS
#ifdef __ZTC__
void * __CDECL halloc(long count,size_t length);
void   __CDECL hfree(void *ptr);
#endif
#if defined(USE_HALLOC)
#if defined(_VCM_) || defined(M_IC80386)
#undef USE_HALLOC
#endif
#endif
#ifdef USE_HALLOC
#define malloc(a) halloc((long) (a),1)
#define free(a) hfree(a)
#endif
#endif /* MSDOS */

#ifdef HAVE_ERRNO_AS_DEFINE
#include <errno.h>			/* errno is a define */
#else
extern int errno;			/* declare errno */
#endif
extern const char ** NEAR my_errmsg[];
extern char NEAR errbuff[NRERRBUFFS][ERRMSGSIZE];
extern char *home_dir;			/* Home directory for user */
extern char *my_progname;		/* program-name (printed in errors) */
extern char NEAR curr_dir[];		/* Current directory for user */
extern int (*error_handler_hook)(uint my_err, const char *str,myf MyFlags);
extern int (*fatal_error_handler_hook)(uint my_err, const char *str,
				       myf MyFlags);

/* charsets */
extern uint get_charset_number(const char *cs_name);
extern const char *get_charset_name(uint cs_number);
extern CHARSET_INFO *get_charset(uint cs_number, myf flags);
extern my_bool set_default_charset(uint cs, myf flags);
extern CHARSET_INFO *get_charset_by_name(const char *cs_name, myf flags);
extern my_bool set_default_charset_by_name(const char *cs_name, myf flags);
extern void free_charsets(void);
extern char *list_charsets(myf want_flags); /* my_free() this string... */
extern char *get_charsets_dir(char *buf);


/* statistics */
extern ulong	_my_cache_w_requests,_my_cache_write,_my_cache_r_requests,
		_my_cache_read;
extern ulong	_my_blocks_used,_my_blocks_changed;
extern uint	key_cache_block_size;
extern ulong	my_file_opened,my_stream_opened, my_tmp_file_created;
extern my_bool	key_cache_inited, my_init_done;

					/* Point to current my_message() */
extern void (*my_sigtstp_cleanup)(void),
					/* Executed before jump to shell */
	    (*my_sigtstp_restart)(void),
	    (*my_abort_hook)(int);
					/* Executed when comming from shell */
extern int NEAR my_umask,		/* Default creation mask  */
	   NEAR my_umask_dir,
	   NEAR my_recived_signals,	/* Signals we have got */
	   NEAR my_safe_to_handle_signal, /* Set when allowed to SIGTSTP */
	   NEAR my_dont_interrupt;	/* call remember_intr when set */
extern my_bool NEAR mysys_uses_curses, my_use_symdir;
extern ulong sf_malloc_cur_memory, sf_malloc_max_memory;

extern ulong	my_default_record_cache_size;
extern my_bool NEAR my_disable_locking,NEAR my_disable_async_io,
               NEAR my_disable_flush_key_blocks, NEAR my_disable_symlinks;
extern char	wild_many,wild_one,wild_prefix;
extern const char *charsets_dir;
extern char *defaults_extra_file;

typedef struct wild_file_pack	/* Struct to hold info when selecting files */
{
  uint		wilds;		/* How many wildcards */
  uint		not_pos;	/* Start of not-theese-files */
  my_string	*wild;		/* Pointer to wildcards */
} WF_PACK;

typedef struct st_typelib {	/* Different types saved here */
  uint count;			/* How many types */
  const char *name;		/* Name of typelib */
  const char **type_names;
} TYPELIB;

enum cache_type
{
  READ_CACHE,WRITE_CACHE,
  SEQ_READ_APPEND		/* sequential read or append */,
  READ_FIFO, READ_NET,WRITE_NET};

enum flush_type
{
  FLUSH_KEEP, FLUSH_RELEASE, FLUSH_IGNORE_CHANGED, FLUSH_FORCE_WRITE
};

typedef struct st_record_cache	/* Used when cacheing records */
{
  File file;
  int	rc_seek,error,inited;
  uint	rc_length,read_length,reclength;
  my_off_t rc_record_pos,end_of_file;
  byte	*rc_buff,*rc_buff2,*rc_pos,*rc_end,*rc_request_pos;
#ifdef HAVE_AIOWAIT
  int	use_async_io;
  my_aio_result aio_result;
#endif
  enum cache_type type;
} RECORD_CACHE;

enum file_type
{
  UNOPEN = 0, FILE_BY_OPEN, FILE_BY_CREATE, STREAM_BY_FOPEN, STREAM_BY_FDOPEN,
  FILE_BY_MKSTEMP, FILE_BY_DUP
};

extern struct my_file_info
{
  my_string		name;
  enum file_type	type;
#if defined(THREAD) && !defined(HAVE_PREAD)
  pthread_mutex_t	mutex;
#endif
} my_file_info[MY_NFILE];


typedef struct st_dynamic_array
{
  char *buffer;
  uint elements,max_element;
  uint alloc_increment;
  uint size_of_element;
} DYNAMIC_ARRAY;

typedef struct st_dynamic_string
{
  char *str;
  uint length,max_length,alloc_increment;
} DYNAMIC_STRING;

struct st_io_cache;
typedef int (*IO_CACHE_CALLBACK)(struct st_io_cache*);

#ifdef THREAD
typedef struct st_io_cache_share
{
  /* to sync on reads into buffer */
  pthread_mutex_t mutex;
  pthread_cond_t  cond;
  int             count, total;
  /* actual IO_CACHE that filled the buffer */
  struct st_io_cache *active;
#ifdef NOT_YET_IMPLEMENTED
  /* whether the structure should be free'd */
  my_bool alloced;
#endif
} IO_CACHE_SHARE;
#endif

typedef struct st_io_cache		/* Used when cacheing files */
{
  /* Offset in file corresponding to the first byte of byte* buffer. */
  my_off_t pos_in_file;
  /*
    The offset of end of file for READ_CACHE and WRITE_CACHE.
    For SEQ_READ_APPEND it the maximum of the actual end of file and
    the position represented by read_end.
  */
  my_off_t end_of_file;
  /* Points to current read position in the buffer */
  byte	*read_pos;
  /* the non-inclusive boundary in the buffer for the currently valid read */
  byte  *read_end;
  byte  *buffer;				/* The read buffer */
  /* Used in ASYNC_IO */
  byte  *request_pos;

  /* Only used in WRITE caches and in SEQ_READ_APPEND to buffer writes */
  byte  *write_buffer;
  /*
    Only used in SEQ_READ_APPEND, and points to the current read position
    in the write buffer. Note that reads in SEQ_READ_APPEND caches can
    happen from both read buffer (byte* buffer) and write buffer
    (byte* write_buffer).
  */
  byte *append_read_pos;
  /* Points to current write position in the write buffer */
  byte *write_pos;
  /* The non-inclusive boundary of the valid write area */
  byte *write_end;

  /*
    Current_pos and current_end are convenience variables used by
    my_b_tell() and other routines that need to know the current offset
    current_pos points to &write_pos, and current_end to &write_end in a
    WRITE_CACHE, and &read_pos and &read_end respectively otherwise
  */
  byte  **current_pos, **current_end;
#ifdef THREAD
  /*
    The lock is for append buffer used in SEQ_READ_APPEND cache
    need mutex copying from append buffer to read buffer.
  */
  pthread_mutex_t append_buffer_lock;
  /*
    The following is used when several threads are reading the
    same file in parallel. They are synchronized on disk
    accesses reading the cached part of the file asynchronously.
    It should be set to NULL to disable the feature.  Only
    READ_CACHE mode is supported.
  */
  IO_CACHE_SHARE *share;
#endif
  /*
    A caller will use my_b_read() macro to read from the cache
    if the data is already in cache, it will be simply copied with
    memcpy() and internal variables will be accordinging updated with
    no functions invoked. However, if the data is not fully in the cache,
    my_b_read() will call read_function to fetch the data. read_function
    must never be invoked directly.
  */
  int (*read_function)(struct st_io_cache *,byte *,uint);
  /*
    Same idea as in the case of read_function, except my_b_write() needs to
    be replaced with my_b_append() for a SEQ_READ_APPEND cache
  */
  int (*write_function)(struct st_io_cache *,const byte *,uint);
  /*
    Specifies the type of the cache. Depending on the type of the cache
    certain operations might not be available and yield unpredicatable
    results. Details to be documented later
  */
  enum cache_type type;
  /*
    Callbacks when the actual read I/O happens. These were added and
    are currently used for binary logging of LOAD DATA INFILE - when a
    block is read from the file, we create a block create/append event, and
    when IO_CACHE is closed, we create an end event. These functions could,
    of course be used for other things
  */
  IO_CACHE_CALLBACK pre_read;
  IO_CACHE_CALLBACK post_read;
  IO_CACHE_CALLBACK pre_close;
  void* arg;				/* for use by pre/post_read */
  char *file_name;			/* if used with 'open_cached_file' */
  char *dir,*prefix;
  File file; /* file descriptor */
  /*
    seek_not_done is set by my_b_seek() to inform the upcoming read/write
    operation that a seek needs to be preformed prior to the actual I/O
    error is 0 if the cache operation was successful, -1 if there was a
    "hard" error, and the actual number of I/O-ed bytes if the read/write was
    partial.
  */
  int	seek_not_done,error;
  /* buffer_length is memory size allocated for buffer or write_buffer */
  uint	buffer_length;
  /* read_length is the same as buffer_length except when we use async io */
  uint  read_length;
  myf	myflags;			/* Flags used to my_read/my_write */
  /*
    alloced_buffer is 1 if the buffer was allocated by init_io_cache() and
    0 if it was supplied by the user.
    Currently READ_NET is the only one that will use a buffer allocated
    somewhere else
  */
  my_bool alloced_buffer;
#ifdef HAVE_AIOWAIT
  /*
    As inidicated by ifdef, this is for async I/O, which is not currently
    used (because it's not reliable on all systems)
  */
  uint inited;
  my_off_t aio_read_pos;
  my_aio_result aio_result;
#endif
} IO_CACHE;

typedef int (*qsort2_cmp)(const void *, const void *, const void *);

	/* defines for mf_iocache */

	/* Test if buffer is inited */
#define my_b_clear(info) (info)->buffer=0
#define my_b_inited(info) (info)->buffer
#define my_b_EOF INT_MIN

#define my_b_read(info,Buffer,Count) \
  ((info)->read_pos + (Count) <= (info)->read_end ?\
   (memcpy(Buffer,(info)->read_pos,(size_t) (Count)), \
    ((info)->read_pos+=(Count)),0) :\
   (*(info)->read_function)((info),Buffer,Count))

#define my_b_write(info,Buffer,Count) \
 ((info)->write_pos + (Count) <=(info)->write_end ?\
  (memcpy((info)->write_pos, (Buffer), (size_t)(Count)),\
   ((info)->write_pos+=(Count)),0) : \
   (*(info)->write_function)((info),(Buffer),(Count)))

#define my_b_get(info) \
  ((info)->read_pos != (info)->read_end ?\
   ((info)->read_pos++, (int) (uchar) (info)->read_pos[-1]) :\
   _my_b_get(info))

	/* my_b_write_byte dosn't have any err-check */
#define my_b_write_byte(info,chr) \
  (((info)->write_pos < (info)->write_end) ?\
   ((*(info)->write_pos++)=(chr)) :\
   (_my_b_write(info,0,0) , ((*(info)->write_pos++)=(chr))))

#define my_b_fill_cache(info) \
  (((info)->read_end=(info)->read_pos),(*(info)->read_function)(info,0,0))

#define my_b_tell(info) ((info)->pos_in_file + \
			 (uint) (*(info)->current_pos - (info)->request_pos))

/* tell write offset in the SEQ_APPEND cache */
my_off_t my_b_append_tell(IO_CACHE* info);

#define my_b_bytes_in_cache(info) (uint) (*(info)->current_end - \
					  *(info)->current_pos)

#include <my_alloc.h>

	/* Prototypes for mysys and my_func functions */

extern int my_copy(const char *from,const char *to,myf MyFlags);
extern int my_append(const char *from,const char *to,myf MyFlags);
extern int my_delete(const char *name,myf MyFlags);
extern int my_getwd(my_string buf,uint size,myf MyFlags);
extern int my_setwd(const char *dir,myf MyFlags);
extern int my_lock(File fd,int op,my_off_t start, my_off_t length,myf MyFlags);
extern gptr my_once_alloc(uint Size,myf MyFlags);
extern void my_once_free(void);
extern my_string my_tempnam(const char *dir,const char *pfx,myf MyFlags);
extern File my_open(const char *FileName,int Flags,myf MyFlags);
extern File my_register_filename(File fd, const char *FileName,
				 enum file_type type_of_file,
				 uint error_message_number, myf MyFlags);
extern File my_create(const char *FileName,int CreateFlags,
		      int AccsesFlags, myf MyFlags);
extern int my_close(File Filedes,myf MyFlags);
extern File my_dup(File file, myf MyFlags);
extern int my_mkdir(const char *dir, int Flags, myf MyFlags);
extern int my_readlink(char *to, const char *filename, myf MyFlags);
extern int my_realpath(char *to, const char *filename, myf MyFlags);
extern File my_create_with_symlink(const char *linkname, const char *filename,
				   int createflags, int access_flags,
				   myf MyFlags);
extern int my_delete_with_symlink(const char *name, myf MyFlags);
extern int my_rename_with_symlink(const char *from,const char *to,myf MyFlags);
extern int my_symlink(const char *content, const char *linkname, myf MyFlags);
extern uint my_read(File Filedes,byte *Buffer,uint Count,myf MyFlags);
extern uint my_pread(File Filedes,byte *Buffer,uint Count,my_off_t offset,
		     myf MyFlags);
extern int my_rename(const char *from,const char *to,myf MyFlags);
extern my_off_t my_seek(File fd,my_off_t pos,int whence,myf MyFlags);
extern my_off_t my_tell(File fd,myf MyFlags);
extern uint my_write(File Filedes,const byte *Buffer,uint Count,
		     myf MyFlags);
extern uint my_pwrite(File Filedes,const byte *Buffer,uint Count,
		      my_off_t offset,myf MyFlags);
extern uint my_fread(FILE *stream,byte *Buffer,uint Count,myf MyFlags);
extern uint my_fwrite(FILE *stream,const byte *Buffer,uint Count,
		      myf MyFlags);
extern my_off_t my_fseek(FILE *stream,my_off_t pos,int whence,myf MyFlags);
extern my_off_t my_ftell(FILE *stream,myf MyFlags);
extern gptr _mymalloc(uint uSize,const char *sFile,
		      uint uLine, myf MyFlag);
extern gptr _myrealloc(gptr pPtr,uint uSize,const char *sFile,
		       uint uLine, myf MyFlag);
extern gptr my_multi_malloc _VARARGS((myf MyFlags, ...));
extern void _myfree(gptr pPtr,const char *sFile,uint uLine, myf MyFlag);
extern int _sanity(const char *sFile,unsigned int uLine);
extern gptr _my_memdup(const byte *from,uint length,
		       const char *sFile, uint uLine,myf MyFlag);
extern my_string _my_strdup(const char *from, const char *sFile, uint uLine,
			    myf MyFlag);
extern char *_my_strdup_with_length(const byte *from, uint length,
				    const char *sFile, uint uLine,
				    myf MyFlag);

#ifndef TERMINATE
extern void TERMINATE(FILE *file);
#endif
extern void init_glob_errs(void);
extern FILE *my_fopen(const char *FileName,int Flags,myf MyFlags);
extern FILE *my_fdopen(File Filedes,const char *name, int Flags,myf MyFlags);
extern int my_fclose(FILE *fd,myf MyFlags);
extern int my_chsize(File fd,my_off_t newlength, int filler, myf MyFlags);
extern int my_error _VARARGS((int nr,myf MyFlags, ...));
extern int my_printf_error _VARARGS((uint my_err, const char *format,
				     myf MyFlags, ...)
				    __attribute__ ((format (printf, 2, 4))));
extern int my_vsnprintf( char *str, size_t n,
                                const char *format, va_list ap );
extern int my_snprintf(char* to, size_t n, const char* fmt, ...);
extern int my_message(uint my_err, const char *str,myf MyFlags);
extern int my_message_no_curses(uint my_err, const char *str,myf MyFlags);
extern int my_message_curses(uint my_err, const char *str,myf MyFlags);
extern void my_init(void);
extern void my_end(int infoflag);
extern int my_redel(const char *from, const char *to, int MyFlags);
extern int my_copystat(const char *from, const char *to, int MyFlags);
extern my_string my_filename(File fd);

#ifndef THREAD
extern void dont_break(void);
extern void allow_break(void);
#else
#define dont_break()
#define allow_break()
#endif

extern void my_remember_signal(int signal_number,sig_handler (*func)(int));
extern void caseup(my_string str,uint length);
extern void casedn(my_string str,uint length);
extern void caseup_str(my_string str);
extern void casedn_str(my_string str);
extern void case_sort(my_string str,uint length);
extern uint dirname_part(my_string to,const char *name);
extern uint dirname_length(const char *name);
#define base_name(A) (A+dirname_length(A))
extern int test_if_hard_path(const char *dir_name);
extern char *convert_dirname(char *to, const char *from, const char *from_end);
extern void to_unix_path(my_string name);
extern my_string fn_ext(const char *name);
extern my_string fn_same(my_string toname,const char *name,int flag);
extern my_string fn_format(my_string to,const char *name,const char *dir,
			   const char *form, uint flag);
extern size_s strlength(const char *str);
extern void pack_dirname(my_string to,const char *from);
extern uint unpack_dirname(my_string to,const char *from);
extern uint cleanup_dirname(my_string to,const char *from);
extern uint system_filename(my_string to,const char *from);
extern my_string unpack_filename(my_string to,const char *from);
extern my_string intern_filename(my_string to,const char *from);
extern my_string directory_file_name(my_string dst, const char *src);
extern int pack_filename(my_string to, const char *name, size_s max_length);
extern my_string my_path(my_string to,const char *progname,
			 const char *own_pathname_part);
extern my_string my_load_path(my_string to, const char *path,
			      const char *own_path_prefix);
extern int wild_compare(const char *str,const char *wildstr);
extern my_string my_strcasestr(const char *src,const char *suffix);
extern int my_strcasecmp(const char *s,const char *t);
extern int my_strsortcmp(const char *s,const char *t);
extern int my_casecmp(const char *s,const char *t,uint length);
extern int my_sortcmp(const char *s,const char *t,uint length);
extern int my_sortncmp(const char *s,uint s_len, const char *t,uint t_len);
extern WF_PACK *wf_comp(my_string str);
extern int wf_test(struct wild_file_pack *wf_pack,const char *name);
extern void wf_end(struct wild_file_pack *buffer);
extern size_s strip_sp(my_string str);
extern void get_date(my_string to,int timeflag,time_t use_time);
extern void soundex(my_string out_pntr, my_string in_pntr,pbool remove_garbage);
extern int init_record_cache(RECORD_CACHE *info,uint cachesize,File file,
			     uint reclength,enum cache_type type,
			     pbool use_async_io);
extern int read_cache_record(RECORD_CACHE *info,byte *to);
extern int end_record_cache(RECORD_CACHE *info);
extern int write_cache_record(RECORD_CACHE *info,my_off_t filepos,
			      const byte *record,uint length);
extern int flush_write_cache(RECORD_CACHE *info);
extern long my_clock(void);
extern sig_handler sigtstp_handler(int signal_number);
extern void handle_recived_signals(void);
extern int init_key_cache(ulong use_mem);
extern int resize_key_cache(ulong use_mem);
extern byte *key_cache_read(File file,my_off_t filepos,byte* buff,uint length,
			    uint block_length,int return_buffer);
extern int key_cache_write(File file,my_off_t filepos,byte* buff,uint length,
			   uint block_length,int force_write);
extern int flush_key_blocks(int file, enum flush_type type);
extern void end_key_cache(void);
extern sig_handler my_set_alarm_variable(int signo);
extern void my_string_ptr_sort(void *base,uint items,size_s size);
extern void radixsort_for_str_ptr(uchar* base[], uint number_of_elements,
				  size_s size_of_element,uchar *buffer[]);
extern qsort_t qsort2(void *base_ptr, size_t total_elems, size_t size,
		      qsort2_cmp cmp, void *cmp_argument);
extern qsort2_cmp get_ptr_compare(uint);
extern int init_io_cache(IO_CACHE *info,File file,uint cachesize,
			 enum cache_type type,my_off_t seek_offset,
			 pbool use_async_io, myf cache_myflags);
extern my_bool reinit_io_cache(IO_CACHE *info,enum cache_type type,
			       my_off_t seek_offset,pbool use_async_io,
			       pbool clear_cache);
extern int _my_b_read(IO_CACHE *info,byte *Buffer,uint Count);
#ifdef THREAD
extern int _my_b_read_r(IO_CACHE *info,byte *Buffer,uint Count);
extern void init_io_cache_share(IO_CACHE *info,
				IO_CACHE_SHARE *s, uint num_threads);
extern void remove_io_thread(IO_CACHE *info);
#endif
extern int _my_b_seq_read(IO_CACHE *info,byte *Buffer,uint Count);
extern int _my_b_net_read(IO_CACHE *info,byte *Buffer,uint Count);
extern int _my_b_get(IO_CACHE *info);
extern int _my_b_async_read(IO_CACHE *info,byte *Buffer,uint Count);
extern int _my_b_write(IO_CACHE *info,const byte *Buffer,uint Count);
extern int my_b_append(IO_CACHE *info,const byte *Buffer,uint Count);
extern int my_b_safe_write(IO_CACHE *info,const byte *Buffer,uint Count);

extern int my_block_write(IO_CACHE *info, const byte *Buffer,
			  uint Count, my_off_t pos);
extern int _flush_io_cache(IO_CACHE *info, int need_append_buffer_lock);

#define flush_io_cache(info) _flush_io_cache((info),1)

extern int end_io_cache(IO_CACHE *info);
extern uint my_b_fill(IO_CACHE *info);
extern void my_b_seek(IO_CACHE *info,my_off_t pos);
extern uint my_b_gets(IO_CACHE *info, char *to, uint max_length);
extern my_off_t my_b_filelength(IO_CACHE *info);
extern uint my_b_printf(IO_CACHE *info, const char* fmt, ...);
extern uint my_b_vprintf(IO_CACHE *info, const char* fmt, va_list ap);
extern my_bool open_cached_file(IO_CACHE *cache,const char *dir,
				 const char *prefix, uint cache_size,
				 myf cache_myflags);
extern my_bool real_open_cached_file(IO_CACHE *cache);
extern void close_cached_file(IO_CACHE *cache);
File create_temp_file(char *to, const char *dir, const char *pfx,
		      int mode, myf MyFlags);
#define my_init_dynamic_array(A,B,C,D) init_dynamic_array(A,B,C,D CALLER_INFO)
#define my_init_dynamic_array_ci(A,B,C,D) init_dynamic_array(A,B,C,D ORIG_CALLER_INFO)
extern my_bool init_dynamic_array(DYNAMIC_ARRAY *array,uint element_size,
	  uint init_alloc,uint alloc_increment CALLER_INFO_PROTO);
extern my_bool insert_dynamic(DYNAMIC_ARRAY *array,gptr element);
extern byte *alloc_dynamic(DYNAMIC_ARRAY *array);
extern byte *pop_dynamic(DYNAMIC_ARRAY*);
extern my_bool set_dynamic(DYNAMIC_ARRAY *array,gptr element,uint array_index);
extern void get_dynamic(DYNAMIC_ARRAY *array,gptr element,uint array_index);
extern void delete_dynamic(DYNAMIC_ARRAY *array);
extern void delete_dynamic_element(DYNAMIC_ARRAY *array, uint array_index);
extern void freeze_size(DYNAMIC_ARRAY *array);
#define dynamic_array_ptr(array,array_index) ((array)->buffer+(array_index)*(array)->size_of_element)
#define dynamic_element(array,array_index,type) ((type)((array)->buffer) +(array_index))
#define push_dynamic(A,B) insert_dynamic(A,B)

extern int find_type(my_string x,TYPELIB *typelib,uint full_name);
extern void make_type(my_string to,uint nr,TYPELIB *typelib);
extern const char *get_type(TYPELIB *typelib,uint nr);
extern my_bool init_dynamic_string(DYNAMIC_STRING *str, const char *init_str,
				   uint init_alloc,uint alloc_increment);
extern my_bool dynstr_append(DYNAMIC_STRING *str, const char *append);
my_bool dynstr_append_mem(DYNAMIC_STRING *str, const char *append,
			  uint length);
extern my_bool dynstr_set(DYNAMIC_STRING *str, const char *init_str);
extern my_bool dynstr_realloc(DYNAMIC_STRING *str, ulong additional_size);
extern void dynstr_free(DYNAMIC_STRING *str);
#ifdef HAVE_MLOCK
extern byte *my_malloc_lock(uint length,myf flags);
extern void my_free_lock(byte *ptr,myf flags);
#else
#define my_malloc_lock(A,B) my_malloc((A),(B))
#define my_free_lock(A,B) my_free((A),(B))
#endif
#define alloc_root_inited(A) ((A)->min_malloc != 0)
extern void init_alloc_root(MEM_ROOT *mem_root, uint block_size,
			    uint pre_alloc_size);
extern gptr alloc_root(MEM_ROOT *mem_root,unsigned int Size);
extern void free_root(MEM_ROOT *root, myf MyFLAGS);
extern void set_prealloc_root(MEM_ROOT *root, char *ptr);
extern char *strdup_root(MEM_ROOT *root,const char *str);
extern char *strmake_root(MEM_ROOT *root,const char *str,uint len);
extern char *memdup_root(MEM_ROOT *root,const char *str,uint len);
extern int load_defaults(const char *conf_file, const char **groups,
			 int *argc, char ***argv);
extern void free_defaults(char **argv);
extern void print_defaults(const char *conf_file, const char **groups);
extern my_bool my_compress(byte *, ulong *, ulong *);
extern my_bool my_uncompress(byte *, ulong *, ulong *);
extern byte *my_compress_alloc(const byte *packet, ulong *len, ulong *complen);
extern ulong checksum(const byte *mem, uint count);
extern uint my_bit_log2(ulong value);
uint my_count_bits(ulonglong v);
extern void my_sleep(ulong m_seconds);

#ifdef __WIN__
extern my_bool have_tcpip;		/* Is set if tcpip is used */
#endif
#ifdef __NETWARE__
void netware_reg_user(const char *ip, const char *user,
		      const char *application);
#endif

C_MODE_END
#include "raid.h"
#endif /* _my_sys_h */
