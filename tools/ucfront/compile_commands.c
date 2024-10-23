#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <sys/file.h>

#include "json.h"
#include "compile_commands.h"

/*
 * Functions to add records to a compile_commands.json database,
 * taking care to update the database quickly and with minimal locking.
 *
 * This is intended to be called from ucfront when it is used to compile
 * something.
 *
 * See the compile_commands.json database format at:
 * - https://clang.llvm.org/docs/JSONCompilationDatabase.html
 * - http://json.schemastore.org/compile-commands
 */

/*
 * Seek the compile_commands.json database for a matching entry.
 * Finds the first JSON entry matching:
 *    { "dir": match_dir, "file": match_file, "output": match_output, ... }
 * If `match_output` is NULL, then any "output" properties are ignored.
 * Returns the offset (>0) of the opening { of the first matching record,
 * and also stores the length of the record in *span_return.
 * Returns 0 if a matching entry is not found.
 * Returns -1 on invalid JSON.
 */
static off_t
find_entry(FILE *db, const char *match_dir, const char *match_file,
	   const char *match_output, off_t *span_return)
{
	int tok;

	rewind(db);
	tok = json_read_token(db, NULL, 0);
	if (tok == EOF)
		return 0;	/* a blank database after creat is OK */
	if (tok != '[')				/* expect [ */
		goto badjson;
	while ((tok = json_read_token(db, NULL, 0)) != ']') {
		if (tok == ',')
			continue;
		if (tok != '{')			/* expect { */
			goto badjson;

		off_t offset = ftell(db) - 1;
		char key[PATH_MAX];
		char dirbuf[PATH_MAX] = "";
		char filebuf[PATH_MAX] = "";
		char outputbuf[PATH_MAX] = "";
		const char *dir = NULL;
		const char *file = NULL;
		const char *output = NULL;
		while ((tok = json_read_token(db, key, sizeof key)) != '}') {
			if (tok == ',')
				continue;
			if (tok != '"')		/* expect "<key>" */
				goto badjson;
			if (json_read_token(db, NULL, 0) != ':') /* expect : */
				goto badjson;
			if (strcmp(key, "directory") == 0) {
				tok = json_read_token(db, dirbuf, sizeof dirbuf);
				if (tok != '"')		/* expect "<str>" */
					goto badjson;
				dir = dirbuf;
			} else if (strcmp(key, "file") == 0) {
				tok = json_read_token(db, filebuf, sizeof filebuf);
				if (tok != '"')		/* expect "<str>" */
					goto badjson;
				file = filebuf;
			} else if (strcmp(key, "output") == 0) {
				tok = json_read_token(db, outputbuf, sizeof outputbuf);
				if (tok != '"')		/* expect "<str>" */
					goto badjson;
				output = outputbuf;
			} else {
				if (!json_skip_value(db))
					goto badjson;
			}
		}
		if (dir && file && (!match_output || output) &&
			strcmp(dir, match_dir) == 0 &&
			strcmp(file, match_file) == 0 &&
			(!match_output || strcmp(output, match_output) == 0))
		{
			if (span_return)
				*span_return = ftell(db) - offset;
			return offset;
		}
	}
	return 0;

badjson:
	return -1;
}

/*
 * Create a new JSON record for a compilation command,
 * storing it in buf[bufsz].
 *
 * The string stored into buf[] is this JSON object:
 *
 *     { "directory": dir,
 *       "file": file,
 *       "output": output,
 *       "command": command,
 *       "arguments": [argv] }
 *
 * Parameters `output`, `command` and `argv` may be NULL, in which
 * case their corresponding member is omitted from the object.
 * The `dir` and `file` parameters are always required.
 *
 * Returns number of bytes that would be added to buf[].
 */
static int
make_record(const char *dir, const char *file, const char *output,
	const char *command, int argc, char *argv[],
	char *buf, size_t bufsz)
{
	int len = 0;
	char jbuf[PATH_MAX + 2];

#define BUF_PRINTF(...) do { \
		if (len < bufsz) \
			len += snprintf(buf + len, bufsz - len, __VA_ARGS__); \
	} while (0)
	BUF_PRINTF("{\"directory\":%s", json_str(dir, jbuf, sizeof jbuf));
	BUF_PRINTF(",\"file\":%s", json_str(file, jbuf, sizeof jbuf));
	if (output)
		BUF_PRINTF(",\"output\":%s", json_str(output, jbuf, sizeof jbuf));
	if (command)
		BUF_PRINTF(",\"command\":%s", json_str(command, jbuf, sizeof jbuf));
	if (argv) {
		BUF_PRINTF(",\"arguments\":[");
		for (int i = 0; i < argc; i++)
			BUF_PRINTF("%s%s", i ? "," : "",
				json_str(argv[i], jbuf, sizeof jbuf));
		BUF_PRINTF("]");
	}
	BUF_PRINTF("}");
	return len;
#undef BUF_PRINTF
}

/*
 * Truncate the file at the current write position.
 * Return EOF on error.
 */
static int
ftrunc(FILE *f)
{
	if (fflush(f) == EOF)
		return EOF;
	if (ftruncate(fileno(f), ftell(f)) == -1)
		return EOF;
	return 0;
}

/*
 * Does string `s` end with substring `end`?
 */
static int
ends_with(const char *s, const char *end)
{
	int slen = strlen(s);
	int endlen = strlen(end);
	return slen >= endlen && strcmp(s + slen - endlen, end) == 0;
}

/*
 * Update the compile_commands.json database with a compile command.
 * If an entry already exists, this function does nothing.
 * If the database file is missing, it is created new.
 */
int
compile_commands_update(const char *dir, const char *file,
	const char *output, int argc, char *argv[])
{
	FILE *db = NULL;
	char *rest = NULL;
	const char *path = "compile_commands.json";
	int ret = -1;

	/* If the directory is named conftest, then creating extra
	 * files is going to upset old GNU autoconf scripts. */
	if (ends_with(dir, "/conftest")) {
		ret = 0;
		goto out;
	}

	/* Construct the new JSON record we intend to add */
	static char newrec[BUFSIZ];
	int newreclen = make_record(dir, file, output, NULL, argc, argv,
				    newrec, sizeof newrec);
	if (newreclen >= sizeof newrec)
		goto out;		/* Ditch huge compile commands */

	/*
	 * Open/create the DB and aquire a shared reader lock.
	 * Note we can't use fopen() directly because
	 * - fopen(,"r+") won't creat the file if it's missing, and
	 * - fopen(,"a+") will make writes append-only (unseekable).
	 */
	db = fdopen(open(path, O_RDWR | O_CREAT, 0666), "r+");
	if (!db || flock(fileno(db), LOCK_SH) == -1)
		goto out;

	/* See if an entry with the same key (dir,file,output)
	 * is already in the DB */
	off_t curlen = -1;
	off_t curpos = find_entry(db, dir, file, output, &curlen);
	if (curpos == -1)
		goto out;
	if (curpos && curlen == newreclen) {
		/* A matching entry exists with the same length */
		char currec[BUFSIZ];
		if (fseek(db, curpos, SEEK_SET) == EOF)
			goto out;
		if (fread(currec, curlen, 1, db) != 1)
			goto out;
		if (memcmp(newrec, currec, curlen) == 0) {
			/* And it has the same content, so it doesn't
			 * need any change. This is typical. */
			ret = 0;
			goto out;
		}
	}

	/*
	 * Upgrade the lock now because we're going to change the file,
	 * either replacing or appending a new record.
	 *
	 * We release the lock momentarily to allow any blocked writers
	 * to progress, even though that means we'll have to rescan later
	 * while we hold the write lock.
	 */
	rewind(db);
	flock(fileno(db), LOCK_UN);
	if (flock(fileno(db), LOCK_EX) == -1)
		goto out;

	/* Check if the database is empty */
	int tok;
	tok = json_read_token(db, NULL, 0);
	if (tok == EOF || (tok == '[' && json_read_token(db, NULL, 0) == ']')) {
		/* The database is empty. Write [newrec]. */
		rewind(db);
		fprintf(db, "[%s]\n", newrec);
		ret = 0;
		goto out;
	}

	/* Rescan since file may have changed */
	curpos = find_entry(db, dir, file, output, &curlen);
	if (curpos == -1)
		goto out;
	if (curpos && curlen == newreclen) {
		char currec[BUFSIZ];
		if (fseek(db, curpos, SEEK_SET) == EOF)
			goto out;
		if (fread(currec, curlen, 1, db) != 1)
			goto out;
		if (memcmp(newrec, currec, curlen) == 0) {
			/* Exists AND doesn't need changing. Shrug. */
			ret = 0;
			goto out;
		}
	}

	off_t restpos = -1;

	/* Did we match the last element in the toplevel array? */
	if (curpos) {
		if (fseek(db, curpos + curlen, SEEK_SET) == EOF)
			goto out;
		int tok = json_read_token(db, NULL, 0);
		if (tok == ']') {	/* ] follows the record, so it's last */
			/*
			 * Disable stdio buffer now, because it doesn't
			 * seem to be reliable with fseek()+fwrite().
			 */
			setbuf(db, NULL);
			/* Back up and overwrite the old last element */
			if (fseek(db, curpos, SEEK_SET) == EOF)
				goto out;
			fprintf(db, "%s]\n", newrec);
			if (ftrunc(db) == EOF)
				goto out;
			ret = 0;
			goto out;
		}
		else if (tok != ',')	/* Expect , after our element */
			goto out;
		restpos = ftell(db);	/* Remember following record's offset */
	}

	/* If the old and new records are the same length */
	if (curpos && curlen == newreclen) {
		/* Then overwrite it in-place */
		setbuf(db, NULL);
		if (fseek(db, curpos, SEEK_SET) == EOF)
			goto out;
		if (fwrite(newrec, newreclen, 1, db) != 1)
			goto out;
		ret = 0;
		goto out;
	}

	/* Locate the closing ] */
	rewind(db);
	if (!json_skip_value(db))	/* Skips [...] to just after ']' */
		goto out;
	off_t closepos = ftell(db) - 1;

	if (!curpos) {
		/* No previous match; replace closing "]" with ",newrec]" */
		setbuf(db, NULL);
		if (fseek(db, closepos, SEEK_SET) == EOF)
			goto out;
		fprintf(db, ",%s]\n", newrec);
		ret = 0;
		goto out;
	}

	/* Replace "oldrec,rest]" with "rest,newrec]" */
	int restlen = closepos - restpos;
	rest = malloc(restlen);
	if (!rest)
		goto out;
	if (fseek(db, restpos, SEEK_SET) == EOF)
		goto out;
	if (fread(rest, restlen, 1, db) != 1)
		goto out;
	setbuf(db, NULL);
	if (fseek(db, curpos, SEEK_SET) == EOF)
		goto out;
	fprintf(db, "%.*s,%s]\n", restlen, rest, newrec);
	if (ftrunc(db) == EOF)
		goto out;
	ret = 0;

out:
	if (db)
		fclose(db);
	free(rest);
	return ret;
}
