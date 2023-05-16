#include <assert.h>
#include <string.h>
#include "mock-vfile.h"
#include "../vfile.h"
#include "../multipart.h"

static const char t1_body[] =
"-----------------------------267955271739594854962863136478\r\n"
"Content-Disposition: form-data; name=\"flashfile\"; filename=\"bar\"\r\n"
"Content-Type: application/octet-stream\r\n"
"\r\n"
"foo\r\n"
"-----------------------------267955271739594854962863136478\r\n"
"Content-Disposition: form-data; name=\"flashopts\"\r\n"
"\r\n"
"\r\n"
"-----------------------------267955271739594854962863136478\r\n"
"Content-Disposition: form-data; name=\"form\"\r\n"
"\r\n"
"firmware\r\n"
"-----------------------------267955271739594854962863136478\r\n"
"Content-Disposition: form-data; name=\"apply\"\r\n"
"\r\n"
"Apply\r\n"
"-----------------------------267955271739594854962863136478--\r\n"
;
int
main()
{
	struct cgi_req t1_req = {
		.request_method = "POST",
		.content_length = "586",
		.content_type = "multipart/form-data; "
	"boundary=---------------------------267955271739594854962863136478",
		.content = mock_vfile(t1_body)
	};
	char name[64];
	struct vfile *vf;

	struct multipart *m = multipart_new(t1_req);
	assert(m);

	/* flashfile part */
	assert((vf = multipart_part(m, name)));
	assert(strcmp(name, "flashfile") == 0);
	assert(vfile_read(vf, name, sizeof name) == 3);
	assert(memcmp(name, "foo", 3) == 0);
	vfile_decref(vf);

	/* flashopts part */
	assert((vf = multipart_part(m, name)));
	assert(strcmp(name, "flashopts") == 0);
	assert(vfile_read(vf, name, sizeof name) == 0);
	vfile_decref(vf);

	/* form part */
	assert((vf = multipart_part(m, name)));
	assert(strcmp(name, "form") == 0);
	vfile_decref(vf);

	/* apply button */
	assert((vf = multipart_part(m, name)));
	assert(strcmp(name, "apply") == 0);
	vfile_decref(vf);

	assert(!multipart_part(m, name));

	multipart_free(m);
}
