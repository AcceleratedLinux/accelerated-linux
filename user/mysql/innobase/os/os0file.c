/******************************************************
The interface to the operating system file i/o primitives

(c) 1995 Innobase Oy

Created 10/21/1995 Heikki Tuuri
*******************************************************/

#include "os0file.h"
#include "os0sync.h"
#include "os0thread.h"
#include "ut0mem.h"
#include "srv0srv.h"
#include "fil0fil.h"
#include "buf0buf.h"

#undef HAVE_FDATASYNC

#ifdef POSIX_ASYNC_IO
/* We assume in this case that the OS has standard Posix aio (at least SunOS
2.6, HP-UX 11i and AIX 4.3 have) */

#endif

/* This specifies the file permissions InnoDB uses when it creates files in
Unix; the value of os_innodb_umask is initialized in ha_innodb.cc to
my_umask */

#ifndef __WIN__
ulint	os_innodb_umask		= S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
#else
ulint	os_innodb_umask		= 0;
#endif

/* If the following is set to TRUE, we do not call os_file_flush in every
os_file_write. We can set this TRUE if the doublewrite buffer is used. */
ibool	os_do_not_call_flush_at_each_write	= FALSE;

/* We use these mutexes to protect lseek + file i/o operation, if the
OS does not provide an atomic pread or pwrite, or similar */
#define OS_FILE_N_SEEK_MUTEXES	16
os_mutex_t	os_file_seek_mutexes[OS_FILE_N_SEEK_MUTEXES];

/* In simulated aio, merge at most this many consecutive i/os */
#define OS_AIO_MERGE_N_CONSECUTIVE	64

/* If this flag is TRUE, then we will use the native aio of the
OS (provided we compiled Innobase with it in), otherwise we will
use simulated aio we build below with threads */

ibool	os_aio_use_native_aio	= FALSE;

ibool	os_aio_print_debug	= FALSE;

/* The aio array slot structure */
typedef struct os_aio_slot_struct	os_aio_slot_t;

struct os_aio_slot_struct{
	ibool		is_read;	/* TRUE if a read operation */
	ulint		pos;		/* index of the slot in the aio
					array */
	ibool		reserved;	/* TRUE if this slot is reserved */
	time_t		reservation_time;/* time when reserved */
	ulint		len;		/* length of the block to read or
					write */
	byte*		buf;		/* buffer used in i/o */
	ulint		type;		/* OS_FILE_READ or OS_FILE_WRITE */
	ulint		offset;		/* 32 low bits of file offset in
					bytes */
	ulint		offset_high;	/* 32 high bits of file offset */
	os_file_t	file;		/* file where to read or write */
	char*		name;		/* file name or path */
	ibool		io_already_done;/* used only in simulated aio:
					TRUE if the physical i/o already
					made and only the slot message
					needs to be passed to the caller
					of os_aio_simulated_handle */
	void*		message1;	/* message which is given by the */
	void*		message2;	/* the requester of an aio operation
					and which can be used to identify
					which pending aio operation was
					completed */
#ifdef WIN_ASYNC_IO
        os_event_t	event;		/* event object we need in the
					OVERLAPPED struct */
	OVERLAPPED	control;	/* Windows control block for the
					aio request */
#elif defined(POSIX_ASYNC_IO)
	struct aiocb	control;	/* Posix control block for aio
					request */
#endif
};

/* The aio array structure */
typedef struct os_aio_array_struct	os_aio_array_t;

struct os_aio_array_struct{
	os_mutex_t	mutex;	  /* the mutex protecting the aio array */
	os_event_t	not_full; /* The event which is set to the signaled
				  state when there is space in the aio
				  outside the ibuf segment */
	os_event_t	is_empty; /* The event which is set to the signaled
				  state when there are no pending i/os
				  in this array */
	ulint		n_slots;  /* Total number of slots in the aio array.
				  This must be divisible by n_threads. */
	ulint		n_segments;/* Number of segments in the aio array of
				  pending aio requests. A thread can wait
				  separately for any one of the segments. */
	ulint		n_reserved;/* Number of reserved slots in the
				  aio array outside the ibuf segment */
	os_aio_slot_t* 	slots;	  /* Pointer to the slots in the array */
#ifdef __WIN__
	os_native_event_t* native_events;	 
				  /* Pointer to an array of OS native event
				  handles where we copied the handles from
				  slots, in the same order. This can be used
				  in WaitForMultipleObjects; used only in
				  Windows */
#endif
};

/* Array of events used in simulated aio */
os_event_t*	os_aio_segment_wait_events	= NULL;

/* The aio arrays for non-ibuf i/o and ibuf i/o, as well as sync aio. These
are NULL when the module has not yet been initialized. */
os_aio_array_t*	os_aio_read_array	= NULL;
os_aio_array_t*	os_aio_write_array	= NULL;
os_aio_array_t*	os_aio_ibuf_array	= NULL;
os_aio_array_t*	os_aio_log_array	= NULL;
os_aio_array_t*	os_aio_sync_array	= NULL;

ulint	os_aio_n_segments	= ULINT_UNDEFINED;

/* If the following is TRUE, read i/o handler threads try to
wait until a batch of new read requests have been posted */
ibool	os_aio_recommend_sleep_for_read_threads	= FALSE;

ulint	os_n_file_reads		= 0;
ulint	os_bytes_read_since_printout = 0;
ulint	os_n_file_writes	= 0;
ulint	os_n_fsyncs		= 0;
ulint	os_n_file_reads_old	= 0;
ulint	os_n_file_writes_old	= 0;
ulint	os_n_fsyncs_old		= 0;
time_t	os_last_printout;

ibool	os_has_said_disk_full	= FALSE;

/* The mutex protecting the following counts of pending pread and pwrite
operations */
os_mutex_t os_file_count_mutex;
ulint	os_file_n_pending_preads  = 0;
ulint	os_file_n_pending_pwrites = 0;


/***************************************************************************
Gets the operating system version. Currently works only on Windows. */

ulint
os_get_os_version(void)
/*===================*/
                  /* out: OS_WIN95, OS_WIN31, OS_WINNT, OS_WIN2000 */
{
#ifdef __WIN__
  	OSVERSIONINFO     os_info;

  	os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	ut_a(GetVersionEx(&os_info));

  	if (os_info.dwPlatformId == VER_PLATFORM_WIN32s) {
    		return(OS_WIN31);
  	} else if (os_info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
    		return(OS_WIN95);
  	} else if (os_info.dwPlatformId == VER_PLATFORM_WIN32_NT) {
		if (os_info.dwMajorVersion <= 4) {
    			return(OS_WINNT);
    		} else {
			return(OS_WIN2000);
    		}
  	} else {
    		ut_error;
    		return(0);
  	}
#else
  	ut_error;

  	return(0);
#endif
}

/***************************************************************************
Retrieves the last error number if an error occurs in a file io function.
The number should be retrieved before any other OS calls (because they may
overwrite the error number). If the number is not known to this program,
the OS error number + 100 is returned. */

ulint
os_file_get_last_error(void)
/*========================*/
		/* out: error number, or OS error number + 100 */
{
	ulint	err;

#ifdef __WIN__

	err = (ulint) GetLastError();

	if (err != ERROR_DISK_FULL && err != ERROR_FILE_EXISTS) {
		ut_print_timestamp(stderr);
	     	fprintf(stderr,
  "  InnoDB: Operating system error number %lu in a file operation.\n"
  "InnoDB: See http://www.innodb.com/ibman.html for installation help.\n",
		err);

		if (err == ERROR_PATH_NOT_FOUND) {
		         fprintf(stderr,
  "InnoDB: The error means the system cannot find the path specified.\n"
  "InnoDB: In installation you must create directories yourself, InnoDB\n"
  "InnoDB: does not create them.\n");
		} else if (err == ERROR_ACCESS_DENIED) {
		         fprintf(stderr,
  "InnoDB: The error means mysqld does not have the access rights to\n"
  "InnoDB: the directory. It may also be you have created a subdirectory\n"
  "InnoDB: of the same name as a data file.\n"); 
		} else {
			 fprintf(stderr,
  "InnoDB: See section 13.2 at http://www.innodb.com/ibman.html\n"
  "InnoDB: about operating system error numbers.\n");
		}
	}

	fflush(stderr);

	if (err == ERROR_FILE_NOT_FOUND) {
		return(OS_FILE_NOT_FOUND);
	} else if (err == ERROR_DISK_FULL) {
		return(OS_FILE_DISK_FULL);
	} else if (err == ERROR_FILE_EXISTS) {
		return(OS_FILE_ALREADY_EXISTS);
	} else {
		return(100 + err);
	}
#else
	err = (ulint) errno;

	if (err != ENOSPC && err != EEXIST) {
		ut_print_timestamp(stderr);

	     	fprintf(stderr,
  "  InnoDB: Operating system error number %lu in a file operation.\n"
  "InnoDB: See http://www.innodb.com/ibman.html for installation help.\n",
		err);

		if (err == ENOENT) {
		         fprintf(stderr,
  "InnoDB: The error means the system cannot find the path specified.\n"
  "InnoDB: In installation you must create directories yourself, InnoDB\n"
  "InnoDB: does not create them.\n");
		} else if (err == EACCES) {
		         fprintf(stderr,
  "InnoDB: The error means mysqld does not have the access rights to\n"
  "InnoDB: the directory.\n");
		} else {
			 if (strerror((int)err) != NULL) {
				fprintf(stderr,
  "InnoDB: Error number %lu means '%s'.\n", err, strerror((int)err));
			 }

			 fprintf(stderr,
  "InnoDB: See also section 13.2 at http://www.innodb.com/ibman.html\n"
  "InnoDB: about operating system error numbers.\n");
		}
	}

	fflush(stderr);

	if (err == ENOSPC ) {
		return(OS_FILE_DISK_FULL);
#ifdef POSIX_ASYNC_IO
	} else if (err == EAGAIN) {
		return(OS_FILE_AIO_RESOURCES_RESERVED);
#endif
	} else if (err == ENOENT) {
		return(OS_FILE_NOT_FOUND);
	} else if (err == EEXIST) {
		return(OS_FILE_ALREADY_EXISTS);
	} else {
		return(100 + err);
	}
#endif
}

/********************************************************************
Does error handling when a file operation fails. */
static
ibool
os_file_handle_error(
/*=================*/
				/* out: TRUE if we should retry the
				operation */
	os_file_t	file,	/* in: file pointer */
	char*		name,	/* in: name of a file or NULL */
	const char*	operation)/* in: operation */
{
	ulint	err;

	UT_NOT_USED(file);

	err = os_file_get_last_error();
	
	if (err == OS_FILE_DISK_FULL) {
		/* We only print a warning about disk full once */

		if (os_has_said_disk_full) {

			return(FALSE);
		}
	
		if (name) {
			ut_print_timestamp(stderr);
			fprintf(stderr,
	"  InnoDB: Encountered a problem with file %s\n", name);
		}

		ut_print_timestamp(stderr);
	        fprintf(stderr,
	"  InnoDB: Disk is full. Try to clean the disk to free space.\n");

		os_has_said_disk_full = TRUE;

		fflush(stderr);

		return(FALSE);

	} else if (err == OS_FILE_AIO_RESOURCES_RESERVED) {
		return(TRUE);

	} else if (err == OS_FILE_ALREADY_EXISTS) {

		return(FALSE);
	} else {
	        if (name) {
	                fprintf(stderr, "InnoDB: File name %s\n", name);
	        }
	  
		fprintf(stderr, "InnoDB: File operation call: '%s'.\n",
							       operation);
		fprintf(stderr, "InnoDB: Cannot continue operation.\n");

		fflush(stderr);

		exit(1);
	}

	return(FALSE);	
}

/********************************************************************
Creates the seek mutexes used in positioned reads and writes. */

void
os_io_init_simple(void)
/*===================*/
{
	ulint	i;

	os_file_count_mutex = os_mutex_create(NULL);

	for (i = 0; i < OS_FILE_N_SEEK_MUTEXES; i++) {
		os_file_seek_mutexes[i] = os_mutex_create(NULL);
	}
}

/********************************************************************
A simple function to open or create a file. */

os_file_t
os_file_create_simple(
/*==================*/
			/* out, own: handle to the file, not defined if error,
			error number can be retrieved with os_get_last_error */
	char*	name,	/* in: name of the file or path as a null-terminated
			string */
	ulint	create_mode,/* in: OS_FILE_OPEN if an existing file is opened
			(if does not exist, error), or OS_FILE_CREATE if a new
			file is created (if exists, error) */
	ulint	access_type,/* in: OS_FILE_READ_ONLY or OS_FILE_READ_WRITE */
	ibool*	success)/* out: TRUE if succeed, FALSE if error */
{
#ifdef __WIN__
	os_file_t	file;
	DWORD		create_flag;
	DWORD		access;
	DWORD		attributes	= 0;
	ibool		retry;
	
try_again:	
	ut_a(name);

	if (create_mode == OS_FILE_OPEN) {
		create_flag = OPEN_EXISTING;
	} else if (create_mode == OS_FILE_CREATE) {
		create_flag = CREATE_NEW;
	} else {
		create_flag = 0;
		ut_error;
	}

	if (access_type == OS_FILE_READ_ONLY) {
		access = GENERIC_READ;
	} else if (access_type == OS_FILE_READ_WRITE) {
		access = GENERIC_READ | GENERIC_WRITE;
	} else {
		access = 0;
		ut_error;
	}

	file = CreateFile(name,
			access,
			FILE_SHARE_READ,/* file can be read also by other
					processes */
			NULL,	/* default security attributes */
			create_flag,
			attributes,
			NULL);	/* no template file */

	if (file == INVALID_HANDLE_VALUE) {
		*success = FALSE;

		retry = os_file_handle_error(file, name,
				create_mode == OS_FILE_OPEN ?
				"open" : "create");
		if (retry) {
			goto try_again;
		}
	} else {
		*success = TRUE;
	}

	return(file);
#else
	os_file_t	file;
	int		create_flag;
	ibool		retry;
	
try_again:	
	ut_a(name);

	if (create_mode == OS_FILE_OPEN) {
		if (access_type == OS_FILE_READ_ONLY) {
			create_flag = O_RDONLY;
		} else {
			create_flag = O_RDWR;
		}
	} else if (create_mode == OS_FILE_CREATE) {
		create_flag = O_RDWR | O_CREAT | O_EXCL;
	} else {
		create_flag = 0;
		ut_error;
	}

	if (create_mode == OS_FILE_CREATE) {
	        file = open(name, create_flag, S_IRUSR | S_IWUSR
						| S_IRGRP | S_IWGRP);
        } else {
                file = open(name, create_flag);
        }
	
	if (file == -1) {
		*success = FALSE;

		retry = os_file_handle_error(file, name,
				create_mode == OS_FILE_OPEN ?
				"open" : "create");
		if (retry) {
			goto try_again;
		}
	} else {
		*success = TRUE;
	}

	return(file);	
#endif
}

/********************************************************************
A simple function to open or create a file. */

os_file_t
os_file_create_simple_no_error_handling(
/*====================================*/
			/* out, own: handle to the file, not defined if error,
			error number can be retrieved with os_get_last_error */
	char*	name,	/* in: name of the file or path as a null-terminated
			string */
	ulint	create_mode,/* in: OS_FILE_OPEN if an existing file is opened
			(if does not exist, error), or OS_FILE_CREATE if a new
			file is created (if exists, error) */
	ulint	access_type,/* in: OS_FILE_READ_ONLY or OS_FILE_READ_WRITE */
	ibool*	success)/* out: TRUE if succeed, FALSE if error */
{
#ifdef __WIN__
	os_file_t	file;
	DWORD		create_flag;
	DWORD		access;
	DWORD		attributes	= 0;
	
	ut_a(name);

	if (create_mode == OS_FILE_OPEN) {
		create_flag = OPEN_EXISTING;
	} else if (create_mode == OS_FILE_CREATE) {
		create_flag = CREATE_NEW;
	} else {
		create_flag = 0;
		ut_error;
	}

	if (access_type == OS_FILE_READ_ONLY) {
		access = GENERIC_READ;
	} else if (access_type == OS_FILE_READ_WRITE) {
		access = GENERIC_READ | GENERIC_WRITE;
	} else {
		access = 0;
		ut_error;
	}

	file = CreateFile(name,
			access,
			FILE_SHARE_READ,/* file can be read also by other
					processes */
			NULL,	/* default security attributes */
			create_flag,
			attributes,
			NULL);	/* no template file */

	if (file == INVALID_HANDLE_VALUE) {
		*success = FALSE;
	} else {
		*success = TRUE;
	}

	return(file);
#else
	os_file_t	file;
	int		create_flag;
	
	ut_a(name);

	if (create_mode == OS_FILE_OPEN) {
		if (access_type == OS_FILE_READ_ONLY) {
			create_flag = O_RDONLY;
		} else {
			create_flag = O_RDWR;
		}
	} else if (create_mode == OS_FILE_CREATE) {
		create_flag = O_RDWR | O_CREAT | O_EXCL;
	} else {
		create_flag = 0;
		ut_error;
	}

	if (create_mode == OS_FILE_CREATE) {
	        file = open(name, create_flag, S_IRUSR | S_IWUSR
						| S_IRGRP | S_IWGRP);
        } else {
                file = open(name, create_flag);
        }
	
	if (file == -1) {
		*success = FALSE;
	} else {
		*success = TRUE;
	}

	return(file);	
#endif
}

/********************************************************************
Opens an existing file or creates a new. */

os_file_t
os_file_create(
/*===========*/
			/* out, own: handle to the file, not defined if error,
			error number can be retrieved with os_get_last_error */
	char*	name,	/* in: name of the file or path as a null-terminated
			string */
	ulint	create_mode, /* in: OS_FILE_OPEN if an existing file is opened
			(if does not exist, error), or OS_FILE_CREATE if a new
			file is created (if exists, error), OS_FILE_OVERWRITE
			if a new is created or an old overwritten */
	ulint	purpose,/* in: OS_FILE_AIO, if asynchronous, non-buffered i/o
			is desired, OS_FILE_NORMAL, if any normal file;
			NOTE that it also depends on type, os_aio_.. and srv_..
			variables whether we really use async i/o or
			unbuffered i/o: look in the function source code for
			the exact rules */
	ulint	type,	/* in: OS_DATA_FILE or OS_LOG_FILE */
	ibool*	success)/* out: TRUE if succeed, FALSE if error */
{
#ifdef __WIN__
	os_file_t	file;
	DWORD		create_flag;
	DWORD		attributes;
	ibool		retry;
	
try_again:	
	ut_a(name);

	if (create_mode == OS_FILE_OPEN) {
		create_flag = OPEN_EXISTING;
	} else if (create_mode == OS_FILE_CREATE) {
		create_flag = CREATE_NEW;
	} else if (create_mode == OS_FILE_OVERWRITE) {
		create_flag = CREATE_ALWAYS;
	} else {
		create_flag = 0;
		ut_error;
	}

	if (purpose == OS_FILE_AIO) {
		/* If specified, use asynchronous (overlapped) io and no
		buffering of writes in the OS */
		attributes = 0;
#ifdef WIN_ASYNC_IO
		if (os_aio_use_native_aio) {
			attributes = attributes | FILE_FLAG_OVERLAPPED;
		}
#endif			
#ifdef UNIV_NON_BUFFERED_IO
		if (type == OS_LOG_FILE && srv_flush_log_at_trx_commit == 2) {
		        /* Do not use unbuffered i/o to log files because
		        value 2 denotes that we do not flush the log at every
		        commit, but only once per second */
		} else if (srv_win_file_flush_method ==
						      SRV_WIN_IO_UNBUFFERED) {
		        attributes = attributes | FILE_FLAG_NO_BUFFERING;
		}
#endif
	} else if (purpose == OS_FILE_NORMAL) {
	        attributes = 0;
#ifdef UNIV_NON_BUFFERED_IO
		if (type == OS_LOG_FILE && srv_flush_log_at_trx_commit == 2) {
		        /* Do not use unbuffered i/o to log files because
		        value 2 denotes that we do not flush the log at every
		        commit, but only once per second */
		} else if (srv_win_file_flush_method ==
						      SRV_WIN_IO_UNBUFFERED) {
		        attributes = attributes | FILE_FLAG_NO_BUFFERING;
		}
#endif
	} else {
		attributes = 0;
		ut_error;
	}

	file = CreateFile(name,
			GENERIC_READ | GENERIC_WRITE, /* read and write
							access */
			FILE_SHARE_READ,/* File can be read also by other
					processes; we must give the read
					permission because of ibbackup. We do
					not give the write permission to
					others because if one would succeed to
					start 2 instances of mysqld on the
					SAME files, that could cause severe
					database corruption! */
			NULL,	/* default security attributes */
			create_flag,
			attributes,
			NULL);	/* no template file */

	if (file == INVALID_HANDLE_VALUE) {
		*success = FALSE;

		retry = os_file_handle_error(file, name,
				create_mode == OS_FILE_OPEN ?
				"open" : "create");
		if (retry) {
			goto try_again;
		}
	} else {
		*success = TRUE;
	}

	return(file);
#else
	os_file_t	file;
	int		create_flag;
	ibool		retry;
	const char*	mode_str	= NULL;
	const char*	type_str	= NULL;
	const char*	purpose_str	= NULL;
	
try_again:	
	ut_a(name);

	if (create_mode == OS_FILE_OPEN) {
		mode_str = "OPEN";

		create_flag = O_RDWR;
	} else if (create_mode == OS_FILE_CREATE) {
		mode_str = "CREATE";

		create_flag = O_RDWR | O_CREAT | O_EXCL;
	} else if (create_mode == OS_FILE_OVERWRITE) {
		mode_str = "OVERWRITE";

		create_flag = O_RDWR | O_CREAT | O_TRUNC;
	} else {
		create_flag = 0;
		ut_error;
	}

	if (type == OS_LOG_FILE) {
		type_str = "LOG";
	} else if (type == OS_DATA_FILE) {
		type_str = "DATA";
	} else {
	        ut_a(0);
	}
	  
	if (purpose == OS_FILE_AIO) {
		purpose_str = "AIO";
	} else if (purpose == OS_FILE_NORMAL) {
		purpose_str = "NORMAL";
	} else {
	        ut_a(0);
	}

/*	printf("Opening file %s, mode %s, type %s, purpose %s\n",
			       name, mode_str, type_str, purpose_str); */
#ifdef O_SYNC
        /* We let O_SYNC only affect log files; note that we map O_DSYNC to
	O_SYNC because the datasync options seemed to corrupt files in 2001
	in both Linux and Solaris */
	if (type == OS_LOG_FILE
	    && srv_unix_file_flush_method == SRV_UNIX_O_DSYNC) {

/*		printf("Using O_SYNC for file %s\n", name); */

	        create_flag = create_flag | O_SYNC;
	}
#endif
#ifdef O_DIRECT
        /* We let O_DIRECT only affect data files */
	if (type != OS_LOG_FILE
	    && srv_unix_file_flush_method == SRV_UNIX_O_DIRECT) {

/*		printf("Using O_DIRECT for file %s\n", name); */

	        create_flag = create_flag | O_DIRECT;
	}
#endif
	if (create_mode == OS_FILE_CREATE) {
	        file = open(name, create_flag, os_innodb_umask);
        } else {
                file = open(name, create_flag);
        }
	
	if (file == -1) {
		*success = FALSE;

		retry = os_file_handle_error(file, name,
				create_mode == OS_FILE_OPEN ?
				"open" : "create");
		if (retry) {
			goto try_again;
		}
	} else {
		*success = TRUE;
	}

	return(file);	
#endif
}

/***************************************************************************
Closes a file handle. In case of error, error number can be retrieved with
os_file_get_last_error. */

ibool
os_file_close(
/*==========*/
				/* out: TRUE if success */
	os_file_t	file)	/* in, own: handle to a file */
{
#ifdef __WIN__
	BOOL	ret;

	ut_a(file);

	ret = CloseHandle(file);

	if (ret) {
		return(TRUE);
	}

	os_file_handle_error(file, NULL, "close");
	return(FALSE);
#else
	int	ret;

	ret = close(file);

	if (ret == -1) {
		os_file_handle_error(file, NULL, "close");
		return(FALSE);
	}

	return(TRUE);
#endif
}

/***************************************************************************
Closes a file handle. */

ibool
os_file_close_no_error_handling(
/*============================*/
				/* out: TRUE if success */
	os_file_t	file)	/* in, own: handle to a file */
{
#ifdef __WIN__
	BOOL	ret;

	ut_a(file);

	ret = CloseHandle(file);

	if (ret) {
		return(TRUE);
	}

	return(FALSE);
#else
	int	ret;

	ret = close(file);

	if (ret == -1) {

		return(FALSE);
	}

	return(TRUE);
#endif
}

/***************************************************************************
Gets a file size. */

ibool
os_file_get_size(
/*=============*/
				/* out: TRUE if success */
	os_file_t	file,	/* in: handle to a file */
	ulint*		size,	/* out: least significant 32 bits of file
				size */
	ulint*		size_high)/* out: most significant 32 bits of size */
{
#ifdef __WIN__
	DWORD	high;
	DWORD	low;

	low = GetFileSize(file, &high);

	if ((low == 0xFFFFFFFF) && (GetLastError() != NO_ERROR)) {
		return(FALSE);
	}

	*size = low;
	*size_high = high;

	return(TRUE);
#else
	off_t	offs;

	offs = lseek(file, 0, SEEK_END);

	if (offs == ((off_t)-1)) {

		return(FALSE);
	}
	
	if (sizeof(off_t) > 4) {
	        *size = (ulint)(offs & 0xFFFFFFFF);
		*size_high = (ulint)(offs >> 32);
	} else {
		*size = (ulint) offs;
		*size_high = 0;
	}
	
	return(TRUE);	
#endif
}

/***************************************************************************
Sets a file size. This function can be used to extend or truncate a file. */

ibool
os_file_set_size(
/*=============*/
				/* out: TRUE if success */
	char*		name,	/* in: name of the file or path as a
				null-terminated string */
	os_file_t	file,	/* in: handle to a file */
	ulint		size,	/* in: least significant 32 bits of file
				size */
	ulint		size_high)/* in: most significant 32 bits of size */
{
	ib_longlong	offset;
	ib_longlong	low;
	ulint   	n_bytes;
	ibool		ret;
	byte*   	buf;
	byte*   	buf2;
	ulint   	i;

	ut_a(size == (size & 0xFFFFFFFF));

	/* We use a very big 8 MB buffer in writing because Linux may be
	extremely slow in fsync on 1 MB writes */

	buf2 = ut_malloc(UNIV_PAGE_SIZE * 513);

	/* Align the buffer for possible raw i/o */
	buf = ut_align(buf2, UNIV_PAGE_SIZE);

	/* Write buffer full of zeros */
	for (i = 0; i < UNIV_PAGE_SIZE * 512; i++) {
	        buf[i] = '\0';
	}

	offset = 0;
	low = (ib_longlong)size + (((ib_longlong)size_high) << 32);

	if (low >= (ib_longlong)(100 * 1024 * 1024)) {
				
		fprintf(stderr, "InnoDB: Progress in MB:");
	}

	while (offset < low) {
	        if (low - offset < UNIV_PAGE_SIZE * 512) {
	        	n_bytes = (ulint)(low - offset);
	        } else {
	        	n_bytes = UNIV_PAGE_SIZE * 512;
	        }
	  
	        ret = os_file_write(name, file, buf,
				(ulint)(offset & 0xFFFFFFFF),
				    (ulint)(offset >> 32),
				n_bytes);
	        if (!ret) {
			ut_free(buf2);
	         	goto error_handling;
	        }
				
		/* Print about progress for each 100 MB written */
		if ((offset + n_bytes) / (ib_longlong)(100 * 1024 * 1024)
		    != offset / (ib_longlong)(100 * 1024 * 1024)) {

		        fprintf(stderr, " %lu00",
				(ulint)((offset + n_bytes)
					/ (ib_longlong)(100 * 1024 * 1024)));
		}
		
	        offset += n_bytes;
	}

	if (low >= (ib_longlong)(100 * 1024 * 1024)) {
				
		fprintf(stderr, "\n");
	}

	ut_free(buf2);

	ret = os_file_flush(file);

	if (ret) {
	        return(TRUE);
	}

error_handling:
	return(FALSE);
}

/***************************************************************************
Flushes the write buffers of a given file to the disk. */

ibool
os_file_flush(
/*==========*/
				/* out: TRUE if success */
	os_file_t	file)	/* in, own: handle to a file */
{
#ifdef __WIN__
	BOOL	ret;

	ut_a(file);

	os_n_fsyncs++;

	ret = FlushFileBuffers(file);

	if (ret) {
		return(TRUE);
	}

	os_file_handle_error(file, NULL, "flush");

	/* It is a fatal error if a file flush does not succeed, because then
	the database can get corrupt on disk */
	ut_a(0);

	return(FALSE);
#else
	int	ret;

#ifdef HAVE_FDATASYNC
	ret = fdatasync(file);
#else
/*	printf("Flushing to file %lu\n", (ulint)file); */
	ret = fsync(file);
#endif
	os_n_fsyncs++;

	if (ret == 0) {
		return(TRUE);
	}
	
	/* Since Linux returns EINVAL if the 'file' is actually a raw device,
	we choose to ignore that error */

	if (errno == EINVAL) {
	        return(TRUE);
	}

	ut_print_timestamp(stderr);
	
	fprintf(stderr,
		"  InnoDB: Error: the OS said file flush did not succeed\n");

	os_file_handle_error(file, NULL, "flush");

	/* It is a fatal error if a file flush does not succeed, because then
	the database can get corrupt on disk */
	ut_a(0);

	return(FALSE);
#endif
}

#ifndef __WIN__
/***********************************************************************
Does a synchronous read operation in Posix. */
static
ssize_t
os_file_pread(
/*==========*/
				/* out: number of bytes read, -1 if error */
	os_file_t	file,	/* in: handle to a file */
	void*		buf,	/* in: buffer where to read */
	ulint		n,	/* in: number of bytes to read */	
	ulint		offset,	/* in: least significant 32 bits of file
				offset from where to read */
	ulint		offset_high) /* in: most significant 32 bits of
				offset */
{
        off_t	offs;
	ssize_t	n_bytes;

	ut_a((offset & 0xFFFFFFFF) == offset);
        
        /* If off_t is > 4 bytes in size, then we assume we can pass a
	64-bit address */

        if (sizeof(off_t) > 4) {
	        offs = (off_t)offset + (((off_t)offset_high) << 32);
        				
        } else {
        	offs = (off_t)offset;

        	if (offset_high > 0) {
        		fprintf(stderr,
			"InnoDB: Error: file read at offset > 4 GB\n");
		}
        }

	os_n_file_reads++;

#ifdef HAVE_PREAD
        os_mutex_enter(os_file_count_mutex);
	os_file_n_pending_preads++;
        os_mutex_exit(os_file_count_mutex);

        n_bytes = pread(file, buf, n, offs);

        os_mutex_enter(os_file_count_mutex);
	os_file_n_pending_preads--;
        os_mutex_exit(os_file_count_mutex);

	return(n_bytes);
#else
	{
	ssize_t	ret;
	ulint	i;

	/* Protect the seek / read operation with a mutex */
	i = ((ulint) file) % OS_FILE_N_SEEK_MUTEXES;
	
	os_mutex_enter(os_file_seek_mutexes[i]);

	ret = lseek(file, offs, 0);

	if (ret < 0) {
		os_mutex_exit(os_file_seek_mutexes[i]);

		return(ret);
	}
	
	ret = read(file, buf, n);

	os_mutex_exit(os_file_seek_mutexes[i]);

	return(ret);
	}
#endif
}

/***********************************************************************
Does a synchronous write operation in Posix. */
static
ssize_t
os_file_pwrite(
/*===========*/
				/* out: number of bytes written, -1 if error */
	os_file_t	file,	/* in: handle to a file */
	void*		buf,	/* in: buffer from where to write */
	ulint		n,	/* in: number of bytes to write */	
	ulint		offset,	/* in: least significant 32 bits of file
				offset where to write */
	ulint		offset_high) /* in: most significant 32 bits of
				offset */
{
	ssize_t	ret;
        off_t	offs;

	ut_a((offset & 0xFFFFFFFF) == offset);

        /* If off_t is > 4 bytes in size, then we assume we can pass a
	64-bit address */

        if (sizeof(off_t) > 4) {
	  	offs = (off_t)offset + (((off_t)offset_high) << 32);
        } else {
        	offs = (off_t)offset;

        	if (offset_high > 0) {
        		fprintf(stderr,
			"InnoDB: Error: file write at offset > 4 GB\n");
		}
        }

	os_n_file_writes++;

#ifdef HAVE_PWRITE
        os_mutex_enter(os_file_count_mutex);
	os_file_n_pending_pwrites++;
        os_mutex_exit(os_file_count_mutex);

	ret = pwrite(file, buf, n, offs);

        os_mutex_enter(os_file_count_mutex);
	os_file_n_pending_pwrites--;
        os_mutex_exit(os_file_count_mutex);

	if (srv_unix_file_flush_method != SRV_UNIX_LITTLESYNC
	    && srv_unix_file_flush_method != SRV_UNIX_NOSYNC
	    && !os_do_not_call_flush_at_each_write) {
	    	
	        /* Always do fsync to reduce the probability that when
                the OS crashes, a database page is only partially
                physically written to disk. */

	        ut_a(TRUE == os_file_flush(file));
	}

        return(ret);
#else
	{
	ulint	i;

	/* Protect the seek / write operation with a mutex */
	i = ((ulint) file) % OS_FILE_N_SEEK_MUTEXES;
	
	os_mutex_enter(os_file_seek_mutexes[i]);

	ret = lseek(file, offs, 0);

	if (ret < 0) {
		os_mutex_exit(os_file_seek_mutexes[i]);

		return(ret);
	}
	
	ret = write(file, buf, n);

	if (srv_unix_file_flush_method != SRV_UNIX_LITTLESYNC
	    && srv_unix_file_flush_method != SRV_UNIX_NOSYNC
	    && !os_do_not_call_flush_at_each_write) {

	        /* Always do fsync to reduce the probability that when
                the OS crashes, a database page is only partially
                physically written to disk. */

	        ut_a(TRUE == os_file_flush(file));
	}

	os_mutex_exit(os_file_seek_mutexes[i]);

	return(ret);
	}
#endif
}
#endif

/***********************************************************************
Requests a synchronous positioned read operation. */

ibool
os_file_read(
/*=========*/
				/* out: TRUE if request was
				successful, FALSE if fail */
	os_file_t	file,	/* in: handle to a file */
	void*		buf,	/* in: buffer where to read */
	ulint		offset,	/* in: least significant 32 bits of file
				offset where to read */
	ulint		offset_high, /* in: most significant 32 bits of
				offset */
	ulint		n)	/* in: number of bytes to read */	
{
#ifdef __WIN__
	BOOL		ret;
	DWORD		len;
	DWORD		ret2;
	DWORD		low;
	DWORD		high;
	ibool		retry;
	ulint		i;
	
	ut_a((offset & 0xFFFFFFFF) == offset);

	os_n_file_reads++;
	os_bytes_read_since_printout += n;

try_again:	
	ut_ad(file);
	ut_ad(buf);
	ut_ad(n > 0);

	low = offset;
	high = offset_high;

	/* Protect the seek / read operation with a mutex */
	i = ((ulint) file) % OS_FILE_N_SEEK_MUTEXES;
	
	os_mutex_enter(os_file_seek_mutexes[i]);

	ret2 = SetFilePointer(file, low, &high, FILE_BEGIN);

	if (ret2 == 0xFFFFFFFF && GetLastError() != NO_ERROR) {

		os_mutex_exit(os_file_seek_mutexes[i]);

		goto error_handling;
	} 
	
	ret = ReadFile(file, buf, n, &len, NULL);

	os_mutex_exit(os_file_seek_mutexes[i]);
	
	if (ret && len == n) {
		return(TRUE);
	}		
#else
	ibool	retry;
	ssize_t	ret;

	os_bytes_read_since_printout += n;

try_again:
	ret = os_file_pread(file, buf, n, offset, offset_high);

	if ((ulint)ret == n) {

		return(TRUE);
	}
#endif	
#ifdef __WIN__
error_handling:
#endif
	retry = os_file_handle_error(file, NULL, "read"); 

	if (retry) {
		goto try_again;
	}
       
	fprintf(stderr,
"InnoDB: Fatal error: cannot read from file. OS error number %lu.\n",
#ifdef __WIN__
		(ulint)GetLastError()
#else
		(ulint)errno
#endif
		);
	fflush(stderr);

	ut_error;

	return(FALSE);
}

/***********************************************************************
Requests a synchronous write operation. */

ibool
os_file_write(
/*==========*/
				/* out: TRUE if request was
				successful, FALSE if fail */
	char*		name,	/* in: name of the file or path as a
				null-terminated string */
	os_file_t	file,	/* in: handle to a file */
	void*		buf,	/* in: buffer from which to write */
	ulint		offset,	/* in: least significant 32 bits of file
				offset where to write */
	ulint		offset_high, /* in: most significant 32 bits of
				offset */
	ulint		n)	/* in: number of bytes to write */	
{
#ifdef __WIN__
	BOOL		ret;
	DWORD		len;
	DWORD		ret2;
	DWORD		low;
	DWORD		high;
	ulint		i;
	ulint		n_retries	= 0;
	ulint		err;

	ut_a((offset & 0xFFFFFFFF) == offset);

	os_n_file_writes++;

	ut_ad(file);
	ut_ad(buf);
	ut_ad(n > 0);
retry:
	low = offset;
	high = offset_high;
	
	/* Protect the seek / write operation with a mutex */
	i = ((ulint) file) % OS_FILE_N_SEEK_MUTEXES;
	
	os_mutex_enter(os_file_seek_mutexes[i]);

	ret2 = SetFilePointer(file, low, &high, FILE_BEGIN);

	if (ret2 == 0xFFFFFFFF && GetLastError() != NO_ERROR) {

		os_mutex_exit(os_file_seek_mutexes[i]);
		
		ut_print_timestamp(stderr);

		fprintf(stderr,
"  InnoDB: Error: File pointer positioning to file %s failed at\n"
"InnoDB: offset %lu %lu. Operating system error number %lu.\n"
"InnoDB: Look from section 13.2 at http://www.innodb.com/ibman.html\n"
"InnoDB: what the error number means.\n",
			name, offset_high, offset,
			(ulint)GetLastError());

		return(FALSE);
	} 

	ret = WriteFile(file, buf, n, &len, NULL);
	
	/* Always do fsync to reduce the probability that when the OS crashes,
	a database page is only partially physically written to disk. */

	if (!os_do_not_call_flush_at_each_write) {
		ut_a(TRUE == os_file_flush(file));
	}

	os_mutex_exit(os_file_seek_mutexes[i]);

	if (ret && len == n) {

		return(TRUE);
	}

	/* If some background file system backup tool is running, then, at
	least in Windows 2000, we may get here a specific error. Let us
	retry the operation 100 times, with 1 second waits. */
	
	if (GetLastError() == ERROR_LOCK_VIOLATION && n_retries < 100) {

		os_thread_sleep(1000000);
	
		n_retries++;

		goto retry;
	}	
	
	if (!os_has_said_disk_full) {
	
		err = (ulint)GetLastError();

		ut_print_timestamp(stderr);

		fprintf(stderr,
"  InnoDB: Error: Write to file %s failed at offset %lu %lu.\n"
"InnoDB: %lu bytes should have been written, only %lu were written.\n"
"InnoDB: Operating system error number %lu.\n"
"InnoDB: Check that your OS and file system support files of this size.\n"
"InnoDB: Check also that the disk is not full or a disk quota exceeded.\n",
			name, offset_high, offset, n, (ulint)len,
			err);

		if (strerror((int)err) != NULL) {
			fprintf(stderr,
"InnoDB: Error number %lu means '%s'.\n", err, strerror((int)err));
		}

		fprintf(stderr,
"InnoDB: See also section 13.2 at http://www.innodb.com/ibman.html\n"
"InnoDB: about operating system error numbers.\n");

		os_has_said_disk_full = TRUE;
	}

	return(FALSE);
#else
	ssize_t	ret;
	
	ret = os_file_pwrite(file, buf, n, offset, offset_high);

	if ((ulint)ret == n) {

		return(TRUE);
	}

	if (!os_has_said_disk_full) {
	
		ut_print_timestamp(stderr);

		fprintf(stderr,
"  InnoDB: Error: Write to file %s failed at offset %lu %lu.\n"
"InnoDB: %lu bytes should have been written, only %ld were written.\n"
"InnoDB: Operating system error number %lu.\n"
"InnoDB: Check that your OS and file system support files of this size.\n"
"InnoDB: Check also that the disk is not full or a disk quota exceeded.\n",
			name, offset_high, offset, n, (long int)ret,
							(ulint)errno);
		if (strerror(errno) != NULL) {
			fprintf(stderr,
"InnoDB: Error number %lu means '%s'.\n", (ulint)errno, strerror(errno));
		}

		fprintf(stderr,
"InnoDB: See also section 13.2 at http://www.innodb.com/ibman.html\n"
"InnoDB: about operating system error numbers.\n");

		os_has_said_disk_full = TRUE;
	}

	return(FALSE);	
#endif
}

/********************************************************************
Returns a pointer to the nth slot in the aio array. */
static
os_aio_slot_t*
os_aio_array_get_nth_slot(
/*======================*/
					/* out: pointer to slot */
	os_aio_array_t*		array,	/* in: aio array */
	ulint			index)	/* in: index of the slot */
{
	ut_a(index < array->n_slots);

	return((array->slots) + index);
}

/****************************************************************************
Creates an aio wait array. */
static
os_aio_array_t*
os_aio_array_create(
/*================*/
				/* out, own: aio array */
	ulint	n,		/* in: maximum number of pending aio operations
				allowed; n must be divisible by n_segments */
	ulint	n_segments) 	/* in: number of segments in the aio array */
{
	os_aio_array_t*	array;
	ulint		i;
	os_aio_slot_t*	slot;
#ifdef WIN_ASYNC_IO
	OVERLAPPED*	over;
#endif	
	ut_a(n > 0);
	ut_a(n_segments > 0);

	array = ut_malloc(sizeof(os_aio_array_t));

	array->mutex 		= os_mutex_create(NULL);
	array->not_full		= os_event_create(NULL);
	array->is_empty		= os_event_create(NULL);

	os_event_set(array->is_empty);
	
	array->n_slots  	= n;
	array->n_segments	= n_segments;
	array->n_reserved	= 0;
	array->slots		= ut_malloc(n * sizeof(os_aio_slot_t));
#ifdef __WIN__
	array->native_events	= ut_malloc(n * sizeof(os_native_event_t));
#endif	
	for (i = 0; i < n; i++) {
		slot = os_aio_array_get_nth_slot(array, i);

		slot->pos = i;
		slot->reserved = FALSE;
#ifdef WIN_ASYNC_IO
		slot->event = os_event_create(NULL);

		over = &(slot->control);

		over->hEvent = slot->event->handle;

		*((array->native_events) + i) = over->hEvent;
#endif
	}
	
	return(array);
}

/****************************************************************************
Initializes the asynchronous io system. Calls also os_io_init_simple.
Creates a separate aio array for
non-ibuf read and write, a third aio array for the ibuf i/o, with just one
segment, two aio arrays for log reads and writes with one segment, and a
synchronous aio array of the specified size. The combined number of segments
in the three first aio arrays is the parameter n_segments given to the
function. The caller must create an i/o handler thread for each segment in
the four first arrays, but not for the sync aio array. */

void
os_aio_init(
/*========*/
	ulint	n,		/* in: maximum number of pending aio operations
				allowed; n must be divisible by n_segments */
	ulint	n_segments,	/* in: combined number of segments in the four
				first aio arrays; must be >= 4 */
	ulint	n_slots_sync)	/* in: number of slots in the sync aio array */
{
	ulint	n_read_segs;
	ulint	n_write_segs;
	ulint	n_per_seg;
	ulint	i;
#ifdef POSIX_ASYNC_IO
	sigset_t   sigset;
#endif
	ut_ad(n % n_segments == 0);
	ut_ad(n_segments >= 4);

	os_io_init_simple();

	for (i = 0; i < n_segments; i++) {
	        srv_io_thread_op_info[i] = (char*)"not started yet";
	}

	n_per_seg = n / n_segments;
	n_write_segs = (n_segments - 2) / 2;
	n_read_segs = n_segments - 2 - n_write_segs;
	
	/* printf("Array n per seg %lu\n", n_per_seg); */

	os_aio_ibuf_array = os_aio_array_create(n_per_seg, 1);

	srv_io_thread_function[0] = (char*)"insert buffer thread";

	os_aio_log_array = os_aio_array_create(n_per_seg, 1);

	srv_io_thread_function[1] = (char*)"log thread";

	os_aio_read_array = os_aio_array_create(n_read_segs * n_per_seg,
							n_read_segs);
	for (i = 2; i < 2 + n_read_segs; i++) {
	        srv_io_thread_function[i] = (char*)"read thread";
	}

	os_aio_write_array = os_aio_array_create(n_write_segs * n_per_seg,
							n_write_segs);
	for (i = 2 + n_read_segs; i < n_segments; i++) {
	        srv_io_thread_function[i] = (char*)"write thread";
	}

	os_aio_sync_array = os_aio_array_create(n_slots_sync, 1);

	os_aio_n_segments = n_segments;

	os_aio_validate();

	os_aio_segment_wait_events = ut_malloc(n_segments * sizeof(void*));

	for (i = 0; i < n_segments; i++) {
		os_aio_segment_wait_events[i] = os_event_create(NULL);
	}

	os_last_printout = time(NULL);

#ifdef POSIX_ASYNC_IO
	/* Block aio signals from the current thread and its children:
	for this to work, the current thread must be the first created
	in the database, so that all its children will inherit its
	signal mask */
	
	/* TODO: to work MySQL needs the SIGALARM signal; the following
	will not work yet! */
        sigemptyset(&sigset);
	sigaddset(&sigset, SIGRTMIN + 1 + 0);
	sigaddset(&sigset, SIGRTMIN + 1 + 1);
	sigaddset(&sigset, SIGRTMIN + 1 + 2);
	sigaddset(&sigset, SIGRTMIN + 1 + 3);

	pthread_sigmask(SIG_BLOCK, &sigset, NULL); */
#endif
}

#ifdef WIN_ASYNC_IO
/****************************************************************************
Wakes up all async i/o threads in the array in Windows async i/o at
shutdown. */
static
void
os_aio_array_wake_win_aio_at_shutdown(
/*==================================*/
	os_aio_array_t*	array)	/* in: aio array */
{
	ulint	i;

	for (i = 0; i < array->n_slots; i++) {

	        os_event_set((array->slots + i)->event);
	}
}
#endif

/****************************************************************************
Wakes up all async i/o threads so that they know to exit themselves in
shutdown. */

void
os_aio_wake_all_threads_at_shutdown(void)
/*=====================================*/
{
	ulint	i;

#ifdef WIN_ASYNC_IO
        /* This code wakes up all ai/o threads in Windows native aio */
	os_aio_array_wake_win_aio_at_shutdown(os_aio_read_array);
	os_aio_array_wake_win_aio_at_shutdown(os_aio_write_array);
	os_aio_array_wake_win_aio_at_shutdown(os_aio_ibuf_array);
	os_aio_array_wake_win_aio_at_shutdown(os_aio_log_array);
#endif
	/* This loop wakes up all simulated ai/o threads */

	for (i = 0; i < os_aio_n_segments; i++) {
	    	
		os_event_set(os_aio_segment_wait_events[i]);
	}	
}
				
/****************************************************************************
Waits until there are no pending writes in os_aio_write_array. There can
be other, synchronous, pending writes. */

void
os_aio_wait_until_no_pending_writes(void)
/*=====================================*/
{
	os_event_wait(os_aio_write_array->is_empty);
}

/**************************************************************************
Calculates segment number for a slot. */
static
ulint
os_aio_get_segment_no_from_slot(
/*============================*/
				/* out: segment number (which is the number
				used by, for example, i/o-handler threads) */
	os_aio_array_t*	array,	/* in: aio wait array */
	os_aio_slot_t*	slot)	/* in: slot in this array */
{
	ulint	segment;
	ulint	seg_len;

	if (array == os_aio_ibuf_array) {
		segment = 0;

	} else if (array == os_aio_log_array) {
		segment = 1;
		
	} else if (array == os_aio_read_array) {
		seg_len = os_aio_read_array->n_slots /
				os_aio_read_array->n_segments;

		segment = 2 + slot->pos / seg_len;
	} else {
		ut_a(array == os_aio_write_array);
		seg_len = os_aio_write_array->n_slots /
				os_aio_write_array->n_segments;

		segment = os_aio_read_array->n_segments + 2
				+ slot->pos / seg_len;
	}

	return(segment);
}

/**************************************************************************
Calculates local segment number and aio array from global segment number. */
static
ulint
os_aio_get_array_and_local_segment(
/*===============================*/
					/* out: local segment number within
					the aio array */
	os_aio_array_t** array,		/* out: aio wait array */
	ulint		 global_segment)/* in: global segment number */
{
	ulint	segment;

	ut_a(global_segment < os_aio_n_segments);	

	if (global_segment == 0) {
		*array = os_aio_ibuf_array;
		segment = 0;

	} else if (global_segment == 1) {
		*array = os_aio_log_array;
		segment = 0;
		
	} else if (global_segment < os_aio_read_array->n_segments + 2) {
		*array = os_aio_read_array;

		segment = global_segment - 2;
	} else {
		*array = os_aio_write_array;

		segment = global_segment - (os_aio_read_array->n_segments + 2);
	}

	return(segment);
}

/***********************************************************************
Gets an integer value designating a specified aio array. This is used
to give numbers to signals in Posix aio. */

#if !defined(WIN_ASYNC_IO) && defined(POSIX_ASYNC_IO)
static
ulint
os_aio_get_array_no(
/*================*/
	os_aio_array_t*	array)	/* in: aio array */
{	
	if (array == os_aio_ibuf_array) {
	
		return(0);

	} else if (array == os_aio_log_array) {

		return(1);

	} else if (array == os_aio_read_array) {

		return(2);
	} else if (array == os_aio_write_array) {

		return(3);
	} else {
		ut_a(0);

		return(0);
	}
}

/***********************************************************************
Gets the aio array for its number. */
static
os_aio_array_t*
os_aio_get_array_from_no(
/*=====================*/
			/* out: aio array */
	ulint	n)	/* in: array number */
{	
	if (n == 0) {
		return(os_aio_ibuf_array);
	} else if (n == 1) {

		return(os_aio_log_array);
	} else if (n == 2) {

		return(os_aio_read_array);
	} else if (n == 3) {

		return(os_aio_write_array);
	} else {
		ut_a(0);

		return(NULL);
	}
}
#endif /* if !defined(WIN_ASYNC_IO) && defined(POSIX_ASYNC_IO) */

/***********************************************************************
Requests for a slot in the aio array. If no slot is available, waits until
not_full-event becomes signaled. */
static
os_aio_slot_t*
os_aio_array_reserve_slot(
/*======================*/
				/* out: pointer to slot */
	ulint		type,	/* in: OS_FILE_READ or OS_FILE_WRITE */
	os_aio_array_t*	array,	/* in: aio array */
	void*		message1,/* in: message to be passed along with
				the aio operation */
	void*		message2,/* in: message to be passed along with
				the aio operation */
	os_file_t	file,	/* in: file handle */
	char*		name,	/* in: name of the file or path as a
				null-terminated string */
	void*		buf,	/* in: buffer where to read or from which
				to write */
	ulint		offset,	/* in: least significant 32 bits of file
				offset */
	ulint		offset_high, /* in: most significant 32 bits of
				offset */
	ulint		len)	/* in: length of the block to read or write */
{
	os_aio_slot_t*	slot;
#ifdef WIN_ASYNC_IO
	OVERLAPPED*	control;

#elif defined(POSIX_ASYNC_IO)

	struct aiocb*	control;
#endif
	ulint		i;
loop:
	os_mutex_enter(array->mutex);

	if (array->n_reserved == array->n_slots) {
		os_mutex_exit(array->mutex);

		if (!os_aio_use_native_aio) {
			/* If the handler threads are suspended, wake them
			so that we get more slots */

			os_aio_simulated_wake_handler_threads();
		}
		
		os_event_wait(array->not_full);

		goto loop;
	}

	for (i = 0;; i++) {
		slot = os_aio_array_get_nth_slot(array, i);

		if (slot->reserved == FALSE) {
			break;
		}
	}

	array->n_reserved++;

	if (array->n_reserved == 1) {
		os_event_reset(array->is_empty);
	}

	if (array->n_reserved == array->n_slots) {
		os_event_reset(array->not_full);
	}
	
	slot->reserved = TRUE;
	slot->reservation_time = time(NULL);
	slot->message1 = message1;
	slot->message2 = message2;
	slot->file     = file;
	slot->name     = name;
	slot->len      = len;
	slot->type     = type;
	slot->buf      = buf;
	slot->offset   = offset;
	slot->offset_high = offset_high;
	slot->io_already_done = FALSE;
	
#ifdef WIN_ASYNC_IO		
	control = &(slot->control);
	control->Offset = (DWORD)offset;
	control->OffsetHigh = (DWORD)offset_high;
	os_event_reset(slot->event);

#elif defined(POSIX_ASYNC_IO)

#if (UNIV_WORD_SIZE == 8)
	offset = offset + (offset_high << 32);
#else
	ut_a(offset_high == 0);
#endif 
	control = &(slot->control);
	control->aio_fildes = file;
	control->aio_buf = buf;
	control->aio_nbytes = len;
	control->aio_offset = offset;
	control->aio_reqprio = 0;
	control->aio_sigevent.sigev_notify = SIGEV_SIGNAL;
	control->aio_sigevent.sigev_signo =
			SIGRTMIN + 1 + os_aio_get_array_no(array);
			/* TODO: How to choose the signal numbers? */
/*
	printf("AIO signal number %lu\n", (ulint) control->aio_sigevent.sigev_signo);
*/
	control->aio_sigevent.sigev_value.sival_ptr = slot;
#endif
	os_mutex_exit(array->mutex);

	return(slot);
}

/***********************************************************************
Frees a slot in the aio array. */
static
void
os_aio_array_free_slot(
/*===================*/
	os_aio_array_t*	array,	/* in: aio array */
	os_aio_slot_t*	slot)	/* in: pointer to slot */
{
	ut_ad(array);
	ut_ad(slot);

	os_mutex_enter(array->mutex);

	ut_ad(slot->reserved);
	
	slot->reserved = FALSE;

	array->n_reserved--;

	if (array->n_reserved == array->n_slots - 1) {
		os_event_set(array->not_full);
	}

	if (array->n_reserved == 0) {
		os_event_set(array->is_empty);
	}

#ifdef WIN_ASYNC_IO		
	os_event_reset(slot->event);
#endif
	os_mutex_exit(array->mutex);
}

/**************************************************************************
Wakes up a simulated aio i/o-handler thread if it has something to do. */
static
void
os_aio_simulated_wake_handler_thread(
/*=================================*/
	ulint	global_segment)	/* in: the number of the segment in the aio
				arrays */
{
	os_aio_array_t*	array;
	os_aio_slot_t*	slot;
	ulint		segment;
	ulint		n;
	ulint		i;

	ut_ad(!os_aio_use_native_aio);

	segment = os_aio_get_array_and_local_segment(&array, global_segment);

	n = array->n_slots / array->n_segments;

	/* Look through n slots after the segment * n'th slot */

	os_mutex_enter(array->mutex);

	for (i = 0; i < n; i++) {
		slot = os_aio_array_get_nth_slot(array, i + segment * n);

		if (slot->reserved) {
			/* Found an i/o request */
			
			break;
		}
	}

	os_mutex_exit(array->mutex);

	if (i < n) {
		os_event_set(os_aio_segment_wait_events[global_segment]);
	}
}

/**************************************************************************
Wakes up simulated aio i/o-handler threads if they have something to do. */

void
os_aio_simulated_wake_handler_threads(void)
/*=======================================*/
{
	ulint	i;

	if (os_aio_use_native_aio) {
		/* We do not use simulated aio: do nothing */

		return;
	}	

	os_aio_recommend_sleep_for_read_threads	= FALSE;

	for (i = 0; i < os_aio_n_segments; i++) {
		os_aio_simulated_wake_handler_thread(i);
	}
}

/**************************************************************************
This function can be called if one wants to post a batch of reads and
prefers an i/o-handler thread to handle them all at once later. You must
call os_aio_simulated_wake_handler_threads later to ensure the threads
are not left sleeping! */

void
os_aio_simulated_put_read_threads_to_sleep(void)
/*============================================*/
{
	os_aio_array_t*	array;
	ulint		g;

	os_aio_recommend_sleep_for_read_threads	= TRUE;

	for (g = 0; g < os_aio_n_segments; g++) {
		os_aio_get_array_and_local_segment(&array, g);

		if (array == os_aio_read_array) {
		
			os_event_reset(os_aio_segment_wait_events[g]);
		}
	}
}

/***********************************************************************
Requests an asynchronous i/o operation. */

ibool
os_aio(
/*===*/
				/* out: TRUE if request was queued
				successfully, FALSE if fail */
	ulint		type,	/* in: OS_FILE_READ or OS_FILE_WRITE */
	ulint		mode,	/* in: OS_AIO_NORMAL, ..., possibly ORed
				to OS_AIO_SIMULATED_WAKE_LATER: the
				last flag advises this function not to wake
				i/o-handler threads, but the caller will
				do the waking explicitly later, in this
				way the caller can post several requests in
				a batch; NOTE that the batch must not be
				so big that it exhausts the slots in aio
				arrays! NOTE that a simulated batch
				may introduce hidden chances of deadlocks,
				because i/os are not actually handled until
				all have been posted: use with great
				caution! */
	char*		name,	/* in: name of the file or path as a
				null-terminated string */
	os_file_t	file,	/* in: handle to a file */
	void*		buf,	/* in: buffer where to read or from which
				to write */
	ulint		offset,	/* in: least significant 32 bits of file
				offset where to read or write */
	ulint		offset_high, /* in: most significant 32 bits of
				offset */
	ulint		n,	/* in: number of bytes to read or write */
	void*		message1,/* in: messages for the aio handler (these
				can be used to identify a completed aio
				operation); if mode is OS_AIO_SYNC, these
				are ignored */
	void*		message2)
{
	os_aio_array_t*	array;
	os_aio_slot_t*	slot;
#ifdef WIN_ASYNC_IO
	ibool		retval;
	BOOL		ret		= TRUE;
	DWORD		len		= n;
	void*		dummy_mess1;
	void*		dummy_mess2;
	ulint		dummy_type;
#endif
	ulint		err		= 0;
	ibool		retry;
	ulint		wake_later;

	ut_ad(file);
	ut_ad(buf);
	ut_ad(n > 0);
	ut_ad(n % OS_FILE_LOG_BLOCK_SIZE == 0);
	ut_ad(offset % OS_FILE_LOG_BLOCK_SIZE == 0);
	ut_ad(os_aio_validate());

	wake_later = mode & OS_AIO_SIMULATED_WAKE_LATER;
	mode = mode & (~OS_AIO_SIMULATED_WAKE_LATER);
	
	if (mode == OS_AIO_SYNC
#ifdef WIN_ASYNC_IO
				&& !os_aio_use_native_aio
#endif
	) {
		/* This is actually an ordinary synchronous read or write:
		no need to use an i/o-handler thread. NOTE that if we use
		Windows async i/o, Windows does not allow us to use
		ordinary synchronous os_file_read etc. on the same file,
		therefore we have built a special mechanism for synchronous
		wait in the Windows case. */

		if (type == OS_FILE_READ) {
			return(os_file_read(file, buf, offset,
							offset_high, n));
		}

		ut_a(type == OS_FILE_WRITE);

		return(os_file_write(name, file, buf, offset, offset_high, n));
	}

try_again:
	if (mode == OS_AIO_NORMAL) {
		if (type == OS_FILE_READ) {
			array = os_aio_read_array;
		} else {
			array = os_aio_write_array;
		}
	} else if (mode == OS_AIO_IBUF) {
		ut_ad(type == OS_FILE_READ);
		/* Reduce probability of deadlock bugs in connection with ibuf:
		do not let the ibuf i/o handler sleep */

		wake_later = FALSE;

		array = os_aio_ibuf_array;
	} else if (mode == OS_AIO_LOG) {

		array = os_aio_log_array;
	} else if (mode == OS_AIO_SYNC) {
		array = os_aio_sync_array;
	} else {
		array = NULL; /* Eliminate compiler warning */
		ut_error;
	}
	
	slot = os_aio_array_reserve_slot(type, array, message1, message2, file,
					name, buf, offset, offset_high, n);
	if (type == OS_FILE_READ) {
		if (os_aio_use_native_aio) {
#ifdef WIN_ASYNC_IO
			os_n_file_reads++;
			os_bytes_read_since_printout += len;
			
			ret = ReadFile(file, buf, (DWORD)n, &len,
							&(slot->control));
#elif defined(POSIX_ASYNC_IO)
			slot->control.aio_lio_opcode = LIO_READ;
			err = (ulint) aio_read(&(slot->control));
			printf("Starting Posix aio read %lu\n", err);
#endif
		} else {
			if (!wake_later) {
				os_aio_simulated_wake_handler_thread(
				 os_aio_get_segment_no_from_slot(array, slot));
			}
		}
	} else if (type == OS_FILE_WRITE) {
		if (os_aio_use_native_aio) {
#ifdef WIN_ASYNC_IO
			os_n_file_writes++;
			ret = WriteFile(file, buf, (DWORD)n, &len,
							&(slot->control));
#elif defined(POSIX_ASYNC_IO)
			slot->control.aio_lio_opcode = LIO_WRITE;
			err = (ulint) aio_write(&(slot->control));
			printf("Starting Posix aio write %lu\n", err);
#endif
		} else {
			if (!wake_later) {
				os_aio_simulated_wake_handler_thread(
				 os_aio_get_segment_no_from_slot(array, slot));
			}
		}
	} else {
		ut_error;
	}

#ifdef WIN_ASYNC_IO
	if (os_aio_use_native_aio) {
		if ((ret && len == n)
		    || (!ret && GetLastError() == ERROR_IO_PENDING)) {
			/* aio was queued successfully! */
		
	    		if (mode == OS_AIO_SYNC) {
	    		    /* We want a synchronous i/o operation on a file
	    		    where we also use async i/o: in Windows we must
	    		    use the same wait mechanism as for async i/o */
	    		
	    		    retval = os_aio_windows_handle(ULINT_UNDEFINED,
					slot->pos,
		    			&dummy_mess1, &dummy_mess2,
					&dummy_type);

			    return(retval);
	    		}

			return(TRUE);
		}

		err = 1; /* Fall through the next if */
	}
#endif
	if (err == 0) {
		/* aio was queued successfully! */

		return(TRUE);
	}

	os_aio_array_free_slot(array, slot);

	retry = os_file_handle_error(file, name,
			type == OS_FILE_READ ? "aio read" : "aio write");
	if (retry) {

		goto try_again;
	}	

	return(FALSE);
}

#ifdef WIN_ASYNC_IO
/**************************************************************************
This function is only used in Windows asynchronous i/o.
Waits for an aio operation to complete. This function is used to wait the
for completed requests. The aio array of pending requests is divided
into segments. The thread specifies which segment or slot it wants to wait
for. NOTE: this function will also take care of freeing the aio slot,
therefore no other thread is allowed to do the freeing! */

ibool
os_aio_windows_handle(
/*==================*/
				/* out: TRUE if the aio operation succeeded */
	ulint	segment,	/* in: the number of the segment in the aio
				arrays to wait for; segment 0 is the ibuf
				i/o thread, segment 1 the log i/o thread,
				then follow the non-ibuf read threads, and as
				the last are the non-ibuf write threads; if
				this is ULINT_UNDEFINED, then it means that
				sync aio is used, and this parameter is
				ignored */
	ulint	pos,		/* this parameter is used only in sync aio:
				wait for the aio slot at this position */  
	void**	message1,	/* out: the messages passed with the aio
				request; note that also in the case where
				the aio operation failed, these output
				parameters are valid and can be used to
				restart the operation, for example */
	void**	message2,
	ulint*	type)		/* out: OS_FILE_WRITE or ..._READ */
{
	ulint		orig_seg	= segment;
	os_aio_array_t*	array;
	os_aio_slot_t*	slot;
	ulint		n;
	ulint		i;
	ibool		ret_val;
	BOOL		ret;
	DWORD		len;

	if (segment == ULINT_UNDEFINED) {
		array = os_aio_sync_array;
		segment = 0;
	} else {
		segment = os_aio_get_array_and_local_segment(&array, segment);
	}
	
	/* NOTE! We only access constant fields in os_aio_array. Therefore
	we do not have to acquire the protecting mutex yet */

	ut_ad(os_aio_validate());
	ut_ad(segment < array->n_segments);

	n = array->n_slots / array->n_segments;

	if (array == os_aio_sync_array) {
		srv_io_thread_op_info[orig_seg] =
						"wait Windows aio for 1 page";
		os_event_wait(os_aio_array_get_nth_slot(array, pos)->event);
		i = pos;
	} else {
		srv_io_thread_op_info[orig_seg] =
						"wait Windows aio";
		i = os_event_wait_multiple(n,
				(array->native_events) + segment * n);
	}

	os_mutex_enter(array->mutex);

	slot = os_aio_array_get_nth_slot(array, i + segment * n);

	ut_a(slot->reserved);

	srv_io_thread_op_info[orig_seg] = "get windows aio return value";
	ret = GetOverlappedResult(slot->file, &(slot->control), &len, TRUE);

	*message1 = slot->message1;
	*message2 = slot->message2;

	*type = slot->type;

	if (ret && len == slot->len) {
		ret_val = TRUE;

		if (slot->type == OS_FILE_WRITE
				&& !os_do_not_call_flush_at_each_write) {
		         ut_a(TRUE == os_file_flush(slot->file));
		}
	} else {
		os_file_handle_error(slot->file, slot->name, "Windows aio");
		
		ret_val = FALSE;
	}		  

	os_mutex_exit(array->mutex);

	os_aio_array_free_slot(array, slot);
	
	return(ret_val);
}
#endif

#ifdef POSIX_ASYNC_IO

/**************************************************************************
This function is only used in Posix asynchronous i/o. Waits for an aio
operation to complete. */

ibool
os_aio_posix_handle(
/*================*/
				/* out: TRUE if the aio operation succeeded */
	ulint	array_no,	/* in: array number 0 - 3 */
	void**	message1,	/* out: the messages passed with the aio
				request; note that also in the case where
				the aio operation failed, these output
				parameters are valid and can be used to
				restart the operation, for example */
	void**	message2)
{
	os_aio_array_t*	array;
	os_aio_slot_t*	slot;
	siginfo_t	info;
	sigset_t	sigset;
	sigset_t        proc_sigset;
	sigset_t        thr_sigset;
	int		ret;
	int             i;
	int             sig;
	
        sigemptyset(&sigset);
	sigaddset(&sigset, SIGRTMIN + 1 + array_no);

	pthread_sigmask(SIG_UNBLOCK, &sigset, NULL);

	/*
	sigprocmask(0, NULL, &proc_sigset);
	pthread_sigmask(0, NULL, &thr_sigset);

	for (i = 32 ; i < 40; i++) {
	  printf("%lu : %lu %lu\n", (ulint)i,
		 (ulint)sigismember(&proc_sigset, i),
		 (ulint)sigismember(&thr_sigset, i));
	}
	*/

	ret = sigwaitinfo(&sigset, &info);

	if (sig != SIGRTMIN + 1 + array_no) {

		ut_a(0);
	
		return(FALSE);
	}
	
	printf("Handling Posix aio\n");

	array = os_aio_get_array_from_no(array_no);

	os_mutex_enter(array->mutex);

	slot = info.si_value.sival_ptr;

	ut_a(slot->reserved);

	*message1 = slot->message1;
	*message2 = slot->message2;

	if (slot->type == OS_FILE_WRITE
				&& !os_do_not_call_flush_at_each_write) {
		ut_a(TRUE == os_file_flush(slot->file));
	}

	os_mutex_exit(array->mutex);

	os_aio_array_free_slot(array, slot);
	
	return(TRUE);
}
#endif

/**************************************************************************
Does simulated aio. This function should be called by an i/o-handler
thread. */

ibool
os_aio_simulated_handle(
/*====================*/
				/* out: TRUE if the aio operation succeeded */
	ulint	global_segment,	/* in: the number of the segment in the aio
				arrays to wait for; segment 0 is the ibuf
				i/o thread, segment 1 the log i/o thread,
				then follow the non-ibuf read threads, and as
				the last are the non-ibuf write threads */
	void**	message1,	/* out: the messages passed with the aio
				request; note that also in the case where
				the aio operation failed, these output
				parameters are valid and can be used to
				restart the operation, for example */
	void**	message2,
	ulint*	type)		/* out: OS_FILE_WRITE or ..._READ */
{
	os_aio_array_t*	array;
	ulint		segment;
	os_aio_slot_t*	slot;
	os_aio_slot_t*	slot2;
	os_aio_slot_t*	consecutive_ios[OS_AIO_MERGE_N_CONSECUTIVE];
	ulint		n_consecutive;
	ulint		total_len;
	ulint		offs;
	ulint		lowest_offset;
	ulint		biggest_age;
	ulint		age;
	byte*		combined_buf;
	byte*		combined_buf2= 0;	/* Remove warning */
	ibool		ret;
	ulint		n;
	ulint		i;
	ulint		len2;
	
	segment = os_aio_get_array_and_local_segment(&array, global_segment);
	
restart:
	/* NOTE! We only access constant fields in os_aio_array. Therefore
	we do not have to acquire the protecting mutex yet */

	ut_ad(os_aio_validate());
	ut_ad(segment < array->n_segments);

	n = array->n_slots / array->n_segments;

	/* Look through n slots after the segment * n'th slot */

	if (array == os_aio_read_array
	    && os_aio_recommend_sleep_for_read_threads) {

		/* Give other threads chance to add several i/os to the array
		at once. */

		goto recommended_sleep;
	}
	
	os_mutex_enter(array->mutex);

	/* Check if there is a slot for which the i/o has already been
	done */
	
	for (i = 0; i < n; i++) {
		slot = os_aio_array_get_nth_slot(array, i + segment * n);

		if (slot->reserved && slot->io_already_done) {

			if (os_aio_print_debug) {
				fprintf(stderr,
"InnoDB: i/o for slot %lu already done, returning\n", i);
			}

			ret = TRUE;
			
			goto slot_io_done;
		}
	}

	n_consecutive = 0;

	/* If there are at least 2 seconds old requests, then pick the oldest
	one to prevent starvation. If several requests have the same age,
	then pick the one at the lowest offset. */

	biggest_age = 0;
	lowest_offset = ULINT_MAX;

	for (i = 0; i < n; i++) {
		slot = os_aio_array_get_nth_slot(array, i + segment * n);

		if (slot->reserved) {
		        age = (ulint)difftime(time(NULL),
						slot->reservation_time);

			if ((age >= 2 && age > biggest_age)
			    || (age >= 2 && age == biggest_age
			        && slot->offset < lowest_offset)) {

			        /* Found an i/o request */
				consecutive_ios[0] = slot;

				n_consecutive = 1;

				biggest_age = age;
				lowest_offset = slot->offset;
			}
		}
	}

	if (n_consecutive == 0) {
	        /* There were no old requests. Look for an i/o request at the
		lowest offset in the array (we ignore the high 32 bits of the
		offset in these heuristics) */

		lowest_offset = ULINT_MAX;
	
		for (i = 0; i < n; i++) {
		        slot = os_aio_array_get_nth_slot(array,
							i + segment * n);

			if (slot->reserved && slot->offset < lowest_offset) {

			        /* Found an i/o request */
				consecutive_ios[0] = slot;

				n_consecutive = 1;

				lowest_offset = slot->offset;
			}
		}
	}

	if (n_consecutive == 0) {

		/* No i/o requested at the moment */

		goto wait_for_io;
	}

	slot = consecutive_ios[0];

	/* Check if there are several consecutive blocks to read or write */

consecutive_loop:	
	for (i = 0; i < n; i++) {
		slot2 = os_aio_array_get_nth_slot(array, i + segment * n);

		if (slot2->reserved && slot2 != slot
		    && slot2->offset == slot->offset + slot->len
		    && slot->offset + slot->len > slot->offset /* check that
						sum does not wrap over */
		    && slot2->offset_high == slot->offset_high
		    && slot2->type == slot->type
		    && slot2->file == slot->file) {

			/* Found a consecutive i/o request */

			consecutive_ios[n_consecutive] = slot2;
			n_consecutive++;

			slot = slot2;

			if (n_consecutive < OS_AIO_MERGE_N_CONSECUTIVE) {
			
				goto consecutive_loop;
			} else {
				break;
			}
		}
	}

	/* We have now collected n_consecutive i/o requests in the array;
	allocate a single buffer which can hold all data, and perform the
	i/o */

	total_len = 0;
	slot = consecutive_ios[0];
	
	for (i = 0; i < n_consecutive; i++) {
		total_len += consecutive_ios[i]->len;
	}

	if (n_consecutive == 1) {
		/* We can use the buffer of the i/o request */
		combined_buf = slot->buf;
	} else {
		combined_buf2 = ut_malloc(total_len + UNIV_PAGE_SIZE);

		ut_a(combined_buf2);

		combined_buf = ut_align(combined_buf2, UNIV_PAGE_SIZE);
	}
	
	/* We release the array mutex for the time of the i/o: NOTE that
	this assumes that there is just one i/o-handler thread serving
	a single segment of slots! */

	os_mutex_exit(array->mutex);

	if (slot->type == OS_FILE_WRITE && n_consecutive > 1) {
		/* Copy the buffers to the combined buffer */
		offs = 0;
		
		for (i = 0; i < n_consecutive; i++) {

			ut_memcpy(combined_buf + offs, consecutive_ios[i]->buf,
						consecutive_ios[i]->len);
			offs += consecutive_ios[i]->len;
		}
	}

	srv_io_thread_op_info[global_segment] = (char*) "doing file i/o";

	if (os_aio_print_debug) {
		fprintf(stderr,
"InnoDB: doing i/o of type %lu at offset %lu %lu, length %lu\n",
			slot->type, slot->offset_high, slot->offset,
			total_len);
	}

	/* Do the i/o with ordinary, synchronous i/o functions: */
	if (slot->type == OS_FILE_WRITE) {
		if (array == os_aio_write_array) {

			/* Do a 'last millisecond' check that the page end
			is sensible; reported page checksum errors from
			Linux seem to wipe over the page end */

			for (len2 = 0; len2 + UNIV_PAGE_SIZE <= total_len;
						len2 += UNIV_PAGE_SIZE) {
				if (mach_read_from_4(combined_buf + len2
						+ FIL_PAGE_LSN + 4)
				    != mach_read_from_4(combined_buf + len2
				    		+ UNIV_PAGE_SIZE
				    	- FIL_PAGE_END_LSN_OLD_CHKSUM + 4)) {
				    	ut_print_timestamp(stderr);
				    	fprintf(stderr,
"  InnoDB: ERROR: The page to be written seems corrupt!\n");
					buf_page_print(combined_buf + len2);
				    	fprintf(stderr,
"InnoDB: ERROR: The page to be written seems corrupt!\n");
				}
			}
		}
	
		ret = os_file_write(slot->name, slot->file, combined_buf,
				slot->offset, slot->offset_high, total_len);
	} else {
		ret = os_file_read(slot->file, combined_buf,
				slot->offset, slot->offset_high, total_len);
	}

	ut_a(ret);
	srv_io_thread_op_info[global_segment] = (char*) "file i/o done";

/* printf("aio: %lu consecutive %lu:th segment, first offs %lu blocks\n",
			n_consecutive, global_segment, slot->offset
					/ UNIV_PAGE_SIZE); */

	if (slot->type == OS_FILE_READ && n_consecutive > 1) {
		/* Copy the combined buffer to individual buffers */
		offs = 0;
		
		for (i = 0; i < n_consecutive; i++) {

			ut_memcpy(consecutive_ios[i]->buf, combined_buf + offs, 
						consecutive_ios[i]->len);
			offs += consecutive_ios[i]->len;
		}
	}

	if (n_consecutive > 1) {
		ut_free(combined_buf2);
	}

	os_mutex_enter(array->mutex);

	/* Mark the i/os done in slots */

	for (i = 0; i < n_consecutive; i++) {
		consecutive_ios[i]->io_already_done = TRUE;
	}

	/* We return the messages for the first slot now, and if there were
	several slots, the messages will be returned with subsequent calls
	of this function */
	
slot_io_done:

	ut_a(slot->reserved);

	*message1 = slot->message1;
	*message2 = slot->message2;

	*type = slot->type;

	os_mutex_exit(array->mutex);

	os_aio_array_free_slot(array, slot);
	
	return(ret);

wait_for_io:
	/* We wait here until there again can be i/os in the segment
	of this thread */
	
	os_event_reset(os_aio_segment_wait_events[global_segment]);

	os_mutex_exit(array->mutex);

recommended_sleep:
	srv_io_thread_op_info[global_segment] =
				(char*)"waiting for i/o request";

	os_event_wait(os_aio_segment_wait_events[global_segment]);

	if (os_aio_print_debug) {
		fprintf(stderr,
"InnoDB: i/o handler thread for i/o segment %lu wakes up\n",
			global_segment);
	}
	
	goto restart;
}

/**************************************************************************
Validates the consistency of an aio array. */
static
ibool
os_aio_array_validate(
/*==================*/
				/* out: TRUE if ok */
	os_aio_array_t*	array)	/* in: aio wait array */
{
	os_aio_slot_t*	slot;
	ulint		n_reserved	= 0;
	ulint		i;
	
	ut_a(array);

	os_mutex_enter(array->mutex);

	ut_a(array->n_slots > 0);
	ut_a(array->n_segments > 0);
	
	for (i = 0; i < array->n_slots; i++) {
		slot = os_aio_array_get_nth_slot(array, i);
	
		if (slot->reserved) {
			n_reserved++;
			ut_a(slot->len > 0);
		}
	}

	ut_a(array->n_reserved == n_reserved);

	os_mutex_exit(array->mutex);

	return(TRUE);
}

/**************************************************************************
Validates the consistency the aio system. */

ibool
os_aio_validate(void)
/*=================*/
				/* out: TRUE if ok */
{
	os_aio_array_validate(os_aio_read_array);
	os_aio_array_validate(os_aio_write_array);
	os_aio_array_validate(os_aio_ibuf_array);
	os_aio_array_validate(os_aio_log_array);
	os_aio_array_validate(os_aio_sync_array);

	return(TRUE);
}

/**************************************************************************
Prints info of the aio arrays. */

void
os_aio_print(
/*=========*/
	char*	buf,	/* in/out: buffer where to print */
	char*	buf_end)/* in: buffer end */
{
	os_aio_array_t*	array;
	os_aio_slot_t*	slot;
	ulint		n_reserved;
	time_t		current_time;
	double		time_elapsed;
	double		avg_bytes_read;
	ulint		i;

	if (buf_end - buf < 1200) {

		return;
	}

	for (i = 0; i < srv_n_file_io_threads; i++) {
		buf += sprintf(buf, "I/O thread %lu state: %s (%s)\n", i,
					srv_io_thread_op_info[i],
					srv_io_thread_function[i]);
	}

	buf += sprintf(buf, "Pending normal aio reads:");

	array = os_aio_read_array;
loop:
	ut_a(array);
	
	os_mutex_enter(array->mutex);

	ut_a(array->n_slots > 0);
	ut_a(array->n_segments > 0);
	
	n_reserved = 0;

	for (i = 0; i < array->n_slots; i++) {
		slot = os_aio_array_get_nth_slot(array, i);
	
		if (slot->reserved) {
			n_reserved++;
			/* printf("Reserved slot, messages %lx %lx\n",
					(ulint)slot->message1,
					(ulint)slot->message2);
			*/			ut_a(slot->len > 0);
		}
	}

	ut_a(array->n_reserved == n_reserved);

	buf += sprintf(buf, " %lu", n_reserved);
	
	os_mutex_exit(array->mutex);

	if (array == os_aio_read_array) {
		buf += sprintf(buf, ", aio writes:");
	
		array = os_aio_write_array;

		goto loop;
	}

	if (array == os_aio_write_array) {
		buf += sprintf(buf, ",\n ibuf aio reads:");
		array = os_aio_ibuf_array;

		goto loop;
	}

	if (array == os_aio_ibuf_array) {
		buf += sprintf(buf, ", log i/o's:");
		array = os_aio_log_array;

		goto loop;
	}

	if (array == os_aio_log_array) {
		buf += sprintf(buf, ", sync i/o's:");		
		array = os_aio_sync_array;

		goto loop;
	}

	buf += sprintf(buf, "\n");
	
	current_time = time(NULL);
	time_elapsed = 0.001 + difftime(current_time, os_last_printout);

	buf += sprintf(buf,
		"Pending flushes (fsync) log: %lu; buffer pool: %lu\n",
	       fil_n_pending_log_flushes, fil_n_pending_tablespace_flushes);
	buf += sprintf(buf,
		"%lu OS file reads, %lu OS file writes, %lu OS fsyncs\n",
		os_n_file_reads, os_n_file_writes, os_n_fsyncs);

	if (os_file_n_pending_preads != 0 || os_file_n_pending_pwrites != 0) {
	        buf += sprintf(buf,
		    "%lu pending preads, %lu pending pwrites\n",
		    os_file_n_pending_preads, os_file_n_pending_pwrites);
	}

	if (os_n_file_reads == os_n_file_reads_old) {
		avg_bytes_read = 0.0;
	} else {
		avg_bytes_read = os_bytes_read_since_printout /
				(os_n_file_reads - os_n_file_reads_old);
	}

	buf += sprintf(buf,
"%.2f reads/s, %lu avg bytes/read, %.2f writes/s, %.2f fsyncs/s\n",
		(os_n_file_reads - os_n_file_reads_old)
		/ time_elapsed,
		(ulint)avg_bytes_read,
		(os_n_file_writes - os_n_file_writes_old)
		/ time_elapsed,
		(os_n_fsyncs - os_n_fsyncs_old)
		/ time_elapsed);

	os_n_file_reads_old = os_n_file_reads;
	os_n_file_writes_old = os_n_file_writes;
	os_n_fsyncs_old = os_n_fsyncs;
	os_bytes_read_since_printout = 0;
	
	os_last_printout = current_time;
}

/**************************************************************************
Refreshes the statistics used to print per-second averages. */

void
os_aio_refresh_stats(void)
/*======================*/
{
	os_n_file_reads_old = os_n_file_reads;
	os_n_file_writes_old = os_n_file_writes;
	os_n_fsyncs_old = os_n_fsyncs;
	os_bytes_read_since_printout = 0;
	
	os_last_printout = time(NULL);
}

/**************************************************************************
Checks that all slots in the system have been freed, that is, there are
no pending io operations. */

ibool
os_aio_all_slots_free(void)
/*=======================*/
				/* out: TRUE if all free */
{
	os_aio_array_t*	array;
	ulint		n_res	= 0;
	
	array = os_aio_read_array;

	os_mutex_enter(array->mutex);

	n_res += array->n_reserved; 
	
	os_mutex_exit(array->mutex);

	array = os_aio_write_array;

	os_mutex_enter(array->mutex);

	n_res += array->n_reserved; 
	
	os_mutex_exit(array->mutex);

	array = os_aio_ibuf_array;

	os_mutex_enter(array->mutex);

	n_res += array->n_reserved; 
	
	os_mutex_exit(array->mutex);

	array = os_aio_log_array;

	os_mutex_enter(array->mutex);

	n_res += array->n_reserved; 
	
	os_mutex_exit(array->mutex);

	array = os_aio_sync_array;

	os_mutex_enter(array->mutex);

	n_res += array->n_reserved; 
	
	os_mutex_exit(array->mutex);

	if (n_res == 0) {

		return(TRUE);
	}

	return(FALSE);
}
