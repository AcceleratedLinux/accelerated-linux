#define __USE_GNU
#define __STDIO_GLIBC_CUSTOM_STREAMS
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <paths.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <poll.h>

#include <scew/scew.h>
#include <opengear/pmctl.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

struct pmctl {
	FILE *cmdfp;
	int cmdfd;
	bool reserved;
};

static void xbasename(const char *src, char *dst, size_t len);
static bool pmctl_isport(const char *portname);
static int pmctl_opensocket(void);
static int pmctl_reserve_cmd(
	pmctl_t *pm, const char* clientname, const char *path, bool reserve);

pmctl_t *
pmctl_open()
{
	pmctl_t *pm = NULL;

	pm = calloc(1, sizeof(*pm));
	if (pm == NULL) {
		return (NULL);
	}

	/* Open the command socket */
	pm->cmdfd = pmctl_opensocket();
	if (pm->cmdfd < 0) {
		goto fail;
	}
	pm->cmdfp = fdopen(pm->cmdfd, "w+");
	if (pm->cmdfp == NULL) {
		goto fail;
	}

	return (pm);
fail:
	if (pm != NULL) {
		if (pm->cmdfp != NULL) {
			fclose(pm->cmdfp);
		} else if (pm->cmdfd != -1) {
			close(pm->cmdfd);
		}
		free(pm);
	}
	return (NULL);
}

void
pmctl_close(pmctl_t *pm)
{
	if (pm != NULL) {
		fclose(pm->cmdfp); /* Implicitly closes pm->cmdfd */
		free(pm);
	}
}

int
pmctl_reserve_port(pmctl_t *pm, const char *clientname, const char *portname)
{
	return pmctl_reserve_cmd(pm, clientname, portname, true);
}

int
pmctl_unreserve_port(pmctl_t *pm, const char *portname)
{
	return pmctl_reserve_cmd(pm, NULL, portname, false);
}

static int
pmctl_opensocket(void)
{
	int cmdfd;
	struct sockaddr_un addr;
	int error = 0;

	cmdfd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (cmdfd < 0) {
		error = errno;
		fprintf(stderr, "socket(AF_LOCAL) failed: %s\n",
			strerror(errno));
		errno = error;
		return (-1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_LOCAL;
	snprintf(addr.sun_path, sizeof(addr.sun_path),
		"%sportmanager/portmanager.cmd", _PATH_VARRUN);

	if (connect(cmdfd, (struct sockaddr *)&addr, SUN_LEN(&addr)) < 0) {
		error = errno;
		fprintf(stderr, "connect(%s) failed: %s\n",
			addr.sun_path, strerror(errno));
		close(cmdfd);
		errno = error;
		return (-1);
	}

	return (cmdfd);
}

static int (*pmctl_rpc_cb)(void *, const scew_tree *);
static void *pmctl_rpc_cbptr;
static int pmctl_rpc_rc;
static bool pmctl_rpc_done;

static scew_bool
pmctl_rpc_callback(scew_parser *parser_unused, void *data, void *user_data)
{
	scew_tree *tree = (scew_tree *)data;
	scew_element *root = scew_tree_root(tree);
	if (root != NULL) {
		pmctl_rpc_done = true;
		if (pmctl_rpc_cb != NULL) {
			pmctl_rpc_rc = (*pmctl_rpc_cb)(pmctl_rpc_cbptr, tree);
		}
	}
	return (true);
}

static int
pmctl_reserve_callback(void *ptr, const scew_tree *tree)
{
	scew_element *root = scew_tree_root(tree);
	scew_element *response = NULL;

	ptr = NULL;
	response = scew_element_by_index(root, 0);
	if (response != NULL &&
		strcmp(scew_element_name(response), "success") == 0) {
		return (0);
	}
	return (-1);
}

static bool
pmctl_mapport(const char *portname, char *buf, size_t len)
{
	char ttypath[200], portpath[200];
	struct stat ttyst, portst;
	int i;

	snprintf(ttypath, sizeof(ttypath), "/dev/%s", portname);
	if (stat(ttypath, &ttyst) != 0) {
		return (false);
	}

	for (i = 1; i <= 1024; ++i) {
		snprintf(portpath, sizeof(portpath), "/dev/port%02d", i);
		if (stat(portpath, &portst) != 0) {
			return (false);
		}
		if (portst.st_rdev != ttyst.st_rdev) {
			continue;
		}
		snprintf(buf, len, "port%02d", i);
		return (true);
	}
	return (false);
}

static bool
pmctl_isport(const char *portname)
{
	char pmpath[200];
	struct stat st;

	/*
	 * If /var/run/portmanager/portXX.data exists, then it is managed by
	 * portmanager, and it needs to be told to get out of the way.
	 */
	snprintf(pmpath, sizeof(pmpath), "%sportmanager/%s.data",
		_PATH_VARRUN, portname);

	return (stat(pmpath, &st) == 0);
}

static int
pmctl_reserve_cmd(
	pmctl_t *pm, const char *clientname, const char *path, bool reserve)
{
	scew_tree *request = scew_tree_create();
	scew_element *elem = NULL, *subelem = NULL, *root = NULL;
	char portname[50];
	int rc = -1;

	if (request == NULL) {
		goto onexit;
	}

	/*
	 * Get the last component of portname if it is /dev/blah
	 */
	xbasename(path, portname, sizeof(portname));

	/*
	 * Map /dev/ttyS* -> /dev/portXX
	 */
	pmctl_mapport(portname, portname, sizeof(portname));

	/*
	 * Return success if portmanager does not manage this port
	 */
	if (!pmctl_isport(portname)) {
		rc = 0;
		goto onexit;
	}

	root = scew_tree_set_root(request, "command");
	if (root == NULL) {
		goto onexit;
	}
	elem = scew_element_add(root, reserve ? "reserve" : "unreserve");
	if (elem == NULL) {
		goto onexit;
	}
	subelem = scew_element_add(elem, "port");
	if (subelem == NULL) {
		goto onexit;
	}
	scew_element_set_contents(subelem, portname);

	if (clientname != NULL) {
		subelem = scew_element_add(elem, "client");
		if (subelem == NULL) {
			goto onexit;
		}
		scew_element_set_contents(subelem, clientname);
	}

	rc = pmctl_xml_request(pm, request, pmctl_reserve_callback, NULL);

onexit:
	if (request != NULL) {
		scew_tree_free(request);
	}
	if (rc < 0) {
		errno = EBUSY;
		return (-1);
	}
	return (0);
}

int
pmctl_xml_request(pmctl_t *pm, const scew_tree *request,
    int (*func)(void *, const scew_tree *), void *cbptr)
{
	char *memptr = NULL;
	size_t memsize = 0;
	FILE *fp = open_memstream(&memptr, &memsize);
	size_t wlen = 0;
	if (fp == NULL) {
		return (-1);
	}
	scew_reader *reader = NULL;
	scew_parser *parser = scew_parser_create();
	scew_writer *writer = scew_writer_fp_create(fp);
	scew_printer *printer = scew_printer_create(writer);
	pmctl_rpc_cb = func;
	pmctl_rpc_cbptr = cbptr;
	pmctl_rpc_done = false;

	if (parser == NULL || writer == NULL || printer == NULL) {
		goto fail;
	}

	if (!scew_printer_print_tree(printer, request)) {
		fprintf(stderr, "Failed to send XML request\n");
		goto fail;
	}

	fflush(fp);
	scew_parser_set_tree_hook(parser, pmctl_rpc_callback, NULL);
	wlen = write(pm->cmdfd, memptr, memsize);
	if (wlen < 0) {
		return (-1);
	}

	while (!pmctl_rpc_done) {
		char buf[8 * 1024];
		struct pollfd pfd;
		ssize_t rlen;
		/*
		 * Wait for the response - error if it isn't timely
		 */
		pfd.fd = pm->cmdfd;
		pfd.events = POLLIN;

		/*
		 * FIXME: bumped timeout for power commands, it is
		 * too long for XML config requests
		 */
		if (poll(&pfd, 1, 30000) < 1) {
			fprintf(stderr, "Timeout while reading XML reply\n");
			goto fail;
		}

		rlen = recv(pm->cmdfd, buf, sizeof(buf), 0);
		if (rlen <= 0) {
			fprintf(stderr, "Failure while reading XML reply\n");
			goto fail;
		}
		reader = scew_reader_buffer_create(buf, rlen);
		if (reader == NULL) {
 			goto fail;
 		}
		scew_parser_load_stream(parser, reader);
		scew_reader_free(reader);
 	}

	scew_printer_free(printer);
	scew_writer_free(writer);
 	scew_parser_free(parser);
	pmctl_rpc_cb = NULL;
	pmctl_rpc_cbptr = NULL;
	pmctl_rpc_done = false;

 	return (pmctl_rpc_rc);
 fail:
	pmctl_rpc_cb = NULL;
	pmctl_rpc_cbptr = NULL;
	pmctl_rpc_done = false;
	if (printer != NULL) {
		scew_printer_free(printer);
	}
	if (writer != NULL) {
		scew_writer_free(writer);
		fp = NULL;
	}
	if (reader != NULL) {
		scew_reader_free(reader);
 	}
 	if (parser != NULL) {
 		scew_parser_free(parser);
 	}
	if(fp != NULL) {
		fclose(fp);
	}
 	return (-1);
 }


static void
xbasename(const char *src, char *dst, size_t len)
{
	char *p = strdup(src);
	if (p != NULL) {
		memset(dst, '\0', len);
		strncpy(dst, basename(p), len - 1);
		free(p);
	}
}
