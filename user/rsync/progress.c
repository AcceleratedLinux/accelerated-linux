/*  -*- c-file-style: "linux" -*-
 *
 * Copyright (C) 1996-2000 by Andrew Tridgell
 * Copyright (C) Paul Mackerras 1996
 * Copyright (C) 2001, 2002 by Martin Pool <mbp@samba.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "rsync.h"

extern struct stats stats;
extern int am_server;

#define PROGRESS_HISTORY_SECS 5

#ifdef GETPGRP_VOID
#define GETPGRP_ARG
#else
#define GETPGRP_ARG 0
#endif

struct progress_history {
	struct timeval time;
	OFF_T ofs;
};

static struct progress_history ph_start;
static struct progress_history ph_list[PROGRESS_HISTORY_SECS];
static int newest_hpos, oldest_hpos;

static unsigned long msdiff(struct timeval *t1, struct timeval *t2)
{
	return (t2->tv_sec - t1->tv_sec) * 1000L
	     + (t2->tv_usec - t1->tv_usec) / 1000;
}


/**
 * @param ofs Current position in file
 * @param size Total size of file
 * @param is_last True if this is the last time progress will be
 * printed for this file, so we should output a newline.  (Not
 * necessarily the same as all bytes being received.)
 **/
static void rprint_progress(OFF_T ofs, OFF_T size, struct timeval *now,
			    int is_last)
{
	char eol[256];
	const char *units;
	int pct = ofs == size ? 100 : (int) (100.0 * ofs / size);
	unsigned long diff;
	double rate, remain;
	int remain_h, remain_m, remain_s;

	if (is_last) {
		/* Compute stats based on the starting info. */
		if (!ph_start.time.tv_sec
		    || !(diff = msdiff(&ph_start.time, now)))
			diff = 1;
		rate = (double) (ofs - ph_start.ofs) * 1000.0 / diff / 1024.0;
		/* Switch to total time taken for our last update. */
		remain = (double) diff / 1000.0;
	} else {
		/* Compute stats based on recent progress. */
		if (!(diff = msdiff(&ph_list[oldest_hpos].time, now)))
			diff = 1;
		rate = (double) (ofs - ph_list[oldest_hpos].ofs) * 1000.0
		     / diff / 1024.0;
		remain = rate ? (double) (size - ofs) / rate / 1000.0 : 0.0;
	}

	if (rate > 1024*1024) {
		rate /= 1024.0 * 1024.0;
		units = "GB/s";
	} else if (rate > 1024) {
		rate /= 1024.0;
		units = "MB/s";
	} else {
		units = "kB/s";
	}

	remain_s = (int) remain % 60;
	remain_m = (int) (remain / 60.0) % 60;
	remain_h = (int) (remain / 3600.0);

	if (is_last) {
		snprintf(eol, sizeof eol, "  (%d, %.1f%% of %d)\n",
			stats.num_transferred_files,
			(float)((stats.current_file_index+1) * 100)
				/ stats.num_files,
			stats.num_files);
	} else
		strcpy(eol, "\r");
	rprintf(FINFO, "%12.0f %3d%% %7.2f%s %4d:%02d:%02d%s",
		(double) ofs, pct, rate, units,
		remain_h, remain_m, remain_s, eol);
}

void end_progress(OFF_T size)
{
	if (!am_server) {
		struct timeval now;
		gettimeofday(&now, NULL);
		rprint_progress(size, size, &now, True);
	}
	memset(&ph_start, 0, sizeof ph_start);
}

void show_progress(OFF_T ofs, OFF_T size)
{
	struct timeval now;
#if defined HAVE_GETPGRP && defined HAVE_TCGETPGRP
	static pid_t pgrp = -1;
	pid_t tc_pgrp;
#endif

	if (am_server)
		return;

#if defined HAVE_GETPGRP && defined HAVE_TCGETPGRP
	if (pgrp == -1)
		pgrp = getpgrp(GETPGRP_ARG);
#endif

	gettimeofday(&now, NULL);

	if (!ph_start.time.tv_sec) {
		int i;

		/* Try to guess the real starting time when the sender started
		 * to send us data by using the time we last received some data
		 * in the last file (if it was recent enough). */
		if (msdiff(&ph_list[newest_hpos].time, &now) <= 1500) {
			ph_start.time = ph_list[newest_hpos].time;
			ph_start.ofs = 0;
		} else {
			ph_start.time.tv_sec = now.tv_sec;
			ph_start.time.tv_usec = now.tv_usec;
			ph_start.ofs = ofs;
		}

		for (i = 0; i < PROGRESS_HISTORY_SECS; i++)
			ph_list[i] = ph_start;
	}
	else {
		if (msdiff(&ph_list[newest_hpos].time, &now) < 1000)
			return;

		newest_hpos = oldest_hpos;
		oldest_hpos = (oldest_hpos + 1) % PROGRESS_HISTORY_SECS;
		ph_list[newest_hpos].time.tv_sec = now.tv_sec;
		ph_list[newest_hpos].time.tv_usec = now.tv_usec;
		ph_list[newest_hpos].ofs = ofs;
	}

#if defined HAVE_GETPGRP && defined HAVE_TCGETPGRP
	tc_pgrp = tcgetpgrp(STDOUT_FILENO);
	if (tc_pgrp != pgrp && tc_pgrp != -1)
		return;
#endif

	rprint_progress(ofs, size, &now, False);
}
