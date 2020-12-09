/*****************************************************************************
* Copyright 2020, Digi International Inc.
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*
*****************************************************************************/

#ifdef RFC2217

#include <syslog.h>
#include <termios.h>
#include <arpa/telnet.h>
#include <linux/serial.h>
#include "telnetd.h"

/*
 *  RFC 2217 Constants
 */

/*
 *  RFC 2217  CLIENT TO SERVER
 */
#define TN_SIGNATURE                   0
#define TN_SET_BAUDRATE                1
#define TN_SET_DATASIZE                2
#define TN_SET_PARITY                  3
#define TN_SET_STOPSIZE                4
#define TN_SET_CONTROL                 5
#define TN_NOTIFY_LINESTATE            6
#define TN_NOTIFY_MODEMSTATE           7
#define TN_FLOWCONTROL_SUSPEND         8
#define TN_FLOWCONTROL_RESUME          9
#define TN_SET_LINESTATE_MASK          10
#define TN_SET_MODEMSTATE_MASK         11
#define TN_PURGE_DATA                  12

/*
 *  RFC 2217  SERVER TO CLIENT
 */
#define TN_SIGNATURE_S                 100
#define TN_SET_BAUDRATE_S              101
#define TN_SET_DATASIZE_S              102
#define TN_SET_PARITY_S                103
#define TN_SET_STOPSIZE_S              104
#define TN_SET_CONTROL_S               105
#define TN_NOTIFY_LINESTATE_S          106
#define TN_NOTIFY_MODEMSTATE_S         107
#define TN_FLOWCONTROL_SUSPEND_S       108
#define TN_FLOWCONTROL_RESUME_S        109
#define TN_SET_LINESTATE_MASK_S        110
#define TN_SET_MODEMSTATE_MASK_S       111
#define TN_PURGE_DATA_S                112

/*
 *  RFC 2217  BAUD RATE
 */                     
#define TN_BAUD_REQUEST                0

/*
 *  RFC 2217  CHAR SIZE
 */
#define TN_DATASIZE_REQUEST            0
#define TN_DATASIZE_5                  5
#define TN_DATASIZE_6                  6
#define TN_DATASIZE_7                  7
#define TN_DATASIZE_8                  8

/*
 *  RFC 2217  PARITY
 */
#define TN_PARITY_REQUEST              0
#define TN_PARITY_NONE                 1
#define TN_PARITY_ODD                  2
#define TN_PARITY_EVEN                 3
#define TN_PARITY_MARK                 4
#define TN_PARITY_SPACE                5
        
/*
 *  RFC 2217  STOP BITS
 */
#define TN_STOPSIZE_REQUEST            0
#define TN_STOPSIZE_1                  1
#define TN_STOPSIZE_2                  2

/*
 *  RFC 2217  SET-CONTROL SB OPTIONS
 */
#define TN_SET_CONTROL_FLOW_REQUEST    0
#define TN_SET_CONTROL_FLOW_NONE       1
#define TN_SET_CONTROL_FLOW_XONXOFF    2
#define TN_SET_CONTROL_FLOW_HARDWARE   3
#define TN_SET_CONTROL_BREAK_REQUEST   4
#define TN_SET_CONTROL_BREAK_ON        5
#define TN_SET_CONTROL_BREAK_OFF       6
#define TN_SET_CONTROL_DTR_REQUEST     7
#define TN_SET_CONTROL_DTR_ON          8
#define TN_SET_CONTROL_DTR_OFF         9
#define TN_SET_CONTROL_RTS_REQUEST     10
#define TN_SET_CONTROL_RTS_ON          11
#define TN_SET_CONTROL_RTS_OFF         12
#define TN_SET_CONTROL_IFLOW_REQUEST   13
#define TN_SET_CONTROL_IFLOW_NONE      14
#define TN_SET_CONTROL_IFLOW_XONXOFF   15
#define TN_SET_CONTROL_IFLOW_HARDWARE  16
#define TN_SET_CONTROL_FLOW_DCD        17
#define TN_SET_CONTROL_FLOW_DTR        18
#define TN_SET_CONTROL_FLOW_DSR        19


#define TN_MODEMSTATE_DCD        0x80
#define TN_MODEMSTATE_RI         0x40
#define TN_MODEMSTATE_DSR        0x20
#define TN_MODEMSTATE_CTS        0x10
#define TN_MODEMSTATE_DCD_DELTA  0x08
#define TN_MODEMSTATE_RI_DELTA   0x04
#define TN_MODEMSTATE_DSR_DELTA  0x02
#define TN_MODEMSTATE_CTS_DELTA  0x01
#define TN_MODEMSTATE_ACTIVE_MASK (TN_MODEMSTATE_DCD | TN_MODEMSTATE_RI | TN_MODEMSTATE_DSR | TN_MODEMSTATE_CTS)
#define TN_MODEMSTATE_DELTA_MASK (TN_MODEMSTATE_DCD_DELTA | TN_MODEMSTATE_RI_DELTA | TN_MODEMSTATE_DSR_DELTA | TN_MODEMSTATE_CTS_DELTA)

#define TN_LINESTATE_TIMEOUT_ERROR 0x80
#define TN_LINESTATE_TRANSFER_SHIFT_REGISTER_EMPTY 0x40
#define TN_LINESTATE_TRANSFER_HOLDING_REGISTER_EMPTY 0x20
#define TN_LINESTATE_BREAK             0x10
#define TN_LINESTATE_FRAMING           0x08
#define TN_LINESTATE_PARITY            0x04
#define TN_LINESTATE_OVERRUN           0x02
#define TN_LINESTATE_DATAREADY         0x01

#define GET_32BIT(cp)	(((cp)[0] << 24) | ((cp)[1] << 16) | ((cp)[2] << 8) | (cp)[3])

#define assert(x)	if (!(x)) syslog(LOG_ERR, "ioctl error %m, line %d", __LINE__);

struct baud {
    unsigned int rate;
    unsigned int val;
};

static struct baud baudtable[] = {
    { 1200, B1200 },
    { 2400, B2400 },
    { 4800, B4800 },
    { 9600, B9600 },
    { 19200, B19200 },
    { 38400, B38400 },
    { 57600, B57600 },
    { 115200, B115200 },
    { 230400, B230400 },
    { 0, B0 }
};

static int serial_fd = -1;
static struct termios save_tios;

static unsigned char linestate_mask;   /* Line Mask */
static unsigned char linestate_last;   /* Last sent line state */

/* the linestate needs to be sent out */
static int update_linestate;

static unsigned char modemstate_mask;  /* Modem Mask */
static unsigned char modemstate_last;  /* Last Sent Modem State */

/* the modemstate needs to be sent out */
static int update_modemstate;

/* used to figure out linestate */
static struct serial_icounter_struct last_counter;


/*******************************************************************
 * telnet_IAC_put32()
 *******************************************************************/
static unsigned int telnet_IAC_put32(char *buffer, unsigned long value32)
{
    unsigned int buffer_index;
    unsigned int shift;
    unsigned int return_length;
    unsigned char value8;
    unsigned int i;

    /* assume normal return length of 4 */
    return_length = 4;

    shift = 24;

    buffer_index = 0;
    for (i=0; i < 4; i++)
    {
	value8 = (value32 >> shift);
	if (value8 == IAC)
	{
	    buffer[buffer_index] = value8;
	    buffer_index++;
	    return_length++;
	}
	buffer[buffer_index] = value8;
	buffer_index++;
	shift -= 8;
    }

    return return_length;
}

/*******************************************************************
 * telnet_IAC_put8()
 *******************************************************************/
static unsigned int telnet_IAC_put8(char *buffer, unsigned char value)
{
    if (value == IAC)
    {
	buffer[0] = IAC;
	buffer[1] = IAC;
	return 2;
    }
    else
    {
	buffer[0] = value;
	return 1;
    }
}

/*******************************************************************
 * com_port_option()
 *******************************************************************/
void com_port_option(unsigned char *option, int optlen)
{
    unsigned char suboption;
    unsigned int value_length;
    unsigned char *value;
    char response[32];
    char *resp = response;
    int retval;
    unsigned int baudrate;
    struct termios tios;
    int flush_flag;
    struct baud *b;
    int signals;

    if (serial_fd < 0 || optlen < 1 || his_state_is_wont(TELOPT_COMPORT))
	return;

    /* retrieve suboption */
    suboption = option[0];

    value = option + 1;
    value_length = optlen - 1;

    /* Make sure the value_length is within the maximum allowed value
     * length for any of the suboptions.  Also set the maximum response
     * length for each option. */
    switch (suboption)
    {
    case TN_SIGNATURE:
	break;

    case TN_SET_BAUDRATE:
	/* the TN_SET_BAUDRATE data value length should be 4 bytes */
	if (value_length != 4)
	{
	    /* invalid suboption */
	    return;
	}
	break;

    default:
	/* the data value length should be 1 byte */
	if (value_length != 1)
	{
	    /* invalid suboption */
	    return;
	}
	break;
    }

    /* get the termios structure */
    retval = tcgetattr(serial_fd, &tios);
    assert(retval >= 0);

    /* setup first part of response */
    *resp++ = IAC;
    *resp++ = SB;
    *resp++ = TELOPT_COMPORT;
    *resp++ = suboption + 100; // server to client

    /* process each suboption */
    switch (suboption)
    {
    case TN_SIGNATURE:
	/* the other side is sending us their signature and we don't
	 * really care about it and so we just throw it away. */

	/* TODO: should we have a better signature? */
	strcpy(resp, "DIGI");
	resp += 4;
	break;

    case TN_SET_BAUDRATE:

	baudrate = GET_32BIT(value);

	if (baudrate != TN_BAUD_REQUEST)
	{
	    /* set baud rate */
	    for (b = baudtable; b->rate; b++) {
		if (b->rate == baudrate) {
		    retval = cfsetspeed(&tios, b->val);
		    assert(retval >= 0);
		    retval = tcsetattr(serial_fd, TCSANOW, &tios);
		    assert(retval >= 0);
		    retval = tcgetattr(serial_fd, &tios);
		    assert(retval >= 0);
		    break;
		}
	    }
	}

	/* get baud rate */
	baudrate = cfgetispeed(&tios);

	for (b = baudtable; b->rate; b++)
	    if (b->val == baudrate)
		break;

	resp += telnet_IAC_put32(resp, b->rate);
	break;

    case TN_SET_DATASIZE:

	if (value[0] != TN_DATASIZE_REQUEST)
	{
	    /* clear datasize bits */
	    tios.c_cflag &= ~CSIZE;

	    /* set datasize */
	    switch (value[0])
	    {
	    case TN_DATASIZE_5:
		tios.c_cflag |= CS5;
		break;
	    case TN_DATASIZE_6:
		tios.c_cflag |= CS6;
		break;
	    case TN_DATASIZE_7:
		tios.c_cflag |= CS7;
		break;
	    case TN_DATASIZE_8:
		tios.c_cflag |= CS8;
		break;
	    default:
		/* unsupported option */
		break;
	    }

	    /* set data size */
	    retval = tcsetattr(serial_fd, TCSANOW, &tios);
	    assert(retval >= 0);
	    retval = tcgetattr(serial_fd, &tios);
	    assert(retval >= 0);
	}

	/* return data size */
	switch (tios.c_cflag & CSIZE)
	{
	case CS5:
	    *resp++ = TN_DATASIZE_5;
	    break;

	case CS6:
	    *resp++ = TN_DATASIZE_6;
	    break;

	case CS7:
	    *resp++ = TN_DATASIZE_7;
	    break;

	case CS8:
	default:
	    *resp++ = TN_DATASIZE_8;
	    break;
	}
	
	break;

    case TN_SET_PARITY:

	if (value[0] != TN_PARITY_REQUEST)
	{
	    /* clear parity bits */
	    tios.c_cflag &= ~(PARENB | PARODD | CMSPAR);

	    switch (value[0])
	    {
	    case TN_PARITY_NONE:
		break;
	    case TN_PARITY_ODD:
		tios.c_cflag |= PARODD | PARENB;
		break;
	    case TN_PARITY_EVEN:
		tios.c_cflag |= PARENB;
		break;
	    case TN_PARITY_MARK:
		tios.c_cflag |= PARODD | PARENB | CMSPAR;
		break;
	    case TN_PARITY_SPACE:
		tios.c_cflag |= PARENB | CMSPAR;
		break;
	    default:
		/* unsupported option */
		break;
	    }

	    /* set parity */
	    retval = tcsetattr(serial_fd, TCSANOW, &tios);
	    assert(retval >= 0);
	    retval = tcgetattr(serial_fd, &tios);
	    assert(retval >= 0);
	}

	/* return parity */
	if (tios.c_cflag & PARENB)
	{
	    if (tios.c_cflag & CMSPAR)
	    {
		if (tios.c_cflag & PARODD)
		{
		    /* mark parity */
		    *resp++ = TN_PARITY_MARK;
		}
		else
		{
		    /* space parity */
		    *resp++ = TN_PARITY_SPACE;
		}
	    }
	    else
	    {
		if (tios.c_cflag & PARODD)
		{
		    /* odd parity */
		    *resp++ = TN_PARITY_ODD;
		}
		else
		{
		    /* even parity */
		    *resp++ = TN_PARITY_EVEN;
		}
	    }
	}
	else
	{
	    /* no parity */
	    *resp++ = TN_PARITY_NONE;
	}

	break;

    case TN_SET_STOPSIZE:

	if (value[0] != TN_STOPSIZE_REQUEST)
	{
	    switch (value[0])
	    {
	    case TN_STOPSIZE_1:
		tios.c_cflag &= ~CSTOPB;
		break;

	    case TN_STOPSIZE_2:
		tios.c_cflag |= CSTOPB;
		break;

	    default:
		/* unexpected option */
		break;
	    }

	    /* set stopsize */
	    retval = tcsetattr(serial_fd, TCSANOW, &tios);
	    assert(retval >= 0);
	    retval = tcgetattr(serial_fd, &tios);
	    assert(retval >= 0);
	}

	/* return stopsize */
	if (tios.c_cflag & CSTOPB)
	{
	    *resp++ = TN_STOPSIZE_2;
	}
	else
	{
	    *resp++ = TN_STOPSIZE_1;
	}

	break;

    case TN_SET_CONTROL:

	switch (value[0])
	{
	case TN_SET_CONTROL_FLOW_REQUEST:
	case TN_SET_CONTROL_FLOW_NONE:
	case TN_SET_CONTROL_FLOW_XONXOFF:
	case TN_SET_CONTROL_FLOW_HARDWARE:

	    if (value[0] != TN_SET_CONTROL_FLOW_REQUEST)
	    {
		switch (value[0])
		{
		case TN_SET_CONTROL_FLOW_NONE:
		    /* clear all flow control flags */
		    tios.c_iflag &= ~(IXON | IXOFF | IXANY);
		    tios.c_cflag &= ~CRTSCTS;
		    break;

		case TN_SET_CONTROL_FLOW_XONXOFF:
		    /* clear hardware flow control flags */
		    tios.c_cflag &= ~CRTSCTS;

		    /* set xon/xoff software flow control */
		    tios.c_iflag |= (IXON | IXOFF);
		    break;

		case TN_SET_CONTROL_FLOW_HARDWARE:
		    /* clear software flow control flags */
		    tios.c_iflag &= ~(IXON | IXOFF);

		    /* set CTS/RTS hardware flow control */
		    tios.c_cflag |= CRTSCTS;
		    break;
		}

		/* set flow control */
		retval = tcsetattr(serial_fd, TCSANOW, &tios);
		assert(retval >= 0);
		retval = tcgetattr(serial_fd, &tios);
		assert(retval >= 0);
	    }

	    if (tios.c_cflag & CRTSCTS)
	    {
		*resp++ = TN_SET_CONTROL_FLOW_HARDWARE;		
	    }
	    else if (tios.c_iflag & (IXON | IXOFF))
	    {
		*resp++ = TN_SET_CONTROL_FLOW_XONXOFF;
	    }
	    else
	    {
		*resp++ = TN_SET_CONTROL_FLOW_NONE;
	    }

	    break;

	case TN_SET_CONTROL_BREAK_REQUEST:
	case TN_SET_CONTROL_BREAK_OFF:
	    *resp++ = TN_SET_CONTROL_BREAK_OFF;
	    break;

	case TN_SET_CONTROL_BREAK_ON:

	    retval = tcsendbreak(serial_fd, 0);
	    if (retval >= 0)
	    {
		*resp++ = TN_SET_CONTROL_BREAK_ON;
	    }
	    else
	    {
		*resp++ = TN_SET_CONTROL_BREAK_OFF;
	    }

	    break;

	case TN_SET_CONTROL_DTR_REQUEST:
	case TN_SET_CONTROL_DTR_ON:
	case TN_SET_CONTROL_DTR_OFF:

	    /* change DTR signal */
	    signals = TIOCM_DTR;
	    switch (value[0])
	    {
	    case TN_SET_CONTROL_DTR_ON:
		retval = ioctl(serial_fd, TIOCMBIS, &signals);
		assert(retval >= 0);
		break;
		    
	    case TN_SET_CONTROL_DTR_OFF:
		retval = ioctl(serial_fd, TIOCMBIC, &signals);
		assert(retval >= 0);
		break;
	    }

	    /* get modem signals */
	    retval = ioctl(serial_fd, TIOCMGET, &signals);
	    assert(retval >= 0);
	    if (signals & TIOCM_DTR)
	    {
		*resp++ = TN_SET_CONTROL_DTR_ON;
	    }
	    else
	    {
		*resp++ = TN_SET_CONTROL_DTR_OFF;
	    }

	    break;

	case TN_SET_CONTROL_RTS_REQUEST:
	case TN_SET_CONTROL_RTS_ON:	       
	case TN_SET_CONTROL_RTS_OFF:

	    /* change RTS signal */
	    signals = TIOCM_RTS;
	    switch (value[0])
	    {
	    case TN_SET_CONTROL_RTS_ON:
		retval = ioctl(serial_fd, TIOCMBIS, &signals);
		break;
		    
	    case TN_SET_CONTROL_RTS_OFF:
		retval = ioctl(serial_fd, TIOCMBIC, &signals);
		break;
	    }

	    /* get modem signals */
	    retval = ioctl(serial_fd, TIOCMGET, &signals);
	    assert(retval >= 0);
	    if (signals & TIOCM_RTS)
	    {
		*resp++ = TN_SET_CONTROL_RTS_ON;
	    }
	    else
	    {
		*resp++ = TN_SET_CONTROL_RTS_OFF;
	    }

	    break;

	case TN_SET_CONTROL_IFLOW_REQUEST:
	case TN_SET_CONTROL_IFLOW_NONE:
	case TN_SET_CONTROL_IFLOW_HARDWARE:
	case TN_SET_CONTROL_IFLOW_XONXOFF:

	    if (value[0] != TN_SET_CONTROL_IFLOW_REQUEST)
	    {
		switch (value[0])
		{
		case TN_SET_CONTROL_IFLOW_NONE:
		    /* clear inbound flow control flags */
		    tios.c_iflag &= ~(IXOFF);
		    tios.c_cflag &= ~(CRTSCTS);
		    break;

		case TN_SET_CONTROL_IFLOW_XONXOFF:
		    /* clear inbound software flow */
		    tios.c_iflag |= (IXOFF);
		    break;

		case TN_SET_CONTROL_IFLOW_HARDWARE:
		    /* set inbound hardware flow */
		    tios.c_cflag |= (CRTSCTS);
		    break;
		}

		/* set input flow control */
		retval = tcsetattr(serial_fd, TCSANOW, &tios);
		assert(retval >= 0);
		retval = tcgetattr(serial_fd, &tios);
		assert(retval >= 0);
	    }

	    if (tios.c_cflag & CRTSCTS)
	    {
		*resp++ = TN_SET_CONTROL_IFLOW_HARDWARE;		
	    }
	    else if (tios.c_iflag & IXOFF)
	    {
		*resp++ = TN_SET_CONTROL_IFLOW_XONXOFF;
	    }
	    else
	    {
		*resp++ = TN_SET_CONTROL_IFLOW_NONE;
	    }
	
	    break;

	default:
	    /* unsupported option */
	    break;
	}
	  
	break;

    case TN_SET_LINESTATE_MASK:
	if (linestate_mask != value[0])
	{
	    /* indicate that the linestate needs a refresh */
	    update_linestate = 1;
	}
	linestate_mask = value[0];
	resp += telnet_IAC_put8(resp, value[0]);
	break;

    case TN_SET_MODEMSTATE_MASK:
	if (modemstate_mask != value[0])
	{
	    /* indicate that the modemstate needs a refresh */
	    update_modemstate = 1;
	}
	modemstate_mask = value[0];
	resp += telnet_IAC_put8(resp, value[0]);
	break;

    case TN_PURGE_DATA:
	switch (value[0])
	{
	case 1:
	    flush_flag = TCIFLUSH;
	    break;

	case 2:
	    flush_flag = TCOFLUSH;
	    break;

	case 3:
	    flush_flag = TCIOFLUSH;
	    break;

	default:
	    /* unsupported option */
	    flush_flag = 0;
	    break;
	}	

	retval = tcflush(serial_fd, flush_flag);
	assert(retval >= 0);

	*resp++ = value[0];
	break;

    default:
	/* unrecognized suboption */
	return;
    }

    /* fill in the end of the response */
    *resp++ = IAC;
    *resp++ = SE;

    /* send response */
    memcpy(nfrontp, response, resp - response);
    nfrontp += resp - response;
}

/**************************************************************
 * telnet_get_rfc2217_linestate()
 **************************************************************/
static unsigned char telnet_get_rfc2217_linestate(void)
{
    unsigned char new_linestate;
    struct serial_icounter_struct current_counter;
    int retval;

    retval = ioctl(serial_fd, TIOCGICOUNT, &current_counter);
    assert(retval >= 0);

    new_linestate = 0;

    if ((current_counter.overrun != last_counter.overrun) ||
	(current_counter.buf_overrun != last_counter.buf_overrun))
    {
        new_linestate |= TN_LINESTATE_OVERRUN;
    }
    if (current_counter.parity != last_counter.parity)
    {
        new_linestate |= TN_LINESTATE_PARITY;
    }
    if (current_counter.frame != last_counter.frame)
    {
        new_linestate |= TN_LINESTATE_FRAMING;
    }
    if (current_counter.brk != last_counter.brk)
    {
        new_linestate |= TN_LINESTATE_BREAK;
    }

    /* save the last update of counters */
    memcpy(&last_counter, &current_counter, sizeof(struct serial_icounter_struct));

    return new_linestate;
}

/**************************************************************
 * telnet_send_rfc2217_linestate()
 **************************************************************/
static void telnet_send_rfc2217_linestate(unsigned char linestate)
{
    /* send line state message */
    *nfrontp++ = IAC;
    *nfrontp++ = SB;
    *nfrontp++ = TELOPT_COMPORT;
    *nfrontp++ = TN_NOTIFY_LINESTATE_S;
    nfrontp += telnet_IAC_put8(nfrontp, linestate);
    *nfrontp++ = IAC;
    *nfrontp++ = SE;

    /* clear update linestate */
    if (update_linestate)
    {
        update_linestate = 0;
    }
}

/**************************************************************
 * telnet_get_rfc2217_modemstate()
 **************************************************************/
static unsigned char telnet_get_rfc2217_modemstate(void)
{
    int retval;
    unsigned char last_mstat;
    unsigned char new_mstat;
    int signals;

    retval = ioctl(serial_fd, TIOCMGET, &signals);
    assert(retval >= 0);
    
    new_mstat = 0;
    
    if (signals & TIOCM_CD)
    {
        new_mstat |= TN_MODEMSTATE_DCD;
    }
    
    if (signals & TIOCM_RI)
    {
        new_mstat |= TN_MODEMSTATE_RI;
    }

    if (signals & TIOCM_DSR)
    {
        new_mstat |= TN_MODEMSTATE_DSR;
    }

    if (signals & TIOCM_CTS)
    {
        new_mstat |= TN_MODEMSTATE_CTS;
    }

    last_mstat = modemstate_last;

    /* set mstat deltas */
    new_mstat |= (new_mstat ^ last_mstat) >> 4;

    modemstate_last = new_mstat;

    return new_mstat;
}

/**************************************************************
 * telnet_send_rfc2217_modemstate()
 **************************************************************/
static void telnet_send_rfc2217_modemstate(unsigned char mstat)
{
    /* send modem state message */
    *nfrontp++ = IAC;
    *nfrontp++ = SB;
    *nfrontp++ = TELOPT_COMPORT;
    *nfrontp++ = TN_NOTIFY_MODEMSTATE_S;
    nfrontp += telnet_IAC_put8(nfrontp, mstat);
    *nfrontp++ = IAC;
    *nfrontp++ = SE;

    /* clear update modemstate */
    if (update_modemstate)
    {
        update_modemstate = 0;
    }
}

/**************************************************************
 * rfc2217_enabled()
 **************************************************************/
int rfc2217_enabled(void)
{
    return serial_fd > 0;
}

/**************************************************************
 * rfc2217_open()
 **************************************************************/
int rfc2217_open(const char *device)
{
    int retval;

    serial_fd = open(device, O_RDWR);
    if (serial_fd < 0) {
	syslog(LOG_ERR, "unable to open %s\n", device);
	return -1;
    }

    /* save serial settings */
    retval = tcgetattr (serial_fd, &save_tios);
    assert(retval >= 0);

    modemstate_last = telnet_get_rfc2217_modemstate();
    linestate_last = telnet_get_rfc2217_linestate();

    linestate_mask = 0;
    update_linestate = 0;

    modemstate_mask = 0xFF;
    update_modemstate = 1;

    return 0;
}

/**************************************************************
 * rfc2217_close()
 **************************************************************/
void rfc2217_close(void)
{
    int retval;

    if (serial_fd < 0)
        return;

    /* restore serial settings */
    retval = tcsetattr(serial_fd, TCSANOW, &save_tios);
    assert(retval >= 0);

    close(serial_fd);
    serial_fd = -1;
}

/**************************************************************
 * rfc2217_poll()
 **************************************************************/
void rfc2217_poll(void)
{
    if (serial_fd < 0 || 
	his_state_is_wont(TELOPT_COMPORT) ||
	(&netobuf[BUFSIZ] - nfrontp) < 16)
	    return;

    if (modemstate_mask)
    {
	unsigned char mstat;

	mstat = telnet_get_rfc2217_modemstate();

	/* did anything change? */
	if ((mstat & TN_MODEMSTATE_DELTA_MASK) ||
	    (update_modemstate))
	{
	    mstat &= modemstate_mask;

	    if ((mstat) || (update_modemstate))
	    {
		/* send update */
		telnet_send_rfc2217_modemstate(mstat);
	    }
	}
    }

    if (linestate_mask)
    {
	unsigned char linestate;
	unsigned char delta;

	linestate = telnet_get_rfc2217_linestate();

	delta = linestate ^ linestate_last;

	/* save the last sent linestate */
	linestate_last = linestate;

	if ((delta) || 
	    (update_linestate))
	{
	    linestate &= linestate_mask;

	    if ((linestate) ||
		(update_linestate))
	    {
		telnet_send_rfc2217_linestate(linestate);
	    }
	}
    }
}

#endif /* RFC2217 */
