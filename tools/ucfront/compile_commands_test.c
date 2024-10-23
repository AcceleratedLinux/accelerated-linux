
/* Unit test driver for functions in compile_commands.c */

#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include "compile_commands.h"

int
main(int argc, char *argv[])
{
	char dirbuf[PATH_MAX];
	const char *dir = NULL;
	const char *file = NULL;
	const char *output = NULL;

	int ch;
	int error = 0;
	while ((ch = getopt(argc, argv, "d:f:o:")) != -1) {
		switch (ch) {
		case 'd': dir = optarg; break;
		case 'f': file = optarg; break;
		case 'o': output = optarg; break;
		default: error = 1;
		}
	}
	if (optind == argc)
		error = 1;
	if (error || !file) {
		fprintf(stderr, "usage: %s"
			" -f file"
			" [-d dir]"
			" [-o output]"
			" -- cmd...\n", argv[0]);
		exit(1);
	}

	if (!dir)
		dir = getcwd(dirbuf, sizeof dirbuf);
	int ret = compile_commands_update(dir, file, output, argc - optind,
		&argv[optind]);
	if (ret == -1)
		exit(1);
}
