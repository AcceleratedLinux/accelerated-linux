#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <sys/stat.h>
#include <opengear/zipstream.h>

/*
 * Simple zip-like program wrapper around the zipstream API.
 */

static ssize_t
my_write(const void *data, size_t datalen, void *p)
{
	FILE *file = p;

	if (datalen) {
		if (fwrite(data, 1, datalen, file) != datalen)
			return -1;
	} else {
		fflush(file);
	}
	return datalen;
}


int
main(int argc, char *argv[])
{
	FILE *zipout;
	const char *zipoutname;
	struct zipstream *zs;

	if (optind >= argc) {
	    fprintf(stderr, "usage: %s out.zip [file ...]\n",
		argv[0]);
	    return 1;
	}

	zipoutname = argv[optind++];
	zipout = fopen(zipoutname, "wb");
	if (!zipout) err(1, "%s", zipoutname);

	zs = zipstream_new(my_write, zipout);
	if (!zs) err(1, "zipstream_new");

	while (optind < argc) {
	    const char *filename = argv[optind++];
	    struct stat sb, *sbp;
	    FILE *filein = NULL;
	    char buffer[BUFSIZ];
	    size_t len;
	    int isdir = 0;

	    printf("  adding: %s ", filename); fflush(stdout);

	    if (strcmp(filename, "-") == 0) {
		filein = stdin;
		sbp = NULL;
	    } else {
	        if (stat(filename, &sb) == -1)
		    err(1, "%s", filename);
		sbp = &sb;
		isdir = S_ISDIR(sb.st_mode);
		if (!isdir) {
		    filein = fopen(filename, "rb");
		    if (!filein)
			err(1, "%s", filename);
		}
	    }

	    if (zipstream_entry(zs, filename, sbp) == -1)
		err(1, "zipstream_entry");

	    if (isdir) {
		printf(" (directory)\n");
	    } else {
		while ((len = fread(buffer, 1, sizeof buffer, filein)) > 0) {
		    if (zipstream_write(zs, buffer, len) == -1)
			err(1, "zipstream_write");
		}
		if (ferror(filein))
		    err(1, "%s", filename);
		if (fclose(filein) == EOF)
		    warn("%s", filename);
		printf(" (something 100%%)\n");
	    }
	}
	if (zipstream_close(zs) == -1)
	    err(1, "zipstream_close");
	if (fclose(zipout) == EOF)
	    err(1, "fclose %s", zipoutname);
	return 0;
}
