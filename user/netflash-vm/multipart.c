#define _DEFAULT_SOURCE
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#include "vfile.h"
#include "multipart.h"

/*
 * Decodes a multipart/form-data CGI POST, and returns the field data and
 * file uploads as separate vfiles.
 */
struct multipart {
	struct vfile *content;
	int clen;
	char *boundary;	/* "--<boundary>" */
	int blen;	/* == strlen(boundary) */
	int more;	/* have seen terminating --<boundary>--\r\n */

	int matchlen;	/* prefix len of boundary[] last matched from read */
	char *buffer; /* length is blen */
	int nextpartpos;
};

static inline int
streq(const char *a, const char *b)
{
	return a ? b && strcmp(a, b) == 0 : !b;
}

/* Seek to just after the next boundary instance in the content
 * Returns:
 *    0 found and consumed \r\n--<boundary>\r\n ; sets more=1
 *    0 found and consumed \r\n--<boundary>--\r\n ; sets more=0
 *   -1 unexpected error or EOF
 */
static int
seek_boundary(struct multipart *mp)
{
	/* The first boundary may not have the leading \r\n */
	int blen = mp->more == -1 ? mp->blen - 2 : mp->blen;
	const char *boundary = mp->more == -1 ? mp->boundary + 2 : mp->boundary;

	int o = mp->matchlen;
	int r = blen - mp->matchlen;
	int n = vfile_read(mp->content, mp->buffer, r);
	if (n != r)
		return -1; /* assumes no short reads */
	if (memcmp(&boundary[mp->matchlen], mp->buffer, r) == 0) {
		/* We matched the boundary */
	        char eos[2];
	        if (vfile_read(mp->content, eos, 2) != 2)
			return -1;
		if (eos[0] == '-' && eos[1] == '-') {	/* --B--\r\n */
			vfile_read(mp->content, eos, 2);
			mp->matchlen = 0;
			mp->more = 0;
			return 0;
		}
		if (eos[0] == '\r' && eos[1] == '\n') { /* --B\r\n */
			mp->matchlen = 0;
			mp->more = 1;
			return 0;
		}
		warn("malformed multipart boundary");
		return -1;
	}

	while (mp->matchlen--) {
		if (memcmp(boundary, &boundary[o - mp->matchlen],
			mp->matchlen) != 0)
				continue;
		if (memcmp(&boundary[mp->matchlen], mp->buffer, n) == 0) {
			mp->matchlen += n;
			return seek_boundary(mp);
		}
	}

	for (mp->matchlen = n; mp->matchlen; mp->matchlen--) {
		if (memcmp(boundary, &mp->buffer[n - mp->matchlen],
		    mp->matchlen) == 0)
			break;
	}
	return seek_boundary(mp);
}

struct multipart *
multipart_new(struct cgi_req cgi_req)
{
	char *ctype = NULL;
	struct multipart *mp = NULL;
	char *boundary;

	/* Obtain CGI headers from environ[] and body data from stdin */

	if (!streq(cgi_req.request_method, "POST")) {
		warnx("bad request method");
		goto out;
	}
	if (!cgi_req.content_length) {
		warnx("bad Content-length");
		goto out;
	}

	if (cgi_req.content_type)
		ctype = strdup(cgi_req.content_type);
	if (!streq(strsep(&ctype, "; "), "multipart/form-data")) {
		warnx("bad Content-type");
		goto out;
	}

	for (boundary = NULL; ctype; ) {
		char *s = strsep(&ctype, "; ");
		if (strncmp(s, "boundary=", strlen("boundary=")) == 0) {
			boundary = s + strlen("boundary=");
			if (*boundary == '"' && strlen(boundary) >= 2) {
				/* Remove quotes around <boundary> */
				char *end = boundary + strlen(boundary);
				if (end[-1] == '"') {
					end[-1] = '\0';
					boundary++;
				}
			}
			*--boundary = '-'; /* prepend \r\n-- */
			*--boundary = '-';
			*--boundary = '\n';
			*--boundary = '\r';
			break;
		}
	}
	if (!boundary) {
		warnx("bad Content-type");
		goto out;
	}

	mp = calloc(1, sizeof *mp);
	if (!mp) {
		warn("calloc");
		goto out;
	}

	mp->blen = strlen(boundary);
	mp->boundary = strdup(boundary);

	mp->content = vfile_incref(cgi_req.content);
	mp->clen = atoi(cgi_req.content_length);
	mp->matchlen = 0;
	mp->buffer = malloc(mp->blen);
	mp->more = -1;

	/* Skip leader and initialise mp->more */
	if (seek_boundary(mp) == -1) {
		warnx("error reading first boundary");
		multipart_free(mp);
		mp = NULL;
	}

	mp->nextpartpos = vfile_tell(mp->content);

out:
	free(ctype);
	return mp;
}

/* Read a line ending with CRLF into buf (strip the CRLF) */
static int
read_line(struct vfile *vf, char *buf, size_t bufsz)
{
	int len = 0;
	char ch;
	int n;

	while ((n = vfile_read(vf, &ch, 1)) == 1 && ch != '\n') {
		if (ch != '\r' && len < bufsz - 1)
			buf[len++] = ch;
	}
	if (n != 1)
		errx(1, "error reading part header");
	buf[len] = '\0';
	return len;
}

/* Construct a vfile that accesses the form-data */
struct vfile *
multipart_part(struct multipart *mp, char name[static 64])
{
	char line[1024];
	int len;
	char *v;
	int start, end;

	if (!mp->more)
		return NULL;
	vfile_seek(mp->content, mp->nextpartpos);

	name[0] = '\0';
	while ((len = read_line(mp->content, line, sizeof line))) {
		if (!name[0]) {
			/* lower the header name */
			for (v = line; *v && *v != ':'; v++)
				*v = tolower(*v);
			sscanf(line, "content-disposition: "
				 "form-data; "
				 "name=\"%63[^\"]\"", name);
		}
	}

	start = vfile_tell(mp->content);
	if (seek_boundary(mp) == -1)
		errx(1, "error finding next boundary");
	mp->nextpartpos = vfile_tell(mp->content);
	end = mp->nextpartpos - mp->blen - (mp->more ? 2 : 4);

	return vfile_range(vfile_incref(mp->content),
		start, end - start);
}

void
multipart_free(struct multipart *mp)
{
	if (mp) {
		vfile_decref(mp->content);
		free(mp->buffer);
		free(mp->boundary);
		free(mp);
	}
}
