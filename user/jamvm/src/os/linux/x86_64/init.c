/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007
 * Robert Lougher <rob@lougher.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef __linux__
#include <fpu_control.h>
#endif

/* Change the x87 FPU precision to double (64-bit) from the extended
   (80-bit) Linux default.  Note, unlike on i386, my testcases pass
   without this.  This is probably because gcc assumes and uses SSE
   by default, not the x87 FPU.  However, keep it in to be safe.
*/

void setDoublePrecision() {
#ifdef __linux__
    fpu_control_t cw;

    _FPU_GETCW(cw);
    cw &= ~_FPU_EXTENDED;
    cw |= _FPU_DOUBLE;
    _FPU_SETCW(cw);
#endif
}

void initialisePlatform() {
    setDoublePrecision();
}
