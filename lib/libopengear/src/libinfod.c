#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <opengear/libinfod.h>
#include <jansson.h>
#include <unistd.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static int infod_open_sock(int *fd) {
	struct sockaddr_un remote;
	int sockfd;
	int len;

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		return -1;
	}

	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, UNIX_SOCK_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(sockfd, (struct sockaddr *)&remote, len) != 0) {
		close(sockfd);
		return -1;
	}

	*fd = sockfd;
	return 0;
}

/* in case write() returns the number of bytes that is less than
   requested, keep trying to send the rest.  In the case that none
   of the bytes has been written, this function will serve as a
   retry.  Unit-test by write(len/2).
*/
static int write_remain(int sock, const char *message, int length) {
	int len = length;	// initialize to non-zero
	int remain = length;
	sleep(5);
	while (remain > 0 && len > 0) {
		len = write(sock, message + (length - remain), remain);
		remain -= len;
	}
	return remain == 0 ? 0 : -1;
}

static int __push_text(int sock, const char *prefix, enum infod_type type, const char *message) {
	char *msg = NULL;
	int msg_len;
	int len;

	msg_len = asprintf(&msg, "    %d\x1e%s\x1e%d\x1e%s", INFOD_OP_PUT, prefix, type, message);
	if (msg_len == -1 || msg == NULL) {
		return -1;
	}

	if (msg_len >= INFOD_MAX_MESSAGE) {
		/* Dont send the message */
		free(msg);
		return 0;
	}

	*(uint32_t *)msg = htonl(msg_len - 4);

	if ((len = write(sock, msg, msg_len)) != msg_len) {
		// write could have sent only part of message
		if (write_remain(sock, msg + len, msg_len - len) != 0) {
			free(msg);
			return -1;
		}
	}
	free(msg);

	return 0;
}

static int __push_long(int sock, const char *prefix, enum infod_type type, long number) {
	char *msg = NULL;
	int msg_len;

	msg_len = asprintf(&msg, "    %d\x1e%s\x1e%d\x1e%ld", INFOD_OP_PUT, prefix, type, number);
	if (msg_len == -1 || msg == NULL) {
		return -1;
	}

	if (msg_len >= INFOD_MAX_MESSAGE) {
		/* Dont send the message */
		free(msg);
		return 0;
	}
	*(uint32_t *)msg = htonl(msg_len - 4);

	if (write(sock, msg, msg_len) != msg_len) {
		free(msg);
		return -1;
	}
	free(msg);

	return 0;
}

/* Read up to n bytes from fd,
 * retrying on EAGAIN and short reads.
 * Returns n on success,
 *         -1 on an error
 *         <n on EOF
 */
static int readn(int fd, void *dst, size_t n)
{
	int len = 0;

	while (len < n) {
		int rlen = read(fd, (char *)dst + len, n - len);
		if (rlen == -1 && errno == EAGAIN)
			continue;
		if (rlen == -1)
			return -1;
		if (rlen == 0)
			break;
		len += rlen;
	}
	return len;
}

static int __parse_result(int sock, char **result_data, char **result_prefix, int *result_type, unsigned long long *result_timestamp) {
	uint32_t rxheader;
	int rxlen;
	char *rx = NULL;
	int finished = 0;
	int success = 0;
	char *data = NULL;
	char *prefix = NULL;
	int type = 0;
	unsigned long long timestamp = 0;

	/* Read the result messages */
	while (!finished) {
		char *tok = NULL;
		char *ctx;
		int op = 0;

		/* Read the length header */
		if (readn(sock, &rxheader, sizeof rxheader) != sizeof rxheader)
			goto error;
		rxlen = ntohl(rxheader);

		/* Read the message and terminate as C string */
		rx = malloc(rxlen + 1);
		if (rx == NULL) {
			goto error;
		}
		if (readn(sock, rx, rxlen) != rxlen)
			goto error;
		rx[rxlen] = '\0';

		/* The first field is either INFOD_OP_RESULT or INFOD_OP_RESULT_FINISHED */
		ctx = rx;
		tok = strsep(&ctx, "\x1e");
		if (!tok || sscanf(tok, "%d", &op) != 1)
			goto error;

		switch (op) {
		case INFOD_OP_RESULT:
			/* XXX If we receive multiple results, we keep
			 *     and use only the last one. */

			/* Prefix field */
			tok = strsep(&ctx, "\x1e");
			if (!tok)
				goto badproto;
			if (result_prefix) {
				if (prefix)
					free(prefix);
				prefix = strdup(tok);
				if (!prefix)
					goto error;
			}

			/* Liftime/type field */
			tok = strsep(&ctx, "\x1e");
			if (!tok)
				goto badproto;
			if (result_type) {
				if (sscanf(tok, "%d", &type) != 1)
					goto badproto;
			}

			/* Timestamp field */
			tok = strsep(&ctx, "\x1e");
			if (!tok)
				goto badproto;
			if (result_timestamp) {
				if (sscanf(tok, "%llu", &timestamp) != 1)
					goto badproto;
			}

			/* Data field continues to eos */
			if (!ctx)
				goto badproto;
			if (result_data) {
				if (data)
					free(data);
				data = strdup(ctx);
				if (!data)
					goto error;
			}

			success = 1;
			break;

		case INFOD_OP_RESULT_FINISHED:
			finished = 1;
			break;

		default:
			goto badproto;
		}

		free(rx);
		rx = NULL;
	}


	if (success) {
		if (result_data != NULL) {
			*result_data = data;
		}
		if (result_prefix != NULL) {
			*result_prefix = prefix;
		}
		if (result_type != NULL) {
			*result_type = type;
		}
		if (result_timestamp != NULL) {
			*result_timestamp = timestamp;
		}
		return 0;
	}

badproto:
	errno = EPROTO;
error:
	if (rx)
		free(rx);
	if (data)
		free(data);
	if (prefix)
		free(prefix);

	return -1;
}

int __delete(int sock, const char *prefix) {
	char *msg = NULL;
	int msg_len;

	msg_len = asprintf (&msg, "    %d\x1e%s", INFOD_OP_DELETE, prefix);
	if (msg_len == -1 || msg == NULL) {
		return -1;
	}

	*(uint32_t *)msg = htonl (msg_len - 4);

	if (write (sock, msg, msg_len) != msg_len) {
		free (msg);
		return -1;
	}
	free (msg);
	return 0;
}
// ----------------------- __get_extended() ---------------------------
int __get_extended(int sock, const char *prefix, char **result_data, char **result_prefix, int *result_type, unsigned long long *result_timestamp) {
	char *msg = NULL;
	int msg_len;

	msg_len = asprintf(&msg, "    %d\x1e%s", INFOD_OP_GET, prefix);
	if (msg_len == -1 || msg == NULL) {
		return -1;
	}

	*(uint32_t *)msg = htonl(msg_len - 4);

	if (write(sock, msg, msg_len) == -1) {
		syslog(LOG_ERR, "write of [%s] failed with [%s]", msg, strerror(errno));
		free(msg);
		return -1;
	}
	fsync(sock);
	free(msg);

	return __parse_result(sock, result_data, result_prefix, result_type, result_timestamp);
}
// ---------------------
int __get(int sock, const char *prefix, char **result_ret) {
	return __get_extended(sock, prefix, result_ret, NULL, NULL, NULL);
}

int __construct_array(const char *msg, char ***array, size_t *num_elements) {
	json_error_t error;
	char **args;
	int nelem, i;

	json_t *root = json_loads(msg, 0, &error);
	if (!root) {
		return -1;
	}

	if (!json_is_array(root)) {
		goto failed;
	}

	nelem = json_array_size(root);

	args = (char **)calloc(sizeof(char *), nelem);
	if (!args) {
		goto failed;
	}

	for (i = 0; i < nelem; i++) {
		json_t *entry = json_array_get(root, i);
		if (!json_is_string(entry)) {
			goto error_array;
		}

		args[i] = strdup(json_string_value(entry));
		if (!args[i]) {
			goto error_array;
		}
	}

	json_decref(root);
	*array = args;
	*num_elements = nelem;
	return 0;

error_array:
	while (i > 0) {
		free(args[--i]);
	}
	free(args);

failed:
	json_decref(root);
	return -1;
}

int __parse_array(char **msg, const char **array, int num_elements) {
	int i;
	json_t *root = json_array();
	if (!root || !array) {
		return -1;
	}
	for (i = 0; i < num_elements; i++) {
		if (array[i]) {
			json_t *str = json_string(array[i]);
			if (str) {
				json_array_append(root, str);
				continue;
			}
		}
		json_decref(root);
		return -1;
	}
	(*msg) = json_dumps(root, JSON_COMPACT | JSON_PRESERVE_ORDER);
	json_decref(root);
	if (!(*msg)) {
		return -1;
	}
	return 0;
}

/*
 * infod sync function
 * Used by clients to check if infod has been running, when clients are
 * started earlier than infod
 *
 * Needs refactoring - the established connection should not be discarded
 * and should have been returned to callers to be furtehr made use of
 */
int infod_sync(int timeout) {
	int counter;
	int infod_sock = -1;

	/* Slice timeout into increments */
	counter = (timeout + (SYNC_INCREMENT - 1)) / SYNC_INCREMENT;

	/* Sleep as long as the socket can't be opened and we haven't timed out */
	while ((infod_open_sock(&infod_sock) != 0) && counter--) {
		sleep(SYNC_INCREMENT);
	}

	/* If we fall out without an opened socket we have failed */
	if (infod_sock == -1) {
		return -1;
	}

	close(infod_sock);
	return 0;
}

/* infod push functions */
/* one off push functions - ie from scripts - dialup up, dialup down */
int infod_push_text(const char *prefix, enum infod_type type, const char *message) {
	int infod_sock;
	if (infod_open_sock(&infod_sock) != 0) {
		return -1;
	}

	if (__push_text(infod_sock, prefix, type, message) != 0) {
		close(infod_sock);
		return -1;
	}

	close(infod_sock);
	return 0;
}

int infod_push_long(const char *prefix, enum infod_type type, long number) {
	int infod_sock;
	if (infod_open_sock(&infod_sock) != 0) {
		return -1;
	}

	if (__push_long(infod_sock, prefix, type, number) != 0) {
		close(infod_sock);
		return -1;
	}

	close(infod_sock);
	return 0;
}
/* multiple push functions - from longer running processes, like emd, upsd, portmanager */
int infod_push_multiple_start(int *fd) {
	return infod_open_sock(fd);
}

int infod_multiple_start_unix(int *fd) {
	return infod_open_sock(fd);
}

int infod_push_multiple_text(int fd, const char *prefix, enum infod_type type, const char *message) {
	return __push_text(fd, prefix, type, message);
}
int infod_push_multiple_long(int fd, const char *prefix, enum infod_type type, long number) {
	return __push_long(fd, prefix, type, number);
}

int infod_push_multiple_end(int fd) {
	close(fd);
	return 0;
}

/* infod get function */
int infod_get(const char *prefix, char **msg_ret) {
	return infod_get_extended(prefix, msg_ret, NULL, NULL, NULL);
}

int infod_delete(const char *prefix) {
	int infod_sock;
	if (infod_open_sock(&infod_sock) != 0) {
		return -1;
	}
	int result = __delete (infod_sock, prefix);
	close (infod_sock);
	return result;
}

int infod_get_extended(const char *prefix, char **result_data, char **result_prefix, int *result_type, unsigned long long *result_timestamp) {

	int infod_sock;
	if (infod_open_sock(&infod_sock) != 0) {
		return -1;
	}

	if (__get_extended(infod_sock, prefix, result_data, result_prefix, result_type, result_timestamp) != 0) {
		close(infod_sock);
		return -1;
	}

	close(infod_sock);
	return 0;
}

/* infod_get multiple */

int infod_get_multiple_start(int *fd) {
	return infod_open_sock(fd);
}

int infod_get_multiple(int fd, const char *prefix, char **ret) {
	return __get(fd, prefix, ret);
}

int infod_get_multiple_extended(int fd, const char *prefix, char **result_data, char **result_prefix, int *result_type, unsigned long long *result_timestamp) {
	return __get_extended(fd, prefix, result_data, result_prefix, result_type, result_timestamp);
}

int infod_get_multiple_end(int fd) {
	close(fd);
	return 0;
}

/* infod_array functions */
int infod_multiple_get_array(int fd, const char *prefix, char ***array, size_t *num_elements) {
	char *msg = NULL;
	if (__get(fd, prefix, &msg) != 0) {
		(*num_elements) = 0;
		(*array) = 0;
		return -1;
	}
	if (__construct_array(msg, array, num_elements) != 0) {
		free(msg);
		return -1;
	}
	free(msg);
	return 0;
}

int infod_multiple_push_array(int fd, const char *prefix, enum infod_type type, const char **array, size_t num_elements) {
	char *msg = NULL;
	if (!array || !prefix) {
		return -1;
	}
	if (__parse_array(&msg, array, num_elements) != 0) {
		return -1;
	}
	
	if (__push_text(fd, prefix, type, msg) != 0) {
		free(msg);
		return -1;
	}
	free(msg);
	return 0;
}
