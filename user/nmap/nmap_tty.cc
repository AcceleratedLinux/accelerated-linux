/***************************************************************************
 * nmap_tty.cc -- Handles runtime interaction with Nmap, so you can        *
 * increase verbosity/debugging or obtain a status line upon request.      *
 *                                                                         *
 ***********************IMPORTANT NMAP LICENSE TERMS************************
 *                                                                         *
 * The Nmap Security Scanner is (C) 1996-2006 Insecure.Com LLC. Nmap is    *
 * also a registered trademark of Insecure.Com LLC.  This program is free  *
 * software; you may redistribute and/or modify it under the terms of the  *
 * GNU General Public License as published by the Free Software            *
 * Foundation; Version 2 with the clarifications and exceptions described  *
 * below.  This guarantees your right to use, modify, and redistribute     *
 * this software under certain conditions.  If you wish to embed Nmap      *
 * technology into proprietary software, we sell alternative licenses      *
 * (contact sales@insecure.com).  Dozens of software vendors already       *
 * license Nmap technology such as host discovery, port scanning, OS       *
 * detection, and version detection.                                       *
 *                                                                         *
 * Note that the GPL places important restrictions on "derived works", yet *
 * it does not provide a detailed definition of that term.  To avoid       *
 * misunderstandings, we consider an application to constitute a           *
 * "derivative work" for the purpose of this license if it does any of the *
 * following:                                                              *
 * o Integrates source code from Nmap                                      *
 * o Reads or includes Nmap copyrighted data files, such as                *
 *   nmap-os-fingerprints or nmap-service-probes.                          *
 * o Executes Nmap and parses the results (as opposed to typical shell or  *
 *   execution-menu apps, which simply display raw Nmap output and so are  *
 *   not derivative works.)                                                * 
 * o Integrates/includes/aggregates Nmap into a proprietary executable     *
 *   installer, such as those produced by InstallShield.                   *
 * o Links to a library or executes a program that does any of the above   *
 *                                                                         *
 * The term "Nmap" should be taken to also include any portions or derived *
 * works of Nmap.  This list is not exclusive, but is just meant to        *
 * clarify our interpretation of derived works with some common examples.  *
 * These restrictions only apply when you actually redistribute Nmap.  For *
 * example, nothing stops you from writing and selling a proprietary       *
 * front-end to Nmap.  Just distribute it by itself, and point people to   *
 * http://insecure.org/nmap/ to download Nmap.                             *
 *                                                                         *
 * We don't consider these to be added restrictions on top of the GPL, but *
 * just a clarification of how we interpret "derived works" as it applies  *
 * to our GPL-licensed Nmap product.  This is similar to the way Linus     *
 * Torvalds has announced his interpretation of how "derived works"        *
 * applies to Linux kernel modules.  Our interpretation refers only to     *
 * Nmap - we don't speak for any other GPL products.                       *
 *                                                                         *
 * If you have any questions about the GPL licensing restrictions on using *
 * Nmap in non-GPL works, we would be happy to help.  As mentioned above,  *
 * we also offer alternative license to integrate Nmap into proprietary    *
 * applications and appliances.  These contracts have been sold to dozens  *
 * of software vendors, and generally include a perpetual license as well  *
 * as providing for priority support and updates as well as helping to     *
 * fund the continued development of Nmap technology.  Please email        *
 * sales@insecure.com for further information.                             *
 *                                                                         *
 * As a special exception to the GPL terms, Insecure.Com LLC grants        *
 * permission to link the code of this program with any version of the     *
 * OpenSSL library which is distributed under a license identical to that  *
 * listed in the included Copying.OpenSSL file, and distribute linked      *
 * combinations including the two. You must obey the GNU GPL in all        *
 * respects for all of the code used other than OpenSSL.  If you modify    *
 * this file, you may extend this exception to your version of the file,   *
 * but you are not obligated to do so.                                     *
 *                                                                         *
 * If you received these files with a written license agreement or         *
 * contract stating terms other than the terms above, then that            *
 * alternative license agreement takes precedence over these comments.     *
 *                                                                         *
 * Source is provided to this software because we believe users have a     *
 * right to know exactly what a program is going to do before they run it. *
 * This also allows you to audit the software for security holes (none     *
 * have been found so far).                                                *
 *                                                                         *
 * Source code also allows you to port Nmap to new platforms, fix bugs,    *
 * and add new features.  You are highly encouraged to send your changes   *
 * to fyodor@insecure.org for possible incorporation into the main         *
 * distribution.  By sending these changes to Fyodor or one the            *
 * Insecure.Org development mailing lists, it is assumed that you are      *
 * offering Fyodor and Insecure.Com LLC the unlimited, non-exclusive right *
 * to reuse, modify, and relicense the code.  Nmap will always be          *
 * available Open Source, but this is important because the inability to   *
 * relicense code has caused devastating problems for other Free Software  *
 * projects (such as KDE and NASM).  We also occasionally relicense the    *
 * code to third parties as discussed above.  If you wish to specify       *
 * special license conditions of your contributions, just say so when you  *
 * send them.                                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * General Public License for more details at                              *
 * http://www.gnu.org/copyleft/gpl.html , or in the COPYING file included  *
 * with Nmap.                                                              *
 *                                                                         *
 ***************************************************************************/

#ifndef WIN32
#include "config.h"
#endif

#include <sys/types.h>
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#if HAVE_TERMIOS_H
#include <termios.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>

#include "output.h"
#include "nmap_tty.h"
#include "NmapOps.h"

extern NmapOps o;

#ifdef WIN32
#include <conio.h>

// Microsoft's runtime makes this fairly simple. :)
void tty_init() { return; }
static int tty_getchar() { return _kbhit() ? _getch() : -1; }
static void tty_done() { return; }

#else
#if !defined(O_NONBLOCK) && defined(O_NDELAY)
#define O_NONBLOCK			O_NDELAY
#endif

#ifdef __CYGWIN32__
#include <string.h>
#include <sys/socket.h>
#ifndef __CYGWIN__
extern int tcgetattr(int fd, struct termios *termios_p);
extern int tcsetattr(int fd, int actions, struct termios *termios_p);
#endif
#endif

static int tty_fd = 0;
static struct termios saved_ti;

static int tty_getchar()
{
	int c, numChars;
#ifdef __CYGWIN32__
	fd_set set;
	struct timeval tv;
#endif
        
	if (tty_fd && tcgetpgrp(tty_fd) == getpid()) {
           
           // This is so that when the terminal has been disconnected, it will be reconnected when possible. If it slows things down, just remove it
           // tty_init();
           
#ifdef __CYGWIN32__
		FD_ZERO(&set); FD_SET(tty_fd, &set);
		tv.tv_sec = 0; tv.tv_usec = 0;
		if (select(tty_fd + 1, &set, NULL, NULL, &tv) <= 0)
			return -1;
#endif
		c = 0;
                numChars = read(tty_fd, &c, 1);
		if (numChars > 0) return c;
	}

	return -1;
}

static void tty_done()
{
	if (!tty_fd) return;

	tcsetattr(tty_fd, TCSANOW, &saved_ti);

	close(tty_fd);
	tty_fd = 0;
}

/*
 * Initializes the terminal for unbuffered non-blocking input. Also
 * registers tty_done() via atexit().  You need to call this before
 * you ever call keyWasPressed().
 */
void tty_init()
{
	struct termios ti;

	if (tty_fd)
		return;

	if ((tty_fd = open("/dev/tty", O_RDONLY | O_NONBLOCK)) < 0) return;

#ifndef __CYGWIN32__
	if (tcgetpgrp(tty_fd) != getpid()) {
		close(tty_fd); return;
	}
#endif

	tcgetattr(tty_fd, &ti);
	saved_ti = ti;
	ti.c_lflag &= ~(ICANON | ECHO);
	ti.c_cc[VMIN] = 1;
	ti.c_cc[VTIME] = 0;
	tcsetattr(tty_fd, TCSANOW, &ti);

	atexit(tty_done);
}

#endif  //!win32

/* Catches all of the predefined
   keypresses and interpret them, and it will also tell you if you
   should print anything. A value of true being returned means a
   nonstandard key has been pressed and the calling method should
   print a status message */
bool keyWasPressed()
{
  int c;

  if (o.noninteractive)
    return false;

  if ((c = tty_getchar()) >= 0) {
    // Eat any extra keys (so they can't queue up and print forever)
    while (tty_getchar() >= 0); 

    // printf("You pressed key '%c'!\n", c);
    if (c == 'v') {
       o.verbose++;
       log_write(LOG_STDOUT, "Verbosity Increased to %d.\n", o.verbose);
    } else if (c == 'V') {
       if (o.verbose > 0)
	 o.verbose--;
       log_write(LOG_STDOUT, "Verbosity Decreased to %d.\n", o.verbose);
    } else if (c == 'd') {
       o.debugging++;
       log_write(LOG_STDOUT, "Debugging Increased to %d.\n", o.debugging);
    } else if (c == 'D') {
       if (o.debugging > 0) o.debugging--;
       log_write(LOG_STDOUT, "Debugging Decreased to %d.\n", o.debugging);
    } else if (c == 'p') {
       o.setPacketTrace(true);
       log_write(LOG_STDOUT, "Packet Tracing enabled.\n");
    } else if (c == 'P') {
       o.setPacketTrace(false);
       log_write(LOG_STDOUT, "Packet Tracing disabled.\n");
    } else if (c == '?') {
      log_write(LOG_STDOUT,
		"Interactive keyboard commands:\n"
		"?               Display this information\n"
		"v/V             Increase/decrease verbosity\n"
		"d/D             Increase/decrease debugging\n"
		"p/P             Enable/disable packet tracing\n"
		"anything else   Print status\n"
                "More help: http://www.insecure.org/nmap/man/man-runtime-interaction.html\n");
    } else {
       printStatusMessage();
       return true;
    }
  }
  return false;
}
