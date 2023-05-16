#pragma once

struct vfile;
struct multipart;

struct cgi_req {
	const char *request_method;
	const char *content_length;
	const char *content_type;
	struct vfile *content;	/* usually stdin */
};

/* Creates the iterator over a multipart/form-data CGI POST */
struct multipart *multipart_new(struct cgi_req cgi_req);
/* Returns the next form-part as a vfile; stores part name in name[]. */
struct vfile *multipart_part(struct multipart *mp, char name[64]);
/* Destroys the multipart iterator. Active form-part vfiles remain OK to use */
void multipart_free(struct multipart *mp);
