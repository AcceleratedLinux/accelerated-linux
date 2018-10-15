#ident "$Id: class1.c,v 4.20 2009/03/19 15:51:17 gert Exp $ Copyright (c) Gert Doering"

/* class1.c
 *
 * High-level functions to handle class 1 fax -- 
 * state machines for fax phase A, B, C, D. Error recovery.
 *
 * Uses library functions in class1lib.c, faxlib.c and modem.c
 *
 * $Log: class1.c,v $
 * Revision 4.20  2009/03/19 15:51:17  gert
 * bugfix: new NSF sending code was introducing extra "AT+FTH=3" commands
 * (due to confusion about internal requirements of fax1_send_frame())
 *
 * Revision 4.19  2009/03/19 15:33:59  gert
 * send NSF frames on reception (if modem quirk 0x40, for now)
 * handle different modem behaviour regarding AT+FTS=8;+FTH=3 gracefully
 *
 * Revision 4.18  2006/10/27 09:07:12  gert
 * add file name of page currently sent to log message
 *
 * Revision 4.17  2006/10/25 10:55:01  gert
 * class 1 receive: log current try #, send DIS only if CSI went without error
 *
 * Revision 4.16  2006/09/29 19:30:26  gert
 * properly calculate scan line time -> padding bytes
 * implement FTT -> retraining, with baud rate reduction
 *
 * Revision 4.15  2006/09/26 15:36:07  gert
 * - handle modems that don't get AT+FTS=8;+FTM=nn right
 *   (re-do AT+FTM if the first command yields "OK")
 * - switch on and off Xon/Xoff flow control before/after sending page data
 *
 * Revision 4.14  2006/09/25 22:26:54  gert
 * fax1_send_page(): move all the G3 file handling to g3file.c functions
 *  (-> cleanup code, build common infrastructure for class 1 + class 2)
 *
 * Revision 4.13  2006/03/22 14:13:12  gert
 * when sending, hand over received NSF to NSF decoder (faxlib.c/hyla_nsf.c)
 *
 * Revision 4.12  2006/03/07 21:31:50  gert
 * class 1 sending implementation:
 *   - handle end-of-page (return to phase B or phase C, or send DCN/hangup)
 *   - move sending of TSI and DCS inside fax1_send_page(), to be able to
 *     handle RTN/RTP transparently - somewhat sloppy "phase B" definition,
 *     but much cleaner this way
 *   - fix reading of G3 files - skip digifax header, also send last chunk
 *   - fix bit swapping (fax_send_swaptable)
 * -> class 1 sending now works, if the receiver doesn't need EOL padding
 *
 * Revision 4.11  2006/03/07 14:16:56  gert
 * fax1_dial_and_phase_AB(): add torture test code, refusing incoming CSI/DIS
 * 2 times before going on (mainly for testing receiver robustness)
 * fax1_send_page(): add log message to point out unimplemented stuff
 *
 * Revision 4.10  2006/01/04 21:07:12  gert
 * remove "speed" argument from fax1_send_dcs() (use fax1_max global)
 *
 * Revision 4.9  2006/01/03 09:12:20  gert
 * initialize "tries" in fax1_receive_page()
 *
 * Revision 4.8  2006/01/01 17:07:43  gert
 * change all fax1_send_dcn() to already set appropriate hangup code
 * add prototypes for local functions
 * add timeout handling and re-try logic to fax1_highlevel_receive(),
 *     fax1_receive_tcf() and fax1_receive_page()
 * add handling of incoming DCNs (give up)
 * add calculation of "how long should TCF be?" + checking to fax1_receive_tcf()
 *
 * Revision 4.7  2005/12/31 17:47:10  gert
 * use fax1_send_dis() instead of doing it here
 *
 * Revision 4.6  2005/12/31 15:52:08  gert
 * more comments
 * fax 1 receiver:
 *   * use global variable fax1_receive_have_connect to
 *     communicate answering status ("have we seen CONNECT?") from mgetty.c
 *   * prepare full T.30 receive state machine, including EOM->phase B
 *     and RTP/RTN->TCF (not complete)
 *   * set fax_hangup/fax_hangup_code correctly upon some error situations
 *
 * Revision 4.5  2005/12/30 23:05:05  gert
 * rework fax1_send_frame(): leading 0xff is now implicit
 *   (symmetric to fax1_receive_frame())
 *   change all callers to use new "frame" layout without 0xff
 * change lots of occurences of "3" to use "T30_CAR_V21"
 * rename fax1_receive() to fax1_highlevel_receive()
 * use "fd" and not "STDIN" everywhere
 * hand DCS frame to class1lib/fax1_parse_dcs()
 *   -> use proper carrier values for TCF and page reception (partially untested)
 * change "simple" frame sending to use fax1_send_simf_final()
 * add more comments about individual fax phases
 *
 * Revision 4.4  2005/12/28 21:53:08  gert
 * fix some compiler warnings and typos
 * adapt to changed fax1_send_idframe()
 * add CVS header
 * add (very rough and incomplete) fax class 1 implementation, consisting
 *  - fax1_receive() (to be called from faxrec.c)
 *  - fax1_receive_tcf() (handle TCF training frame + responses)
 *  - fax1_receive_page() (setup page reception, hand to fax_get_page_data())
 *
 */

#ifdef CLASS1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include "mgetty.h"
#include "fax_lib.h"
#include "tio.h"
#include "class1.h"
#include "policy.h"

enum T30_phases { Phase_A, Phase_B, Phase_C, Phase_D, Phase_E } fax1_phase;

/* maping of "st" codes to actual time (for normal res.) */
static int fax_scan_times[8] = { 0, 5, 10, 10, 20, 20, 40, 40 };

int fax1_dial_and_phase_AB _P2( (dial_cmd,fd),  char * dial_cmd, int fd )
{
char * p;			/* modem response */
uch framebuf[FRAMESIZE];
int first;
int len;
#ifdef TORTURE_TEST
int t_tries=0;
#endif

    /* send dial command */
    if ( fax_send( dial_cmd, fd ) == ERROR )
    {
	fax_hangup = TRUE; fax_hangup_code = FHUP_ERROR;
	return ERROR;
    }

    /* wait for ERROR/NO CARRIER/CONNECT */
    signal( SIGALRM, fax1_sig_alarm );
    alarm(FAX_RESPONSE_TIMEOUT);

    while( !fax_hangup )
    {
        p = mdm_get_line ( fd );

	if ( p == NULL )
	    { lprintf( L_ERROR, "fax1_dial: hard error dialing out" );
	      fax_hangup = TRUE; fax_hangup_code = FHUP_ERROR; break; }

	lprintf( L_NOISE, "fax1_dial: string '%s'", p );

	if ( strcmp( p, "ERROR" ) == 0 ||
	     strcmp( p, "NO CARRIER" ) == 0 )
	    { fax_hangup = TRUE; fax_hangup_code = FHUP_ERROR; break; }

	if ( strcmp( p, "NO DIALTONE" ) == 0 ||
	     strcmp( p, "NO DIAL TONE" ) == 0 )
	    { fax_hangup = TRUE; fax_hangup_code = FHUP_NODIAL; break; }

	if ( strcmp( p, "BUSY" ) == 0 )
	    { fax_hangup = TRUE; fax_hangup_code = FHUP_BUSY; break; }

        if ( strcmp( p, "CONNECT" ) == 0 )		/* gotcha! */
	    { break; }
    }

    alarm(0);
    if ( fax_hangup ) return ERROR;

    /* now start fax negotiation (receive CSI, DIS, send DCS)
     * read all incoming frames until FINAL bit is set
     */
    first=TRUE;
again:
    do
    {
	if ( (len = fax1_receive_frame( fd, first? 0:3, 30, framebuf ) )
	       == ERROR )
	{
	    /*!!!! try 3 times! (flow diagram from T.30 / T30_T1 timeout) */
	    fax_hangup = TRUE; fax_hangup_code = 11; return ERROR;
	}
	switch ( framebuf[1] )		/* FCF */
	{
	    case T30_CSI: fax1_copy_id( framebuf ); break;
	    case T30_NSF: fax1_incoming_nsf( framebuf+2, len-2 ); break;
	    case T30_DIS: fax1_parse_dis( framebuf ); break;
	    case T30_DCN: fax1_send_dcn( fd, 20 ); return ERROR;
	    default:
	        lprintf( L_WARN, "unexpected frame type 0x%02x", framebuf[1] );
	}
	first=FALSE;
    }
    while( ( framebuf[0] & T30_FINAL ) == 0 );

#ifdef TORTURE_TEST
    while( ++t_tries < 3 )
	{ mdm_command( "AT+FRS=200", fd ); goto again; }
#endif


    fax1_phase = Phase_B;			/* Phase A done */

    return NOERROR;
}


/* fax1_send_page
 *
 * send a page of G3 data
 * - if phase is "B", include sending of DCS, TCF and possibly 
 *   baud rate stepdown and repeated transmission of DCS.
 * - if phase is "C", directly send page data
 */

int fax1_send_page _P5( (g3_file, bytes_sent, tio, ppm, fd),
		        char * g3_file, int * bytes_sent, TIO * tio,
		        Post_page_messages ppm, int fd )
{
uch framebuf[FRAMESIZE];
char * line;
char cmd[40];
char dleetx[] = { DLE, ETX };
char rtc[] = { 0x00, 0x08, 0x80, 0x00, 0x08, 0x80, 0x00, 0x08 };
int pad_bytes, s_time;

    /* if we're in T.30 phase B, send DCS + training frame (TCF) now...
     * don't forget delay (75ms +/- 20ms)!
     *
     * NOTE: this is not strictly "phase B" - in T.30, phase B begins 
     * after sending the DCS, and before sending TCF.  But since RTN/RTP
     * return to sending DCS (bullet "D" in T.30 chart 5-2a), grouping it 
     * this way is much more logical to implement
     */

    /* calculate scan line time
     * in fine mode, some of the values get divided by 2
     */
    s_time = fax_scan_times[ remote_cap.st & 0x7 ];
    if ( remote_cap.vr == 1 && 
	(remote_cap.st & 0x1) == 0 ) s_time /= 2;

    if ( fax1_phase == Phase_B )
    {
        char train[150];
	int i, num;
	int tries=0;

retrain:			/* "(D)" in T.30/Figure 5-2a */

	/* send local id frame (TSI) */
	fax1_send_idframe( fd, T30_TSI|0x01, T30_CAR_V21 );

	/* send DCS */
	if ( fax1_send_dcs( fd, s_time ) == ERROR )
	{
	    fax_hangup = TRUE; fax_hangup_code = 10; return ERROR;
	}

	sprintf( cmd, "AT+FTS=8;+FTM=%d", dcs_btp->c_long );
	fax_send( cmd, fd );

	line = mdm_get_line( fd );
	if ( line != NULL && strcmp( line, cmd ) == 0 )
		line = mdm_get_line( fd );

	if ( strcmp( line, "OK" ) == 0 )
	{
	    lprintf( L_MESG, "fax1_send_page: unexpected OK, re-do AT+FTM" );

	    sprintf( cmd, "AT+FTM=%d", dcs_btp->c_long );
	    fax_send( cmd, fd );

	    line = mdm_get_line( fd );
	    if ( line != NULL && strcmp( line, cmd ) == 0 )
		line = mdm_get_line( fd );
	}

	if ( line == NULL || strcmp( line, "CONNECT" ) != 0 )
	{
	    lprintf( L_ERROR, "fax1_send_page: unexpected response 1: '%s'", line );
	    fax_hangup = TRUE; fax_hangup_code = 20; return ERROR;
	}

	/* send data for training (1.5s worth) */

	num = (dcs_btp->speed/8)*1.5;
	lprintf( L_NOISE, "fax1_send_page: send %d bytes training (TCF)", num );
	memset( train, 0, sizeof(train));

	for( i=0; i<num; i+=sizeof(train))
		write( fd, train, sizeof(train) );
	write( fd, dleetx, 2 );

	line = mdm_get_line( fd );
	if ( line == NULL || strcmp( line, "OK" ) != 0 )
	{
	    lprintf( L_ERROR, "fax1_send_page: unexpected response 2: '%s'", line );
	    fax_hangup = TRUE; fax_hangup_code = 20; return ERROR;
	}

	/* receive frame - FTT or CFR */
	/*!!! return code! TODO: retry logic - T.30 fig. 5-2a/p.16 */
	fax1_receive_frame( fd, 3, 30, framebuf );

	if ( ( framebuf[0] & T30_FINAL ) == 0 ||
	     framebuf[1] != T30_CFR )
	{
	    tries++;
	    lprintf( L_WARN, "fax1_send_frame: failed to train (%d)", tries);

	    if ( tries > 1 ) fax1_reduce_max();
	    if ( tries < 3 ) goto retrain;

	    /* give up */
	    fax1_send_dcn( fd, 27 ); return ERROR;
	}

	/* phase B done, go to phase C */
	fax1_phase = Phase_C;
    }

    if ( fax1_phase != Phase_C )
    {
        lprintf( L_ERROR, "fax1_send_page: internal error: not Phase C" );
	fax_hangup = TRUE; fax_hangup_code = FHUP_ERROR;
	return ERROR;
    }

    /* open G3 file, read first chunk, potentially skipping digifax header
     */
    if ( g3_open_read( g3_file ) < 0 )
    {
	/*!!! do something smart here...? */
	fax1_send_dcn( fd, FHUP_ERROR );
	return ERROR;
    }


    /* number of bytes to pad depend on scan time */
    pad_bytes = ((dcs_btp->speed/8) * s_time + 999) / 1000;
    lprintf( L_MESG, "scan line time: %d ms -> %d bytes/line at %d bps",
		s_time, pad_bytes, dcs_btp->speed );

    /* Phase C: send page data with high-speed carrier
     */
    sprintf( cmd, "AT+FTM=%d", dcs_btp->c_short );
    fax_send( cmd, fd );

    line = mdm_get_line( fd );
    if ( line != NULL && strcmp( line, cmd ) == 0 )
	    line = mdm_get_line( fd );

    if ( line == NULL || strcmp( line, "CONNECT" ) != 0 )
    {
	lprintf( L_ERROR, "fax1_send_page: unexpected response 3: '%s'", line );
	fax_hangup = TRUE; fax_hangup_code = 40; return ERROR;
    }

    lprintf( L_NOISE, "send page data (\"%s\")...", g3_file );

    /* turn on xon/xoff flow control now, for page data sending
     */
    if ( (FAXSEND_FLOW) & FLOW_SOFT )
    {
	tio_set_flow_control( fd, tio, (FAXSEND_FLOW) & (FLOW_HARD|FLOW_XON_OUT));
	tio_set( fd, tio );
    }

    /* read page data from file, invert byte order, 
     * insert padding bits (if scan line time > 0), 
     * at end-of-file, add RTC
     */
    if ( g3_send_file( g3_rf_chunk, fd, TRUE, TRUE, pad_bytes, 0 /*TODO!*/) < 0 )
    {
	lprintf( L_ERROR, "error in g3_send_file()" ); 
	return ERROR;
    }

    /*!!! ERROR HANDLING!! */
    /*!!! PARANOIA: alarm()!! */
    /* end of page: RTC */
    write( fd, rtc, sizeof(rtc) );

    /* end of data: DLE ETX */
    write( fd, dleetx, 2 );

    line = mdm_get_line( fd );
    if ( line == NULL || strcmp( line, "OK" ) != 0 )
    {
	lprintf( L_ERROR, "fax1_send_page: unexpected response 3a: '%s'", line );
	fax_hangup = TRUE; fax_hangup_code = 40; return ERROR;
    }

    /* turn off xon/xoff flow control (will interfere with received frames) */
    if ( (FAXSEND_FLOW) & FLOW_SOFT )
    {
	tio_set_flow_control( fd, tio, (FAXSEND_FLOW) & FLOW_HARD );
	tio_set( fd, tio );
    }

    /* now send end-of-page frame (MPS/EOM/EOP) and get pps */

    fax1_phase = Phase_D;
    lprintf( L_MESG, "page data sent, sending end-of-page frame (C->D)" );
    sprintf( cmd, "AT+FTS=8;+FTH=3" );
    fax_send( cmd, fd );

    line = mdm_get_line( fd );
    if ( line != NULL && strcmp( line, cmd ) == 0 )
	    line = mdm_get_line( fd );

    if ( line == NULL || strcmp( line, "CONNECT" ) != 0 )
    {
        if ( strcmp( line, "OK" ) == 0 ) goto tryanyway;
	lprintf( L_ERROR, "fax1_send_page: unexpected response 4: '%s'", line );
	fax_hangup = TRUE; fax_hangup_code = 50; return ERROR;
    }

    /* some modems seemingly can't handle AT+FTS=8;+FTH=3 (returning 
     * "OK" instead of "CONNECT"), so send AT+FTH=3 again for those.
     */
tryanyway:

    framebuf[0] = 0x03 | T30_FINAL;
    switch( ppm )
    {
        case pp_eom: framebuf[1] = T30_EOM | fax1_dis; break;
	case pp_eop: framebuf[1] = T30_EOP | fax1_dis; break;
	case pp_mps: framebuf[1] = T30_MPS | fax1_dis; break;
	default: 
	   lprintf( L_WARN, "fax1_send_page: canthappen(1) - PRI not supported" );
    }

    if ( strcmp( line, "OK" ) == 0 )	/* re-send AT+FTH=3 */
    {
        fax1_send_frame( fd, T30_CAR_V21, framebuf, 2 );
    }
    else				/* it got sent & acked */
    {
        fax1_send_frame( fd, T30_CAR_HAVE_V21, framebuf, 2 );
    }

    /* get MPS/RTP/RTN code */
    fax1_receive_frame( fd, T30_CAR_V21, 30, framebuf );

    /*!!! T.30 flow chart... */

    switch( framebuf[1] )
    {
        case T30_MCF:		/* page good */
		fax_page_tx_status = 1; fax1_phase = Phase_C; break;
	case T30_RTN:		/* retrain / negative */
		fax_page_tx_status = 2; fax1_phase = Phase_B; break;
	case T30_RTP:		/* retrain / positive */
		fax_page_tx_status = 3; fax1_phase = Phase_B; break;
	case T30_PIN:		/* procedure interrupt */
		fax_page_tx_status = 4; fax1_phase = Phase_B; break;
	case T30_PIP:
		fax_page_tx_status = 5; fax1_phase = Phase_C; break;
	default:
		lprintf( L_ERROR, "fax1_transmit_page: unexpected frame" );
		fax1_send_dcn(fd, 53); break;
    }

    /* if this was the last page, and the receiver is happy
     * (MCF or RTP), send DCN, and call it quits
     */
    if ( ppm == pp_eop && 
	 ( fax_page_tx_status == 1 || fax_page_tx_status == 3 || 
	   fax_page_tx_status == 5 ) )
    {
	fax1_send_dcn( fd, 0 ); 
	fax1_phase = Phase_E;
    }

    /* otherwise, nothing to do - caller will re-enter fax1_send_page(),
     * for next page / re-send of same page (we won't know)
     */
    return NOERROR;
}

boolean fax1_receive_have_connect = FALSE;

int fax1_receive_tcf _PROTO((int fd, int carrier, int wantbytes));
int fax1_receive_page _PROTO((int fd, int carrier, int * pagenum, 
			      char * dirlist, int uid, int gid, int mode ));

int fax1_highlevel_receive _P6( (fd, pagenum, dirlist, uid, gid, mode ),
			       int fd, int * pagenum, char * dirlist,
			       int uid, int gid, int mode)
{
int rc;
uch frame[FRAMESIZE];
char * p;
Post_page_messages ppm = pp_mps;
boolean first = TRUE;
int tries;
boolean have_dcs;

    /* after ATA/CONNECT, first AT+FTH=3 is implicit -> send frames
       right away (first=TRUE) - when coming back to phase B after EOM,
       AT+FTH=3 must be sent */

    /* with +FAE=1, the sequence seems to be
     *  RING
     *     ATA
     *  FAX
     *  CONNECT
     */

    if ( !fax1_receive_have_connect )
    {
	alarm(10);
	p = mdm_get_line( fd );
	alarm(0);

	if ( p == NULL || strcmp( p, "CONNECT" ) != 0 )
	{
	    lprintf( L_WARN, "fax1_receive: initial CONNECT not seen" );
	    fax_hangup = TRUE; fax_hangup_code = FHUP_TIMEOUT;
	    return ERROR;
	}
    }

    *pagenum = 0;

    /* phase B: Fax negotiations
     */
    tries=0; have_dcs=FALSE;

receive_phase_b:
    lprintf( L_MESG, "fax1 T.30 receive phase B (try %d)", tries+1 );

    /* send NSF frame (if requested) */
    if ( modem_quirks & MQ_SHOW_NSF )
    {
	rc = fax1_send_nsf( fd, first?T30_CAR_HAVE_V21: T30_CAR_V21 );
    }

    /* send local ID frame (CSI) - non-final */
    rc = fax1_send_idframe( fd, T30_CSI, first?T30_CAR_HAVE_V21: T30_CAR_V21 );
    first = FALSE;

    /* send DIS (but only if we haven't seen an error sending CSI) */
    if ( rc == NOERROR )
        rc = fax1_send_dis( fd );

    /* now see what the other side has to say... */
wait_for_dcs:

    do
    {
        if ( fax1_receive_frame( fd, T30_CAR_V21, 30, frame ) == ERROR )
	{
	    if ( ++tries < 3 ) goto receive_phase_b;
	    fax1_send_dcn( fd, 70 );
	    return ERROR;
	}
	switch( frame[1] & 0xfe )		/* FCF, ignoring X-bit */
	{
	    case T30_TSI: fax1_copy_id( frame ); break;
	    case T30_NSF: break;
	    case T30_DCS: fax1_parse_dcs( frame ); have_dcs=TRUE; break;
	    case T30_DCN: fax1_send_dcn( fd, 70 ); 
			  return ERROR; break;
	    default:
		lprintf( L_WARN, "unexpected frame type 0x%02x", frame[1] );
	}
    }
    while( ( frame[0] & T30_FINAL ) == 0 );

    if ( !have_dcs )
    {
	lprintf( L_WARN, "T.30 B: final frame, but no DCS seen, re-send DIS" );
	if ( ++tries < 3 ) goto receive_phase_b;
	fax1_send_dcn( fd, 70 );
	return ERROR;
    }

    /* fax1_parse_dcs() has setup dcs_btp and fax_par_d for us */

    /* receive 1.5s training sequence */
    rc = fax1_receive_tcf( fd, dcs_btp->c_long, (dcs_btp->speed/8)*1.5 );

    /* error receiving TCF ("no carrier seen") -> redo DIS/DCS */
    if ( rc < 0 )
	goto receive_phase_b;

    /* TCF bad? send FTT (failure to train), wait for next DCS */
    if ( rc == 0 )
    {
	rc = fax1_send_simf_final( fd, T30_CAR_V21, T30_FTT );
	if ( ++tries < 10 ) goto wait_for_dcs;
	fax1_send_dcn( fd, 73 );
	return ERROR;
    }

    /* TCF good, send CFR frame (confirmation to receive) */
    rc = fax1_send_simf_final( fd, T30_CAR_V21, T30_CFR );

receive_next_page:

    /* phase C: start page reception & get page data 
     */
    lprintf( L_MESG, "fax1 T.30 receive phase C" );

    rc = fax1_receive_page( fd, dcs_btp->c_short, pagenum, 
			    dirlist, uid, gid, mode );

    /* if we have already hung up, not worth doing any retries etc.
     */
    if ( rc == ERROR && fax_hangup ) return ERROR;

    /* phase D: post-message status
     * switch back to low-speed carrier, get (PRI-)EOM/MPS/EOP code
     */

    tries=0;
receive_phase_d:
    lprintf( L_MESG, "fax1 T.30 receive phase D" );

    /* TODO: T.30 flow chart: will we ever hit non-final frames here?
     * what to do on error?
     */
    do
    {
        if ( fax1_receive_frame( fd, T30_CAR_V21, 30, frame ) == ERROR )
	{
	    /* retry post-page handshake 3 times, then give up */
	    if ( ++tries < 3 ) goto receive_phase_d;
	    fax1_send_dcn( fd, 100 );
	    return ERROR;
	}
	switch( frame[1] & 0xfe & ~T30_PRI )	/* FCF, ignoring X-bit */
	{
	    case T30_MPS: 
	        lprintf( L_MESG, "MPS: end of page, more to come" ); ppm = pp_mps;
	        break;
	    case T30_EOM: 
		lprintf( L_MESG, "EOM: back to phase B" ); ppm = pp_eom;
		break;
	    case T30_EOP: 
		lprintf( L_MESG, "EOP: end of transmission" ); 
		ppm = pp_eop;
		break;
	    case T30_DCN: 
		fax1_send_dcn( fd, 101 ); 
	        return ERROR; 
		break;
	    default:
		lprintf( L_WARN, "unexpected frame type 0x%02x", frame[1] );
	}
    }
    while( ( frame[0] & T30_FINAL ) == 0 );

    /* send back page good/bad return code (TODO: RTN/RTP codes) */
    /* TODO: for RTN/RTP, restart training sequence (TCF) (??) */
    rc = fax1_send_simf_final( fd, T30_CAR_V21, T30_MCF );

    /* go back to phase B (EOM), go to next page (MPS), done (EOP) */
    if ( ppm == pp_eom )
	{ goto receive_phase_b; }
    if ( ppm == pp_mps )
	{ goto receive_next_page; }

    /* EOP - get goodbye frame (DCN) from remote end, hang up */
    fax1_receive_frame( fd, T30_CAR_V21, 30, frame );

    if ( (frame[1] & 0xfe) != T30_DCN )
    {
	lprintf( L_WARN, "fax1_receive: unexpected frame 0x%02x 0x%02x after EOP", frame[0], frame[1] );
    }

    fax_hangup = TRUE;
    fax_hangup_code = 0;
    return NOERROR;
}

int fax1_receive_tcf _P3((fd,carrier,wantbytes), 
			  int fd, int carrier, int wantbytes)
{
int rc, count, notnull;
char c, *p;
boolean wasDLE=FALSE;
int tries;

    tries=0;

    lprintf( L_NOISE, "fax1_r_tcf: carrier=%d", carrier );

get_carrier:
    rc = fax1_init_FRM( fd, carrier );
    if ( rc == ERROR ) 
    {
	while( ++tries<3 ) goto get_carrier;
	return ERROR;
    }

    /* TODO: proper timeout settings as per T.30 (?) */
    alarm(5);
    count = notnull = 0;

    lprintf( L_JUNK, "fax1_r_tcf: got: " );
    while(1)
    {
	if ( mdm_read_byte( fd, &c ) != 1 ) 
	{
	    /* timeout? corrupted DLE/ETX?  let caller send FTT and retry */
	    lprintf( L_ERROR, "fax1_r_tcf: cannot read byte, return" );
	    return 0;
	}
	if ( c != 0 ) lputc( L_JUNK, c );

	if ( wasDLE ) 
	{
	    if ( c == ETX ) break;
	    wasDLE = 0;
	}
	if ( c == DLE ) { wasDLE = 1; continue; }

	count++;
	if ( c != 0 ) notnull++;
    }

    /* read post-frame "NO CARRIER" message */
    p = mdm_get_line( fd );
    alarm(0);
    if ( p == NULL || strcmp( p, "NO CARRIER" ) != 0 )
	lprintf( L_WARN, "unexpected post-TCF modem response: '%s'", p );

    if ( count < (wantbytes*3)/4 ||		/* not enough bytes */
	 notnull > count/10  )			/* or too many errors */
    {
	lprintf( L_NOISE, "TCF: want %d, got %d, %d non-null -> retry",
		 wantbytes, count, notnull );
	return 0;				/* try again */
    }

    lprintf( L_NOISE, "TCF: want %d, got %d, %d non-null -> OK", 
	     wantbytes, count, notnull );
    return 1;					/* acceptable, go ahead */
}

int fax1_receive_page _P7( (fd,carrier,pagenum,dirlist,uid,gid,mode), 
				int fd, int carrier,
				int * pagenum, char * dirlist,
				int uid, int gid, int mode )
{
int rc;
char *p;
int tries;

char directory[MAXPATH];

    fax_find_directory( dirlist, directory, sizeof(directory) );

    tries=0;

    lprintf( L_NOISE, "fax1_rp: carrier=%d", carrier );

get_carrier:
    /* TODO: proper timeout settings as per T.30 */
    alarm(10);
    rc = fax1_init_FRM( fd, carrier );
    alarm(0);

    if ( rc == ERROR ) 
    { 
	if ( ++tries < 3 ) goto get_carrier;
	fax1_send_dcn( fd, 90 );
	return ERROR;
    }

    /* now get page data (common function for class 1 and class 2)
     * note: fax_get_page_data() has its own alarm/timeout handling
     */
    rc = fax_get_page_data( fd, ++(*pagenum), directory, uid, gid, mode );

    /* read post-page "NO CARRIER" message */
    alarm(10);
    p = mdm_get_line( fd );
    alarm(0);
    if ( p == NULL || strcmp( p, "NO CARRIER" ) != 0 )
	lprintf( L_WARN, "unexpected post-page modem response: '%s'", p );

    if ( rc == ERROR )
	{ fax1_send_dcn( fd, 90 ); return ERROR; }

    /* TODO: copy quality checking */

    return NOERROR;
}

#endif /* CLASS 1 */
