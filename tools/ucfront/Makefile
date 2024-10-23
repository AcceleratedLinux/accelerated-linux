HOSTCC ?= cc

UTIL=args.c util.c execute.c

all: ucfront ucfront-ld ucfront-env jlibtool

ucfront: ucfront.c $(UTIL)
ucfront: compile_commands.c json.c
ucfront:
	$(HOSTCC) -g -Werror -Wall -DHAVE_MKSTEMP -o $@ $^

jlibtool: jlibtool.c
	$(HOSTCC) -o $@ $^

ucfront-ld: ucfront-ld.c $(UTIL)
	$(HOSTCC) -g -Werror -Wall -DHAVE_MKSTEMP -o $@ $^

ucfront-env: ucfront-env.c
	$(HOSTCC) -g -Werror -Wall -o $@ $^

clean:
	$(RM) -f *.o ucfront ucfront-ld ucfront-env jlibtool core tags
	$(RM) compile_commands_test

ucfront.1: ucfront.yo
	-yodl2man -o ucfront.1 ucfront.yo

# Tests for the compile_commands.json integration
compile_commands_test: CFLAGS = -g -Wall -pedantic
compile_commands_test: CC = $(HOSTCC)
compile_commands_test: compile_commands.o
compile_commands_test: compile_commands_test.o
compile_commands_test: json.o
compile_commands_test:
	$(LINK.c) -o $@ $^ $(LDLIBS)
check:: compile_commands_test compile_commands-tests.sh
	bash compile_commands-tests.sh ./compile_commands_test
