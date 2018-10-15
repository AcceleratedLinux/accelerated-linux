/* Copyright (C) 2001-2007 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */

/* $Id: backend_pdf.h 147 2007-04-09 00:44:09Z selinger $ */

#ifndef BACKEND_PDF_H
#define BACKEND_PDF_H

#include "potracelib.h"
#include "main.h"

int init_pdf(FILE *fout);
int page_pdf(FILE *fout, potrace_path_t *plist, imginfo_t *imginfo);
int term_pdf(FILE *fout);

int page_pdf(FILE *fout, potrace_path_t *plist, imginfo_t *imginfo);

#endif /* BACKEND_PDF_H */

