
/***************************************************************************
 * osscan.h -- Routines used for OS detection via TCP/IP fingerprinting.   *
 * For more information on how this works in Nmap, see my paper at         *
 * http://www.insecure.org/nmap/nmap-fingerprinting-article.html           *
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

/* $Id: osscan.h 4004 2006-09-25 09:08:56Z fyodor $ */

#ifndef OSSCAN_H
#define OSSCAN_H

#include "nmap.h"
#include "tcpip.h"
#include "global_structures.h"
#include "FingerPrintResults.h"

#define OSSCAN_SUCCESS 0
#define OSSCAN_NOMATCHES -1
#define OSSCAN_TOOMANYMATCHES -2

/* We won't even consider matches with a lower accuracy than this */
#define OSSCAN_GUESS_THRESHOLD 0.85

/**********************  STRUCTURES  ***********************************/

/* moved to global_structures.h */

/**********************  PROTOTYPES  ***********************************/
int os_scan(Target *target);
char *fp2ascii(FingerPrint *FP);

/* Parses a single fingerprint from the memory region given.  If a
 non-null fingerprint is returned, the user is in charge of freeing it
 when done.  This function does not require the fingerprint to be 100%
 complete since it is used by scripts such as scripts/fingerwatch for
 which some partial fingerpritns are OK. */
FingerPrint *parse_single_fingerprint(char *fprint_orig);

/* These functions take a file/db name and open+parse it, returning an
   (allocated) FingerPrintDB containing the results.  They exit with
   an error message in the case of error. */
FingerPrintDB *parse_fingerprint_file(char *fname);
FingerPrintDB *parse_fingerprint_reference_file(char *dbname);

void free_fingerprint_file(FingerPrintDB *DB);

/* Compares 2 fingerprints -- a referenceFP (can have expression
   attributes) with an observed fingerprint (no expressions).  If
   verbose is nonzero, differences will be printed.  The comparison
   accuracy (between 0 and 1) is returned).  If MatchPoints is not NULL, it is 
   a special "fingerprints" which tells how many points each test is worth. */
double compare_fingerprints(FingerPrint *referenceFP, FingerPrint *observedFP,
			    FingerPrint *MatchPoints, int verbose);

/* Takes a fingerprint and looks for matches inside the passed in
   reference fingerprint DB.  The results are stored in in FPR (which
   must point to an instantiated FingerPrintResults class) -- results
   will be reverse-sorted by accuracy.  No results below
   accuracy_threshhold will be included.  The max matches returned is
   the maximum that fits in a FingerPrintResults class.  */
void match_fingerprint(FingerPrint *FP, FingerPrintResults *FPR, 
		       FingerPrintDB *DB, double accuracy_threshold);

/* Returns true if perfect match -- if num_subtests & num_subtests_succeeded are non_null it updates them.  if shortcircuit is zero, it does all the tests, otherwise it returns when the first one fails */

void freeFingerPrint(FingerPrint *FP);
char *mergeFPs(FingerPrint *FPs[], int numFPs, bool isGoodFP, const struct in_addr * const addr, int distance, const u8 *mac, int openTcpPort, int closedTcpPort, int closedUdpPort, bool wrapit);

/* This function takes an array of "numSamples" IP IDs and analyzes
 them to determine their sequenceability classification.  It returns
 one of the IPID_SEQ_* classifications defined in nmap.h .  If the
 function cannot determine the sequence, IPID_SEQ_UNKNOWN is returned.
 This islocalhost argument is a boolean specifying whether these
 numbers were generated by scanning localhost.  NOTE: the "ipids" argument
 may be modified if localhost is set to true. */
int ipid_sequence(int numSamples, u16 *ipids, int islocalhost);

#endif /*OSSCAN_H*/





