/*
 * Copyright (c) 1999, 2000, 2003 Greg Haerr <greg@censoft.com>
 *
 * 4bpp Packed Linear Video Driver for Microwindows
 * 	This driver is written for the Vr41xx Palm PC machines
 * 	Hopefully, we can get the 4bpp mode running 320x240x16
 *
 * If INVERT4BPP is defined, then the values are inverted before drawing.
 * 	This is used for the VTech Helio
 *
 * 	In this driver, psd->linelen is line byte length, not line pixel length
 */
/*#define NDEBUG*/
#include <assert.h>
#include <string.h>
#include "device.h"
#include "fb.h"

#if INVERT4BPP
#define INVERT(c)	((c) = (~c & 0x0f))
#else
#define INVERT(c)
#endif

static unsigned char notmask[2] = { 0x0f, 0xf0};

/* Calc linelen and mmap size, return 0 on fail*/
static int
linear4_init(PSD psd)
{
	if (!psd->size)
		psd->size = psd->yres * psd->linelen;
	/* linelen in bytes for bpp 1, 2, 4, 8 so no change*/
	return 1;
}

/* Set pixel at x, y, to pixelval c*/
static void
linear4_drawpixel(PSD psd, MWCOORD x, MWCOORD y, MWPIXELVAL c)
{
	ADDR8	addr = psd->addr;

	assert (addr != 0);
	assert (x >= 0 && x < psd->xres);
	assert (y >= 0 && y < psd->yres);
	assert (c < psd->ncolors);

	INVERT(c);
	DRAWON;
	addr += (x>>1) + y * psd->linelen;
	if(gr_mode == MWMODE_XOR)
		*addr ^= c << ((1-(x&1))<<2);
	else
		*addr = (*addr & notmask[x&1]) | (c << ((1-(x&1))<<2));
	DRAWOFF;
}

/* Read pixel at x, y*/
static MWPIXELVAL
linear4_readpixel(PSD psd, MWCOORD x, MWCOORD y)
{
	ADDR8		addr = psd->addr;
	MWPIXELVAL	c;

	assert (addr != 0);
	assert (x >= 0 && x < psd->xres);
	assert (y >= 0 && y < psd->yres);

	c = (addr[(x>>1) + y * psd->linelen] >> ((1-(x&1))<<2) ) & 0x0f;
	INVERT(c);
	return c;
	
}

/* Draw horizontal line from x1,y to x2,y including final point*/
static void
linear4_drawhorzline(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y, MWPIXELVAL c)
{
	ADDR8	addr = psd->addr;

	assert (addr != 0);
	assert (x1 >= 0 && x1 < psd->xres);
	assert (x2 >= 0 && x2 < psd->xres);
	assert (x2 >= x1);
	assert (y >= 0 && y < psd->yres);
	assert (c < psd->ncolors);

        INVERT(c);
	DRAWON;
	addr += (x1>>1) + y * psd->linelen;
	if(gr_mode == MWMODE_XOR) {
		while(x1 <= x2) {
			*addr ^= c << ((1-(x1&1))<<2);
			if((++x1 & 1) == 0)
				++addr;
		}
	} else {
		while(x1 <= x2) {
			*addr = (*addr & notmask[x1&1]) | (c << ((1-(x1&1))<<2));
			if((++x1 & 1) == 0)
				++addr;
		}
	}
	DRAWOFF;
}

/* Draw a vertical line from x,y1 to x,y2 including final point*/
static void
linear4_drawvertline(PSD psd, MWCOORD x, MWCOORD y1, MWCOORD y2, MWPIXELVAL c)
{
	ADDR8	addr = psd->addr;
	int	linelen = psd->linelen;

	assert (addr != 0);
	assert (x >= 0 && x < psd->xres);
	assert (y1 >= 0 && y1 < psd->yres);
	assert (y2 >= 0 && y2 < psd->yres);
	assert (y2 >= y1);
	assert (c < psd->ncolors);

        INVERT(c);
	DRAWON;
	addr += (x>>1) + y1 * linelen;
	if(gr_mode == MWMODE_XOR)
		while(y1++ <= y2) {
			*addr ^= c << ((1-(x&1))<<2);
			addr += linelen;
		}
	else
		while(y1++ <= y2) {
			*addr = (*addr & notmask[x&1]) | (c << ((1-(x&1))<<2));
			addr += linelen;
		}
	DRAWOFF;
}

/* srccopy bitblt, opcode is currently ignored*/
static void
linear4_blit(PSD dstpsd, MWCOORD dstx, MWCOORD dsty, MWCOORD w, MWCOORD h,
	PSD srcpsd, MWCOORD srcx, MWCOORD srcy, long op)
{
	ADDR8	dst;
	ADDR8	src;
	int	i;
	int	dlinelen = dstpsd->linelen;
	int	slinelen = srcpsd->linelen;

	assert (dstpsd->addr != 0);
	assert (dstx >= 0 && dstx < dstpsd->xres);
	assert (dsty >= 0 && dsty < dstpsd->yres);
	assert (w > 0);
	assert (h > 0);
	assert (srcpsd->addr != 0);
	assert (srcx >= 0 && srcx < srcpsd->xres);
	assert (srcy >= 0 && srcy < srcpsd->yres);
	assert (dstx+w <= dstpsd->xres);
	assert (dsty+h <= dstpsd->yres);
	assert (srcx+w <= srcpsd->xres);
	assert (srcy+h <= srcpsd->yres);

	DRAWON;
	dst = ((ADDR8)dstpsd->addr) + (dstx>>1) + dsty * dlinelen;
	src = ((ADDR8)srcpsd->addr) + (srcx>>1) + srcy * slinelen;
	while(--h >= 0) {
		ADDR8	d = dst;
		ADDR8	s = src;
		MWCOORD	dx = dstx;
		MWCOORD	sx = srcx;
		for(i=0; i<w; ++i) {
#if INVERT4BPP
			unsigned char c = *s;
			INVERT(c);
			*d = (*d & notmask[dx&1]) |
			   ((c >> ((1-(sx&1))<<2) & 0x0f) << ((1-(dx&1))<<2));
#else
			*d = (*d & notmask[dx&1]) |
			   ((*s >> ((1-(sx&1))<<2) & 0x0f) << ((1-(dx&1))<<2));
#endif
			if((++dx & 1) == 0)
				++d;
			if((++sx & 1) == 0)
				++s;
		}
		dst += dlinelen;
		src += slinelen;
	}
	DRAWOFF;
}

SUBDRIVER fblinear4 = {
	linear4_init,
	linear4_drawpixel,
	linear4_readpixel,
	linear4_drawhorzline,
	linear4_drawvertline,
	gen_fillrect,
	linear4_blit
};
