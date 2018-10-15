dnl Check for mmap()
dnl AC_FUNC_MMAP checks for private fixed mappings, we don't need
dnl fixed mappings, so check only wether private mappings work.
dnl AC_FUNC_MMAP would fail on HP-UX for example.
AC_DEFUN([AC_C_FUNC_MMAP_PRIVATE],
[ 
	AC_CACHE_CHECK([for working mmap], [ac_cv_c_mmap_private],
	[
		AC_RUN_IFELSE([AC_LANG_SOURCE([
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <fcntl.h> 
int main(void)
{
	char *data, *data2, *data3;
	unsigned i, datasize = 1024;
	int fd;

  	/* First, make a file with some known garbage in it. */
	data = (char*) malloc(datasize);
	if(!data)
		return 1;
	for(i=0;i<datasize;i++)
		*(data + i) = rand();
	umask(0);
	fd = creat("conftest.mmap", 0600);
	if(fd < 0)
		return 1;
	if(write (fd, data, datasize) != datasize)
		return 1;
	close(fd);
	fd = open("conftest.mmap", O_RDWR);
	if (fd < 0)
		return 1;
	/* Next, try to mmap the file at a fixed address which already has
	   something else allocated at it.  If we can, also make sure that
	   we see the same garbage.  */
	data2 = mmap(NULL, sizeof(data), PROT_READ | PROT_WRITE,
		MAP_PRIVATE, fd, 0L);	
	if(data2 == MAP_FAILED)
		return 2;
	for(i=0;i<sizeof(data);i++)
		if(*(data + i) != *(data2+ i))
			return 3;
	  /* Finally, make sure that changes to the mapped area do not
     	     percolate back to the file as seen by read().  (This is a bug on
             some variants of i386 svr4.0.)  */
	  for (i = 0; i < datasize; ++i)
	    *(data2 + i) = *(data2 + i) + 1;
	data3 = (char*) malloc(datasize);
	if(!data3)
		return 1;
	if(read (fd, data3, datasize) != datasize)
		return 1;
	for(i=0;i<sizeof(data);i++)
		if(*(data + i) != *(data3 + i))
			return 3;
	close(fd);
	return 0;
}])],
	[ac_cv_c_mmap_private=yes],
	[ac_cv_c_mmap_private=no],
	[ac_cv_c_mmap_private=no])])
if test $ac_cv_c_mmap_private = yes; then
	AC_DEFINE(HAVE_MMAP, 1,
		[Define to 1 if you have a working `mmap' system call that supports MAP_PRIVATE.])
fi
rm -f conftest.mmap
])


AC_DEFUN([AC_C_FUNC_MMAP_ANONYMOUS],
[
	AC_CACHE_CHECK([for MAP_ANON(YMOUS)], [ac_cv_c_mmap_anonymous],[
		ac_cv_c_mmap_anonymous='no'
		AC_COMPILE_IFELSE(
			[AC_LANG_PROGRAM([[#include <sys/mman.h>]], [[mmap((void *)0, 0, PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);]])],
			[ac_cv_c_mmap_anonymous='MAP_ANONYMOUS'],
			[
				AC_LINK_IFELSE(
					[AC_LANG_PROGRAM([[
/* OPENBSD WORKAROUND - DND*/
#include <sys/types.h>
/* OPENBSD WORKAROUND - END*/
#include <sys/mman.h>
]], [[mmap((void *)0, 0, PROT_READ | PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);]])],
					[ac_cv_c_mmap_anonymous='MAP_ANON']
				)
			]
		)
	])
	if test "$ac_cv_c_mmap_anonymous" != "no"; then
		AC_DEFINE_UNQUOTED([ANONYMOUS_MAP],[$ac_cv_c_mmap_anonymous],[mmap flag for anonymous maps])
	fi
])
