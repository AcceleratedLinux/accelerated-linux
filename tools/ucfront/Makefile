HOSTCC ?= cc

UTIL=args.c util.c execute.c

all: ucfront ucfront-ld ucfront-env jlibtool

ucfront: ucfront.c $(UTIL)
	$(HOSTCC) -g -Werror -Wall -DHAVE_MKSTEMP -o $@ $^

jlibtool: jlibtool.c
	$(HOSTCC) -o $@ $^

ucfront-ld: ucfront-ld.c $(UTIL)
	$(HOSTCC) -g -Werror -Wall -DHAVE_MKSTEMP -o $@ $^

ucfront-env: ucfront-env.c
	$(HOSTCC) -g -Werror -Wall -o $@ $^

clean:
	$(RM) -f *.o ucfront ucfront-ld ucfront-env jlibtool core tags

ucfront.1: ucfront.yo
	-yodl2man -o ucfront.1 ucfront.yo
