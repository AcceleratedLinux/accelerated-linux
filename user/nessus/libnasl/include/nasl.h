#ifndef __LIB_NASL_H__
#define __LIB_NASL_H__

/*
 * NASL language level
 * Below 1000 is 1.2.6 and before
 *
 * Level 1000:
 * ACT_INIT, ACT_KILL_HOST and ACT_END
 *
 * Level 2000:
 * NASL2
 *
 * Level 2010:
 * Fix repeat / until loop
 * Handle icmp_seq parameter in forge_icmp_packet
 *
 * Level 2020:
 * Allow \0 or \x00 in string constants
 *
 * Level 2100
 * "Constant arrays" added, e.g. v = [ 'a' => 1, 'x' => 'zzz' ];
 *
 * Level 2150
 * ACT_FLOOD added
 *
 * Level 2160
 * Unnamed function arguments
 *
 * Level 2170
 * "continue" instruction
 *
 * Level 2172
 * forge_ip_packet does not ignore its ip_dst argument any more
 *
 * Level 2180
 * __FCT_ANON_ARGS special variable - Maybe $ident should be dropped
 *
 * Level 2181
 * Improve libnasl for NASL wrappers: fix pread() and add get_preference()
 *
 * Level 2190
 * Remove "dollar identifiers" $1, $2... $*
 *
 * Level 2191
 * CVE, BID and Xrefs can be much more numerous now
 *
 * Level 2200
 * New functions: fwrite, script_get_preference_file_location
 *
 * Level 2201
 * New functions: file_stat(), file_read(), file_write(), file_close(), file_seek()
 *
 * Level 2202
 * Only signed scripts can write 'Secret/' entries in the KB
 * New function: open_sock_to_kdc()
 *
 * Level 2203
 * New function send_capture()
 * Fixed shared sockets
 *
 * Level 2204
 * Arrays of arrays 
 *
 */
#define NASL_LEVEL 2204


int execute_nasl_script(struct arglist *, const char *, const char *, int);
int execute_preparsed_nasl_script(struct arglist *, char *, char *, int, int);
char * nasl_version();
pid_t nasl_server_start(char *, char *);
void  nasl_server_recompile(char*, char *);

/* execute_nasl_script modes */
#define NASL_EXEC_DESCR			   (1 << 0)
#define NASL_EXEC_PARSE_ONLY		   (1 << 1)
#define NASL_EXEC_DONT_CLEANUP  	   (1 << 2)
#define NASL_ALWAYS_SIGNED		   (1 << 3)
#define NASL_COMMAND_LINE		   (1 << 4)
#define NASL_LINT			   (1 << 5)


#define NASL_ERR_NOERR		0
#define NASL_ERR_ETIMEDOUT 	1
#define NASL_ERR_ECONNRESET	2
#define NASL_ERR_EUNREACH	3
#define NASL_ERR_EUNKNOWN	99
#endif
