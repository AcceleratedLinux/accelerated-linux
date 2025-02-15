# Generated automatically from Makefile.in by configure.

SHELL=/bin/sh

####################################

# CC = gcc

# CFLAGS =   -O

# MATHLIB = -lm

YACC = byacc 

# where to put mawk
BINDIR = /usr/local/bin
# where to put the man pages
MANDIR = /usr/local/man/man1
MANEXT = 1
#######################################

O=parse.o scan.o memory.o main.o hash.o execute.o code.o\
  da.o error.o init.o bi_vars.o cast.o print.o bi_funct.o\
  kw.o jmp.o array.o field.o  split.o re_cmpl.o zmalloc.o\
  fin.o files.o  scancode.o matherr.o  fcall.o version.o\
  missing.o

REXP_O=rexp/rexp.o rexp/rexp0.o rexp/rexp1.o rexp/rexp2.o\
    rexp/rexp3.o 

REXP_C=rexp/rexp.c rexp/rexp0.c rexp/rexp1.c rexp/rexp2.c\
    rexp/rexp3.c 

all: mawk
mawk_and_test :  mawk  mawk_test  fpe_test

mawk : $(O)  rexp/.done
	$(CC) $(LDFLAGS) -o mawk $(O) $(REXP_O) $(LIBM) $(LDLIBS)

mawk_test :  mawk  # test that we have a sane mawk
	@cp mawk test/mawk
	cd test ; ./mawktest 
	@rm test/mawk

fpe_test :  mawk # test FPEs are handled OK
	@cp mawk test/mawk
	@echo ; echo testing floating point exception handling
	cd test ; ./fpe_test
	@rm test/mawk

rexp/.done : $(REXP_C)
	cd rexp ;\
	$(MAKE) CC="$(CC)" CFLAGS="$(CFLAGS) -DMAWK -I.." 

parse.c  : parse.y  
ifdef REBUILD_AUTOGEN_FILES
	@echo  expect 4 shift/reduce conflicts
	$(YACC) -d parse.y
	mv y.tab.c parse.c
	-if cmp -s y.tab.h parse.h ;\
	   then rm y.tab.h ;\
	   else mv y.tab.h parse.h ; fi
else
	@touch parse.c
endif

array.c : # array.w
	notangle -R'"array.c"' array.w | cpif array.c

array.h : # array.w
	notangle -R'"array.h"' array.w | cpif array.h

scancode.c :  # makescan.c  scan.h
	$(CC) -o makescan.exe  makescan.c
	rm -f scancode.c
	./makescan.exe > scancode.c
	rm makescan.exe

MAWKMAN = $(MANDIR)/mawk.$(MANEXT)
install :  mawk
	cp mawk $(BINDIR)
	chmod  0755  $(BINDIR)/mawk
	cp  man/mawk.1  $(MAWKMAN)
	chmod  0644  $(MAWKMAN)

clean :
	rm -f *.o rexp/*.o rexp/.done test/mawk core test/core mawk *.gdb

distclean :  clean
	rm -f config.h Makefile \
	    config.status config.user config.log config.cache
	rm -f defines.out maxint.out fpe_check
	cp config-user/.config.user config.user ; chmod +w config.user

configure  :  configure.in mawk.ac.m4
	autoconf

romfs:
	$(ROMFSINST) mawk /bin/awk

# output from  mawk -f deps.awk *.c
array.o : config.h field.h bi_vars.h mawk.h symtype.h nstd.h memory.h array.h zmalloc.h types.h sizes.h
bi_funct.o : config.h field.h bi_vars.h mawk.h init.h regexp.h symtype.h nstd.h repl.h memory.h bi_funct.h array.h files.h zmalloc.h fin.h types.h sizes.h
bi_vars.o : config.h field.h bi_vars.h mawk.h init.h symtype.h nstd.h memory.h array.h zmalloc.h types.h sizes.h
cast.o : config.h field.h mawk.h parse.h symtype.h nstd.h memory.h repl.h scan.h array.h zmalloc.h types.h sizes.h
code.o : config.h field.h code.h mawk.h init.h symtype.h nstd.h memory.h array.h jmp.h zmalloc.h types.h sizes.h
da.o : config.h field.h code.h mawk.h symtype.h nstd.h memory.h repl.h bi_funct.h array.h zmalloc.h types.h sizes.h
error.o : config.h bi_vars.h mawk.h parse.h vargs.h symtype.h nstd.h scan.h array.h types.h sizes.h
execute.o : config.h field.h bi_vars.h code.h mawk.h regexp.h symtype.h nstd.h memory.h repl.h bi_funct.h array.h zmalloc.h types.h fin.h sizes.h
fcall.o : config.h code.h mawk.h symtype.h nstd.h memory.h array.h zmalloc.h types.h sizes.h
field.o : config.h field.h bi_vars.h mawk.h init.h parse.h regexp.h symtype.h nstd.h memory.h repl.h scan.h array.h zmalloc.h types.h sizes.h
files.o : config.h mawk.h nstd.h memory.h files.h zmalloc.h types.h fin.h sizes.h
fin.o : config.h field.h bi_vars.h mawk.h parse.h symtype.h nstd.h memory.h scan.h array.h zmalloc.h types.h fin.h sizes.h
hash.o : config.h mawk.h symtype.h nstd.h memory.h array.h zmalloc.h types.h sizes.h
init.o : config.h field.h bi_vars.h code.h mawk.h init.h symtype.h nstd.h memory.h array.h zmalloc.h types.h sizes.h
jmp.o : config.h code.h mawk.h init.h symtype.h nstd.h memory.h array.h jmp.h zmalloc.h types.h sizes.h
kw.o : config.h mawk.h init.h parse.h symtype.h nstd.h array.h types.h sizes.h
main.o : config.h code.h mawk.h init.h symtype.h nstd.h memory.h array.h files.h zmalloc.h types.h sizes.h
makescan.o : parse.h symtype.h scan.h array.h
matherr.o : config.h mawk.h nstd.h types.h sizes.h
memory.o : config.h mawk.h nstd.h memory.h zmalloc.h types.h sizes.h
missing.o : config.h nstd.h
parse.o : config.h field.h bi_vars.h code.h mawk.h symtype.h nstd.h memory.h bi_funct.h array.h files.h zmalloc.h jmp.h types.h sizes.h
print.o : config.h field.h bi_vars.h mawk.h parse.h symtype.h nstd.h memory.h scan.h bi_funct.h array.h files.h zmalloc.h types.h sizes.h
re_cmpl.o : config.h mawk.h parse.h regexp.h symtype.h nstd.h memory.h repl.h scan.h array.h zmalloc.h types.h sizes.h
scan.o : config.h field.h code.h mawk.h init.h parse.h symtype.h nstd.h memory.h repl.h scan.h array.h files.h zmalloc.h types.h fin.h sizes.h
split.o : config.h field.h bi_vars.h mawk.h parse.h regexp.h symtype.h nstd.h memory.h scan.h bi_funct.h array.h zmalloc.h types.h sizes.h
version.o : config.h mawk.h patchlev.h nstd.h types.h sizes.h
zmalloc.o : config.h mawk.h nstd.h zmalloc.h types.h sizes.h
