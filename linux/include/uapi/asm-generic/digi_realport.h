#ifndef __ASM_GENERIC_DIGI_REALPORT_H
#define __ASM_GENERIC_DIGI_REALPORT_H

#include <linux/types.h>
#include <linux/ioctl.h>

struct realport_hw_stats {
	int tiocoutq;
	unsigned int tiocmget;
	struct {
		int norun, noflow, nframe, nparity, nbreak;
	} counters;
	unsigned short estat;
	char tiocser_temt;
};

struct serial_rts_toggle {
	unsigned int enable;
	__u32	predelay;	/* Delay before send (milliseconds) */
	__u32	postdelay;	/* Delay after send (milliseconds) */
};

#define N_REALPORT	28

#define RP_OPU    0x0001
#define RP_OPS    0x0002
#define RP_OPX    0x0004
#define RP_OPH    0x0008
#define RP_OPALL  0x000F
#define RP_IPU    0x0010
#define RP_IPS    0x0020
#define RP_TXB    0x0040
#define RP_TXI    0x0080
#define RP_TXF    0x0100
#define RP_RXB    0x0200
#define RP_EVENTS 0x03FF

#define DIGI_REALPORT_SEND_IMMEDIATE		_IOW('R', 0x01, char)
#define DIGI_REALPORT_SET_FLOW_SIGNALS	_IOW('R', 0x02, unsigned int)
#define DIGI_REALPORT_GET_HW_STATS		_IOR('R', 0x03, struct realport_hw_stats)
#define DIGI_REALPORT_START_BREAK		_IO('R', 0x04)
#define DIGI_REALPORT_STOP_BREAK		_IO('R', 0x05)
#define DIGI_REALPORT_GET_HW_EVENTS		_IOR('R', 0x06, unsigned short)
#define DIGI_RTS_TOGGLE_CONFIG		_IOW('R', 0x0E, struct serial_rts_toggle)
#define DIGI_REALPORT_DUMP_DRIVER_STATE	_IOW('R', 0x11, unsigned char)

#endif
