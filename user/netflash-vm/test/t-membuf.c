#include <assert.h>
#include <string.h>
#include "mock-vfile.h"
#include "../vfile.h"

int
main()
{
	{
		/* We can get the size of a small file */
		struct vfile *mock = mock_vfile("Hello\n");
		struct vfile *f = vfile_membuf(mock);
		assert(vfile_getsize(f) == 6);
		vfile_decref(f);
	}

	{
		/* We can get the size of a 100k file */
		struct vfile *mock = mock_vfilen("0123456789", 10000, 0);
		struct vfile *f = vfile_membuf(mock);
		char buf[10];
		assert(vfile_getsize(f) == 100000);

		/* We can seek and read near the front */
		assert(vfile_seek(f, 1) == 1);
		assert(vfile_tell(f) == 1);
		assert(vfile_read(f, buf, 10) == 10);
		assert(memcmp(buf, "1234567890", 10) == 0);

		/* We can seek and read over two chunks*/
		assert(vfile_seek(f, 65275) == 65275);
		assert(vfile_read(f, buf, 10) == 10);
		assert(memcmp(buf, "5678901234", 10) == 0);

		/* We can seek and short-read to EOF */
		memset(buf, 'x', sizeof buf);
		assert(vfile_seek(f, 99997) == 99997);
		assert(vfile_read(f, buf, 10) == 3);
		assert(memcmp(buf, "789xxxxxxx", 10) == 0);

		/* We can repeatedly read EOF */
		assert(vfile_read(f, buf, 10) == 0);
		assert(vfile_read(f, buf, 10) == 0);
		assert(vfile_tell(f) == 100000);

		vfile_decref(f);
	}
}
