#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int ret;
	char rpcdata[1024];
	struct rpcent rpcbuf, *ent;

	while ((ret = getrpcent_r(&rpcbuf, rpcdata, sizeof(rpcdata), &ent)) == 0) {
		printf("%s: %i", ent->r_name, ent->r_number);
		while (ent->r_aliases[0])
			printf(" %s", *ent->r_aliases++);
		printf("\n");
	}

	if (ret != ENOENT)
		printf("Test failed: %s\n", strerror(ret));

	endrpcent();

	return 0;
}
