/* inittab editor */

#include <err.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/file.h>

#include "pathnames.h"

#define OP_ADD 'a'
#define OP_DEL 'd'

/* Seek the FILE to a line beginning with "<key>:".
 * The key string is terminated either by ':' or NUL.
 * If found, returns length of the line in bytes (incl \n), and leaves
 * the file pointer at the beginning of the matched line.
 * Otherwise returns 0 and leaves pointer at EOF. */
static int
seek_to_key(FILE *f, const char *key)
{
	const char *k = key; /* NULL when skipping rest of line */
	int ch;
	int len;

	while ((ch = getc(f)) != EOF) {
		if (ch == '\n') {
			/* Begin scanning a new line */
			k = key;
		} else if (!k) {
			/* ignore rest of line */
		} else if (*k == ':' || *k == '\0') {
			if (ch == ':')
				break;
			k = NULL;
		} else if (*k == ch) {
			k++;
		} else {
			k = NULL;
		}
	}
	if (ch == EOF)
		return 0;

	/* Measure length of line */
	len = k - key + 1;
	while ((ch = getc(f)) != EOF) {
		len++;
		if (ch == '\n')
			break;
	}
	if (fseek(f, -len, SEEK_CUR) == -1)
		err(1, "seek");
	return len;
}

/* Inserts <len> undefined bytes at the current file pointer,
 * by copying the moving subsequent file data upwards by <len> bytes.
 * On return the filepointer will appear unchanged, but
 * point at <len> undefined bytes. */
void
insert_bytes(const char *filename, FILE *f, int len)
{
	off_t start, pos;

	start = ftell(f);

	/* Move to the existing end of the file */
	if (fseek(f, 0, SEEK_END) == -1)
		err(1, "seek %s", filename);
	pos = ftell(f);

	/* Expand the file which we hope checks early that
	 * the disk can allocate us the required space */
	if (ftruncate(fileno(f), pos + len) == -1)
		err(1, "ftruncate %s", filename);
	if (fsync(fileno(f)) == -1)
		err(1, "fsync %s", filename);
	fflush(f);

	while (pos > start) {
		char buf[8192];
		int n = sizeof buf;
		if (start + n > pos)
			n = pos - start;
/*		fprintf(stderr, "start=%lu pos=%lu n=%d\n",start,pos,n); */
		pos -= n;
		if (fseek(f, pos, SEEK_SET) == -1)
			err(1, "seek %s", filename);
		if (fread(buf, 1, n, f) != n)
			err(1, "read %s", filename);
		if (fseek(f, pos + len, SEEK_SET) == -1)
			err(1, "seek %s", filename);
		if (fwrite(buf, 1, n, f) != n)
			err(1, "write %s", filename);
	}
	if (fseek(f, start, SEEK_SET) == -1)
		err(1, "seek %s", filename);
}

/* Removes <len> bytes of data at the current file pointer.
 * Moves the subsequent file data backwards by <len> bytes.
 * File pointer remains at same offset from start of file. */
void
delete_bytes(const char *filename, FILE *f, int len)
{
	off_t pos, start;
	int n;
	char buf[8192];

	pos = start = ftell(f);
	if (fseek(f, pos + len, SEEK_SET) == -1)
		err(1, "seek %s", filename);
	while ((n = fread(buf, 1, sizeof buf, f)) != 0) {
		if (fseek(f, pos, SEEK_SET) == -1)
			err(1, "seek %s", filename);
		if (fwrite(buf, 1, n, f) != n)
			err(1, "write %s", filename);
		pos += n;
		if (fseek(f, pos + len, SEEK_SET) == -1)
			err(1, "seek %s", filename);
	}
	if (!feof(f))
		err(1, "read %s", filename);
	fflush(f);
	if (ftruncate(fileno(f), pos) == -1)
		err(1, "ftruncate %s", filename);
	if (fseek(f, start, SEEK_SET) == -1)
		err(1, "seek %s", filename);
}

void
resize_bytes(const char *filename, FILE *f, int oldlen, int newlen)
{
	if (newlen > oldlen)
		insert_bytes(filename, f, newlen - oldlen);
	else if (newlen < oldlen)
		delete_bytes(filename, f, oldlen - newlen);
}

int
main(int argc, char *argv[])
{
	const char *filename = _PATH_VARRUNTAB;
	int error = 0;
	int ch;
	int send_sighup = 1;
	char *line;
	char op;
	int fd;
	FILE *f;
	int len, newlen;

	/* Process command-line arguments */

	while ((ch = getopt(argc, argv, "nf:")) != -1) {
		switch (ch) {
		case 'n':
			send_sighup = 0;
			break;
		case 'f':
			filename = optarg;
			break;
		default:
			error = 1;
		}
	}

	if (optind < argc) {
		const char *cmd = argv[optind++];
		if (strcmp(cmd, "add") == 0 && optind < argc) {
			op = OP_ADD;
			line = argv[optind++];
			if (!strchr(line, ':')) {
				warnx("missing colon after key");
				error = 1;
			}
		} else if (strcmp(cmd, "del") == 0 && optind < argc) {
			op = OP_DEL;
			line = argv[optind++];
			if (strchr(line, ':')) {
				warnx("colon not permitted in key");
				error = 1;
			}
		} else {
			warnx("unknown command '%s'", cmd);
			error = 1;
		}
	} else {
		warnx("missing command");
		error = 1;
	}

	if (optind < argc) {
		warnx("unexpected arguments");
		error = 1;
	}

	if (error) {
		fprintf(stderr,
		"usage: %s [-n] [-f inittab] add <key>:<tty>:<command>\n"
		"       %s [-n] [-f inittab] del <key>\n"
		"  -f inittab    File to edit (default: %s)\n"
		"  -n            Don't send SIGHUP to init\n"
		, argv[0], argv[0], _PATH_VARRUNTAB);
		exit(2);
	}

	/* Open and lock the file for editing */
	fd = open(filename, O_RDWR | O_CREAT, 0666);
	if (fd == -1)
		err(1, "%s", filename);
	f = fdopen(fd, "r+");
	if (!f)
		err(1, "fdopen %s", filename);
	if (flock(fd, LOCK_EX) == -1)
		err(1, "flock %s", filename);

	/* Look for an existing key if any */
	rewind(f);
	len = seek_to_key(f, line);

	switch (op) {
	case OP_ADD:
		/* Make space for the new entry */
		newlen = strlen(line) + 1 /*'\n'*/;
		resize_bytes(filename, f, len, newlen);
		if (fprintf(f, "%s\n", line) != newlen)
			err(1, "write %s", filename);
		break;
	case OP_DEL:
		/* Delete any found entry */
		if (len)
			delete_bytes(filename, f, len);
		break;
	}

	/* Close and unlock */
	if (fclose(f) == EOF)
		err(1, "%s", filename);
	fd = -1;

	if (send_sighup) {
		/* Tell init to re-read its files */
		if (kill(1, SIGHUP) == -1)
			err(1, "kill 1");
	}

	exit(0);
}
