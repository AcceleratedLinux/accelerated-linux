
/*
 * Passwd, shadow and group file editor.
 *
 * This does no sanity checking of the databases. It just handles:
 *    - write-locking the databases (readers can still read while write-locked)
 *    - loading the databases into memory using standard structures
 *      and simple realloc'd arrays.
 *    - handles memory management of string fields
 *    - writing the strucures back to disk (only if the file would change)
 */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <limits.h>
#include <syslog.h>

#include <sys/file.h>	/* flock */
#include <sys/types.h>
#include <sys/stat.h>

#include <opengear/og_config.h>
#include <opengear/pwgrp.h>

#define HASHMAX 31
#define ALLOC_STEP 32

#ifndef _PATH_ETCDIR
# define _PATH_ETCDIR "/etc/config"
#endif
#ifndef _PATH_TMP
# define _PATH_TMP "/tmp"
#endif

#undef _PATH_PASSWD
#undef _PATH_SHADOW
#undef _PATH_GROUP
#define _PATH_PASSWD _PATH_ETCDIR "/passwd"
#define _PATH_SHADOW _PATH_ETCDIR "/shadow"
#define _PATH_GROUP  _PATH_ETCDIR "/group"

#ifdef HAVE_SHADOW
void spwd_release(struct spwd **sp);
#endif

/**
 * Extend an array of entry pointers.
 * This is used to grow the in-memory database arrays.
 * Growth occurs in chunks of #ALLOC_STEP.
 * An array is represented by a base pointer, and a length integer.
 *
 * Basically, start with NULL and 0 and keep extending to get the next slot.
 * New slots are always zeroed.
 *
 * @param basep   Pointer to base pointer to array. The base pointer will
 *                be extended using #realloc() on the first call and on every
 *                #ALLOC_STEP call thereafter.
 * @param lenp    Pointer to the element counter. This will be incremented
 *                by one on success.
 * @param sz      Size of each element of the array.
 * @return NULL on failure, otherwise
 *         the pointer to the newly added (and zeroed) element in the
 *         array. This is the pointer <code>&base[len - 1]</code>.
 */
static void *
extend(void **basep, unsigned *lenp, size_t sz)
{
	unsigned len = *lenp;
	void *base = *basep;
	void *ptr;

	/* Allocate to ensure there is enough space to increment len by 1 */
	if ((len % ALLOC_STEP) == 0) {
		/* Re-allocate in steps of ALLOC_STEP elements */
		base = realloc(base, sz * (len + ALLOC_STEP));
		if (!base)
			return NULL;
		*basep = base;
	}

	*lenp = len + 1;
	ptr = (char *)base + (sz * len);
	memset(ptr, 0, sz);
	return ptr;
}

/** Extend an array. @return the type-correct pointer to added element (or 0) */
#define EXTEND(base, len) \
	((typeof (base))extend((void**)&(base), &(len), sizeof *(base)))

static char empty_string[] = "";


/* Duplication of strings. Basically strdup() but treating "" specially */

/**
 * Return a privately-owned copy of the string.
 * Empty strings are replaced with the constant #empty_string.
 * @note The return type is a non-const pointer because that
 *       is the field type of struct #passwd etc.
 * @return a pointer that should be released with #xfree().
 */
static char *
xstrdup(const char *s)
{
	if (!s)
		return NULL;
	if (!*s)
		return empty_string;
	return strdup(s);
}

/** Frees a string returned by #xstrdup(). */
static void
xfree(void *s)
{
	if (s != empty_string)
		free(s);
}

char *
pwgrp_setstr(char **destp, const char *src)
{
	char *cp = xstrdup(src);
	if (!cp)
		return NULL;

	xfree(*destp);
	*destp = cp;
	return cp;
}

char *
pwgrp_setf(char **destp, const char *fmt, ...)
{
	char *s;
	va_list ap;

	va_start(ap, fmt);
	s = pwgrp_vsetf(destp, fmt, ap);
	va_end(ap);
	return s;
}

char *
pwgrp_vsetf(char **destp, const char *fmt, va_list ap)
{
	int len;
	va_list ap2;
	char *s;

	va_copy(ap2, ap);
	len = vsnprintf(NULL, 0, fmt, ap2);
	va_end(ap2);

	if (len == 0) {
		s = pwgrp_setstr(destp, "");
	} else {
		s = malloc(len + 1);
		if (s) {
			vsnprintf(s, len + 1, fmt, ap);
			xfree(*destp);
			*destp = s;
		}
	}
	return s;
}

/* Group membership manipulation */

static int
group_init_mem(struct group *gr)
{
	char **mem;

	mem = calloc(1, sizeof *mem);
	if (!mem)
		return -1;
	gr->gr_mem = mem;
	return 0;
}

int
group_has_mem(const struct group *gr, const char *username)
{
	char **mem;

	if (!gr->gr_mem)
		return 0;
	for (mem = gr->gr_mem; *mem; mem++) {
		if (strcmp(*mem, username) == 0)
			return 1;
	}
	return 0;
}

/** Remove the first instance of name from a group's membership,
 *  releasing its memory. */
void
group_del_mem(struct group *gr, const char *name)
{
	char **mem;
	if (!gr->gr_mem)
		return;
	for (mem = gr->gr_mem; *mem; mem++) {
		if (strcmp(*mem, name) == 0) {
			xfree(*mem);
			while (*mem) {
				*mem = *(mem + 1);
				mem++;
			}
			break;
		}
	}
}

/** Empty the group's membership. */
void
group_clear_mem(struct group *gr)
{
	char **mem;

	if (!gr->gr_mem)
		return;
	for (mem = gr->gr_mem; *mem; mem++) {
		xfree(*mem);
		*mem = NULL;
	}
}

/** Append a name to a group's membership. */
int
group_add_mem(struct group *gr, const char *name)
{
	int n = 0;
	char **mem;
	char *name2 = xstrdup(name);

	if (!name2)
		return -1;
	if (gr->gr_mem) {
		for (n = 0; gr->gr_mem[n]; n++) {
			if (strcmp(gr->gr_mem[n], name) == 0) {
				/* Already present. */
				return 0;
			}
		}
	}
	mem = gr->gr_mem;
	mem = realloc(mem, (n + 2) * sizeof *mem);
	if (!mem) {
		xfree(name2);
		return -1;
	}
	mem[n] = name2;
	mem[n + 1] = NULL;
	gr->gr_mem = mem;
	return 0;
}


/* Duplicating an entry */

struct passwd *
passwd_dup(const struct passwd *opw)
{
	struct passwd *pw;

	pw = malloc(sizeof *pw);
	if (!pw)
		goto fail;

	/* copy integer fields */
	memcpy(pw, opw, sizeof *pw);
	pw->pw_name = NULL;
	pw->pw_passwd = NULL;
	pw->pw_gecos = NULL;
	pw->pw_dir = NULL;
	pw->pw_shell = NULL;
	/* copy string fields */
	if (!pwgrp_setstr(&pw->pw_name, opw->pw_name))
		goto fail;
	if (!pwgrp_setstr(&pw->pw_passwd, opw->pw_passwd))
		goto fail;
	if (!pwgrp_setstr(&pw->pw_gecos, opw->pw_gecos))
		goto fail;
	if (!pwgrp_setstr(&pw->pw_dir, opw->pw_dir))
		goto fail;
	if (!pwgrp_setstr(&pw->pw_shell, opw->pw_shell))
		goto fail;
	return pw;
fail:
	passwd_release(&pw);
	return NULL;
}

#ifdef HAVE_SHADOW
struct spwd *
spwd_dup(const struct spwd *osp)
{
	struct spwd *sp;

	sp = malloc(sizeof *sp);
	if (!sp)
		goto fail;

	/* copy integer fields */
	memcpy(sp, osp, sizeof *sp);
	sp->sp_namp = NULL;
	sp->sp_pwdp = NULL;
	/* copy string fields */
	if (!pwgrp_setstr(&sp->sp_namp, osp->sp_namp))
		goto fail;
	if (!pwgrp_setstr(&sp->sp_pwdp, osp->sp_pwdp))
		goto fail;
	return sp;
fail:
	spwd_release(&sp);
	return NULL;
}
#endif /* HAVE_SHADOW */

struct group *
group_dup(const struct group *ogr)
{
	char **mem;
	struct group *gr;

	gr = malloc(sizeof *gr);
	if (!gr)
		goto fail;

	/* copy integer fields */
	memcpy(gr, ogr, sizeof *ogr);
	gr->gr_name = NULL;
	gr->gr_passwd = NULL;
	gr->gr_mem = NULL;
	/* copy string fields */
	if (!pwgrp_setstr(&gr->gr_name, ogr->gr_name))
		goto fail;
	if (!pwgrp_setstr(&gr->gr_passwd, ogr->gr_passwd))
		goto fail;
	/* copy list fields */
	if (group_init_mem(gr) == -1)
		goto fail;
	for (mem = ogr->gr_mem; *mem; mem++) {
		if (group_add_mem(gr, *mem) == -1)
			goto fail;
	}
	return gr;
fail:
	group_release(&gr);
	return NULL;
}



/* Freeing a duplicated entry */

void
passwd_release(struct passwd **pwp)
{
	struct passwd *pw = *pwp;
	if (pw) {
		xfree(pw->pw_name);
		xfree(pw->pw_passwd);
		xfree(pw->pw_gecos);
		xfree(pw->pw_dir);
		xfree(pw->pw_shell);
		xfree(pw);
		*pwp = NULL;
	}
}

#ifdef HAVE_SHADOW
void
spwd_release(struct spwd **spp)
{
	struct spwd *sp = *spp;
	if (sp) {
		xfree(sp->sp_namp);
		xfree(sp->sp_pwdp);
		xfree(sp);
		*spp = NULL;
	}
}
#endif /* HAVE_SHADOW */

void
group_release(struct group **grp)
{
	struct group *gr = *grp;
	if (gr) {
		group_clear_mem(gr);
		xfree(gr->gr_mem);
		xfree(gr->gr_name);
		xfree(gr->gr_passwd);
		xfree(gr);
		*grp = NULL;
	}
}


/* Appending a "new duplicated" entry */

struct passwd *
pwgrp_new_passwd(struct pwgrp *pg)
{
	struct passwd *pw, **pwp;

	pw = calloc(1, sizeof *pw);
	if (!pw)
		goto fail;
	if (!pwgrp_setstr(&pw->pw_name, ""))
		goto fail;
	if (!pwgrp_setstr(&pw->pw_passwd, ""))
		goto fail;
	if (!pwgrp_setstr(&pw->pw_gecos, ""))
		goto fail;
	if (!pwgrp_setstr(&pw->pw_dir, ""))
		goto fail;
	if (!pwgrp_setstr(&pw->pw_shell, ""))
		goto fail;
	pwp = EXTEND(pg->passwd, pg->npasswd);
	if (!pwp)
		goto fail;
	*pwp = pw;
	return pw;

fail:
	passwd_release(&pw);
	return NULL;
}

#ifdef HAVE_SHADOW
struct spwd *
pwgrp_new_spwd(struct pwgrp *pg)
{
	struct spwd *sp, **spp;

	sp = calloc(1, sizeof *sp);
	if (!sp)
		goto fail;

	if (!pwgrp_setstr(&sp->sp_namp, ""))
		goto fail;
	if (!pwgrp_setstr(&sp->sp_pwdp, ""))
		goto fail;
	sp->sp_lstchg = -1;
	sp->sp_min = -1;
	sp->sp_max = -1;
	sp->sp_warn = -1;
	sp->sp_inact = -1;
	sp->sp_expire = -1;
	sp->sp_flag = -1; /* XXX */
	spp = EXTEND(pg->spwd, pg->nspwd);
	if (!spp)
		goto fail;
	*spp = sp;
	return sp;

fail:
	spwd_release(&sp);
	return NULL;
}
#endif

struct group *
pwgrp_new_group(struct pwgrp *pg)
{
	struct group *gr, **grp;

	gr = calloc(1, sizeof *gr);
	if (!gr)
		goto fail;
	if (!pwgrp_setstr(&gr->gr_name, ""))
		goto fail;
	if (!pwgrp_setstr(&gr->gr_passwd, ""))
		goto fail;
	if (group_init_mem(gr) == -1)
		goto fail;
	grp = EXTEND(pg->group, pg->ngroup);
	if (!grp)
		goto fail;
	*grp = gr;
	return gr;
fail:
	group_release(&gr);
	return NULL;
}



/* File locking */

/**
 * Acquire the database lock.
 *
 * We can't use lckpwdf() because it uses flock on /etc/passwd, and
 * that is one of the files we will be renaming over. We use rename()
 * because getpwent() does not use locking and so an atomic replacement of
 * files is required to avoid read-write collision. But, rename breaks
 * flock (because flock works on inodes, not paths).
 *
 * So, this function just creates the empty lock file _PATH_PASSWD_LOCK
 * and uses flock on it.
 *
 * @returns file descriptor to the lock file, with lock held.
 */
static int
pwgrp_lockfd()
{
	int fd;

#ifdef O_CLOEXEC
	fd = open(_PATH_PASSWD_LOCK, O_CLOEXEC | O_WRONLY | O_CREAT, 0666);
#else /* old versions of glibc don't have O_CLOEXEC */
	fd = open(_PATH_PASSWD_LOCK,             O_WRONLY | O_CREAT, 0666);
#endif
	if (fd == -1)
		return -1;
	if (flock(fd, LOCK_EX) == -1) {
		close(fd);
		return -1;
	}
	return fd;
}


/* pwgrp management */

struct pwgrp *
pwgrp_new()
{
	struct pwgrp *pg = calloc(1, sizeof *pg);
	pg->lockfd = -1;
	return pg;
}

static void
pwgrp_free(struct pwgrp *pg)
{
	unsigned i;

	if (!pg)
		return;

	if (pg->lockfd != -1)
		close(pg->lockfd);

	for (i = 0; i < pg->npasswd; i++)
		passwd_release(&pg->passwd[i]);
	xfree(pg->passwd);
#ifdef HAVE_SHADOW
	for (i = 0; i < pg->nspwd; i++)
		spwd_release(&pg->spwd[i]);
	xfree(pg->spwd);
#endif
	for (i = 0; i < pg->ngroup; i++)
		group_release(&pg->group[i]);
	xfree(pg->group);

	xfree(pg);
}



/* loading database entries */

static int
load_passwd(struct pwgrp *pg, FILE *f)
{
	struct passwd *fpw;

	while ((fpw = fgetpwent(f))) {
		struct passwd *pw, **pwp;

		pw = passwd_dup(fpw);
		if (!pw)
			return -1;
		pwp = EXTEND(pg->passwd, pg->npasswd);
		if (!pwp) {
			passwd_release(&pw);
			return -1;
		}
		*pwp = pw;
	}
	return 0;
}

#ifdef HAVE_SHADOW
static int
load_spwd(struct pwgrp *pg, FILE *f)
{
	struct spwd *fsp;

	while ((fsp = fgetspent(f))) {
		struct spwd *sp, **spp;

		sp = spwd_dup(fsp);
		if (!sp)
			return -1;
		spp = EXTEND(pg->spwd, pg->nspwd);
		if (!spp) {
			spwd_release(&sp);
			return -1;
		}
		*spp = sp;
	}
	return 0;
}
#endif /* HAVE_SHADOW */

static int
load_group(struct pwgrp *pg, FILE *f)
{
	struct group *fgr;

	while ((fgr = fgetgrent(f))) {
		struct group *gr, **grp;

		gr = group_dup(fgr);
		if (!gr)
			return -1;
		grp = EXTEND(pg->group, pg->ngroup);
		if (!grp) {
			group_release(&gr);
			return -1;
		}
		*grp = gr;
	}
	return 0;
}



/* storing database entries */

static int
store_passwd(struct pwgrp *pg, FILE *f)
{
	unsigned i;

	for (i = 0; i < pg->npasswd; i++) {
		if (pg->passwd[i] && putpwent(pg->passwd[i], f) == -1)
			return -1;
	}
	return 0;
}

#ifdef HAVE_SHADOW
static int
store_spwd(struct pwgrp *pg, FILE *f)
{
	unsigned i;

	for (i = 0; i < pg->nspwd; i++) {
		if (pg->spwd[i] && putspent(pg->spwd[i], f) == -1)
			return -1;
	}
	return 0;
}
#endif /* HAVE_SHADOW */

static int
store_group(struct pwgrp *pg, FILE *f)
{
	unsigned i;

	for (i = 0; i < pg->ngroup; i++) {
		if (pg->group[i] && putgrent(pg->group[i], f))
			return -1;
	}
	return 0;
}


/* File manipulation */

/** Open a unique, writable temporary file */
static FILE *
fopentmp(char *template)
{
	int fd;
#if 0
	fd = mkostemp(template, O_CLOEXEC);
#else
	/* uClibc has mkstemp */
	fd = mkstemp(template);
#endif
	return fd == -1 ? NULL : fdopen(fd, "w+");
}

/**
 * Read content of each file, looking for first different character.
 * @returns 0 iff content is same length and byte identical.
 */
static int
fcmp(FILE *a, FILE *b)
{
	int cha, chb;
	for (;;) {
		cha = getc(a);
		chb = getc(b);
		if (cha == EOF && chb == EOF)
			return 0;
		if (cha < chb)
			return -1;
		if (cha > chb)
			return 1;
	}
}

/** Prepare for atomic file copy.
 *  Writes src stream into a file adjacent to a reference path, unless
 *  it would be identical to the reference file.
 *
 *  If the reference path exists and has identical content to @a src, then
 *  this function has no effect. In that case it sets @a adjpath to the empty
 *  string and returns success.
 *
 *  Othewise, the adjacent file is created, filled with @a src, and its
 *  mode and ownership set from the reference file or set from defaults.
 *
 *  @param[in]  src           Source stream. The stream should be rewound.
 *  @param[in]  dstpath       The reference or destination filename.
 *  @param[in]  default_owner The adjacent file's default owner. This is used
 *			      if the reference file does not exist.
 *  @param[in]  default_group The adjacent file's default group owner.
 *  @param[in]  default_modep The adjacent file's default permissions mode.
 *  @param[out] adjpath       Storage for the pathname to the adjacent file
 *                            created. This will be set to "" if there is no
 *                            adjacent file.
 *  @return -1 on error.
 */
static int
copyfile_prepare(FILE *src, const char *dstpath,
	uid_t default_owner, gid_t default_grp, mode_t default_mode,
	char adjpath[static PATH_MAX])
{
	FILE *dst = NULL;
	FILE *adj = NULL;
	int ch;
	int diff;
	struct stat dst_stat;

	adjpath[0] = '\0';

	if (stat(dstpath, &dst_stat) == -1) {
		if (errno != ENOENT)
			goto fail;

		/* The dst file doesn't exist. */
		dst_stat.st_mode = default_mode;
		dst_stat.st_uid = default_owner;
		dst_stat.st_gid = default_grp;
	} else {
		/* Compare the src and existing dst files' contents */
		dst = fopen(dstpath, "r");
		if (!dst)
			goto fail;
		rewind(src);
		diff = fcmp(dst, src);
		if (ferror(dst) || ferror(src))
			goto fail;
		fclose(dst);
		dst = NULL;
		if (diff == 0) {
			/* same content: success with no effort! */
			return 0;
		}
	}

	/* Open a file 'adjacent' to dst so that rename() works */
	snprintf(adjpath, PATH_MAX, "%s.new.XXXXXX", dstpath);
	adj = fopentmp(adjpath);
	if (!adj) {
		adjpath[0] = '\0';
		goto fail;
	}

	/* Copy the src content into the adjacent file */
	rewind(src);
	while ((ch = getc(src)) != EOF) {
		if (putc(ch, adj) == EOF)
			break;
	}
	if (ferror(src) || ferror(adj)) {
		errno = EIO;
		goto fail;
	}

	fflush(adj);

	/* Duplicate ownership and permissions */
	if (fchmod(fileno(adj), dst_stat.st_mode & 07777) == -1)
		goto fail;
	if (fchown(fileno(adj), dst_stat.st_uid, dst_stat.st_gid) == -1)
		goto fail;

	if (fclose(adj)) {
		adj = NULL;
		goto fail;
	}

	return 0;

fail:
	if (adj) fclose(adj);
	if (dst) fclose(dst);
	if (adjpath[0]) {
		unlink(adjpath);
		adjpath[0] = '\0';
	}
	return -1;
}

/** Complete an atomic file copy.
 *  Renames the adjacent file prepared by #copyfile_prepare() over the
 *  destination path.
 *
 *  This function has no effect if the preparation decided not to create the
 *  adjacent file. This state is indicated by @a adjpath being an empty string.
 *
 *  @param[in,out] adjpath File created by #copyfile_prepare() or empty
 *                         if no copy should be performed.
 *                         This string will be set to empty on successful
 *                         rename.
 *  @param[in] dstpath     The path to rename @a adjpath to.
 *  @return -1 on error.
 */
static int
copyfile_commit(char *adjpath, const char *dstpath)
{
	if (!*adjpath)
		return 0;
	if (rename(adjpath, dstpath) == -1) {
		syslog(LOG_ERR, "cannot update %s: %m", dstpath);
		return -1;
	}
	syslog(LOG_INFO, "updated %s", dstpath);
	/* Set adjpath to empty to indicate that it has gone */
	adjpath[0] = '\0';
	return 0;
}

/* Write all arrays to their system databases. */
static int
store_all(struct pwgrp *pg)
{
	char pw_tmp[PATH_MAX] = "";
	char pw_adj[PATH_MAX] = "";
	FILE *fpw = NULL;
#ifdef HAVE_SHADOW
	char sp_tmp[PATH_MAX] = "";
	char sp_adj[PATH_MAX] = "";
	FILE *fsp = NULL;
#endif
	char gr_tmp[PATH_MAX] = "";
	char gr_adj[PATH_MAX] = "";
	FILE *fgr = NULL;
	int ret = -1;

	/* Write arrays out the files in /tmp */
	strcpy(pw_tmp, _PATH_TMP "/passwd.XXXXXX");
	fpw = fopentmp(pw_tmp);
	if (!fpw)
		goto out;
	if (store_passwd(pg, fpw) == -1)
		goto out;
	if (fflush(fpw))
		goto out;
	rewind(fpw);
#ifdef HAVE_SHADOW
	strcpy(sp_tmp, _PATH_TMP "/shadow.XXXXXX");
	fsp = fopentmp(sp_tmp);
	if (!fsp)
		goto out;
	if (store_spwd(pg, fsp) == -1)
		goto out;
	if (fflush(fsp))
		goto out;
	rewind(fsp);
#endif
	strcpy(gr_tmp, _PATH_TMP "/group.XXXXXX");
	fgr = fopentmp(gr_tmp);
	if (!fgr)
		goto out;
	if (store_group(pg, fgr) == -1)
		goto out;
	if (fflush(fgr))
		goto out;
	rewind(fgr);

	/* Prepare to replace each file.
	 * Doing it this way will find ENOSPC early. */
	if (copyfile_prepare(fpw, _PATH_PASSWD, 0, 0, 0644, pw_adj) == -1)
		goto out;
#ifdef HAVE_SHADOW
	if (copyfile_prepare(fsp, _PATH_SHADOW, 0, 0, 0600, sp_adj) == -1)
		goto out;
#endif
	if (copyfile_prepare(fgr, _PATH_GROUP, 0, 0, 0644, gr_adj) == -1)
		goto out;

	/* Perform the replacements. Don't stop on errors as we're committed. */
	ret = 0;
	if (copyfile_commit(pw_adj, _PATH_PASSWD) == -1)
		ret = -1;
#ifdef HAVE_SHADOW
	if (copyfile_commit(sp_adj, _PATH_SHADOW) == -1)
		ret = -1;
#endif
	if (copyfile_commit(gr_adj, _PATH_GROUP) == -1)
		ret = -1;

out:
	if (fgr) fclose(fgr);
	if (gr_tmp[0]) unlink(gr_tmp);
	if (gr_adj[0]) unlink(gr_adj);
#ifdef HAVE_SHADOW
	if (fsp) fclose(fsp);
	if (sp_tmp[0]) unlink(sp_tmp);
	if (sp_adj[0]) unlink(sp_adj);
#endif
	if (fpw) fclose(fpw);
	if (pw_tmp[0]) unlink(pw_tmp);
	if (pw_adj[0]) unlink(pw_adj);

	return ret;
}

static int
load_all(struct pwgrp *pg)
{
	FILE *f = NULL;

	f = fopen(_PATH_PASSWD, "r");
	if (!f)
		goto error;
	if (load_passwd(pg, f) == -1)
		goto error;
	fclose(f);

#ifdef HAVE_SHADOW
	f = fopen(_PATH_SHADOW, "r");
	if (!f)
		goto error;
	if (load_spwd(pg, f) == -1)
		goto error;
	fclose(f);
#endif

	f = fopen(_PATH_GROUP, "r");
	if (!f)
		goto error;
	if (load_group(pg, f) == -1)
		goto error;
	fclose(f);

	return 0;
error:
	if (f)
		fclose(f);
	return -1;
}



struct pwgrp *
pwgrp_open()
{
	struct pwgrp *pg;

	pg = pwgrp_new();
	if (!pg)
		goto fail;

	pg->lockfd = pwgrp_lockfd();
	if (pg->lockfd == -1)
		goto fail;
	if (load_all(pg) == -1)
		goto fail;
	return pg;

fail:
	pwgrp_free(pg);
	return NULL;
}

int
pwgrp_close(struct pwgrp *pg)
{
	int ret;

	if (pg->lockfd == -1) {
		errno = EBADF;
		return -1;
	}
	ret = store_all(pg);
	pwgrp_free(pg);
	return ret;
}

void
pwgrp_abort(struct pwgrp *pg)
{
	pwgrp_free(pg);
}

/* Convenience search functions */

struct passwd *
pwgrp_getpwnam(struct pwgrp *pg, const char *name)
{
	unsigned i;

	for (i = 0; i < pg->npasswd; i++) {
		struct passwd *pw = pg->passwd[i];
		if (pw && strcmp(name, pw->pw_name) == 0) {
			return pw;
		}
	}
	return NULL;
}

struct passwd *
pwgrp_getpwuid(struct pwgrp *pg, uid_t uid)
{
	unsigned i;

	for (i = 0; i < pg->npasswd; i++) {
		struct passwd *pw = pg->passwd[i];
		if (pw && pw->pw_uid == uid) {
			return pw;
		}
	}
	return NULL;
}

#ifdef HAVE_SHADOW
struct spwd *
pwgrp_getspnam(struct pwgrp *pg, const char *name)
{
	unsigned i;

	for (i = 0; i < pg->nspwd; i++) {
		struct spwd *sp = pg->spwd[i];
		if (sp && strcmp(name, sp->sp_namp) == 0) {
			return sp;
		}
	}
	return NULL;
}
#endif

struct group *
pwgrp_getgrnam(struct pwgrp *pg, const char *name)
{
	unsigned i;

	for (i = 0; i < pg->ngroup; i++) {
		struct group *gr = pg->group[i];
		if (gr && strcmp(name, gr->gr_name) == 0) {
			return gr;
		}
	}
	return NULL;
}

struct group *
pwgrp_getgrgid(struct pwgrp *pg, gid_t gid)
{
	unsigned i;

	for (i = 0; i < pg->ngroup; i++) {
		struct group *gr = pg->group[i];
		if (gr && gr->gr_gid == gid) {
			return gr;
		}
	}
	return NULL;
}

int
pwgrp_getgrouplist(struct pwgrp *pg, const char *user, gid_t group,
	gid_t *groups, int *ngroups)
{
	int n = 0;
	int nmax = *ngroups;
	unsigned i;

	if (group >= 0) {
		if (n < nmax) groups[n] = group;
		n++;
	}

	for (i = 0; i < pg->ngroup; i++) {
		char **mem;
		struct group *gr = pg->group[i];
		if (!gr || !gr->gr_mem || gr->gr_gid == group)
			continue;
		for (mem = gr->gr_mem; *mem; mem++) {
			if (strcmp(*mem, user) == 0) {
				if (n < nmax) groups[n] = gr->gr_gid;
				n++;
				break;
			}
		}
	}
	return n;
}

