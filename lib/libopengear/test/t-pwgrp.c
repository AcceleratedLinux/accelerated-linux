
/*
 * Unit test to exercise all the functionality in pwgrp.h
 */

#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <opengear/pwgrp.h>

#define _PATH_PASSWD _PATH_ETCDIR "/passwd"
#define _PATH_SHADOW _PATH_ETCDIR "/shadow"
#define _PATH_GROUP  _PATH_ETCDIR "/group"

static void
setup()
{
	FILE *f;
	int ret;

	assert(strcmp(_PATH_ETCDIR, "/tmp/etc") == 0);

	/* Create some dummy passwd, group files */
	ret = mkdir(_PATH_ETCDIR, 0777);
	assert(ret == 0 || errno == EEXIST);

	unlink(_PATH_PASSWD);
	f = fopen(_PATH_PASSWD, "w");
	fprintf(f, "root:x:0:1:Root User:/:/bin/bash\n"
		   "davidl:x:1000:1000:David:/home/davidl:/bin/pdksh\n");
	fclose(f);
	chmod(_PATH_PASSWD, 0644);

	unlink(_PATH_SHADOW);
	f = fopen(_PATH_SHADOW, "w");
	fprintf(f, "root:$1$xyz:::::::\n"
		   "davidl:$1$abc:1:2:3:4:5:6:7\n");
	fclose(f);
	chmod(_PATH_SHADOW, 0600);

	unlink(_PATH_GROUP);
	f = fopen(_PATH_GROUP, "w");
	fprintf(f, "root:x:0:root\n"
	           "admin:x:1:root,nobody,davidl\n");
	fclose(f);
	chmod(_PATH_GROUP, 0644);
}


/* Functions used to convert structures to string for easy compare assertion */

#define streq(a,b) (strcmp(a,b) == 0)

static void
asserteq_(const char *file, int line, const char *expr,
	const char *a, const char *b)
{
	if (a && b && strcmp(a, b) == 0)
		return;
	fprintf(stderr, "%s:%d: assertion failed: %s\n", file, line, expr);
	fprintf(stderr, "   a = \"%s\",\n", a);
	fprintf(stderr, "   b = \"%s\",\n", b);
	fflush(stderr);
	abort();
}

static void
grow(char **s, char ch, int *szp)
{
#define STEP 2048
	if (*szp % STEP == 0) {
		*s = realloc(*s, *szp + STEP);
		if (!*s) err(1, "realloc");
	}
	(*s)[*szp] = ch;
	++*szp;
#undef STEP
}

/* Loads a file into memory */
static char *
load_file(const char *path)
{
	int sz = 0;
	FILE *f = fopen(path, "r");
	int ch;
	char *data = NULL;

	if (!f)
		err(1, "%s", path);
	while ((ch = getc(f)) != EOF)
		grow(&data, ch, &sz);

	grow(&data, '\0', &sz);
	return data;
}

#define asserteq(a, b) asserteq_(__FILE__, __LINE__, "asserteq("#a", "#b")", a, b)

static const char *N(const char *s) { return s ? s : "(*NULL*)"; }

static const char *
pw_to_str(const struct passwd *pw) {
	static char buf[2048];
	if (!pw) return "(*NULL*)";
	snprintf(buf, sizeof buf, "%s:%s:%d:%d:%s:%s:%s",
		N(pw->pw_name),
		N(pw->pw_passwd),
		pw->pw_uid,
		pw->pw_gid,
		N(pw->pw_gecos),
		N(pw->pw_dir),
		N(pw->pw_shell));
	return buf;
}

static void
strcati(char *buf, size_t sz, long v, long ignore)
{
	int len = strnlen(buf, sz);
	if (v == ignore)
		snprintf(buf + len, sz - len, ":");
	else
		snprintf(buf + len, sz - len, ":%ld", v);
}


static const char *
sp_to_str(const struct spwd *sp)
{
	static char buf[2048];
	if (!sp) return "(*NULL*)";
	snprintf(buf, sizeof buf, "%s:%s",
		N(sp->sp_namp), N(sp->sp_pwdp));
	strcati(buf, sizeof buf, sp->sp_lstchg, -1);
	strcati(buf, sizeof buf, sp->sp_min, -1);
	strcati(buf, sizeof buf, sp->sp_max, -1);
	strcati(buf, sizeof buf, sp->sp_warn, -1);
	strcati(buf, sizeof buf, sp->sp_inact, -1);
	strcati(buf, sizeof buf, sp->sp_expire, -1);
	strcati(buf, sizeof buf, (long)sp->sp_flag, -1);
	return buf;
}

static const char *
gr_to_str(const struct group *gr) {
	static char buf[2048];
	char **mem;

	snprintf(buf, sizeof buf, "%s:%s:%d:",
		N(gr->gr_name),
		N(gr->gr_passwd),
		gr->gr_gid);
	if (!gr->gr_mem)
		strcat(buf, "(*NULL*)");
	else {
		const char *sep = "";
		for (mem = gr->gr_mem; *mem; mem++) {
			strcat(buf, sep);
			strcat(buf, *mem);
			sep = ",";
		}
	}
	return buf;
}

static void
stat_files(struct stat sb[3])
{
	if (stat(_PATH_PASSWD, &sb[0]) == -1)
		err(1, "%s", _PATH_PASSWD);
	if (stat(_PATH_SHADOW, &sb[1]) == -1)
		err(1, "%s", _PATH_SHADOW);
	if (stat(_PATH_GROUP, &sb[2]) == -1)
		err(1, "%s", _PATH_GROUP);
}

static int
stat_eq(const struct stat *a, const struct stat *b)
{
	return a->st_ino == b->st_ino &&
	       a->st_size == b->st_size &&
	       a->st_mtim.tv_sec == b->st_mtim.tv_sec &&
	       a->st_mtim.tv_nsec == b->st_mtim.tv_nsec &&
	       a->st_mode == b->st_mode;
}

int
main()
{
	struct pwgrp *pg;
	struct passwd *pw;
	struct group *gr;
	struct spwd *sp;
	char *s;
	int ret;
	gid_t groups[4];
	int ngroups;

	setup();
	pg = pwgrp_open();
	if (!pg)
		err(1, "pwgrp_open");

	assert(pg->npasswd == 2);
	asserteq(pw_to_str(pg->passwd[0]), "root:x:0:1:Root User:/:/bin/bash");
	asserteq(pw_to_str(pg->passwd[1]),
		   "davidl:x:1000:1000:David:/home/davidl:/bin/pdksh");
	assert(pg->nspwd == 2);
	asserteq(sp_to_str(pg->spwd[0]), "root:$1$xyz:::::::");
	asserteq(sp_to_str(pg->spwd[1]), "davidl:$1$abc:1:2:3:4:5:6:7");
	assert(pg->ngroup == 2);
	asserteq(gr_to_str(pg->group[0]), "root:x:0:root");
	asserteq(gr_to_str(pg->group[1]), "admin:x:1:root,nobody,davidl");

	/* Modify some entries */
	pg->passwd[0]->pw_gid = 0;
	pwgrp_setstr(&pg->passwd[0]->pw_passwd, "y");
	asserteq(pw_to_str(pg->passwd[0]), "root:y:0:0:Root User:/:/bin/bash");

	group_del_mem(pg->group[1], "");
	asserteq(gr_to_str(pg->group[1]), "admin:x:1:root,nobody,davidl");
	group_del_mem(pg->group[1], "nobody");
	asserteq(gr_to_str(pg->group[1]), "admin:x:1:root,davidl");
	group_add_mem(pg->group[1], "fred");
	asserteq(gr_to_str(pg->group[1]), "admin:x:1:root,davidl,fred");

	/* Delete some entries */
	passwd_release(&pg->passwd[1]); /* davidl */
	spwd_release(&pg->spwd[1]);     /* davidl */
	group_release(&pg->group[0]);   /* root */

	/* Append some new entries */
	pw = pwgrp_new_passwd(pg);
	assert(pg->npasswd == 3);
	asserteq(pw_to_str(pw), "::0:0:::");

	sp = pwgrp_new_spwd(pg);
	assert(pg->nspwd == 3);
	asserteq(sp_to_str(sp), "::::::::");

	gr = pwgrp_new_group(pg);
	assert(pg->ngroup == 3);
	asserteq(gr_to_str(gr), "::0:");

	/* Fill in some new entries */
	pw->pw_uid = 1001;
	asserteq(pw_to_str(pw), "::1001:0:::");
	assert(pwgrp_setstr(&pw->pw_name, "foo"));
	asserteq(pw_to_str(pw), "foo::1001:0:::");
	assert(pwgrp_setf(&pw->pw_gecos, "%s%d", "random", 1));
	asserteq(pw_to_str(pw), "foo::1001:0:random1::");

	assert(pwgrp_setstr(&gr->gr_name, "bar"));
	asserteq(gr_to_str(gr), "bar::0:");
	assert(group_add_mem(gr, "baz") == 0);
	asserteq(gr_to_str(gr), "bar::0:baz");
	assert(group_add_mem(gr, "qux") == 0);
	asserteq(gr_to_str(gr), "bar::0:baz,qux");
	group_clear_mem(gr);
	asserteq(gr_to_str(gr), "bar::0:");
	assert(group_add_mem(gr, "yul") == 0);
	asserteq(gr_to_str(gr), "bar::0:yul");
	gr->gr_gid = 99;
	asserteq(gr_to_str(gr), "bar::99:yul");

	/* Searching */
	pw = pwgrp_getpwnam(pg, "root");
	asserteq(pw->pw_name, "root");
	pw = pwgrp_getpwuid(pg, 1001);
	asserteq(pw->pw_name, "foo");
	gr = pwgrp_getgrgid(pg, 99);
	asserteq(gr->gr_name, "bar");
	gr = pwgrp_getgrnam(pg, "bar");
	assert(gr->gr_gid == 99);

	/* getgrouplist */
	ngroups = sizeof groups / sizeof groups[0];
	memset(groups, 0, sizeof groups);
	assert(pwgrp_getgrouplist(pg, "yul", 9, groups, &ngroups) == 2);
	assert(groups[0] == 9);
	assert(groups[1] == 99);

	/* write to output files */
	ret = pwgrp_close(pg);
	if (ret == -1)
		err(1, "pwgrp_close");

	s = load_file(_PATH_PASSWD);
	asserteq(s, "root:y:0:0:Root User:/:/bin/bash\n"
		    "foo::1001:0:random1::\n");
	free(s);
	s = load_file(_PATH_SHADOW);
	asserteq(s, "root:$1$xyz:::::::\n"
		    "::::::::\n");
	free(s);
	s = load_file(_PATH_GROUP);
	asserteq(s, "admin:x:1:root,davidl,fred\n"
		    "bar::99:yul\n");
	free(s);

	/* Load and write back; nothing should have changed */
	{
		struct stat before[3], after[3];

		stat_files(before);
		pg = pwgrp_open();

		pwgrp_close(pg);
		stat_files(after);
		assert(stat_eq(&before[0], &after[0])); /* passwd */
		assert(stat_eq(&before[1], &after[1])); /* shadow */
		assert(stat_eq(&before[2], &after[2])); /* group */
	}

	/* Load, change shell, write back; only shell should change */
	{
		struct stat before[3], after[3];

		stat_files(before);
		pg = pwgrp_open();

		pwgrp_setstr(&pg->passwd[0]->pw_shell, "/bin/nologin");

		pwgrp_close(pg);
		stat_files(after);
		assert(!stat_eq(&before[0], &after[0])); /* passwd */
		assert(stat_eq(&before[1], &after[1])); /* shadow */
		assert(stat_eq(&before[2], &after[2])); /* group */
	}

	exit(0);
}
