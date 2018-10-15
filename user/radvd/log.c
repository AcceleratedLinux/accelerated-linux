/*
 *	$Id: log.c,v 1.3 2001/11/14 19:58:11 lutchann Exp $
 *
 *	Authors:
 *	 Lars Fenneberg		<lf@elemental.net>	 
 *
 *	This software is Copyright 1996,1997 by the above mentioned author(s), 
 *	All Rights Reserved.
 *
 *	The license which is distributed with this software in the file
 *	COPYRIGHT applies to this software. If your distribution is missing 
 *	this file, you may request it from <lutchann@litech.org>.
 *
 */

#include <config.h>
#include <includes.h>
#include <radvd.h>

static int	log_method = L_NONE;
static char *log_ident;
static char *log_file;
static FILE *log_file_fd;
static int log_facility;
static int debug_level = 0;

int
log_open(int method, char *ident, char *log, int facility)
{
	log_method = method;
	log_ident = ident;
	
	switch (log_method) {
		case L_NONE:
		case L_STDERR:
			break;
		case L_SYSLOG:
			if (facility == -1)
				log_facility = LOG_DAEMON;
			else 
				log_facility = facility;
				
			openlog(log_ident, LOG_PID, log_facility);
			break;
		case L_LOGFILE:
			if (!log)
			{
				fprintf(stderr, "%s: no logfile specified\n", log_ident);
				return (-1);				
			}
			log_file = log;
			if ((log_file_fd = fopen(log_file, "a")) == NULL)
			{
				fprintf(stderr, "%s: can't open %s: %s\n", log_ident, log_file, strerror(errno));
				return (-1);				
			}
			break;
		default:
			fprintf(stderr, "%s: unknown logging method: %d\n", log_ident, log_method);
			log_method = L_NONE;
			return (-1);				
	}
	return 0;
}

static int
vlog(int prio, char *format, va_list ap)
{
	char tstamp[64], buff[1024];
	struct tm *tm;
	time_t current;
                  
	switch (log_method) {
		case L_NONE:
			break;
		case L_SYSLOG:
	    		vsnprintf(buff, sizeof(buff), format, ap);
			syslog(prio, "%s", buff);
			break;
		case L_STDERR:
			current = time(NULL);
			tm = localtime(&current);
			(void) strftime(tstamp, sizeof(tstamp), LOG_TIME_FORMAT, tm);

			fprintf(stderr, "[%s] %s: ", tstamp, log_ident);
	    		vfprintf(stderr, format, ap);
    			fputs("\n", stderr);
    			fflush(stderr);
			break;
		case L_LOGFILE:
			current = time(NULL);
			tm = localtime(&current);
			(void) strftime(tstamp, sizeof(tstamp), LOG_TIME_FORMAT, tm);

			fprintf(log_file_fd, "[%s] %s: ", tstamp, log_ident);
    			vfprintf(log_file_fd, format, ap);
    			fputs("\n", log_file_fd);
    			fflush(log_file_fd);
			break;
		default:
			fprintf(stderr, "%s: unknown logging method: %d\n", log_ident, log_method);
			log_method = L_NONE;
			return (-1);				
	}
	return 0;
}

int
dlog(int prio, int level, char *format, ...)
{
	va_list ap;
	int res;

	if (debug_level < level)
		return 0;
	
	va_start(ap, format);
	res = vlog(prio, format, ap);
	va_end(ap);		
	
	return res;
}

int
log(int prio, char *format, ...)
{
	va_list ap;
	int res;

	va_start(ap, format);
	res = vlog(prio, format, ap);
	va_end(ap);		
	
	return res;
}

int
log_close(void)
{
	switch (log_method) {
		case L_NONE:
		case L_STDERR:
			break;
		case L_SYSLOG:
			closelog();
			break;
		case L_LOGFILE:
			fclose(log_file_fd);
			break;
		default:
			fprintf(stderr, "%s: unknown logging method: %d\n", log_ident, log_method);
			log_method = L_NONE;
			return (-1);				
	}
	return 0;
}

int
log_reopen(void)
{
	log_close();
	return log_open(log_method, log_ident, log_file, log_facility);
}

void
set_debuglevel(int level) 
{
	debug_level = level;
}

int
get_debuglevel(void) 
{
	return debug_level;
}
