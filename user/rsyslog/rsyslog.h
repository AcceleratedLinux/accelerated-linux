/* Header file with global definitions for the whole
 * rsyslog project (including all subprojects like
 * rfc3195d).
 * Begun 2005-09-15 RGerhards
 *
 * Copyright (C) 2005 by Rainer Gerhards and Adiscon GmbH
 *
 * This file is part of rsyslog.
 *
 * Rsyslog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rsyslog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rsyslog.  If not, see <http://www.gnu.org/licenses/>.
 *
 * A copy of the GPL can be found in the file "COPYING" in this distribution.
*/
#ifndef INCLUDED_RSYSLOG_H
#define INCLUDED_RSYSLOG_H

/* ############################################################# *
 * #                    Config Settings                        # *
 * ############################################################# */
#define RS_STRINGBUF_ALLOC_INCREMENT 128

/* ############################################################# *
 * #                  End Config Settings                      # *
 * ############################################################# */

#ifndef	NOLARGEFILE
#	undef _LARGEFILE_SOURCE  
#	undef _LARGEFILE64_SOURCE  
#	undef _FILE_OFFSET_BITS
#	define _LARGEFILE_SOURCE  
#	define _LARGEFILE64_SOURCE  
#	define _FILE_OFFSET_BITS 64
#endif


/* some universal 64 bit define... */
typedef long long int64;
typedef long long unsigned uint64;
typedef int64 number_t; /* type to use for numbers - TODO: maybe an autoconf option? */

#ifdef __hpux
typedef unsigned int u_int32_t; /* TODO: is this correct? */
typedef int socklen_t;
#endif

/* settings for flow control
 * TODO: is there a better place for them? -- rgerhards, 2008-03-14
 */
typedef enum {
	eFLOWCTL_NO_DELAY = 0,		/**< UDP and other non-delayable sources */
	eFLOWCTL_LIGHT_DELAY = 1,	/**< some light delay possible, but no extended period of time */
	eFLOWCTL_FULL_DELAY = 2	/**< delay possible for extended period of time */
} flowControl_t;


/* The error codes below are orginally "borrowed" from
 * liblogging. As such, we reserve values up to -2999
 * just in case we need to borrow something more ;)
*/
enum rsRetVal_				/** return value. All methods return this if not specified otherwise */
{
	RS_RET_NOT_IMPLEMENTED = -7,	/**< implementation is missing (probably internal error or lazyness ;)) */
	RS_RET_OUT_OF_MEMORY = -6,	/**< memory allocation failed */
	RS_RET_PROVIDED_BUFFER_TOO_SMALL = -50,/**< the caller provided a buffer, but the called function sees the size of this buffer is too small - operation not carried out */
	RS_RET_TRUE = -1,		/**< to indicate a true state (can be used as TRUE, legacy) */
	RS_RET_FALSE = -2,		/**< to indicate a false state (can be used as FALSE, legacy) */
	RS_RET_NO_IRET = -8,	/**< This is a trick for the debuging system - it means no iRet is provided  */
	RS_RET_ERR = -3000,	/**< generic failure */
	RS_TRUNCAT_TOO_LARGE = -3001, /**< truncation operation where too many chars should be truncated */
	RS_RET_FOUND_AT_STRING_END = -3002, /**< some value found, but at the last pos of string */
	RS_RET_NOT_FOUND = -3003, /**< some requested value not found */
	RS_RET_MISSING_TRAIL_QUOTE = -3004, /**< an expected trailing quote is missing */
	RS_RET_NO_DIGIT = -3005,	/**< an digit was expected, but none found (mostly parsing) */
	RS_RET_NO_MORE_DATA = -3006,	/**< insufficient data, e.g. end of string during parsing */
	RS_RET_INVALID_IP = -3007,	/**< invalid ip found where valid was expected */
	RS_RET_OBJ_CREATION_FAILED = - 3008, /**< the creation of an object failed (no details available) */
	RS_RET_PARAM_ERROR = -1000,	/**< invalid parameter in call to function */
	RS_RET_MISSING_INTERFACE = -1001,/**< interface version mismatch, required missing */
	RS_RET_INVALID_CORE_INTERFACE = -1002,/**< interface provided by host invalid, can not be used */
	RS_RET_ENTRY_POINT_NOT_FOUND = -1003,/**< a requested entry point was not found */
	RS_RET_MODULE_ENTRY_POINT_NOT_FOUND = -1004,/**< a entry point requested from a module was not present in it */
	RS_RET_OBJ_NOT_AVAILABLE = -1005,/**< something could not be completed because the required object is not available*/
	RS_RET_LOAD_ERROR = -1006,/**< we had an error loading the object/interface and can not continue */
	RS_RET_MODULE_STILL_REFERENCED = -1007,/**< module could not be unloaded because it still is referenced by someone */
	RS_RET_OBJ_UNKNOWN = -1008,/**< object is unknown where required */
	RS_RET_OBJ_NOT_REGISTERED = -1009,/**< tried to unregister an object that is not registered */
	/* return states for config file processing */
	RS_RET_NONE = -2000,		/**< some value is not available - not necessarily an error */
	RS_RET_CONFLINE_UNPROCESSED = -2001,/**< config line was not processed, pass to other module */
	RS_RET_DISCARDMSG = -2002,	/**< discard message (no error state, processing request!) */
	RS_RET_INCOMPATIBLE = -2003,	/**< function not compatible with requested feature */
	RS_RET_NOENTRY = -2004,		/**< do not create an entry for (whatever) - not necessary an error */
	RS_RET_NO_SQL_STRING = -2005,	/**< string is not suitable for use as SQL */
	RS_RET_DISABLE_ACTION = -2006,  /**< action requests that it be disabled */
	RS_RET_SUSPENDED = -2007,  /**< something was suspended, not neccesarily an error */
	RS_RET_RQD_TPLOPT_MISSING = -2008,/**< a required template option is missing */
	RS_RET_INVALID_VALUE = -2009,/**< some value is invalid (e.g. user-supplied data) */
	RS_RET_INVALID_INT = -2010,/**< invalid integer */
	RS_RET_INVALID_CMD = -2011,/**< invalid command */
	RS_RET_VAL_OUT_OF_RANGE = -2012, /**< value out of range */
	RS_RET_FOPEN_FAILURE = -2013,	/**< failure during fopen, for example file not found - see errno */
	RS_RET_END_OF_LINKEDLIST = -2014,	/**< end of linked list, not an error, but a status */
	RS_RET_CHAIN_NOT_PERMITTED = -2015, /**< chaining (e.g. of config command handlers) not permitted */
	RS_RET_INVALID_PARAMS = -2016,/**< supplied parameters are invalid */
	RS_RET_EMPTY_LIST = -2017, /**< linked list is empty */
	RS_RET_FINISHED = -2018, /**< some opertion is finished, not an error state */
	RS_RET_INVALID_SOURCE = -2019, /**< source (address) invalid for some reason */
	RS_RET_ADDRESS_UNKNOWN = -2020, /**< an address is unknown - not necessarily an error */
	RS_RET_MALICIOUS_ENTITY = -2021, /**< there is an malicious entity involved */
	RS_RET_NO_KERNEL_LOGSRC = -2022, /**< no source for kernel logs can be obtained */
	RS_RET_TCP_SEND_ERROR = -2023, /**< error during TCP send process */
	RS_RET_GSS_SEND_ERROR = -2024, /**< error during GSS (via TCP) send process */
	RS_RET_TCP_SOCKCREATE_ERR = -2025, /**< error during creation of TCP socket */
	RS_RET_GSS_SENDINIT_ERROR = -2024, /**< error during GSS (via TCP) send initialization process */
	RS_RET_QUEUE_FULL = -2025, /**< queue is full, operation could not be completed */
	RS_RET_EOF = -2026, /**< end of file reached, not necessarily an error */
	RS_RET_IO_ERROR = -2027, /**< some kind of IO error happened */
	RS_RET_INVALID_OID = -2028, /**< invalid object ID */
	RS_RET_INVALID_HEADER = -2029, /**< invalid header */
	RS_RET_INVALID_HEADER_VERS = -2030, /**< invalid header version */
	RS_RET_INVALID_DELIMITER = -2031, /**< invalid delimiter, e.g. between params */
	RS_RET_INVALID_PROPFRAME = -2032, /**< invalid framing in serialized property */
	RS_RET_NO_PROPLINE = -2033, /**< line is not a property line */
	RS_RET_INVALID_TRAILER = -2034, /**< invalid trailer */
	RS_RET_VALUE_TOO_LOW = -2035, /**< a provided value is too low */
	RS_RET_FILE_PREFIX_MISSING = -2036, /**< a required file prefix (parameter?) is missing */
	RS_RET_INVALID_HEADER_RECTYPE = -2037, /**< invalid record type in header or invalid header */
	RS_RET_QTYPE_MISMATCH = -2038, /**< different qType when reading back a property type */
	RS_RET_NO_FILE_ACCESS = -2039, /**< covers EACCES error on file open() */
	RS_RET_FILE_NOT_FOUND = -2040, /**< file not found */
	RS_RET_TIMED_OUT = -2041, /**< timeout occured (not necessarily an error) */
	RS_RET_QSIZE_ZERO = -2042, /**< queue size is zero where this is not supported */
	RS_RET_ALREADY_STARTING = -2043, /**< something (a thread?) is already starting - not necessarily an error */
	RS_RET_NO_MORE_THREADS = -2044, /**< no more threads available, not necessarily an error */
	RS_RET_NO_FILEPREFIX = -2045, /**< file prefix is not specified where one is needed */
	RS_RET_CONFIG_ERROR = -2046, /**< there is a problem with the user-provided config settigs */
	RS_RET_OUT_OF_DESRIPTORS = -2047, /**< a descriptor table's space has been exhausted */
	RS_RET_NO_DRIVERS = -2048, /**< a required drivers missing */
	RS_RET_NO_DRIVERNAME = -2049, /**< driver name missing where one was required */
	RS_RET_EOS = -2050, /**< end of stream (of whatever) */
	RS_RET_SYNTAX_ERROR = -2051, /**< syntax error, eg. during parsing */
	RS_RET_INVALID_OCTAL_DIGIT = -2052, /**< invalid octal digit during parsing */
	RS_RET_INVALID_HEX_DIGIT = -2053, /**< invalid hex digit during parsing */
	RS_RET_INTERFACE_NOT_SUPPORTED = -2054, /**< interface not supported */
	RS_RET_OUT_OF_STACKSPACE = -2055, /**< a stack data structure is exhausted and can not be grown */
	RS_RET_STACK_EMPTY = -2056, /**< a pop was requested on a stack, but the stack was already empty */
	RS_RET_INVALID_VMOP = -2057, /**< invalid virtual machine instruction */
	RS_RET_INVALID_VAR = -2058, /**< a var_t or its content is unsuitable, eg. VARTYPE_NONE */
	RS_RET_INVALID_NUMBER = -2059, /**< number invalid during parsing */
	RS_RET_NOT_A_NUMBER = -2060, /**< e.g. conversion impossible because the string is not a number */
	RS_RET_OBJ_ALREADY_REGISTERED = -2061, /**< object (name) is already registered */
	RS_RET_OBJ_REGISTRY_OUT_OF_SPACE = -2062, /**< the object registry has run out of space */
	RS_RET_HOST_NOT_PERMITTED = -2063, /**< a host is not permitted to perform an action it requested */
	RS_RET_MODULE_LOAD_ERR = -2064, /**< module could not be loaded */
	RS_RET_MODULE_LOAD_ERR_PATHLEN = -2065, /**< module could not be loaded - path to long */
	RS_RET_MODULE_LOAD_ERR_DLOPEN = -2066, /**< module could not be loaded - problem in dlopen() */
	RS_RET_MODULE_LOAD_ERR_NO_INIT = -2067, /**< module could not be loaded - init() missing */
	RS_RET_MODULE_LOAD_ERR_INIT_FAILED = -2068, /**< module could not be loaded - init() failed */

	/* RainerScript error messages (range 1000.. 1999) */
	RS_RET_SYSVAR_NOT_FOUND = 1001, /**< system variable could not be found (maybe misspelled) */

	/* some generic error/status codes */
	RS_RET_OK_DELETE_LISTENTRY = 1,	/**< operation successful, but callee requested the deletion of an entry (special state) */
	RS_RET_TERMINATE_NOW = 2,	/**< operation successful, function is requested to terminate (mostly used with threads) */
	RS_RET_NO_RUN = 3,		/**< operation successful, but function does not like to be executed */
	RS_RET_OK = 0			/**< operation successful */
};
typedef enum rsRetVal_ rsRetVal; /**< friendly type for global return value */

/* some helpful macros to work with srRetVals.
 * Be sure to call the to-be-returned variable always "iRet" and
 * the function finalizer always "finalize_it".
 */
#define CHKiRet(code) if((iRet = code) != RS_RET_OK) goto finalize_it
/* macro below is to be used if we need our own handling, eg for cleanup */
#define CHKiRet_Hdlr(code) if((iRet = code) != RS_RET_OK)
/* macro below is to handle failing malloc/calloc/strdup... which we almost always handle in the same way... */
#define CHKmalloc(operation) if((operation) == NULL) ABORT_FINALIZE(RS_RET_OUT_OF_MEMORY)
/* macro below is used in conjunction with CHKiRet_Hdlr, else use ABORT_FINALIZE */
#define FINALIZE goto finalize_it;
#define DEFiRet BEGINfunc rsRetVal iRet = RS_RET_OK
#define RETiRet do{ ENDfuncIRet return iRet; }while(0)

#define ABORT_FINALIZE(errCode)			\
	do {					\
		iRet = errCode;			\
		goto finalize_it;		\
	} while (0)

/** Object ID. These are for internal checking. Each
 * object is assigned a specific ID. This is contained in
 * all Object structs (just like C++ RTTI). We can use 
 * this field to see if we have been passed a correct ID.
 * Other than that, there is currently no other use for
 * the object id.
 */
enum rsObjectID
{
	OIDrsFreed = -1,		/**< assigned, when an object is freed. If this
				 *   is seen during a method call, this is an
				 *   invalid object pointer!
				 */
	OIDrsInvalid = 0,	/**< value created by calloc(), so do not use ;) */
	/* The 0x3412 is a debug aid. It helps us find object IDs in memory
	 * dumps (on X86, this is 1234 in the dump ;)
	 * If you are on an embedded device and you would like to save space
	 * make them 1 byte only.
	 */
	OIDrsCStr = 0x34120001,
	OIDrsPars = 0x34120002
};
typedef enum rsObjectID rsObjID;

/* support to set object types */
#ifdef NDEBUG
#define rsSETOBJTYPE(pObj, type)
#define rsCHECKVALIDOBJECT(x, type)
#else
#define rsSETOBJTYPE(pObj, type) pObj->OID = type;
#define rsCHECKVALIDOBJECT(x, type) {assert(x != NULL); assert(x->OID == type);}
#endif

/**
 * This macro should be used to free objects. 
 * It aids in interpreting dumps during debugging.
 */
#ifdef NDEBUG
#define RSFREEOBJ(x) free(x)
#else
#define RSFREEOBJ(x) {(x)->OID = OIDrsFreed; free(x);}
#endif

/* get rid of the unhandy "unsigned char"
 */
typedef unsigned char uchar;

/* for the time being, we do our own portability handling here. It
 * looks like autotools either does not yet support checks for it, or
 * I wasn't smart enough to find them ;) rgerhards, 2007-07-18
 */
#ifndef __GNUC__
#  define  __attribute__(x)  /*NOTHING*/
#endif

/* The following prototype is convenient, even though it may not be the 100% correct place.. -- rgerhards 2008-01-07 */
void dbgprintf(char *, ...) __attribute__((format(printf, 1, 2)));

#include "debug.h"

#endif /* multi-include protection */
/*
 * vi:set ai:
 */
