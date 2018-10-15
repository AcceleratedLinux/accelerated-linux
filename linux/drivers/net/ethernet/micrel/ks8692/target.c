/* ---------------------------------------------------------------------------
             Copyright (c) 2003-2008 Micrel, Inc.  All rights reserved.
------------------------------------------------------------------------------

    target.c - Target platform functions

Author  Date      Version  Description
PCD     04/08/05  0.1.1    Changed Print MAC address from LSB to MSB.
THa     10/04/04           Updated for PCI version.
THa     12/10/03           Created file.
------------------------------------------------------------------------------
*/


#ifdef _WIN32
/* -------------------------------------------------------------------------- *
 *                               WIN32 OS                                     *
 * -------------------------------------------------------------------------- */

#ifdef NDIS_MINIPORT_DRIVER

#ifndef UNDER_CE
#include <ntddk.h>
typedef unsigned int UINT;

#else
#include <ndis.h>
#endif


#define DBG_PRINT  DbgPrint
#define NEWLINE    "\n"

/* -------------------------------------------------------------------------- */

#define MAX_DELAY_MICROSEC  50
#define TIME_MILLISEC       10000
#define TIME_MICROSEC       10

/*
    DelayMicrosec

    Description:
        This routine delays in microseconds.

    Parameters:
        UINT microsec
            Number of microseconds to delay.

    Return (None):
*/

void DelayMicrosec (
    UINT microsec )
{
    while ( microsec >= MAX_DELAY_MICROSEC )
    {
#ifdef UNDER_CE
        NdisStallExecution( MAX_DELAY_MICROSEC );
#else
        KeStallExecutionProcessor( MAX_DELAY_MICROSEC );
#endif
        microsec -= MAX_DELAY_MICROSEC;
    }
    if ( microsec )
    {
#ifdef UNDER_CE
        NdisStallExecution( microsec );
#else
        KeStallExecutionProcessor( microsec );
#endif
    }
}  /* DelayMicrosec */


/*
    DelayMillisec

    Description:
        This routine delays in milliseconds.

    Parameters:
        UINT millisec
            Number of milliseconds to delay.

    Return (None):
*/

void DelayMillisec (
    UINT millisec )
{
#ifndef UNDER_CE
    UINT count;
#endif

    /* convert to microsecond */
    UINT microsec = millisec * 1000;

#ifndef UNDER_CE
    if ( KeGetCurrentIrql() > PASSIVE_LEVEL  ||
            microsec < KeQueryTimeIncrement() / TIME_MICROSEC )
    {
        DelayMicrosec( microsec );
    }
    else
    {
        LARGE_INTEGER interval;
        ULONGLONG     diffTime;
        ULONGLONG     lastTime;

        lastTime = KeQueryInterruptTime();
        interval = RtlConvertLongToLargeInteger( microsec * -TIME_MICROSEC );
        KeDelayExecutionThread( KernelMode, FALSE, &interval );
        diffTime = KeQueryInterruptTime() - lastTime;
        count = ( UINT )( diffTime / TIME_MICROSEC );

        /* delay not long enough */
        if ( microsec > count )
        {
            microsec -= count;
            DelayMicrosec( microsec );
        }
    }

#else
    NdisMSleep( microsec );
#endif
}  /* DelayMillisec */

#else
#include "target.h"
#include <time.h>


/*
    DelayMicrosec

    Description:
        This routine delays in microseconds.

    Parameters:
        UINT microsec
            Number of microseconds to delay.

    Return (None):
*/

void DelayMicrosec (
    UINT microsec )
{
}  /* DelayMicrosec */


/*
    DelayMillisec

    Description:
        This routine delays in milliseconds.

    Parameters:
        UINT millisec
            Number of milliseconds to delay.

    Return (None):
*/

void DelayMillisec (
    UINT millisec )
{
    clock_t start;
    clock_t stop;

    if ( millisec ) {
        start = clock();
        do {
            stop = clock();
        } while ( ( UINT )( stop - start ) < millisec );
    }
}  /* DelayMillisec */
#endif
#endif  /* #ifdef _WIN32 */

#ifdef DEF_LINUX
/* -------------------------------------------------------------------------- *
 *                             LINUX OS                                       *
 * -------------------------------------------------------------------------- */
#if defined( KS_ISA_BUS )  ||  !defined( KS_ISA )
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <asm/processor.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include "target.h"


/*
    DelayMicrosec

    Description:
        This routine delays in microseconds.

    Parameters:
        UINT microsec
            Number of microseconds to delay.

    Return (None):
*/

void DelayMicrosec (
    UINT microsec )
{
    DWORD millisec = microsec / 1000;

    microsec %= 1000;
    if ( millisec )
        mdelay( millisec );
    if ( microsec )
        udelay( microsec );
}  /* DelayMicrosec */


/*
    DelayMillisec

    Description:
        This routine delays in milliseconds.

    Parameters:
        UINT millisec
            Number of milliseconds to delay.

    Return (None):
*/

void DelayMillisec (
    UINT millisec )
{
    unsigned long ticks = millisec * HZ / 1000;

    if ( !ticks  ||  in_interrupt() ) {
        mdelay( millisec );
    }
    else {
        set_current_state( TASK_INTERRUPTIBLE );
        schedule_timeout( ticks );
    }
}  /* DelayMillisec */
#endif
#endif /* #ifdef DEF_LINUX */


#ifdef KS_ARM
#include "target.h"
#endif


/* -------------------------------------------------------------------------- */

#if !defined( DEF_LINUX )  ||  defined( KS_ISA_BUS )  ||  !defined( KS_ISA )
void PrintMacAddress (
    PUCHAR bAddr )
{
    DBG_PRINT( "%02x:%02x:%02x:%02x:%02x:%02x",
        bAddr[ 0 ], bAddr[ 1 ], bAddr[ 2 ],
        bAddr[ 3 ], bAddr[ 4 ], bAddr[ 5 ]);
}  /* PrintMacAddress */

void PrintIpAddress (
    UINT32 IpAddr )
{

    DBG_PRINT( "%u.%u.%u.%u",
               ((IpAddr >> 24) & 0x000000ff),
               ((IpAddr >> 16) & 0x000000ff),
               ((IpAddr >> 8 ) & 0x000000ff),
               (IpAddr & 0x000000ff)
             );
}  /* PrintIpAddress */

/*
 * PrintPacketData
 *	This function is use to dump given packet for debugging.
 *
 * Argument(s)
 *	data		pointer to the beginning of the packet to dump
 *	len			length of the packet
 *  flag        1: dump tx packet, 2: dump rx packet
 *
 * Return(s)
 *	NONE.
 */

void PrintPacketData
(
    UCHAR  *data,
    int    len,
    UINT32 port,
    UINT32 flag
)
{
    if  ( (flag < 1)  || (flag > 2) )
        return;


    if ( flag == 1 )
         DBG_PRINT ("Tx On port %d "NEWLINE, ( int ) port);
    else
         DBG_PRINT ("Rx On port %d "NEWLINE, ( int ) port);


	/* if (len >= 18) */
    {
		DBG_PRINT("Pkt Len=%d"NEWLINE, len);
		DBG_PRINT("DA=%02x:%02x:%02x:%02x:%02x:%02x"NEWLINE,
				*data, *(data + 1), *(data + 2), *(data + 3), *(data + 4), *(data + 5));
		DBG_PRINT("SA=%02x:%02x:%02x:%02x:%02x:%02x"NEWLINE,
				*(data + 6), *(data + 7), *(data + 8), *(data + 9), *(data + 10), *(data + 11));
		DBG_PRINT("Type=%02x%02x"NEWLINE, (*(UCHAR *)(data + 12)), (*(UCHAR *)(data + 13)) );

        {
			int	j = 0, k;

			do {
				DBG_PRINT(NEWLINE" %04x   ", j);
				for (k = 0; (k < 16 && len); k++, data++, len--)
					DBG_PRINT("%02x  ", *data);
				j += 16;
			} while (len > 0);
			DBG_PRINT(NEWLINE);
		}
	}
}
#endif
