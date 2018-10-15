/* ------------------------------------------------------------------------ */
/* LHa for UNIX    															*/
/*				lhlist.c -- LHarc list										*/
/*																			*/
/*		Copyright (C) MCMLXXXIX Yooichi.Tagawa								*/
/*		Modified          		Nobutaka Watazaki							*/
/*																			*/
/*  Ver. 0.00	Original						1988.05.23  Y.Tagawa		*/
/*  Ver. 1.00	Fixed							1989.09.22  Y.Tagawa		*/
/*  Ver. 1.01	Bug Fix for month name			1989.12.25  Y.Tagawa		*/
/*  Ver. 1.10	Changed list format				1993.10.01	N.Watazaki		*/
/*	Ver. 1.14	Source All chagned				1995.01.14	N.Watazaki		*/
/*	Ver. 1.14e	Bug Fix for many problems		1999.05.25  T.Okamoto       */
/* ------------------------------------------------------------------------ */
#include "lha.h"

/* ------------------------------------------------------------------------ */
static long     packed_size_total;
static long     original_size_total;
static int      list_files;

/* ------------------------------------------------------------------------ */
/* Print Stuff																*/
/* ------------------------------------------------------------------------ */
/* need 14 or 22 (when verbose_listing is TRUE) column spaces */
static void
print_size(packed_size, original_size)
	long            packed_size, original_size;
{
	if (verbose_listing)
		printf("%7d ", packed_size);

	printf("%7d ", original_size);

	if (original_size == 0L)
		printf("******");
	else	/* Changed N.Watazaki */
		printf("%5.1f%%", packed_size * 100.0 / original_size);
}

/* ------------------------------------------------------------------------ */
/* need 12 or 17 (when verbose_listing is TRUE) column spaces */
static void
print_stamp(t)
	time_t          t;
{
	static boolean  got_now = FALSE;
	static time_t   now;
	static unsigned int threshold;
	static char     t_month[12 * 3 + 1] = "JanFebMarAprMayJunJulAugSepOctNovDec";
	struct tm      *p;

	if (t == 0) {
		printf("            ");	/* 12 spaces */
		return;
	}

	if (!got_now) {
		now = time((time_t *) 0);
		p = localtime(&now);
		threshold = p->tm_year * 12 + p->tm_mon - 6;
		got_now = TRUE;
	}

	p = localtime(&t);

	if (p->tm_year * 12 + p->tm_mon > threshold)
		printf("%.3s %2d %02d:%02d",
		&t_month[p->tm_mon * 3], p->tm_mday, p->tm_hour, p->tm_min);
	else
		printf("%.3s %2d  %04d",
		    &t_month[p->tm_mon * 3], p->tm_mday, p->tm_year + 1900);
}

/* ------------------------------------------------------------------------ */
static void
print_bar()
{
	char           *p, *q;
	/* 17+1+(0 or 7+1)+7+1+6+1+(0 or 1+4)+(12 or 17)+1+20 */
	/* 12345678901234567_  1234567_123456  _123456789012   1234 */

	if (verbose_listing) {
		p = "- ------ ---------- ";
		q = " -------------";
	}
	else {
		p = " ";
		q = " --------------------";
	}

	if (verbose)
		q = "";

	printf("---------- ----------- ------- ------%s------------%s\n", p, q);
}

/* ------------------------------------------------------------------------ */
/*																			*/
/* ------------------------------------------------------------------------ */
static void
list_header()
{
	char           *p, *q;

	if (verbose_listing) {
		p = "PACKED    SIZE  RATIO METHOD CRC";
		q = "          NAME";
	}
	else {
		p = "  SIZE  RATIO";
		q = "           NAME";
	}

	if (verbose)
		q = "";

	printf(" PERMSSN    UID  GID    %s     STAMP%s\n", p, q);
#if 0
	printf(" PERMSSN  UID GID %s   SIZE  RATIO%s %s    STAMP%s%s\n",
	       verbose_listing ? " PACKED " : "",	/* 8,0 */
	       verbose_listing ? "  CRC" : "",	/* 5,0 */
	       verbose_listing ? "  " : "",	/* 2,0 */
	       verbose_listing ? "      " : "   ",	/* 6,3 */
	       verbose ? "" : " NAME");
#endif
	print_bar();
}

/* ------------------------------------------------------------------------ */
static void
list_one(hdr)
	register LzHeader *hdr;
{
	register int    mode;
	register char  *p;
	char            method[6];
	char modebits[11];

	if (verbose)
		printf("%s\n", hdr->name);

	strncpy(method, hdr->method, 5);
	method[5] = '\0';

	switch (mode = hdr->extend_type) {
	case EXTEND_UNIX:
		mode = hdr->unix_mode;

		if (mode & UNIX_FILE_DIRECTORY)
			modebits[0] = 'd';
		else if ((mode & UNIX_FILE_SYMLINK) == UNIX_FILE_SYMLINK)
			modebits[0] = 'l';
		else
			modebits[0] = '-';
		modebits[1] = ((mode & UNIX_OWNER_READ_PERM) ? 'r' : '-');
		modebits[2] = ((mode & UNIX_OWNER_WRITE_PERM) ? 'w' : '-');
		modebits[3] = (mode & UNIX_SETUID) ? 's' :
			((mode & UNIX_OWNER_EXEC_PERM) ? 'x' : '-');
		modebits[4] = ((mode & UNIX_GROUP_READ_PERM) ? 'r' : '-');
		modebits[5] = ((mode & UNIX_GROUP_WRITE_PERM) ? 'w' : '-');
		modebits[6] = (mode & UNIX_SETGID) ? 's' :
			((mode & UNIX_GROUP_EXEC_PERM) ? 'x' : '-');
		modebits[7] = ((mode & UNIX_OTHER_READ_PERM) ? 'r' : '-');
		modebits[8] = ((mode & UNIX_OTHER_WRITE_PERM) ? 'w' : '-');
		modebits[9] = (mode & UNIX_STYCKYBIT) ? 't' :
			((mode & UNIX_OTHER_EXEC_PERM) ? 'x' : '-');
		modebits[10] = 0;

		printf("%s %5d/%-5d ", modebits,
		       hdr->unix_uid, hdr->unix_gid);
		break;
	case EXTEND_OS68K:
	 /**/ case EXTEND_XOSK:/**/
		mode = hdr->unix_mode;
		printf("%c%c%c%c%c%c%c%c %5d/%-5d",
		       ((mode & OSK_DIRECTORY_PERM) ? 'd' : '-'),
		       ((mode & OSK_SHARED_PERM) ? 's' : '-'),
		       ((mode & OSK_OTHER_EXEC_PERM) ? 'e' : '-'),
		       ((mode & OSK_OTHER_WRITE_PERM) ? 'w' : '-'),
		       ((mode & OSK_OTHER_READ_PERM) ? 'r' : '-'),
		       ((mode & OSK_OWNER_EXEC_PERM) ? 'e' : '-'),
		       ((mode & OSK_OWNER_WRITE_PERM) ? 'w' : '-'),
		       ((mode & OSK_OWNER_READ_PERM) ? 'r' : '-'),
		       hdr->unix_uid, hdr->unix_gid);
		break;
	default:
		switch (hdr->extend_type) {	/* max 18 characters */
		case EXTEND_GENERIC:
			p = "[generic]";
			break;
		case EXTEND_CPM:
			p = "[CP/M]";
			break;
		case EXTEND_FLEX:
			p = "[FLEX]";
			break;
		case EXTEND_OS9:
			p = "[OS-9]";
			break;
		case EXTEND_OS68K:
			p = "[OS-9/68K]";
			break;
		case EXTEND_MSDOS:
			p = "[MS-DOS]";
			break;
		case EXTEND_MACOS:
			p = "[Mac OS]";
			break;
		case EXTEND_OS2:
			p = "[OS/2]";
			break;
		case EXTEND_HUMAN:
			p = "[Human68K]";
			break;
		case EXTEND_OS386:
			p = "[OS-386]";
			break;
		case EXTEND_RUNSER:
			p = "[Runser]";
			break;
#ifdef EXTEND_TOWNSOS
			/* This ID isn't fixed */
		case EXTEND_TOWNSOS:
			p = "[TownsOS]";
			break;
#endif
			/* Ouch!  Please customize it's ID.  */
		default:
			p = "[unknown]";
			break;
		}
		printf("%-23.23s", p);
		break;
	}

	print_size(hdr->packed_size, hdr->original_size);

	if (verbose_listing)
		if (hdr->has_crc)
			printf(" %s %04x", method, hdr->crc);
		else
			printf(" %s ****", method);

	printf(" ");
	print_stamp(hdr->unix_last_modified_stamp);

	if (!verbose)
		if ((mode & UNIX_FILE_SYMLINK) != UNIX_FILE_SYMLINK)
			printf(" %s", hdr->name);
		else {
			char            buf[256], *b1, *b2;
			strcpy(buf, hdr->name);
			b1 = strtok(buf, "|");
			b2 = strtok(NULL, "|");
			printf(" %s -> %s", b1, b2);
		}

	if (verbose)
		printf(" [%d]", hdr->header_level);
	printf("\n");

}

/* ------------------------------------------------------------------------ */
static void
list_tailer()
{
	struct stat     stbuf;

	print_bar();

	printf(" Total %9d file%c ",
	       list_files, (list_files == 1) ? ' ' : 's');
	print_size(packed_size_total, original_size_total);
	printf(" ");

	if (verbose_listing)
		printf("           ");

	if (stat(archive_name, &stbuf) < 0)
		print_stamp((time_t) 0);
	else
		print_stamp(stbuf.st_mtime);

	printf("\n");
}

/* ------------------------------------------------------------------------ */
/* LIST COMMAND MAIN														*/
/* ------------------------------------------------------------------------ */
void
cmd_list()
{
	FILE           *afp;
	LzHeader        hdr;
	int             i;

	/* initialize total count */
	packed_size_total = 0L;
	original_size_total = 0L;
	list_files = 0;

	/* open archive file */
	if ((afp = open_old_archive()) == NULL) {
		error(archive_name, "");
		exit(1);
	}
	if (archive_is_msdos_sfx1(archive_name))
		skip_msdos_sfx1_code(afp);

	/* print header message */
	if (!quiet)
		list_header();

	/* print each file information */
	while (get_header(afp, &hdr)) {
		if (need_file(hdr.name)) {
			list_one(&hdr);
			list_files++;
			packed_size_total += hdr.packed_size;
			original_size_total += hdr.original_size;
		}

		if (afp != stdin)
			fseek(afp, hdr.packed_size, SEEK_CUR);
		else {
			i = hdr.packed_size;
			while (i--)
				fgetc(afp);
		}
	}

	/* close archive file */
	fclose(afp);

	/* print tailer message */
	if (!quiet)
		list_tailer();

	return;
}

/* Local Variables: */
/* mode:c */
/* tab-width:4 */
/* compile-command:"gcc -c lhlist.c" */
/* End: */
