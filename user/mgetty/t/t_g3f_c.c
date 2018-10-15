/* $Id: t_g3f_c.c,v 1.2 2007/03/09 16:07:40 gert Exp $
 *
 * test program for mgetty/sendfax "g3file.c"
 *
 * read g3 files, output to stdout, validate vs g3cat
 *  (called from t_g3file script)
 *
 * $Log: t_g3f_c.c,v $
 * Revision 1.2  2007/03/09 16:07:40  gert
 * add stdlib.h
 *
 * Revision 1.1  2006/09/25 22:33:38  gert
 * test C program + script helper for g3file.o
 *
 *
 */

#include "mgetty.h"
#include "fax_lib.h"

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#ifdef T_LOG_VERBOSE
# include <stdarg.h>
#endif

/* fake logging functions */
int lputc( int level, char ch ) { return 0; }
int lputs( int level, char * s ) { return 0; }
int lprintf( int level, const char * format, ...) 
#ifdef T_LOG_VERBOSE
    { va_list pvar; va_start( pvar, format ); 
      vfprintf( stderr, format, pvar ); putc('\n', stderr); }
#else
    { return 0; }
#endif

/* some more fake functions - we need to link faxlib.c, and that 
 * wants class 1 stuff and other things we don't want to link
 */
char * fax_strerror( int h ) { return NULL; }
int fax1_set_fdcc( int fd, int fine, int max, int min ) { return 0; }
int fax1_set_l_id( int fd, char * fax_id ) { return 0; }

int main( int argc, char ** argv )
{
int opt;
int pad_bytes = 0;
int reverse = FALSE;

    while( (opt = getopt(argc, argv, "p:r" )) != EOF )
    {
	switch(opt)
	{
	  case 'p': 
	    pad_bytes = atoi(optarg);
	    break;
	  case 'r':
	    reverse = TRUE;
	    break;
	  case '?':
	    fprintf( stderr, "options: -p <padbytes>, -r\n" );
	    exit(1);
	}
    }

    fax_init_swaptable( !reverse, fax_send_swaptable );

    while( optind < argc )
    {
        if ( g3_open_read( argv[optind] ) < 0 )
	{
	    perror( "g3_open" ); exit(2);
	}

	if ( g3_send_file( g3_rf_chunk, /* out_fd */ 1, /* is_dev */ FALSE, 
			   /* escape_dle */ FALSE, pad_bytes, -1 ) )
	{
	    perror( "g3_send_file" ); exit(3);
	}

	optind++;
    }

    exit(0);
}
