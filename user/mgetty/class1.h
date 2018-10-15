#ident "$Id: class1.h,v 4.12 2009/03/19 15:29:16 gert Exp $ Copyright (c) Gert Doering"

/* class1.h
 *
 * common definitions for class 1 fax modules
 *
 * $Log: class1.h,v $
 * Revision 4.12  2009/03/19 15:29:16  gert
 * add T30_CAR_HAVE_V21 (do not send AT, we have V21 carrier)
 *
 * Revision 4.11  2006/09/29 19:31:50  gert
 * add "scan time" parameter to fax1_send_dcs()
 * add function fax1_reduce_max()  (baud rate stepdown)
 * add extern declaration for "remote_cap" (in class1lib.c)
 *
 * Revision 4.10  2006/06/14 09:54:03  gert
 * dcs_btp needs to be declared 'extern'
 *
 * Revision 4.9  2006/03/29 12:25:02  gert
 * change type of fax1_dis to uch (unsigned char)
 * change type of "fcf" in fax1_send_idframe() to uch
 *
 * Revision 4.8  2006/01/04 21:06:25  gert
 * remove "speed" argument from fax1_send_dcs() (use fax1_max global)
 *
 * Revision 4.7  2006/01/01 16:02:25  gert
 * introduce extra argument to fax1_send_dcn to set fax_hangup_code
 *
 * Revision 4.6  2005/12/31 17:46:43  gert
 * add fax1_send_dis()
 *
 * Revision 4.5  2005/12/31 15:52:46  gert
 * move typedef...uch from class1.h to mgetty.h
 *
 * Revision 4.4  2005/12/30 23:05:34  gert
 * update & add various prototypes
 *
 * Revision 4.3  2005/12/28 21:46:11  gert
 * T30_DCN had wrong hex value (should be 0xfa, was 0xfc)
 * add symbolic values for T30 carrier values (+FRH=3/+FTH=3)
 * add prototypes for most functions in class1lib.c
 * add CVS + file description header
 *
 */

#define FRAMESIZE	300

extern uch fax1_dis;		/* "X"-Bit (received DIS) */

/* class1lib.c */
RETSIGTYPE fax1_sig_alarm(SIG_HDLR_ARGS);
void fax1_dump_frame _PROTO(( char io, uch * frame, int len ));
int fax1_send_frame _PROTO(( int fd, int carrier, uch * frame, int len ));
int fax1_send_simf_final _PROTO(( int fd, int carrier, uch fcf));
int fax1_send_simf_nonfinal _PROTO(( int fd, int carrier, uch fcf));
int fax1_send_dcn _PROTO(( int fd, int code ));

void fax1_copy_id _PROTO(( uch * frame ));
int fax1_send_idframe _PROTO(( int fd, uch fcf, int carrier));
void fax1_parse_dis _PROTO(( uch * frame ));
void fax1_parse_dcs _PROTO(( uch * frame ));
int fax1_send_dis _PROTO(( int fd ));
int fax1_send_dcs _PROTO(( int fd, int s_time ));
int fax1_receive_frame _PROTO (( int fd, int carrier, 
			         int timeout, uch * framebuf ));
int fax1_init_FRM _PROTO(( int fd, int carrier ));
void fax1_reduce_max _PROTO(( void ));

/* class1.c */
int fax1_send_page _PROTO(( char * g3_file, int * bytes_sent, TIO * tio,
			    Post_page_messages ppm, int fd ));

struct fax1_btable { int speed;			/* bit rate */
                     int flag;			/* flag (for capabilities) */
		     int c_long, c_short;	/* carrier numbers */
		     int dcs_bits;		/* bits to be set in DCS */
		    };
extern struct fax1_btable * dcs_btp;		/* current modulation */

extern fax_param_t remote_cap;			/* receiver capabilities */

/* --- Definitions from ITU T.30, 07/96 --- */

/* control field - bit set on final frame, T.30 5.3.5 */
#define T30_FINAL	0x10

/* frame types (FCF), T.30 5.3.6, bits reversed! */
#define T30_DIS	0x80		/* Digital Information Signal */
#define T30_CSI	0x40		/* Called Subscriber Information */
#define T30_NSF	0x20		/* Non-Standard Facilities */

#define T30_DTC	0x81		/* Digital Transmit Command */
#define T30_CIG	0x41		/* Calling Subscriber Information */
#define T30_NSC	0x21		/* Non-Standard facilities Command */
#define T30_PWD 0xc1		/* Password (for polling) */
#define T30_SEP 0xa1		/* Selective Polling (subaddress) */

#define	T30_DCS	0x82		/* Digital Command Signal */
#define T30_TSI	0x42		/* Transmit Subscriber Information */
#define T30_NSS	0x22		/* Non-Standard facilities Setup */
#define T30_SUB 0xc2		/* Subaddress */
#define T30_PWDT 0xa2		/* Password for Transmission */

#define T30_CFR	0x84		/* Confirmation To Receive */
#define T30_FTT 0x44		/* Failure To Train */

#define T30_EOM	0x8e		/* End Of Message (end of page -> phase B) */
#define T30_MPS 0x4e		/* MultiPage Signal (end of page -> phase C) */
#define T30_EOP 0x2e		/* End Of Procedures (over and out) */
#define T30_PRI_EOM	0x9e	/* EOM + PRI */
#define T30_PRI_MPS	0x5e	/* MPS + PRI */
#define T30_PRI_EOP	0x3e	/* EOP + PRI */
#define T30_PRI		0x10	/* bit 4 in FCF -> Procedure Interrupt */

#define T30_MCF	0x8c		/* Message Confirmation (page good) */
#define T30_RTP	0xcc		/* Retrain Positive */
#define T30_RTN	0x4c		/* Retrain Negative */
#define T30_PIP	0xac		/* Procedure Interrupt Positive */
#define T30_PIN	0x2c		/* Procedure Interrupt Negative */

#define T30_DCN	0xfa		/* Disconnect Now (phase E) */
#define T30_CRP	0x1c		/* Command Repeat (optional) */


/* carrier values */
#define T30_CAR_SAME	0	/* pseudo-header, don't send AT+FTH/AT+FRH */
#define T30_CAR_V21	3	/* 300 bps for negotiation */
#define T30_CAR_HAVE_V21 -3	/* 300 bps for negotiation (already set) */
