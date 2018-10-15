/* Expand one page of fax data
   Copyright (C) 1990, 1995  Frank D. Cringle.

This file is part of viewfax - g3/g4 fax processing software.
     
viewfax is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.
     
This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.
     
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include "faxexpand.h"

/* Note that NeedBits() only works for n <= 16 */
#define NeedBits(n) do {						\
    if (BitsAvail < (n)) {						\
	BitAcc |= *sp++ << BitsAvail;					\
	BitsAvail += 16;						\
    }									\
} while (0)
#define GetBits(n)	(BitAcc & ((1<<(n))-1))
#define ClrBits(n) do {							\
    BitAcc >>= (n);							\
    BitsAvail -= (n);							\
} while (0)

#ifdef DEBUG
#define DEBUG_SHOW putchar(BitAcc & (1 << t) ? '1' : '0')
#define LOOKUP(wid,tab)	do {						\
    int t;								\
    NeedBits(wid);							\
    TabEnt = tab + GetBits(wid);					\
    printf("%08lX/%d: %s%5d\t", BitAcc, BitsAvail,			\
	   StateNames[TabEnt->State], TabEnt->Param);			\
    for (t = 0; t < TabEnt->Width; t++)					\
	DEBUG_SHOW;							\
    putchar('\n');							\
    fflush(stdout);							\
    ClrBits(TabEnt->Width);						\
} while (0)

#define SETVAL(x) do {							\
    *pa++ = RunLength + (x);						\
    printf("SETVAL: %d\t%d\n", RunLength + (x), a0);			\
    a0 += x;								\
    RunLength = 0;							\
} while (0)

char *StateNames[] = {
    "Null   ",
    "Pass   ",
    "Horiz  ",
    "V0     ",
    "VR     ",
    "VL     ",
    "Ext    ",
    "TermW  ",
    "TermB  ",
    "MakeUpW",
    "MakeUpB",
    "MakeUp ",
    "EOL    ",
};

#else
#define LOOKUP(wid,tab)	do {						\
    NeedBits(wid);							\
    TabEnt = tab + GetBits(wid);					\
    ClrBits(TabEnt->Width);						\
} while (0)

#define SETVAL(x) do {							\
    *pa++ = RunLength + (x);						\
    a0 += x;								\
    RunLength = 0;							\
} while (0)
#endif

#define dumpruns(runs) do {						\
    printf("-------------------- %d\n", LineNum);			\
    for (pa = runs, a0 = 0; a0 < lastx; a0 += *pa++)			\
	printf("%4d %d\n", a0, *pa);					\
} while (0)

#define EndOfData(pn)	(sp >= pn->data + pn->length/sizeof(*pn->data))

/* This macro handles coding errors in G3 data.
   We redefine it below for the G4 case */
#define SKIP_EOL do {							\
    while (!EndOfData(pn)) {						\
	NeedBits(11);							\
	if (GetBits(11) == 0)						\
	    break;							\
	ClrBits(1);							\
    }									\
    ClrBits(11);							\
    goto EOL;								\
} while (0)
#define eol2lab	EOL2:

/* the line expanders are written as macros so that they can be reused
   (twice each) but still have direct access to the local variables of
   the "calling" function */
#define expand1d() do {							\
    while (a0 < lastx) {						\
	int done = 0;							\
	while (!done) {		/* white first */			\
	    LOOKUP(12, WhiteTable);					\
	    switch (TabEnt->State) {					\
	    case S_EOL:							\
		EOLcnt = 1;						\
		goto EOL;						\
	    case S_TermW:						\
		SETVAL(TabEnt->Param);					\
		done = 1;						\
		break;							\
	    case S_MakeUpW:						\
	    case S_MakeUp:						\
		a0 += TabEnt->Param;					\
		RunLength += TabEnt->Param;				\
		break;							\
	    case S_Ext:							\
		unexpected("Extension code", LineNum);			\
		SKIP_EOL;						\
		break;							\
	    default:							\
		unexpected("WhiteTable", LineNum);			\
		SKIP_EOL;						\
		break;							\
	    }								\
	}								\
	done = a0 >= lastx;						\
	while (!done) {		/* then black */			\
	    LOOKUP(13, BlackTable);					\
	    switch (TabEnt->State) {					\
	    case S_EOL:							\
		EOLcnt = 1;						\
		goto EOL;						\
	    case S_TermB:						\
		SETVAL(TabEnt->Param);					\
		done = 1;						\
		break;							\
	    case S_MakeUpB:						\
	    case S_MakeUp:						\
		a0 += TabEnt->Param;					\
		RunLength += TabEnt->Param;				\
		break;							\
	    case S_Ext:							\
		unexpected("Extension code", LineNum);			\
		SKIP_EOL;						\
		break;							\
	    default:							\
		unexpected("BlackTable", LineNum);			\
		SKIP_EOL;						\
		break;							\
	    }								\
	}								\
    }									\
 EOL: ;									\
} while (0)

#define CHECK_b1 do {							\
    if (pa != thisrun) while (b1 <= a0 && b1 < lastx) {			\
	b1 += pb[0] + pb[1];						\
	pb += 2;							\
    }									\
} while (0)

#define expand2d(eolab) do {						\
    while (a0 < lastx) {						\
	LOOKUP(7, MainTable);						\
	switch (TabEnt->State) {					\
	case S_Pass:							\
	    CHECK_b1;							\
	    b1 += *pb++;						\
	    RunLength += b1 - a0;					\
	    a0 = b1;							\
	    b1 += *pb++;						\
	    break;							\
	case S_Horiz:							\
	    if ((pa-run0)&1) {						\
		int done = 0;						\
		while (!done) { /* black first */			\
		    LOOKUP(13, BlackTable);				\
		    switch (TabEnt->State) {				\
		    case S_TermB:					\
			SETVAL(TabEnt->Param);				\
			done = 1;					\
			break;						\
		    case S_MakeUpB:					\
		    case S_MakeUp:					\
			a0 += TabEnt->Param;				\
			RunLength += TabEnt->Param;			\
			break;						\
		    default:						\
			unexpected("BlackTable", LineNum);		\
			SKIP_EOL;					\
			break;						\
		    }							\
		}							\
		done = 0;						\
		while (!done) { /* then white */			\
		    LOOKUP(12, WhiteTable);				\
		    switch (TabEnt->State) {				\
		    case S_TermW:					\
			SETVAL(TabEnt->Param);				\
			done = 1;					\
			break;						\
		    case S_MakeUpW:					\
		    case S_MakeUp:					\
			a0 += TabEnt->Param;				\
			RunLength += TabEnt->Param;			\
			break;						\
		    default:						\
			unexpected("WhiteTable", LineNum);		\
			SKIP_EOL;					\
			break;						\
		    }							\
		}							\
	    }								\
	    else {							\
		int done = 0;						\
		while (!done) { /* white first */			\
		    LOOKUP(12, WhiteTable);				\
		    switch (TabEnt->State) {				\
		    case S_TermW:					\
			SETVAL(TabEnt->Param);				\
			done = 1;					\
			break;						\
		    case S_MakeUpW:					\
		    case S_MakeUp:					\
			a0 += TabEnt->Param;				\
			RunLength += TabEnt->Param;			\
			break;						\
		    default:						\
			unexpected("WhiteTable", LineNum);		\
			SKIP_EOL;					\
			break;						\
		    }							\
		}							\
		done = 0;						\
		while (!done) { /* then black */			\
		    LOOKUP(13, BlackTable);				\
		    switch (TabEnt->State) {				\
		    case S_TermB:					\
			SETVAL(TabEnt->Param);				\
			done = 1;					\
			break;						\
		    case S_MakeUpB:					\
		    case S_MakeUp:					\
			a0 += TabEnt->Param;				\
			RunLength += TabEnt->Param;			\
			break;						\
		    default:						\
			unexpected("BlackTable", LineNum);		\
			SKIP_EOL;					\
			break;						\
		    }							\
		}							\
	    }								\
	    CHECK_b1;							\
	    break;							\
	case S_V0:							\
	    CHECK_b1;							\
	    SETVAL(b1 - a0);						\
	    b1 += *pb++;						\
	    break;							\
	case S_VR:							\
	    CHECK_b1;							\
	    SETVAL(b1 - a0 + TabEnt->Param);				\
	    b1 += *pb++;						\
	    break;							\
	case S_VL:							\
	    CHECK_b1;							\
	    SETVAL(b1 - a0 - TabEnt->Param);				\
	    b1 -= *--pb;						\
	    break;							\
	case S_Ext:							\
	    *pa++ = lastx - a0;						\
	    if (verbose)						\
		fprintf(stderr, "Line %d: extension code\n", LineNum);	\
	    SKIP_EOL;							\
	    break;							\
	case S_EOL:							\
	    *pa++ = lastx - a0;						\
	    NeedBits(4);						\
	    if (GetBits(4) && verbose) /* already seen 7 zeros */	\
		fprintf(stderr, "Line %d: Bad EOL\n", LineNum);		\
	    ClrBits(4);							\
	    EOLcnt = 1;							\
	    goto eolab;							\
	    break;							\
	default:							\
	    unexpected("MainTable", LineNum);				\
	    SKIP_EOL;							\
	    break;							\
	}								\
    }									\
    if (RunLength) {							\
	if (RunLength + a0 < lastx) {					\
	    /* expect a final V0 */					\
	    NeedBits(1);						\
	    if (!GetBits(1)) {						\
		unexpected("MainTable", LineNum);			\
		SKIP_EOL;						\
	    }								\
	    ClrBits(1);							\
	}								\
	SETVAL(0);							\
    }									\
 eol2lab ;								\
} while (0)

static void
unexpected(char *what, int LineNum)
{
    if (verbose)
	fprintf(stderr, "Line %d: Unexpected state in %s\n",
		LineNum, what);
}

/* Expand tiff modified huffman data (g3-1d without EOLs) */
void
MHexpand(struct pagenode *pn, drawfunc df)
{
    int a0;			/* reference element */
    int lastx = pn->width;	/* copy line width to register */
    t32bits BitAcc;		/* bit accumulator */
    int BitsAvail;		/* # valid bits in BitAcc */
    int RunLength;		/* Length of current run */
    t16bits *sp;		/* pointer into compressed data */
    pixnum *pa;			/* pointer into new line */
    int EOLcnt;			/* number of consecutive EOLs */
    int	LineNum;		/* line number */
    pixnum *runs;		/* list of run lengths */
    struct tabent *TabEnt;

    sp = pn->data;
    BitAcc = 0;
    BitsAvail = 0;
    lastx = pn->width;
    runs = (pixnum *) xmalloc(lastx * sizeof(pixnum));
    for (LineNum = 0; LineNum < pn->rowsperstrip; ) {
#ifdef DEBUG
	printf("\nBitAcc=%08lX, BitsAvail = %d\n", BitAcc, BitsAvail);
	printf("-------------------- %d\n", LineNum);
	fflush(stdout);
#endif
	RunLength = 0;
	pa = runs;
	a0 = 0;
	EOLcnt = 0;
	if (BitsAvail & 7)	/* skip to byte boundary */
	    ClrBits(BitsAvail & 7);
	expand1d();
	if (RunLength)
	    SETVAL(0);
	if (a0 != lastx) {
	    if (verbose)
		fprintf(stderr, "Line %d: length is %d (expected %d)\n", LineNum, a0, lastx);
	    while (a0 > lastx)
		a0 -= *--pa;
	    if (a0 < lastx) {
		if ((pa - runs) & 1)
		    SETVAL(0);
		SETVAL(lastx - a0);
	    }
	}
	(*df)(runs, LineNum++, pn);
    }
    free(runs);
}

/* Expand group-3 1-dimensional data */
void
g31expand(struct pagenode *pn, drawfunc df)
{
    int a0;			/* reference element */
    int lastx = pn->width;	/* copy line width to register */
    t32bits BitAcc;		/* bit accumulator */
    int BitsAvail;		/* # valid bits in BitAcc */
    int RunLength;		/* Length of current run */
    t16bits *sp;		/* pointer into compressed data */
    pixnum *pa;			/* pointer into new line */
    int EOLcnt;			/* number of consecutive EOLs */
    int	LineNum;		/* line number */
    pixnum *runs;		/* list of run lengths */
    struct tabent *TabEnt;

    sp = pn->data;
    BitAcc = 0;
    BitsAvail = 0;
    lastx = pn->width;
    runs = (pixnum *) xmalloc(lastx * sizeof(pixnum));
    EOLcnt = 0;
    for (LineNum = 0; LineNum < pn->rowsperstrip; ) {
#ifdef DEBUG
	printf("\nBitAcc=%08lX, BitsAvail = %d\n", BitAcc, BitsAvail);
	printf("-------------------- %d\n", LineNum);
	fflush(stdout);
#endif
	if (EOLcnt == 0)
	    while (!EndOfData(pn)) {
		/* skip over garbage after a coding error */
		NeedBits(11);
		if (GetBits(11) == 0)
		    break;
		ClrBits(1);
	    }
	for (EOLcnt = 1; !EndOfData(pn); EOLcnt++) {
	    /* we have seen 11 zeros, which implies EOL,
	       skip possible fill bits too */
	    while (1) {
		NeedBits(8);
		if (GetBits(8))
		    break;
		ClrBits(8);
	    }
	    while (GetBits(1) == 0)
		ClrBits(1);
	    ClrBits(1);		/* the eol flag */
	    NeedBits(11);
	    if (GetBits(11))
		break;
	    ClrBits(11);
	}
	if (EOLcnt > 1 && EOLcnt != 6 && verbose)
	    fprintf(stderr, "Line %d: bad RTC (%d EOLs)\n", LineNum, EOLcnt);
	if (EOLcnt >= 6 || EndOfData(pn)) {
	    free(runs);
	    return;
	}
	RunLength = 0;
	pa = runs;
	a0 = 0;
	EOLcnt = 0;
	expand1d();
	if (RunLength)
	    SETVAL(0);
	if (a0 != lastx) {
	    if (verbose)
		fprintf(stderr, "Line %d: length is %d (expected %d)\n", LineNum, a0, lastx);
	    while (a0 > lastx)
		a0 -= *--pa;
	    if (a0 < lastx) {
		if ((pa - runs) & 1)
		    SETVAL(0);
		SETVAL(lastx - a0);
	    }
	}
	(*df)(runs, LineNum++, pn);
    }
    free(runs);
}

/* Expand group-3 2-dimensional data */
void
g32expand(struct pagenode *pn, drawfunc df)
{
    int RunLength;		/* Length of current run */
    int a0;			/* reference element */
    int b1;			/* next change on previous line */
    int lastx = pn->width;	/* copy line width to register */
    pixnum *run0, *run1;	/* run length arrays */
    pixnum *thisrun, *pa, *pb;	/* pointers into runs */
    t16bits *sp;		/* pointer into compressed data */
    t32bits BitAcc;		/* bit accumulator */
    int BitsAvail;		/* # valid bits in BitAcc */
    int EOLcnt;			/* number of consecutive EOLs */
    int	refline = 0;		/* 1D encoded reference line */
    int	LineNum;		/* line number */
    struct tabent *TabEnt;

    sp = pn->data;
    BitAcc = 0;
    BitsAvail = 0;
    /* allocate space for 2 runlength arrays */
    run0 = (pixnum *) xmalloc(2 * ((lastx+5)&~1) * sizeof(pixnum));
    run1 = run0 + ((lastx+5)&~1);
    run1[0] = lastx;
    run1[1] = 0;
    EOLcnt = 0;
    for (LineNum = 0; LineNum < pn->rowsperstrip; ) {
#ifdef DEBUG
	printf("\nBitAcc=%08lX, BitsAvail = %d\n", BitAcc, BitsAvail);
	printf("-------------------- %d\n", LineNum);
	fflush(stdout);
#endif
	if (EOLcnt == 0)
	    while (!EndOfData(pn)) {
		/* skip over garbage after a coding error */
		NeedBits(11);
		if (GetBits(11) == 0)
		    break;
		ClrBits(1);
	    }
	for (EOLcnt = 1; !EndOfData(pn); EOLcnt++) {
	    /* we have seen 11 zeros, which implies EOL,
	       skip possible fill bits too */
	    while (1) {
		NeedBits(8);
		if (GetBits(8))
		    break;
		ClrBits(8);
	    }
	    while (GetBits(1) == 0)
		ClrBits(1);
	    ClrBits(1);		/* the eol flag */
	    NeedBits(12);
	    refline = GetBits(1); /* 1D / 2D flag */
	    ClrBits(1);
	    if (GetBits(11))
		break;
	    ClrBits(11);
	}
	if (EOLcnt > 1 && EOLcnt != 6 && verbose)
	    fprintf(stderr, "Line %d: bad RTC (%d EOLs)\n", LineNum, EOLcnt);
	if (EOLcnt >= 6 || EndOfData(pn)) {
	    free(run0);
	    return;
	}
	if (LineNum == 0 && refline == 0 && verbose)
	    fprintf(stderr, "First line is 2-D encoded\n");
	RunLength = 0;
	if (LineNum & 1) {
	    pa = run1;
	    pb = run0;
	}
	else {
	    pa = run0;
	    pb = run1;
	}
	thisrun = pa;
	EOLcnt = 0;
	a0 = 0;
	b1 = *pb++;

	if (refline) {
	    expand1d();
	}
	else {
	    expand2d(EOL2);
	}
	if (RunLength)
	    SETVAL(0);
	if (a0 != lastx) {
	    if (verbose)
		fprintf(stderr, "Line %d: length is %d (expected %d)\n", LineNum, a0, lastx);
	    while (a0 > lastx)
		a0 -= *--pa;
	    if (a0 < lastx) {
		if ((pa - run0) & 1)
		    SETVAL(0);
		SETVAL(lastx - a0);
	    }
	}
	SETVAL(0);	/* imaginary change at end of line for reference */
	(*df)(thisrun, LineNum++, pn);
    }
    free(run0);
}

/* Redefine the "skip to eol" macro.  We cannot recover from coding
   errors in G4 data */
#undef SKIP_EOL
#undef eol2lab
#define SKIP_EOL do {							\
    if (verbose)							\
	fprintf(stderr, "Line %d: G4 coding error\n", LineNum);		\
    free(run0);								\
    return;								\
} while (0)
#define eol2lab

/* Expand group-4 data */
void
g4expand(struct pagenode *pn, drawfunc df)
{
    int RunLength;		/* Length of current run */
    int a0;			/* reference element */
    int b1;			/* next change on previous line */
    int lastx = pn->width;	/* copy line width to register */
    pixnum *run0, *run1;	/* run length arrays */
    pixnum *thisrun, *pa, *pb;	/* pointers into runs */
    t16bits *sp;		/* pointer into compressed data */
    t32bits BitAcc;		/* bit accumulator */
    int BitsAvail;		/* # valid bits in BitAcc */
    int	LineNum;		/* line number */
    int EOLcnt;
    struct tabent *TabEnt;

    sp = pn->data;
    BitAcc = 0;
    BitsAvail = 0;
    /* allocate space for 2 runlength arrays */
    run0 = (pixnum *) xmalloc(2 * ((lastx+5)&~1) * sizeof(pixnum));
    run1 = run0 + ((lastx+5)&~1);
    run1[0] = lastx;		/* initial reference line */
    run1[1] = 0;

    for (LineNum = 0; LineNum < pn->rowsperstrip; ) {
#ifdef DEBUG
	printf("\nBitAcc=%08lX, BitsAvail = %d\n", BitAcc, BitsAvail);
	printf("-------------------- %d\n", LineNum);
	fflush(stdout);
#endif
	RunLength = 0;
	if (LineNum & 1) {
	    pa = run1;
	    pb = run0;
	}
	else {
	    pa = run0;
	    pb = run1;
	}
	thisrun = pa;
	a0 = 0;
	b1 = *pb++;
	expand2d(EOFB);
	if (a0 < lastx) {
	    if ((pa - run0) & 1)
		SETVAL(0);
	    SETVAL(lastx - a0);
	}
	SETVAL(0);	/* imaginary change at end of line for reference */
	(*df)(thisrun, LineNum++, pn);
	continue;
    EOFB:
	NeedBits(13);
	if (GetBits(13) != 0x1001 && verbose)
	    fputs("Bad RTC\n", stderr);
	break;
    }
    free(run0);
}

static unsigned char zerotab[256] = {
	0x88, 0x07, 0x16, 0x06, 0x25, 0x05, 0x15, 0x05,
	0x34, 0x04, 0x14, 0x04, 0x24, 0x04, 0x14, 0x04,
	0x43, 0x03, 0x13, 0x03, 0x23, 0x03, 0x13, 0x03,
	0x33, 0x03, 0x13, 0x03, 0x23, 0x03, 0x13, 0x03,
	0x52, 0x02, 0x12, 0x02, 0x22, 0x02, 0x12, 0x02,
	0x32, 0x02, 0x12, 0x02, 0x22, 0x02, 0x12, 0x02,
	0x42, 0x02, 0x12, 0x02, 0x22, 0x02, 0x12, 0x02,
	0x32, 0x02, 0x12, 0x02, 0x22, 0x02, 0x12, 0x02,
	0x61, 0x01, 0x11, 0x01, 0x21, 0x01, 0x11, 0x01,
	0x31, 0x01, 0x11, 0x01, 0x21, 0x01, 0x11, 0x01,
	0x41, 0x01, 0x11, 0x01, 0x21, 0x01, 0x11, 0x01,
	0x31, 0x01, 0x11, 0x01, 0x21, 0x01, 0x11, 0x01,
	0x51, 0x01, 0x11, 0x01, 0x21, 0x01, 0x11, 0x01,
	0x31, 0x01, 0x11, 0x01, 0x21, 0x01, 0x11, 0x01,
	0x41, 0x01, 0x11, 0x01, 0x21, 0x01, 0x11, 0x01,
	0x31, 0x01, 0x11, 0x01, 0x21, 0x01, 0x11, 0x01,
	0x70, 0x00, 0x10, 0x00, 0x20, 0x00, 0x10, 0x00,
	0x30, 0x00, 0x10, 0x00, 0x20, 0x00, 0x10, 0x00,
	0x40, 0x00, 0x10, 0x00, 0x20, 0x00, 0x10, 0x00,
	0x30, 0x00, 0x10, 0x00, 0x20, 0x00, 0x10, 0x00,
	0x50, 0x00, 0x10, 0x00, 0x20, 0x00, 0x10, 0x00,
	0x30, 0x00, 0x10, 0x00, 0x20, 0x00, 0x10, 0x00,
	0x40, 0x00, 0x10, 0x00, 0x20, 0x00, 0x10, 0x00,
	0x30, 0x00, 0x10, 0x00, 0x20, 0x00, 0x10, 0x00,
	0x60, 0x00, 0x10, 0x00, 0x20, 0x00, 0x10, 0x00,
	0x30, 0x00, 0x10, 0x00, 0x20, 0x00, 0x10, 0x00,
	0x40, 0x00, 0x10, 0x00, 0x20, 0x00, 0x10, 0x00,
	0x30, 0x00, 0x10, 0x00, 0x20, 0x00, 0x10, 0x00,
	0x50, 0x00, 0x10, 0x00, 0x20, 0x00, 0x10, 0x00,
	0x30, 0x00, 0x10, 0x00, 0x20, 0x00, 0x10, 0x00,
	0x40, 0x00, 0x10, 0x00, 0x20, 0x00, 0x10, 0x00,
	0x30, 0x00, 0x10, 0x00, 0x20, 0x00, 0x10, 0x00
};

#define check(v) do {							\
    prezeros = zerotab[v];						\
    postzeros = prezeros & 15;						\
    prezeros >>= 4;							\
    if (prezeros == 8) {						\
	zeros += 8;							\
	continue;							\
    }									\
    if (zeros + prezeros < 11) {					\
	empty = 0;							\
	zeros = postzeros;						\
	continue;							\
    }									\
    zeros = postzeros;							\
    if (empty)								\
	EOLcnt++;							\
    lines++;								\
    empty = 1;								\
} while (0)

/* count fax lines */
int
G3count(struct pagenode *pn, int twoD)
{
    t16bits *p = pn->data;
    t16bits *end = p + pn->length/sizeof(*p);
    int lines = 0;		/* lines seen so far */
    int zeros = 0;		/* number of consecutive zero bits seen */
    int EOLcnt = 0;		/* number of consecutive EOLs seen */
    int empty = 1;		/* empty line */
    int prezeros, postzeros;

    while (p < end && EOLcnt < 6) {
	t16bits bits = *p++;
	check(bits&255);
	if (twoD && (prezeros + postzeros == 7)) {
	    if (postzeros || ((bits & 0x100) == 0))
		zeros--;
	}
	check(bits>>8);
	if (twoD && (prezeros + postzeros == 7)) {
	    if (postzeros || ((p < end) && ((*p & 1) == 0)))
		zeros--;
	}
    }
    return lines - EOLcnt;	/* don't count trailing EOLs */
}
