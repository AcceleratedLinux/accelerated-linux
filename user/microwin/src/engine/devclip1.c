/*
 * Copyright (c) 1999 Greg Haerr <greg@censoft.com>
 * Copyright (c) 1991 David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 *
 * Device-independent routines to determine clipping regions.
 */
#include "device.h"

/* Clip cache rectangle information.
 * After calling GdClipPoint, this rectangle is guaranteed to contain the
 * specified point (among others), and all points in the rectangle are
 * plottable or not according to the value of clipresult.
 */
MWCOORD clipminx;		/* minimum x value of cache rectangle */
MWCOORD clipminy;		/* minimum y value of cache rectangle */
MWCOORD clipmaxx;		/* maximum x value of cache rectangle */
MWCOORD clipmaxy;		/* maximum y value of cache rectangle */

static MWBOOL	clipresult;	/* whether clip rectangle is plottable */
int 	clipcount;		/* number of clip rectangles */
MWCLIPRECT cliprects[MAX_CLIPRECTS];	/* clip rectangles */

/**
 * Set an array of clip rectangles for future drawing actions.
 * Each pixel will be drawn only if lies in one or more of the specified
 * clip rectangles.  As a special case, specifying no rectangles implies
 * clipping is for the complete screen.  All clip rectangles are modified
 * if necessary to lie within the device area.  Call only after device
 * has been initialized.
 *
 * @param psd Drawing surface.
 * @param count Number of clip rectangles
 * @param table Clipping rectangles.
 */
void
GdSetClipRects(PSD psd,int count, MWCLIPRECT *table)
{
  register MWCLIPRECT *rp;		/* current rectangle */

  /* If there are no clip rectangles, then default to the full device area. */
  if (count <= 0) {
	clipminx = 0;
	clipminy = 0;
	clipmaxx = psd->xvirtres - 1;
	clipmaxy = psd->yvirtres - 1;
	clipcount = 0;
	clipresult = TRUE;
	return;
  }

  /* Copy the clip table to our own static array, modifying each
   * rectangle as necesary to fit within the device area.  If the clip
   * rectangle lies entirely outside of the device area, then skip it.
   */
  rp = cliprects;
  clipcount = 0;
  if (count > MAX_CLIPRECTS) count = MAX_CLIPRECTS;
  while (count-- > 0) {
	*rp = *table++;
	if (rp->x < 0) {
		rp->width += rp->x;
		rp->x = 0;
	}
	if (rp->y < 0) {
		rp->height += rp->y;
		rp->y = 0;
	}
	if ((rp->x >= psd->xvirtres) || (rp->width <= 0) ||
	    (rp->y >= psd->yvirtres) || (rp->height <= 0))
		continue;
	if (rp->x + rp->width > psd->xvirtres)
		rp->width = psd->xvirtres - rp->x;
	if (rp->y + rp->height > psd->yvirtres)
		rp->height = psd->yvirtres - rp->y;
	rp++;
	clipcount++;
  }

  /* If there were no surviving clip rectangles, then set the clip
   * cache to prevent all drawing.
   */
  if (clipcount == 0) {
	clipminx = MIN_MWCOORD;
	clipminy = MIN_MWCOORD;
	clipmaxx = MAX_MWCOORD;
	clipmaxy = MAX_MWCOORD;
	clipresult = FALSE;
	return;
  }

  /* There was at least one valid clip rectangle. Default the clip
   * cache to be the first clip rectangle.
   */
  clipminx = cliprects[0].x;
  clipminy = cliprects[0].y;
  clipmaxx = clipminx + cliprects[0].width - 1;
  clipmaxy = clipminy + cliprects[0].height - 1;
  clipresult = TRUE;
}


/**
 * Check a point against the list of clip rectangles.
 * Returns TRUE if the point is within one or more rectangles and thus
 * can be plotted, or FALSE if the point is not within any rectangle and
 * thus cannot be plotted.  Also remembers the coordinates of a clip cache
 * rectangle containing the specified point such that every point in the
 * rectangle would give the same result.  By examining this clip cache
 * rectangle after a call to this routine, the caller can efficiently
 * check many nearby points without needing any further calls.  If the
 * point lies within the cursor, then the cursor is removed.
 *
 * @param psd Drawing surface.
 * @param x X co-ordintat of point to check.
 * @param y Y co-ordinate of point to check.
 * @return TRUE iff the point is visible.
 */
MWBOOL
GdClipPoint(PSD psd,MWCOORD x,MWCOORD y)
{
  int count;
  MWCLIPRECT *rp;
  MWCOORD temp;

  /* First see whether the point lies within the current clip cache
   * rectangle.  If so, then we already know the result.
   */
  if ((x >= clipminx) && (x <= clipmaxx) &&
      (y >= clipminy) && (y <= clipmaxy)) {
	if (clipresult) GdCheckCursor(psd, x, y, x, y);
	return clipresult;
  }

  /* If the point is outside of the screen area, then it is not
   * plottable, and the clip cache rectangle is the whole half-plane
   * outside of the screen area.
   */
  if (x < 0) {
	clipminx = MIN_MWCOORD;
	clipmaxx = -1;
	clipminy = MIN_MWCOORD;
	clipmaxy = MAX_MWCOORD;
	clipresult = FALSE;
	return FALSE;
  }
  if (y < 0) {
	clipminx = MIN_MWCOORD;
	clipmaxx = MAX_MWCOORD;
	clipminy = MIN_MWCOORD;
	clipmaxy = -1;
	clipresult = FALSE;
	return FALSE;
  }
  if (x >= psd->xvirtres) {
	clipminx = psd->xvirtres;
	clipmaxx = MAX_MWCOORD;
	clipminy = MIN_MWCOORD;
	clipmaxy = MAX_MWCOORD;
	clipresult = FALSE;
	return FALSE;
  }
  if (y >= psd->yvirtres) {
	clipminx = MIN_MWCOORD;
	clipmaxx = MAX_MWCOORD;
	clipminy = psd->yvirtres;
	clipmaxy = MAX_MWCOORD;
	clipresult = FALSE;
	return FALSE;
  }

  /* The point is within the screen area. If there are no clip
   * rectangles, then the point is plottable and the rectangle is the
   * whole screen.
   */
  count = clipcount;
  if (count <= 0) {
	clipminx = 0;
	clipmaxx = psd->xvirtres - 1;
	clipminy = 0;
	clipmaxy = psd->yvirtres - 1;
	clipresult = TRUE;
	GdCheckCursor(psd, x, y, x, y);
	return TRUE;
  }

  /* We need to scan the list of clip rectangles to calculate a new
   * clip cache rectangle containing this point, and the result. First
   * see if the point lies within any of the clip rectangles. If so,
   * then it is plottable and use that clip rectangle as the cache
   * rectangle.  This is not necessarily the best result, but works ok
   * and is fast.
   */
  for (rp = cliprects; count-- > 0; rp++) {
	if ((x >= rp->x) && (y >= rp->y) && (x < rp->x + rp->width)
	    && (y < rp->y + rp->height)) {
		clipminx = rp->x;
		clipminy = rp->y;
		clipmaxx = rp->x + rp->width - 1;
		clipmaxy = rp->y + rp->height - 1;
		clipresult = TRUE;
		GdCheckCursor(psd, x, y, x, y);
		return TRUE;
	}
  }

  /* The point is not plottable. Scan the clip rectangles again to
   * determine a rectangle containing more non-plottable points.
   * Simply pick the largest rectangle whose area doesn't contain any
   * of the same coordinates as appropriate sides of the clip
   * rectangles.  This is not necessarily the best result, but works ok
   * and is fast.
   */
  clipminx = MIN_MWCOORD;
  clipminy = MIN_MWCOORD;
  clipmaxx = MAX_MWCOORD;
  clipmaxy = MAX_MWCOORD;
  count = clipcount;
  for (rp = cliprects; count-- > 0; rp++) {
	if ((x < rp->x) && (rp->x <= clipmaxx)) clipmaxx = rp->x - 1;
	temp = rp->x + rp->width - 1;
	if ((x > temp) && (temp >= clipminx)) clipminx = temp + 1;
	if ((y < rp->y) && (rp->y <= clipmaxy)) clipmaxy = rp->y - 1;
	temp = rp->y + rp->height - 1;
	if ((y > temp) && (temp >= clipminy)) clipminy = temp + 1;
  }
  clipresult = FALSE;
  return FALSE;
}


/**
 * Check the area determined by the specified pair of points against the
 * list of clip rectangles.  The area will either be totally visible,
 * totally visible, or possibly partially visible.  This routine updates
 * the clip cache rectangle, and returns one of the following values:
 *	CLIP_VISIBLE		The whole rectangle is visible
 *	CLIP_INVISIBLE		The whole rectangle is invisible
 *	CLIP_PARTIAL		The rectangle may be partially visible
 * In the case that the area is totally visible, the cursor is removed
 * if it overlaps the clip area.
 *
 * @param psd Drawing surface.
 * @param x1 Left of rectangle.
 * @param y1 Top of rectangle.
 * @param x2 Right of rectangle.
 * @param y2 Bottom of rectangle.
 * @return CLIP_VISIBLE, CLIP_INVISIBLE, or CLIP_PARTIAL.
 */
int
GdClipArea(PSD psd,MWCOORD x1, MWCOORD y1, MWCOORD x2, MWCOORD y2)
{
  if ((x1 < clipminx) || (x1 > clipmaxx) ||
      (y1 < clipminy) || (y1 > clipmaxy))
	GdClipPoint(psd, x1, y1);

  if ((x2 >= clipminx) && (x2 <= clipmaxx) &&
      (y2 >= clipminy) && (y2 <= clipmaxy)) {
	if (!clipresult) return CLIP_INVISIBLE;
	GdCheckCursor(psd, x1, y1, x2, y2);
	return CLIP_VISIBLE;
  }
  return CLIP_PARTIAL;
}
