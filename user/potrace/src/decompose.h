/* Copyright (C) 2001-2007 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

/* $Id: decompose.h 147 2007-04-09 00:44:09Z selinger $ */

#ifndef DECOMPOSE_H
#define DECOMPOSE_H

#include "potracelib.h"
#include "progress.h"

int bm_to_pathlist(const potrace_bitmap_t *bm, path_t **plistp, const potrace_param_t *param, progress_t *progress);

#endif /* DECOMPOSE_H */

