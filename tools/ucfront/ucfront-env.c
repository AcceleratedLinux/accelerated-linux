/*
 * ucfront-env -- front end for automake building
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *   
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *   
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>

#define PNAME "ucfront-env"

/*
 * List of vars to remove from environment (regular expressions)
 */
const char *rmlist[] = {
	"^dir_y",
	"^dir_n",
	"^CONFIG_",
};
#define RMSIZE (sizeof(rmlist) / sizeof(char *))

/*
 * List of vars that we must keep (regular expressions).
 * Mostly these are the ones that other ucfront tools use.
 */
const char *keeplist[] = {
	"^CONFIG_LIBCDIR",
	"^CONFIG_DEFAULTS_LIBC_",
	"^CONFIG_LIB_STLPORT",
	"^CONFIG_LIB_UCLIBCXX",
};
#define KEEPSIZE (sizeof(keeplist) / sizeof(char *))

extern char **environ;
regex_t rmregex[RMSIZE];
regex_t keepregex[KEEPSIZE];

void regex_init(void)
{
	int i;

	for (i = 0; i < RMSIZE; i++) {
		if (regcomp(&rmregex[i], rmlist[i], REG_NOSUB)) {
			printf(PNAME ": bad regular expression \"%s\"\n",
				rmlist[i]);
			exit(1);
		}
	}
	for (i = 0; i < KEEPSIZE; i++) {
		if (regcomp(&keepregex[i], keeplist[i], REG_NOSUB)) {
			printf(PNAME ": bad regular expression \"%s\"\n",
				keeplist[i]);
			exit(1);
		}
	}
}

int regex_rmcheck(const char *name)
{
	int i;
	for (i = 0; i < RMSIZE; i++) {
		if (regexec(&rmregex[i], name, 0, NULL, 0) == 0)
			break;
	}
	return (i == RMSIZE) ? 0 : 1;
}

int regex_keepcheck(const char *name)
{
	int i;
	for (i = 0; i < KEEPSIZE; i++) {
		if (regexec(&keepregex[i], name, 0, NULL, 0) == 0)
			break;
	}
	return (i == KEEPSIZE) ? 0 : 1;
}

char **namelist_ptrs;
size_t namelist_size;
int namelist_count;
int namelist_free;
#define ALLOCNUM	10
#define ALLOCSIZE	(ALLOCNUM * sizeof(char *))

void namelist_add(const char *name)
{
	char *s;

	if (namelist_free == 0) {
		namelist_size += ALLOCSIZE;
		namelist_ptrs = realloc(namelist_ptrs, namelist_size);
		if (namelist_ptrs == NULL) {
			printf(PNAME ": failed to malloc(%d)\n",
				(int) namelist_size);
			exit(1);
		}
		namelist_free += ALLOCNUM;
	}

	namelist_ptrs[namelist_count] = strdup(name);
	if (namelist_ptrs[namelist_count] == NULL) {
		printf(PNAME ": failed to strdup(%s)\n", name);
		exit(1);
	}

	for (s = namelist_ptrs[namelist_count]; *s != 0; s++) {
		if (*s == '=') {
			*s = 0;
			break;
		}
	}
	namelist_count++;
	namelist_free--;
}

void namelist_unset(void)
{
	int i;
	for (i = 0; i < namelist_count; i++)
		unsetenv(namelist_ptrs[i]);
}

char **mkargs(int argc, char *argv[])
{
	char **newargv;
	int len, i;

	len = argc * sizeof(char *);
	newargv = malloc(len);
	if (newargv == NULL) {
		printf(PNAME ": failed to mallooc(%d)\n", len);
		exit(1);
	}

	for (i = 0; i < argc-1; i++)
		newargv[i] = argv[i + 1];
	newargv[i] = NULL;
	return newargv;
}

int main(int argc, char *argv[])
{
	char **newargv;
	char *name;
	int i;

	if (argc <= 1)
		return 0;

	regex_init();
	for (i = 0, name = environ[i]; name != NULL; name = environ[++i]) {
		if (regex_rmcheck(name)) {
			if (! regex_keepcheck(name))
				namelist_add(name);
		}
	}
	namelist_unset();

	newargv = mkargs(argc, argv);
	execvp(newargv[0], newargv);

	printf(PNAME ": failed to execute %s\n", newargv[0]);
	return 1;
}

