/*
 *  Copyright (C) 2007-2009 Sourcefire, Inc.
 *
 *  Authors: Tomasz Kojm
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301, USA.
 */

#ifndef __GLOBAL_H
#define __GLOBAL_H

struct s_info {
    unsigned int sigs;		/* number of signatures */
    unsigned int dirs;		/* number of scanned directories */
    unsigned int files;		/* number of scanned files */
    unsigned int ifiles;	/* number of infected files */
    unsigned long int blocks;	/* number of *scanned* 16kb blocks */
    unsigned long int rblocks;	/* number of *read* 16kb blocks */
};

extern struct s_info info;
extern short recursion, printinfected, bell;

#endif
