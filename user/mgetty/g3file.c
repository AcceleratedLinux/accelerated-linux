#ident "$Id: g3file.c,v 1.2 2006/09/26 15:37:55 gert Exp $"

/* g3file.c
 *
 * High-level functions to handle fax G3 I/O
 *  - open disk file for reading (skipping digifax header)
 *  - open disk file for writing (create "magic" header)
 *  - transport G3 data from file to "out file descriptor" (file/device)
 *    + optionally adding EOL padding (sending)
 *    + optionally doing transcoding (1D/2D/...) (sending)  [unimp]
 *  - transport G3 data from "in file descriptor" to file
 *    + optionally removing EOL padding (receiving)         [unimp]
 *    + checking receive copy quality                       [unimp]
 *
 * $Log: g3file.c,v $
 * Revision 1.2  2006/09/26 15:37:55  gert
 * - on "device" file descriptors, check after each write() block whether there
 *   is any sort of input data available - Xon/Xoff, modem errors, ...
 * - use sizeof(wbuf) to validate pad_bytes, not CHUNK
 *
 * Revision 1.1  2006/09/25 22:21:02  gert
 * G3 file handling functions, first draft (file->fd, bit padding)
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <errno.h>

#include "mgetty.h"
#include "fax_lib.h"
#include "tio.h"
#include "class1.h"

static int g3_read_fd = -1;

static uch g3_rf_buf[64];
static int g3_rf_idx = 0;		/* read position in buffer */
static int g3_rf_num = 0;		/* number of bytes in buffer */

static int g3_rf_fine = -1;		/* resolution, -1 = unknown */

static uch bits_left[256];		/* 0-bits at left side of byte */
static uch bits_right[256];		/* 0-bits at right side of byte */
static int bits_lr_init = 0;

/* build left/right bits table
 * (how many consecutive zero-bits on the left/right side of a byte?)
 */
void g3_init_lr_bittable _P0(void)
{
signed int i,j;
    if ( bits_lr_init ) return;

    bits_left[0] = bits_right[0] = 8;

    for( i=0; i<=7; i++ )
    {
	for( j=(1<<(7-i))-1;j>=0; j--)
	{
	    /* 0000 1 (jjj),  1-bit is at 1<<(7-i) */
	    bits_left[ (1<<(7-i)) + j ] = i;
	    /* (jjj) 1 0000,  1-bit is at 1<<i, j at <<(i+1) */
	    bits_right[ ((j<<1)+1) << i ] = i;
	}
    }

    bits_lr_init = 1;
}

/* open G3 file for reading */
int g3_open_read _P1((filename), char * filename )
{

    /* make sure left/right bittables are initialized (for padding) */
    g3_init_lr_bittable(); 

    g3_read_fd = open( filename, O_RDONLY );

    if ( g3_read_fd < 0 )
    {
	lprintf( L_ERROR, "g3_open_read: can't open '%s'", filename );
	return -1;
    }

    g3_rf_fine = -1;
    g3_rf_idx = 0;
    g3_rf_num = read( g3_read_fd, g3_rf_buf, sizeof(g3_rf_buf) );

    if ( g3_rf_num < 0 )
    {
	lprintf( L_ERROR, "g3_open_read: can't read from '%s'", filename );
	close( g3_read_fd );
	return -1;
    }

    /* digifax file?  (G3 + 64 byte header) -> skip */
    if ( g3_rf_num >= 64 &&
	 strcmp( g3_rf_buf+1, "PC Research, Inc" ) == 0 )
    {
	lprintf( L_MESG, "skipping over DigiFax header" );
	g3_rf_idx = 64;

	/* for dfax files, remember resolution */
	g3_rf_fine = ( g3_rf_buf[29] != 0 );
    }

    return g3_read_fd;
}

/* close G3 file (if one is open), reset all static variables
 * called by g3_send_file(), g3_receive_file() at EOF or ERROR
 */
void g3_close _P0(void)
{
    if (g3_read_fd < 0 ) return;

    close(g3_read_fd);
    g3_read_fd = -1;
    g3_rf_idx = 0;
    g3_rf_num = 0;
    g3_rf_fine = -1;
}

/* fill "buf" with new data from file, max size = max
 */
int g3_rf_chunk _P2((buf, max), char * buf, int max )
{
int r, rp;

    rp = 0;

    /* stuff still left in buffer from open? */
    if ( g3_rf_idx < g3_rf_num )
    {
	r = g3_rf_num - g3_rf_idx;
	if ( r > max ) { r=max; }
	memcpy( buf, g3_rf_buf, g3_rf_num-g3_rf_idx );
	g3_rf_idx += r;

	if ( r == max ) return r;
	rp = r;
    }

    /* all the arithmetics with "rp" is there to be able to 
     * "look ahead" into the file in g3_open_read(), make sure the 
     * system has the first bytes read (-> timing), but still read
     * CHUNK aligned reads on all further reads...
     * (we could do it with a larger static buffer, but that costs
     *  memory all the time, not only when actively faxing)
     */

    r = read( g3_read_fd, buf+rp, max-rp );
    if ( r < 0 )
	{ lprintf( L_ERROR, "g3_read" ); }
    lprintf( L_JUNK, "rp=%d, r=%d", rp, r );
    return r+rp;
}

/* send G3 file data to output file descriptor 
 *   in_func = function that will re-fill buffer
 *   out_fd  = output file descriptor, file or device
 *   is_device = boolean, if true, check "out_fd" for modem responses
 *   pad_bytes = pad each scan line to the given (min.) number of bytes
 *   fax_res   = normal/fine resolution (for warnings/format conversion)
 *  [fax_comp  = compression mode, for transcoding]
 */

int g3_send_file _P6(( in_func, out_fd, is_device, 
		       escape_dle, pad_bytes, fax_res ),
		      in_func_t * in_func, int out_fd, int is_device, 
		      int escape_dle, int pad_bytes, int fax_res )
{
#define CHUNK 1024
uch rbuf[CHUNK], wbuf[CHUNK+2];

int r, w, r_num;
uch r_ch;
int bytes_cur;			/* bytes in current line (->padding) */
int have_0bits;			/* 0-bits seen so far */
int lines_seen=-1;		/* scan lines in file */

    if ( g3_rf_fine != -1 && fax_res != g3_rf_fine )
    {
	fprintf( stderr, "WARNING: sending in %s mode, fax data is %s mode\n",
		 fax_res?    "fine" : "normal",
		 g3_rf_fine? "fine" : "normal" );
	lprintf( L_WARN, "resolution mismatch" );
    }

    /* we're going to be lazy further down, and always assume 
     * "there is more space left in the buffer than we need padding",
     * so better make sure CHUNK is large enough
     */
    if ( pad_bytes > sizeof(wbuf)/2 )
    {
	errno=EINVAL;
	lprintf( L_ERROR, "g3_send_file: too much padding requested, pad_bytes=%d, CHUNK=%d", pad_bytes, CHUNK );
	fprintf( stderr, "g3_send_file: internal error (pad_bytes)\n" );
	return FAIL;
    }
    if ( pad_bytes < 0 ) pad_bytes = 0;		/* be foolproof */

    r_num = in_func( rbuf, sizeof(rbuf) );

    r=0;
    w=0;
    bytes_cur=pad_bytes;			/* never! pad first EOL */
    have_0bits=0;
    do
    {
	r_ch = (uch)rbuf[r++];
	bytes_cur++;

	/* do we have EOL? (11 consecutive 0-bits) */
#ifdef DEBUG_G3FILE
	fprintf( stderr, "h0b=%2d, r_ch=%03o -> l=%d, r=%d\n", have_0bits, 
			  r_ch, bits_left[r_ch], bits_right[r_ch] );
#endif

	if ( r_ch == 0 )
	    { have_0bits += 8; }
	else
	{
	    if ( have_0bits + bits_left[r_ch] >= 11 )
	    {
		int need = (bytes_cur<pad_bytes)? pad_bytes-bytes_cur: 0;
#ifdef DEBUG_G3FILE
		fprintf( stderr, "EOL! pad=%d\n", need );
#endif
		while( need>0 ) 
		    { wbuf[w++] = 0; need--; }
		bytes_cur=0;
		lines_seen++;
	    }
	    have_0bits = bits_right[r_ch];
	}


	wbuf[w] = fax_send_swaptable[ r_ch ];
	if ( escape_dle && wbuf[w] == DLE ) wbuf[++w] = DLE;
	w++;

        if ( r >= r_num )			/* buffer empty, read more */
	{
	    r_num = in_func( rbuf, sizeof(rbuf) );
	    if ( r_num < 0 )
		{ break; }
	    r = 0;
	}


	if ( ( w >= (sizeof(wbuf)-2 - pad_bytes ) ) || 	/* write buffer full */
	     (r_num == 0) )				/* or end of input */
	{
	    if ( w != write( out_fd, wbuf, w ) )
	    {
	        lprintf( L_ERROR, "fax1_send_page: can't write %d bytes", w );
		break;
	    }
	    lprintf( L_JUNK, "write %d", w );
	    w=0;

	    /* if this is going to a serial device, check for pending input 
             * (stray Xon/Xoff from flow control, modem errors, ...)
             */
	    if ( is_device && check_for_input( out_fd ) )
	    {
		lprintf( L_NOISE, "input: got " );
		do
		{
		    /* intentionally don't use mdm_read_byte here */
		    if ( read( out_fd, &r_ch, 1 ) != 1 )
		    {
			lprintf( L_ERROR, "read failed" );
			break;
		    }
		    else
			lputc( L_NOISE, r_ch );
		}
		while ( check_for_input( out_fd ) );
	    }

	}		/* end if (need to write buffer) */
    }
    while(r_num>0);

    g3_close();

    lprintf( L_NOISE, "end of page, %d lines sent", lines_seen );

    /*!!! ERROR HANDLING!! */
    /*!!! PARANOIA: alarm()!! */
    return SUCCESS;
}
