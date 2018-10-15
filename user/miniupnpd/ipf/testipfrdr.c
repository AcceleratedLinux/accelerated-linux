/* $Id: testipfrdr.c,v 1.1 2008-09-15 12:28:53 winfred Exp $ */

#include <stdio.h>
#include <syslog.h>
#include <netinet/in.h>
#include "ipfrdr.h"

extern void
test_list_nat_rules();
/* test program for ipfrdr.c */

int
main(int argc, char * * argv)
{
	openlog("testipfrdrd", LOG_CONS|LOG_PERROR, LOG_USER);
	printf("List nat rules :\n");
	test_list_nat_rules();
	printf("Add redirection !\n");
	add_redirect_rule2("ep0", 12345, "1.2.3.4", 54321, IPPROTO_UDP,
	                   "redirection description");
	printf("List nat rules :\n");
	test_list_nat_rules();
	return 0;
}

