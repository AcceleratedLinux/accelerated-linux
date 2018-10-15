#ident "$Id: class1lib.c,v 4.23 2009/03/19 15:31:21 gert Exp $ Copyright (c) Gert Doering"

/* class1lib.c
 *
 * Low-level functions to handle class 1 fax -- 
 * send a frame, receive a frame, dump frame to log file, ...
 *
 * $Log: class1lib.c,v $
 * Revision 4.23  2009/03/19 15:31:21  gert
 * add CVS tags
 *
 */

#ifdef CLASS1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>

#include "mgetty.h"
#include "fax_lib.h"
#include "tio.h"
#include "class1.h"
#include "version.h"

/* static variables
 *
 * only set by functions in this module and used by other functions, 
 * but have to be module-global
 */

#define F1LID 20
static char fax1_local_id[F1LID];	/* local system ID */
static int fax1_min = 2400,		/* min/max speed */
	   fax1_max = 14400;
static int fax1_res;			/* flag for normal resolution */

       uch fax1_dis = 0x00;		/* "X"-bit (last received DIS) */

static int fax1_fth, fax1_ftm,		/* modem carrier capabilities */
	   fax1_frh, fax1_frm;

/* symbolic constants for capability check
 */
#define V17		0xF00
#define V17_14400	0x800
#define V17_12000	0x400
#define V17_9600	0x200
#define V17_7200	0x100
#define V29		0x0F0
#define V29_9600	0x080
#define V29_7200	0x040
#define V27ter		0x00e
#define V27t_4800	0x008
#define V27t_2400	0x004
#define V21		0x001

/* table of baud rate / carrier number / DCS bits
 */
struct fax1_btable fax1_btable[] = {
	{ 14400, V17_14400, 145, 146, 0x20 /* 0001 */ },
	{ 12000, V17_12000, 121, 122, 0x28 /* 0101 */ },
	{  9600, V17_9600,   97,  98, 0x24 /* 1001 */ },
	{  9600, V29_9600,   96,  96, 0x04 /* 1000 */ },
	{  7200, V17_7200,   73,  74, 0x2c /* 1101 */ },
	{  7200, V29_7200,   72,  72, 0x0c /* 1100 */ },
	{  4800, V27t_4800,  48,  48, 0x08 /* 0100 */ },
	{  2400, V27t_2400,  24,  24, 0x00 /* 0000 */ },
	{   300, V21, 3, 3, 0 },
	{    -1, -1,0,0,0 }};

/* pointer to current modulation in fax1_btable
 * (increment == fallback after FTT!)
 */
struct fax1_btable * dcs_btp = fax1_btable;

/* table of bits-to-scan line time mappings, index = bits
 */
struct fax1_st_table { int st; int ms_n; int ms_f; char * txt; };
struct fax1_st_table fax1_st_table[8] = {
	/* ST, ms normal, ms fine                  bits    */
	{ 5, 20, 20, " 20ms" },			/* 0 = 000 */
	{ 1,  5,  5, " 5ms" },			/* 1 = 100 */
	{ 3, 10, 10, " 10ms" },			/* 2 = 010 */
	{ 4, 20, 10, " 20/10ms" },		/* 3 = 110 */
	{ 7, 40, 40, " 40ms" },			/* 4 = 001 */
	{ 6, 40, 20, " 40/20ms" },		/* 5 = 101 */
	{ 2, 10,  5, " 10/5ms" },		/* 6 = 011 */
	{ 0,  0,  0, " 0ms" }};			/* 7 = 111 */

int fax1_set_l_id _P2( (fd, fax_id), int fd, char * fax_id )
{
    int i,l;
    char *p;

    l = strlen( fax_id );
    if ( l > F1LID ) { l = F1LID; }

    /* bytes are transmitted in REVERSE order! */
    p = &fax1_local_id[F1LID-1];

    for ( i=0; i<l; i++ )     *(p--) = *(fax_id++);
    for (    ; i<F1LID; i++ ) *(p--) = ' ';

    return NOERROR;
}

static int fax1_carriers _P1((p), char * p )
{
    int cbits = 0;			/* carrier bits (V17_14400, ...) */
    int cnr;				/* carrier number (3,24,...) */
    char * ep;
    struct fax1_btable * btp;		/* pointer to baud rate table */

    while( *p )
    {
	cnr = strtol( p, &ep, 10 );

	if ( *ep ) ep++;		/* skip "," */
	p = ep;

	btp = fax1_btable;
	while( btp->speed > 0 )
	{
	    if ( cnr == btp->c_short || cnr == btp->c_long )
		    { cbits |= btp->flag; break; }
	    btp++;
    	}
    }
    return cbits;
}

/* step down max speed (at retrain time)
 */
void fax1_reduce_max _P0(void)
{
    if ( dcs_btp->speed > 2400 )
    { 
	fax1_max = dcs_btp->speed - 2400; 
	lprintf( L_NOISE, "reduce max speed to %d", fax1_max );
    }
}

/* set fine/normal resolution flags and min/max transmission speed
 * including finding out maximum speed modem can do!
 */
int fax1_set_fdcc _P4( (fd, fine, max, min),
		       int fd, int fine, int max, int min )
{
    char * p;

    lprintf( L_MESG, "fax1_set_fdcc: fine=%d, max=%d, min=%d", fine, max, min );

    fax1_max = max;
    fax1_min = min;
    fax1_res = fine;

    if ( fax1_min == 0 ) fax1_min=2400;

    if ( fax1_max < fax1_min ||
	 fax1_max < 2400 || fax1_max > 14400 ||
	 fax1_min < 2400 || fax1_min > 14400 ) 
    {
	lprintf( L_WARN, "min/max values (%d/%d) out of range, use 2400/14400", min, max );
	fax1_min = 2400; fax1_max = 14400;
    }

    if ( fax1_res < 0 || fax1_res > 1 ) 
    {
	lprintf( L_WARN, "fax1_res (%d) out of range, use fine (1)", fax1_res );
	fax1_res = 1;
    }

#if 0			/* we don't support anything but AT+FTH=3 yet */
    p = mdm_get_idstring( "AT+FTH=?", 1, fd );
    fax1_fth = fax1_carriers( p );
    lprintf( L_MESG, "modem can send HDLC headers: %03x", fax1_fth );
#else
    fax1_fth = 001;
#endif

    p = mdm_get_idstring( "AT+FTM=?", 1, fd );
    fax1_ftm = fax1_carriers( p );
    lprintf( L_MESG, "modem can send page data: %03x", fax1_ftm );

#if 0			/* we don't support anything but AT+FRH=3 yet */
    p = mdm_get_idstring( "AT+FRH=?", 1, fd );
    fax1_frh = fax1_carriers( p );
    lprintf( L_MESG, "modem can recv HDLC headers: %03x", fax1_fth );
#else
    fax1_frh = 001;
#endif

    p = mdm_get_idstring( "AT+FRM=?", 1, fd );
    fax1_frm = fax1_carriers( p );
    lprintf( L_MESG, "modem can recv page data: %03x", fax1_ftm );

    return NOERROR;
}


/* timeout handler
 */
static boolean fax1_got_timeout = FALSE;

RETSIGTYPE fax1_sig_alarm(SIG_HDLR_ARGS)
{
    signal( SIGALRM, fax1_sig_alarm );
    lprintf( L_WARN, "Warning: fax1: got alarm signal!" );
    fax1_got_timeout = TRUE;
}

/* receive ONE frame, put it into *framebuf
 *
 * timeout set to "timout * 1/10 seconds"
 */
int fax1_receive_frame _P4 ( (fd, carrier, timeout, framebuf),
			     int fd, int carrier, 
			     int timeout, uch * framebuf)
{
    int count=0;			/* bytes in frame */
    int rc = NOERROR;			/* return code */
    char gotsync = FALSE;		/* got 0xff frame sync */
    char WasDLE = FALSE;		/* got <DLE> character */
    char * line, c;

    if ( timeout > 0 )
    {
	signal( SIGALRM, fax1_sig_alarm );
        alarm( (timeout/10)+1 );
    }

    if ( carrier > 0 )
    {
    char cmd[20];
	sprintf( cmd, "AT+FRH=%d", carrier );

	/*!!! TOOD: DO NOT USE fax_send (FAX_COMMAND_DELAY) */

	/* normally, fax_send() can only fail if something is really 
	 * messed up with the serial port, e.g. "stuck flow control"
	 */
	if ( fax_send( cmd, fd ) == ERROR )
		    { alarm(0); return ERROR; }

	/* wait for CONNECT/NO CARRIER */
	line = mdm_get_line( fd );
	if ( line != NULL && strcmp( line, cmd ) == 0 )
		    { line = mdm_get_line( fd ); }		/* skip echo */

	if ( line == NULL || strcmp( line, "CONNECT" ) != 0 )
	{
	    alarm(0);
	    if ( line == NULL )
	    {
		char cancel_str[] = { CAN };
		lprintf( L_WARN, "fax1_receive_frame: no carrier (timeout), send CAN" );
		write( fd, cancel_str, 1 );
		alarm(2);
		line = mdm_get_line( fd );
		alarm(0);
	    }
	    else
		lprintf( L_WARN, "fax1_receive_frame: no carrier (%s)", line );
	    return ERROR;
	}
    }

    lprintf( L_NOISE, "fax1_receive_frame: got:" );

    /* we have a CONNECT now - now find the first byte of the frame
     * (0xFF), and read in <DLE> shielded data up to the <DLE><ETX>
     */

    while(1)
    {
        if ( mdm_read_byte( fd, &c ) != 1 )
	{
	    lprintf( L_ERROR, "fax1_receive_frame: cannot read byte, return" );
	    rc = ERROR; break;
	}

	/*!!!! TODO: CONNECT/ERROR statt Frame-Daten erkennen */

	lputc( L_NOISE, c );

	if ( !gotsync ) 		/* wait for preamble */
	{
	    if ( c == (char) 0xFF ) gotsync = TRUE;
	    continue;
	}

	/* got preamble, all further bytes are put into buffer */

	/* enough room? */
	if ( count >= FRAMESIZE-5 )
	{
	    lprintf( L_ERROR, "fax1_receive_frame: too many octets in frame" );
	    rc = ERROR; break;
	}

	if ( WasDLE )			/* previous character was DLE */
	{
	    if ( c == DLE )		/* DLE DLE -> DLE */
		{ framebuf[count++] = DLE; }
	    else if ( c == SUB )	/* DLE SUB -> DLE DLE */
	        { framebuf[count++] = DLE; framebuf[count++] = DLE; }
	    else if ( c == ETX )	/* end of frame detected */
	        { rc = count; break; }
	    
	    WasDLE = 0;
	    continue;
	}

	/* previous character was not DLE, check for DLE now... */
	if ( c == DLE )
	{  
	    WasDLE = 1; continue;
	}

	/* all other characters are stored in buffer */
	framebuf[count++] = c;
    }

    /* now read OK / ERROR response codes (only if we're still happy) */
    if ( rc != ERROR )
    {
	line = mdm_get_line( fd );

	if ( line == NULL ||				/* timeout ... */
	     strcmp( line, "ERROR" ) == 0 )		/* or FCS error */
	{
	    lprintf( L_MESG, "fax1_receive_frame: dropping frame" );
	    rc = ERROR;
	}
    }

    /* turn off alarm */
    alarm(0);

    if ( rc > 0 )
    {
        fax1_dump_frame( '<', framebuf, count );
    }

    return rc;
}


void fax1_dump_frame _P3((io, frame, len), 
			  char io, unsigned char * frame, int len)
{
int fcf = frame[1];

    lprintf( L_MESG, "%c frame type: 0x%02x  len: %d  %s%s", io,
                      fcf, len, frame[0]&0x10? "final": "non-final",
		      (fcf & 0x0e) && ( fcf & 0x01 ) ? " X": "");

    if ( fcf & 0x0e ) fcf &= ~0x01;	/* clear "X" bit */

    switch( fcf )
    {
    	/* simple frames */
	case T30_CSI:
	    lprintf( L_NOISE, "CSI: '%20.20s'", &frame[2] ); break;
	case T30_CIG:
	    lprintf( L_NOISE, "CIG: '%20.20s'", &frame[2] ); break;
	case T30_TSI:
	    lprintf( L_NOISE, "TSI: '%20.20s'", &frame[2] ); break;
	case T30_NSF:
	    lprintf( L_NOISE, "NSF" ); break;
	case T30_CFR:
	    lprintf( L_NOISE, "CFR" ); break;
	case T30_FTT:
	    lprintf( L_NOISE, "FTT" ); break;
	case T30_MCF:
	    lprintf( L_NOISE, "MCF" ); break;
	case T30_RTP:
	    lprintf( L_NOISE, "RTP" ); break;
	case T30_RTN:
	    lprintf( L_NOISE, "RTN" ); break;

	case T30_EOM:
	    lprintf( L_NOISE, "EOM" ); break;
	case T30_MPS:
	    lprintf( L_NOISE, "MPS" ); break;
	case T30_EOP:
	    lprintf( L_NOISE, "EOP" ); break;

	case T30_DCN:
	    lprintf( L_NOISE, "DCN" ); break;

	/* complicated ones... */
        case T30_DIS:
	    lprintf( L_NOISE, "DIS:" ); 
	    if ( frame[2] & 0x40 ) lputs( L_NOISE, " V8" );
	    lputs( L_NOISE, frame[2] & 0x80 ? " 64": " 256" );

	    if ( frame[3] & 0x01 ) lputs( L_NOISE, " +FPO" );
	    if ( frame[3] & 0x02 ) lputs( L_NOISE, " RCV" );

	    switch( (frame[3] >> 2) &0x0f )
	    {
	        case 0x00: lputs( L_NOISE, " V27ter_fb" ); break;
		case 0x02: lputs( L_NOISE, " V27ter" ); break;
		case 0x01: lputs( L_NOISE, " V29" ); break;
		case 0x03: lputs( L_NOISE, " V27ter+V29" ); break;
		case 0x0b: lputs( L_NOISE, " V27ter+V29+V17" ); break;
		default:   lputs( L_NOISE, " V.???" ); break;
	    }

	    if ( frame[3] & 0x40 ) lputs( L_NOISE, " 200" );
	    if ( frame[3] & 0x80 ) lputs( L_NOISE, " 2D" );

            switch( frame[4] & 0x03 )
	    {
	        case 0x00: lputs( L_NOISE, " 215mm" ); break;
		case 0x02: lputs( L_NOISE, " 215+255+303" ); break;
		case 0x01: lputs( L_NOISE, " 215+255" ); break;
	    }
	    switch( (frame[4]>>2) & 0x03 )
	    {
	        case 0x00: lputs( L_NOISE, " A4" ); break;
		case 0x02: lputs( L_NOISE, " unlim" ); break;
		case 0x01: lputs( L_NOISE, " A4+B4" ); break;
	    }
	    lputs( L_NOISE, fax1_st_table[ (frame[4]>>4) & 0x07 ].txt );

	    if ( ( frame[4] & 0x80 ) == 0 ) break;	/* extent bit */

	    if ( frame[5] & 0x04 ) lputs( L_NOISE, " ECM" );
	    if ( frame[5] & 0x40 ) lputs( L_NOISE, " T.6" );
	    if ( ( frame[5] & 0x80 ) == 0 ) break;	/* extent bit */

	    if ( ( frame[6] & 0x80 ) == 0 ) break;	/* extent bit */
	    /* the next bytes specify 300/400 dpi, color fax, ... */

	    break;
	case T30_DCS:
	    lprintf( L_NOISE, "DCS:" );

	    if ( frame[2] & 0x40 ) lputs( L_NOISE, " V8" );
	    lputs( L_NOISE, frame[2] & 0x80 ? " 64": " 256" );

	    if ( frame[3] & 0x02 ) lputs( L_NOISE, " RCV!" );

	    switch( (frame[3] >> 2) &0x0f )
	    {
	        case 0x00: lputs( L_NOISE, " V27ter_2400" ); break;
	        case 0x02: lputs( L_NOISE, " V27ter_4800" ); break;
	        case 0x01: lputs( L_NOISE, " V29_9600" ); break;
	        case 0x03: lputs( L_NOISE, " V29_7200" ); break;
	        case 0x04: lputs( L_NOISE, " V33_14400" ); break;
	        case 0x06: lputs( L_NOISE, " V33_12000" ); break;
	        case 0x08: lputs( L_NOISE, " V17_14400" ); break;
	        case 0x0a: lputs( L_NOISE, " V17_12000" ); break;
	        case 0x09: lputs( L_NOISE, " V17_9600" ); break;
	        case 0x0b: lputs( L_NOISE, " V17_7200" ); break;
		default:   lputs( L_NOISE, " V.???" ); break;
	    }

	    if ( frame[3] & 0x40 ) lputs( L_NOISE, " 200!" );
	    if ( frame[3] & 0x80 ) lputs( L_NOISE, " 2D!" );

            switch( frame[4] & 0x03 )
	    {
	        case 0x00: lputs( L_NOISE, " 1728/215mm" ); break;
		case 0x02: lputs( L_NOISE, " 2432/303mm" ); break;
		case 0x01: lputs( L_NOISE, " 2048/255mm" ); break;
	    }
	    switch( (frame[4]>>2) & 0x03 )
	    {
	        case 0x00: lputs( L_NOISE, " A4" ); break;
		case 0x02: lputs( L_NOISE, " unlim" ); break;
		case 0x01: lputs( L_NOISE, " B4" ); break;
	    }
	    lputs( L_NOISE, fax1_st_table[ (frame[4]>>4) & 0x07 ].txt );

	    if ( ( frame[4] & 0x80 ) == 0 ) break;	/* extent bit */

	    if ( frame[5] & 0x04 ) lputs( L_NOISE, " ECM" );
	    if ( frame[5] & 0x40 ) lputs( L_NOISE, " T.6" );
	    if ( ( frame[5] & 0x80 ) == 0 ) break;	/* extent bit */

	    if ( ( frame[6] & 0x80 ) == 0 ) break;	/* extent bit */
	    /* the next bytes specify 300/400 dpi, color fax, ... */

	    break;
	default:
	    lprintf( L_NOISE, "frame FCF 0x%02x not yet decoded", fcf );
    }
}

/* send arbitrary frame
 */
int fax1_send_frame _P4( (fd, carrier, frame, len), 
                         int fd, int carrier, uch * frame, int len )
{
char * line;
static int carrier_active = -2;		/* inter-frame marker */
uch dle_buf[FRAMESIZE*2+2];		/* for DLE-coded frame */
int r,w;

    fax1_dump_frame( '>', frame, len );

    /* this should never take more than a few msec, so the 10s timeout
     * is more a safeguard for modem lockups, implementation mistakes, etc.
     * - make sure we're not getting stuck in protocol loops
     */
    alarm(10);

    /* send AT+FTH=3, wait for CONNECT 
     * (but only if we've not sent an non-final frame before!)
     */

    /* special-case (AT+FTS=8;+FTH=3)
     */
    if ( carrier < 0 )		/* T30_CAR_HAVE_V21 */
    {
	carrier_active = -carrier;	/* -> T30_CAR_V21 */
	carrier = T30_CAR_SAME;
    }

    /* catch internal out-of-sync condition ('canthappen')
     * (this is OK for the very first frame sent in receive mode - ugly, yes)
     */
    if ( carrier == T30_CAR_SAME && carrier_active == -1 )
    {
	errno = EINVAL;
	lprintf( L_ERROR, "fax1_send_frame: internal error - no carrier, but T30_CAR_SAME requested" );
	return ERROR;
    }

    if ( carrier > 0 && carrier_active != carrier )
    {
    char cmd[20];

	sprintf( cmd, "AT+FTH=%d", carrier );
	if ( fax_send( cmd, fd ) == ERROR )
		    { alarm(0); carrier_active=-1; return ERROR; }

	/* wait for CONNECT/NO CARRIER */
	line = mdm_get_line( fd );
	if ( line != NULL && strcmp( line, cmd ) == 0 )
		    { line = mdm_get_line( fd ); }		/* skip echo */

	if ( line == NULL || strcmp( line, "CONNECT" ) != 0 )
	{
	    alarm(0);
	    lprintf( L_WARN, "fax1_send_frame: no carrier (%s)", line );
	    carrier_active=-1;
	    return ERROR;
	}

	carrier_active=carrier;
    }

    /* first 0xff is mandatory, but not passed by caller */
    dle_buf[0] = 0xff;
    w=1;

    /* send <DLE> encoded frame data */
    for( r=0; r<len; r++ )
    {
        if ( frame[r] == DLE ) { dle_buf[w++] = DLE; }
	dle_buf[w++] = frame[r];
    }

    /* end-of-frame: <DLE><ETX> */
    dle_buf[w++] = DLE; dle_buf[w++] = ETX;

    lprintf( L_JUNK, "fax1_send_frame: %d/%d", len+1, w );

    if ( write( fd, dle_buf, w ) != w )
    {
        lprintf( L_ERROR, "fax1_send_frame: can't write all %d bytes", w );
	alarm(0);
	fax_hangup=TRUE;
	return ERROR;
    }

    /*!!! alarm */
    /*!!! LASAT schickt "CONNECT\r\nOK" bzw. nur "OK" (final/non-final)
     *    --> ist das normal und richtig so??!?
     *
     * Nein... - es kommt immer entweder-oder, aber nach CONNECT muss
     * man OHNE neues AT+FTH *SOFORT* weitersenden!
     */
    line = mdm_get_line( fd );
    lprintf( L_NOISE, "fax1_send_frame: frame sent, got '%s'", line );

    if ( frame[0] & T30_FINAL )
    {
        carrier_active = -1;		/* carrier is off */
	lprintf( L_NOISE, "carrier is off - OK='%s'", line );
    }

    /* as we're sending, we shouldn't see NO CARRIER response - but this
     * can happen, e.g. when the modem notices a remote hangup (ISDN etc.)
     */
    if ( line == NULL || strcmp( line, "NO CARRIER" ) == 0 )
    {
	lprintf( L_WARN, "fax1_send_frame: unexpected post-frame string '%s', assuming carrier off", line? line: "(null)" );
	carrier_active = -1;
	return ERROR;
    }

#if 0
    if ( line != NULL && strcmp( line, "CONNECT" ) == 0 )
    {
	line = mdm_get_line( fd );
	lprintf( L_NOISE, "fax1_send_frame(2): got '%s'", line );
    }
#endif

    return NOERROR;
}

/* send simple frame, consisting only of FCF and no arguments
 * (non-final frame)
 */
int fax1_send_simf_nonfinal _P3( (fd, carrier, fcf), 
                                 int fd, int carrier, uch fcf )
{
uch frame[2];
   frame[0] = 0x03;
   frame[1] = fcf;
   return fax1_send_frame( fd, carrier, frame, 2 );
}

/* send simple frame, consisting only of FCF and no arguments
 * (final frame)
 */
int fax1_send_simf_final _P3( (fd, carrier, fcf), 
                              int fd, int carrier, uch fcf )
{
uch frame[2];
   frame[0] = 0x03 | T30_FINAL;
   frame[1] = fcf;
   return fax1_send_frame( fd, carrier, frame, 2 );
}

/* send "disconnect now" frame (DCN), and move internal state to "game over"
 * Note: this is always a "final" frame
 */
int fax1_send_dcn _P2((fd, code), int fd, int code )
{
    if ( code != -1 )
        { fax_hangup_code = code; fax_hangup = TRUE; }
    return fax1_send_simf_final( fd, T30_CAR_V21, T30_DCN|fax1_dis );
}

/* send non-standard frame (NSF) with mgetty T.35 vendor ID
 */
int fax1_send_nsf _P2( (fd, carrier), int fd, int carrier )
{
uch frame[FRAMESIZE];

    /* NSF is never final frame */
    frame[0] = 0x03;
    frame[1] = T30_NSF;
    frame[2] = 0x20;		/* germany */
    frame[3] = 0x81;		/* mgetty + */
    frame[4] = 0x70;		/*  sendfax */
    frame[5] = 0x00;		/* frame version */
    strncpy( &frame[6], "mgetty ", 7 );
    strncpy( &frame[13], VERSION_SHORT, 7 );

    return fax1_send_frame( fd, carrier, frame, 20 );
}

/* send local identification (CSI, CIG or TSI) 
 * Note: "final" bit is never set, as these frames are always optional.
 */
int fax1_send_idframe _P3((fd,fcf,carrier), int fd, uch fcf, int carrier)
{
    unsigned char frame[F1LID+2];

    frame[0] = 0x03;
    frame[1] = fcf;
    memcpy( &frame[2], fax1_local_id, F1LID );

    return fax1_send_frame( fd, carrier, frame, sizeof(frame) );
}

void fax1_copy_id _P1((frame), uch * frame )
{
int w, r;
char c;

    frame += 2;				/* go to start of ID */
    r = F1LID-1; w = 0;

    while ( r>= 0 && isspace(frame[r]) ) r--;	/* skip leading whitespace */

    while ( r>=0 )			/* copy backwards! */
    {
        c = frame[r--];
        if ( c == '"' || c == '\'' ) fax_remote_id[w++] = '_';
				else fax_remote_id[w++] = c;
    }
    while( w>0 && isspace(fax_remote_id[w-1]) ) w--;
    fax_remote_id[w]=0;

    lprintf( L_MESG, "fax_id: '%s'", fax_remote_id );
}

/* set local capabilities in DIS frame, announce to remote sender
 */
int fax1_send_dis _P1( (fd), int fd )
{
uch frame[FRAMESIZE];
int speedbits = 0;
int r_flags;
struct fax1_btable * dis_btp = fax1_btable;

    /* start with modem capabilities, restrain by FDCC values */
    r_flags = fax1_frm;
    while( dis_btp->speed > 2400 )
    {
	if ( dis_btp->speed < fax1_min || dis_btp->speed > fax1_max )
	{
	    r_flags &= ~(dis_btp->flag);
	    lprintf( L_NOISE, "fax1_dis: %d out of range -> r_flags=%03x", dis_btp->speed, r_flags );
	}
	dis_btp++;
    }

    /* some devices advertise V.17, but it doesn't work - so provide override
     * switch via modem_quirks that unconditionally turns off V.17
     */
    if ( modem_quirks & MQ_C1_NO_V17 ) r_flags &= ~V17;


    /* this is a bit messy, but I don't know a really elegant way to
     * use the fax1_btable structure for that  (see T.30, Table 2, p.46)
     */
    if ( (r_flags & V17) && (r_flags & V29) && (r_flags & V27ter) )
    {
	speedbits = 0x2c;		/* 1101 */
    }
    else 
    {
	if ( r_flags & V27t_4800 ) { speedbits |= 0x08; /* 0100 */ }
	if ( r_flags & V29 )       { speedbits |= 0x04; /* 1000 */ }
    }

    lprintf( L_NOISE, "fax1_dis: r_flags=%03x -> speedbits=0x%02x",
                      r_flags, speedbits );

    /* DIS is always final frame */
    frame[0] = 0x03 | T30_FINAL;
    frame[1] = T30_DIS;

    frame[2] = 0x00;		/* bits 1..8: group 1/2 - unwanted */
    frame[3] = 0x00 |		/* bit 9: can transmit - TODO: polling! */
	       0x02 |           /* bit 10: can receive */
	       speedbits |	/* bit 11..14: receive rates - TODO!! */
	       ((fax1_res&1) <<6) |	/* bit 15: can fine */
	       0x00;		/* bit 16: can 2D */
    frame[4] = 0x08 |		/* bits 17..20 = 215mm width, unlim. length */
               0x70 |		/* bits 21..23 = 0ms scan time */
	       0x00;		/* bit 24: extend bit - final */

    return fax1_send_frame( fd, T30_CAR_SAME, frame, 5 );
}

/* parse incoming DIS frame, set remote capability flags
 */

fax_param_t remote_cap;

void fax1_parse_dis _P1((frame), uch * frame )
{
    remote_cap.vr = remote_cap.br = remote_cap.wd = remote_cap.ln =
    remote_cap.df = remote_cap.ec = remote_cap.bf = remote_cap.st = 0;

    frame += 2;		/* go to start of FIF */

    /* bit 9: ready to transmit fax (polling) */
    if ( frame[1] & 0x01 ) fax_to_poll = TRUE;

    /* bit 10: receiving capabilities */
    if ( ( frame[1] & 0x02 ) == 0  )
    {
	/*!!!! HANDLE THIS */
        lprintf( L_WARN, "remote station can't receive!" );
	fax_hangup = TRUE; fax_hangup_code = 21; return;
    }

    switch( frame[1] & 0x3c )	/* bits 11..14 - data signalling rate */
    {
        case 0x00: remote_cap.br = V27t_2400; break;
	case 0x08: remote_cap.br = V27ter; break;
	case 0x04: remote_cap.br = V29; break;
	case 0x0c: remote_cap.br = V29 | V27ter; break;
	case 0x1c: remote_cap.br = V29 | V27ter; break;		/* V.33 */
	case 0x2c: remote_cap.br = V17 | V29 | V27ter; break;
	default:
	    lprintf( L_WARN, "unknown signalling rate: 0x%02x, use V27ter", frame[1] & 0x3c );
	    remote_cap.br = V27ter;
    }

    if ( frame[1] & 0x40 )	/* bit 15: fine res. */
    {
        remote_cap.vr = 1;
	/*!! check bits 42 + 43 for "super-fine" (300/400 dpi) */
    }

    if ( frame[1] & 0x80 )	/* bit 16: 2D */
    	remote_cap.df = 1;	/* df??? */

    /* bit 17+18: recording width, valid: 0/1/2 = 215/255/303 mm */
    remote_cap.wd = frame[2] & 0x03;

    /* bit 19+20: recording length, valid: 0/1/2 = A4/B4/unlimited */
    remote_cap.ln = ( frame[2] >> 2 ) & 0x03;

    /* bit 21-23: minimum scan line time */
    remote_cap.st = fax1_st_table[ (frame[2] >> 4) & 0x07 ].st;

    if ( frame[2] & 0x80 )	/* extend bit */
    {
	/* bit 27: ECM */
        if ( frame[3] & 0x04 ) remote_cap.ec = 1;
    }

    fax1_dis = 0x01;			/* set "X" bit (= received DIS OK) */

    lprintf( L_MESG, "+FIS: %d,%03x,%d,%d,%d,%d,%d,%d",
    			remote_cap.vr, remote_cap.br, remote_cap.wd,
			remote_cap.ln, remote_cap.df, remote_cap.ec,
			remote_cap.bf, remote_cap.st );
}

int fax1_send_dcs _P2((fd, s_time), int fd, int s_time )
{
uch framebuf[FRAMESIZE];
int i;

    /* find baud/carrier table entry that has a speed not over
     * "speed", and that uses a modulation scheme supported by both
     * the local and remote modem
     */
    dcs_btp = fax1_btable;

    while( dcs_btp->speed > 2400 &&
	   ( dcs_btp->speed > fax1_max ||
            ( dcs_btp->flag & fax1_ftm & remote_cap.br ) == 0 ) ) dcs_btp++;
    
    lprintf( L_NOISE, "+DCS: 1,%03x", dcs_btp->flag );

    /*!!! calculate ALL values from DIS and to-be-sent page */
    framebuf[0] = 0x03 | T30_FINAL;	/* DCS is always final frame */
    framebuf[1] = fax1_dis | T30_DCS;	/* FCF */
    framebuf[2] = 0;			/* bits 1..8 */
    framebuf[3] = 0x02 |		/* bit 10: receiver operation */
                  dcs_btp->dcs_bits |	/* bits 11..14: signalling rate */
		  ((fax1_res&remote_cap.vr)<<6) | /* bit 15: fine mode */
		  0x00;			/* bit 16: 2D */
    framebuf[4] = 0x00 |		/* bit 17+18: 215 mm width */
    		  0x04 |		/* bit 19+20: B4 length */
		  0x00 |		/* bits 21-23: scan line time */
		  0x00;			/* bit 24: extend bit - final */

    /* calculate correct bit settings for scan line time (bits 21-23) */
    for( i=0;i<=7;i++ )
    {
	if ( fax1_st_table[i].ms_n == s_time )
		{ framebuf[4] |= i<<4; break; }
    }
    return fax1_send_frame( fd, T30_CAR_V21, framebuf, 5 );
}

/* parse incoming DCS frame
 * put communication parameters into global variables (dcs_btp, fax_par_d)
 */
/* TODO: error handling? "I don't understand this -> DCN" */
void fax1_parse_dcs _P1((frame), uch *frame)
{
    /* bit rate + modulation requested (bits 11..14) */
    dcs_btp = fax1_btable;
    while( dcs_btp->speed > 2400 &&
	   dcs_btp->dcs_bits != (frame[3] & 0x3c) ) dcs_btp++;

    fax_par_d.br = dcs_btp->speed/2400 - 1;

    /* fine resolution: bit 15 (98/196 lpi) */
    fax_par_d.vr = ( frame[3] & 0x40 )? 1: 0;

    /* 2D: bit 16 (1D/2D mod read) */
    fax_par_d.df = ( frame[3] & 0x80 )? 1: 0;

    /* page width: bits 17+18 (byte 4, bits 0+1) */
    fax_par_d.wd = frame[4] & 0x03;

    /* page length: bits 19+20 */
    fax_par_d.ln = (frame[4] >> 2 ) & 0x03;

    /* scan line time: bits 21-23 */
    fax_par_d.st = fax1_st_table[ (frame[4] >> 4) & 0x07 ].st;

    /* extend bit? */
    if ( ( frame[4] && 0x80 ) == 0 ) goto done;

    /* ECM - TODO */
    
done:
    lprintf( L_NOISE, "DCS: speed=%d, flag=%03x", 
	     dcs_btp->speed, dcs_btp->flag );
}

int fax1_init_FRM _P2((fd,carrier), int fd, int carrier )
{
    char cmd[20];
    char *line;
    int timeout = 30;

    /* TODO: is this sufficient timeout handling? */
    if ( timeout > 0 )
    {
	signal( SIGALRM, fax1_sig_alarm );
        alarm( (timeout/10)+1 );
    }

    sprintf( cmd, "AT+FRM=%d", carrier );
    if ( fax_send( cmd, fd ) )
		{ alarm(0); return ERROR; }

    /* wait for CONNECT/NO CARRIER */
    line = mdm_get_line( fd );
    if ( line != NULL && strcmp( line, cmd ) == 0 )
		{ line = mdm_get_line( fd ); }		/* skip echo */

    alarm(0);
    if ( line == NULL || strcmp( line, "CONNECT" ) != 0 )
    {
	lprintf( L_WARN, "fax1_init_FRM: no carrier (%s)", line );
	return ERROR;
    }
    return NOERROR;
}

#endif /* CLASS1 */ 
