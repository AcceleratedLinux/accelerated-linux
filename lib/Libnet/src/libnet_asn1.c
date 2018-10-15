/*
 *  $Id: libnet_asn1.c,v 1.1.1.1 2000/05/25 00:28:49 route Exp $
 *
 *  libnet
 *  libnet_asn1.c -  Abstract Syntax Notation One routines
 *
 *  Abstract Syntax Notation One, ASN.1
 *  As defined in ISO/IS 8824 and ISO/IS 8825
 *  This implements a subset of the above International Standards that
 *  is sufficient to implement SNMP.
 *
 *  Encodes abstract data types into a machine independent stream of bytes.
 *
 *  Copyright 1988, 1989, 1991, 1992 by Carnegie Mellon University
 *  All rights reserved.
 *
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation, and that the name of CMU not be
 *  used in advertising or publicity pertaining to distribution of the
 *  software without specific, written prior permission.
 *
 *  CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 *  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 *  CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 *  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 *  SOFTWARE.
 *
 *  Copyright (c) 1998 - 2001 Mike D. Schiffman <mike@infonexus.com>
 *  All rights reserved.
 *
 * Copyright (c) 1993, 1994, 1995, 1996, 1998
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if (HAVE_CONFIG_H)
#include "../include/config.h"
#endif
#include "../include/libnet.h"

u_char *
libnet_build_asn1_int(u_char *data, int *datalen, u_char type, long *int_p,
            int int_s)
{
    /*
     *  ASN.1 integer ::= 0x02 asnlength byte {byte}*
     */
     register long integer;
     register u_long mask;

    if (int_s != sizeof (long))
    {
        return (NULL);
    }
    integer = *int_p;

    /*
     *  Truncate "unnecessary" bytes off of the most significant end of this
     *  2's complement integer.  There should be no sequence of 9 consecutive
     *  1's or 0's at the most significant end of the integer.
     */
    mask = ((u_long) 0x1FF) << ((8 * (sizeof (long) - 1)) - 1);
    /* mask is 0xFF800000 on a big-endian machine */

    while ((((integer & mask) == 0) || ((integer & mask) == mask)) && int_s > 1)
    {
        int_s--;
        integer <<= 8;
    }

    data = libnet_build_asn1_header(data, datalen, type, int_s);

    if (data == NULL || *datalen < int_s)
    {
        return (NULL);
    }

    *datalen -= int_s;

    mask = ((u_long) 0xFF) << (8 * (sizeof(long) - 1));
    /* mask is 0xFF000000 on a big-endian machine */

    while (int_s--)
    {
        *data++ = (u_char)((integer & mask) >> (8 * (sizeof (long) - 1)));
        integer <<= 8;
    }
    return (data);
}


u_char *
libnet_build_asn1_uint(u_char *data, int *datalen, u_char type, u_long *int_p,
            int int_s)
{
    /*
     *  ASN.1 integer ::= 0x02 asnlength byte {byte}*
     */
    register u_long integer;
    register u_long mask;
    int add_null_byte = 0;

    if (int_s != sizeof (long))
    {
        return (NULL);
    }
    integer = *int_p;

    mask = ((u_long) 0xFF) << (8 * (sizeof (long) - 1));
    /* mask is 0xFF000000 on a big-endian machine */

    if ((u_char)((integer & mask) >> (8 * (sizeof (long) - 1))) & 0x80)
    {
        /* if MSB is set */
        add_null_byte = 1;
        int_s++;
    }
    else
    {
        /*
         * Truncate "unnecessary" bytes off of the most significant end of this
         * 2's complement integer.  There should be no sequence of 9
         * consecutive 1's or 0's at the most significant end of the
         * integer.
         */
        mask = ((u_long) 0x1FF) << ((8 * (sizeof(long) - 1)) - 1);
        /* mask is 0xFF800000 on a big-endian machine */

        while (((integer & mask) == 0) && int_s > 1)
        {
            int_s--;
            integer <<= 8;
        }
    }

    data = libnet_build_asn1_header(data, datalen, type, int_s);

    if (data == NULL || *datalen < int_s)
    {
        return (NULL);
    }

    *datalen -= int_s;

    if (add_null_byte == 1)
    {
        *data++ = '\0';
        int_s--;
    }

    mask = ((u_long) 0xFF) << (8 * (sizeof(long) - 1));
    /* mask is 0xFF000000 on a big-endian machine */

    while (int_s--)
    {
        *data++ = (u_char)((integer & mask) >> (8 * (sizeof (long) - 1)));
        integer <<= 8;
    }
    return (data);
}


u_char *
libnet_build_asn1_string(u_char *data, int *datalen, u_char type,
            u_char *string, int str_s)
{

    /*
     *  ASN.1 octet string ::= primstring | cmpdstring
     *  primstring ::= 0x04 asnlength byte {byte}*
     *  cmpdstring ::= 0x24 asnlength string {string}*
     *  This code will never send a compound string.
     */
    data = libnet_build_asn1_header(data, datalen, type, str_s);

    if (data == NULL || *datalen < str_s)
    {
        return (NULL);
    }
    memmove(data, string, str_s);
    *datalen -= str_s;

    return (data + str_s);
}


u_char *
libnet_build_asn1_header(u_char *data, int *datalen, u_char type, int len)
{
    if (*datalen < 1)
    {
        return (NULL);
    }
    *data++ = type;
    (*datalen)--;

    return (libnet_build_asn1_length(data, datalen, len));
}


u_char *
libnet_build_asn1_sequence(u_char *data, int *datalen, u_char type, int len)
{
    *datalen -= 4;
    if (*datalen < 0)
    {
        *datalen += 4;       /* fix up before punting */
        return (NULL);
    }
    *data++ = type;
    *data++ = (u_char)(0x02 | ASN_LONG_LEN);
    *data++ = (u_char)((len >> 8) & 0xFF);
    *data++ = (u_char)(len & 0xFF);
    return (data);
}


u_char *
libnet_build_asn1_length(u_char *data, int *datalen, int len)
{
    u_char *start_data = data;

    /* no indefinite lengths sent */
    if (len < 0x80)
    {
        if (*datalen < 1)
        {
            return (NULL);
        }
        *data++ = (u_char)len;
    }
    else if (len <= 0xFF)
    {
        if (*datalen < 2)
        {
            return (NULL);
        }
        *data++ = (u_char)(0x01 | ASN_LONG_LEN);
        *data++ = (u_char)len;
    }
    else    /* 0xFF < len <= 0xFFFF */
    {
        if (*datalen < 3)
        {
            return (NULL);
        }
        *data++ = (u_char)(0x02 | ASN_LONG_LEN);
        *data++ = (u_char)((len >> 8) & 0xFF);
        *data++ = (u_char)(len & 0xFF);
    }
    *datalen -= (data - start_data);
    return (data);
}


u_char *
libnet_build_asn1_objid(u_char *data, int *datalen, u_char type, oid *objid,
            int objidlen)
{
    /*
     *  ASN.1 objid ::= 0x06 asnlength subidentifier {subidentifier}*
     *  subidentifier ::= {leadingbyte}* lastbyte
     *  leadingbyte ::= 1 7bitvalue
     *  lastbyte ::= 0 7bitvalue
     */
    int asnlen;
    register oid *op = objid;
    u_char objid_size[MAX_OID_LEN];
    register u_long objid_val;
    u_long first_objid_val;
    register int i;

    /* check if there are at least 2 sub-identifiers */
    if (objidlen < 2)
    {
        /* there are not, so make OID have two with value of zero */
        objid_val = 0;
        objidlen  = 2;
    }
    else
    {
        /* combine the first two values */
        objid_val = (op[0] * 40) + op[1];
        op += 2;
    }
    first_objid_val = objid_val;

    /* calculate the number of bytes needed to store the encoded value */
    for (i = 1, asnlen = 0;;)
    {
        if (objid_val < (unsigned)0x80)
        {
            objid_size[i] = 1;
            asnlen += 1;
        }
        else if (objid_val < (unsigned)0x4000)
        {
            objid_size[i] = 2;
            asnlen += 2;
        }
        else if (objid_val < (unsigned)0x200000)
        {
            objid_size[i] = 3;
            asnlen += 3;
        }
        else if (objid_val < (unsigned)0x10000000)
        {
            objid_size[i] = 4;
            asnlen += 4;
        }
        else
        {
            objid_size[i] = 5;
            asnlen += 5;
        }
        i++;
        if (i >= objidlen)
        {
            break;
        }
        objid_val = *op++;
    }

    /* store the ASN.1 tag and length */
    data = libnet_build_asn1_header(data, datalen, type, asnlen);
    if (data == NULL || *datalen < asnlen)
    {
        return (NULL);
    }

    /* store the encoded OID value */
    for (i = 1, objid_val = first_objid_val, op = objid + 2; i < objidlen; i++)
    {
        if (i != 1)
        {
            objid_val = *op++;
        }
        switch (objid_size[i])
        {
            case 1:
                *data++ = (u_char)objid_val;
                break;

            case 2:
                *data++ = (u_char)((objid_val >> 7) | 0x80);
                *data++ = (u_char)(objid_val & 0x07f);
                break;
            case 3:
                *data++ = (u_char)((objid_val >> 14) | 0x80);
                *data++ = (u_char)((objid_val >> 7 & 0x7f) | 0x80);
                *data++ = (u_char)(objid_val & 0x07f);
                break;

            case 4:
                *data++ = (u_char)((objid_val >> 21) | 0x80);
                *data++ = (u_char)((objid_val >> 14 & 0x7f) | 0x80);
                *data++ = (u_char)((objid_val >> 7 & 0x7f) | 0x80);
                *data++ = (u_char)(objid_val & 0x07f);
                break;

            case 5:
                *data++ = (u_char)((objid_val >> 28) | 0x80);
                *data++ = (u_char)((objid_val >> 21 & 0x7f) | 0x80);
                *data++ = (u_char)((objid_val >> 14 & 0x7f) | 0x80);
                *data++ = (u_char)((objid_val >> 7 & 0x7f) | 0x80);
                *data++ = (u_char)(objid_val & 0x07f);
                break;
        }
    }

    /* return the length and data ptr */
    *datalen -= asnlen;
    return (data);
}


u_char *
libnet_build_asn1_null(u_char *data, int *datalen, u_char type)
{
    /*
     *  ASN.1 null ::= 0x05 0x00
     */       
    return (libnet_build_asn1_header(data, datalen, type, 0));
}


u_char *
libnet_build_asn1_bitstring(u_char *data, int *datalen, u_char type,
            u_char *string, int str_s)
{

    /*
     *  ASN.1 bit string ::= 0x03 asnlength unused {byte}*
     */
    if (str_s < 1 || *string > 7)
    {
        return (NULL);
    }
    data = libnet_build_asn1_header(data, datalen, type, str_s);

    if (data == NULL || *datalen < str_s)
    {
        return (NULL);
    }

    memmove(data, string, str_s);
    *datalen -= str_s;

    return (data + str_s);
}

/* EOF */
