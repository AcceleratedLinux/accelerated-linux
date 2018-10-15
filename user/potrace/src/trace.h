/* Copyright (C) 2001-2007 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

/* $Id: trace.h 147 2007-04-09 00:44:09Z selinger $ */

#ifndef TRACE_H
#define TRACE_H

#include "potracelib.h"
#include "progress.h"

int process_path(path_t *plist, const potrace_param_t *param, progress_t *progress);

#endif /* TRACE_H */
