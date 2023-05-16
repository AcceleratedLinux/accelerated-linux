#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <openssl/sha.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#define MAX_USER 255
#define STATE_AUTH_SUCCESS 7
#define SESSION_LEN 32		/* Maximum session ID strlen */

static bool is_initialised;
static char username[MAX_USER + 1];
static char session[SESSION_LEN + 1];	/* Session ID string */
static int auth_pid;
static int auth_state;

/*
 * Longevity of session ID file, in seconds. Used to decide whether
 * the current session ID file is expired or not
 */
static long expiry_time;

/*
 * Longevity of session ID file, in minutes. Used to renew the current
 * session's expiry_time whenever the current session ID file is checked
 * (along with user's requests), if configured so
 */
static long session_len;
static bool autorefresh;
static char *src_ip = NULL;
static long src_port;
static long session_start;
static char *session_key = NULL;

#define SESSION_ID_PATH_PREFIX "/var/run/.sessions/session-"
#define SESSION_ID_PATH_FORMAT "/var/run/.sessions/session-%s"
#define SESSION_ID_PATH_LEN (strlen(SESSION_ID_PATH_PREFIX) + SESSION_LEN + 1)

enum parse_state {
	COOKIE_NAME,
	COOKIE_VALUE
};

static void reset_session() {
	is_initialised = false;
	*username = '\0';
	*session = '\0';
	auth_pid = 0;
	auth_state = 0;
	expiry_time = 0;
	session_len = 0;
	autorefresh = false;
	free(src_ip);
	src_ip = NULL;
	free(session_key);
	session_key = NULL;
	src_port = 0;
	session_start = 0;
}

static char **get_session_cookie(const char *cookie_str)
{
	const char *cookie = cookie_str;
	char **ret = NULL;
	int pass = 1;

	if (!cookie) {
		cookie = getenv("HTTP_COOKIE");
		if (!cookie) {
			return NULL;
		}
	}

	/*
	 * First pass counts the cookies and allocs the return array
	 * second pass allocs and inserts the cookies into the array
	 */
	while (1) {
		int num_cookies = 0;
		char *cookie_cpy = strdupa(cookie);
		enum parse_state state = COOKIE_NAME;
		char *tok = strtok(cookie_cpy, ";=\t\r\n ");
		while (tok != NULL) {
			switch (state) {
				case COOKIE_NAME:
				if (strcmp(tok, "OgSessionId") == 0) {
					state = COOKIE_VALUE;
				}
				break;
				case COOKIE_VALUE:
				state = COOKIE_NAME;
				if (pass == 2) {
					ret[num_cookies] = strdup(tok);
					if (!ret[num_cookies]) {
						return NULL;
					}
				}
				num_cookies++;

				break;
			}
			tok = strtok(NULL, ";=\t\r\n ");

                }
		if (pass == 1) {
			/* Allocate our cookie array - leave the last entry free to indicate the end
			 * of the array
			 */
			ret = (char **)calloc(num_cookies + 1, sizeof(char *));
			if (!ret)
				return NULL;
			pass++;
		} else if (pass == 2) {
			return ret;
		}
	}
}

static bool load_session(char **session_ids)
{
	FILE *fp = NULL;
	char *session_id = NULL;
	int i;
	char fname[SESSION_ID_PATH_LEN];
	char *line = NULL;
	char *toki = NULL;
	char *tokj = NULL;
	size_t n = 0;

	for (i = 0; session_ids[i] != NULL; i++) {
		snprintf(fname, SESSION_ID_PATH_LEN, SESSION_ID_PATH_FORMAT,
			 session_ids[i]);
		fp = fopen(fname, "r");
		if (fp) {
			session_id = session_ids[i];
			break;
		}
	}

	if (fp == NULL) {
		return false;
	}

	while (!feof(fp)) {
		if (getline(&line, &n, fp) == -1) {
			/* error only, eof has been detected by feof() */
			break;
		}

		if (line == NULL) {
			continue;
		}

		/* Tokenize the input line, splitting on white space */
		toki = strtok(line, "= \r\n");
		tokj = strtok(NULL, "= \r\n");

		if (toki == NULL || tokj == NULL) {
			free(line);
			continue;
		}
		if (strcmp(toki, "username") == 0) {
			strcpy(username, tokj);
		} else if (strcmp(toki, "expiry_time") == 0) {
			expiry_time = strtol(tokj, NULL, 10);
		} else if (strcmp(toki, "auth_state") == 0) {
			auth_state = strtol(tokj, NULL, 10);
		} else if (strcmp(toki, "session_len") == 0) {
			session_len = strtol(tokj, NULL, 10);
		} else if (strcmp(toki, "autorefresh") == 0) {
			autorefresh = (strcmp(tokj, "true") == 0);
		} else if (strcmp(toki, "auth_pid") == 0) {
			auth_pid = strtol(tokj, NULL, 10);
		} else if (strcmp(toki, "src_ip") == 0) {
			free(src_ip);
			src_ip = strdup(tokj);
		} else if (strcmp(toki, "src_port") == 0) {
			src_port = strtol(tokj, NULL, 10);
		} else if (strcmp(toki, "session_start") == 0) {
			session_start = strtol(tokj, NULL, 10);
		} else if (strcmp(toki, "session_key") == 0) {
			free(session_key);
			session_key = strdup(tokj);
		}
	}

	if (line)
		free(line);

	fclose(fp);

	/* Check the expiry time */
	if (time(NULL) > expiry_time) {
		reset_session();
		unlink(fname);
		return false;
	}

	strncpy(session, session_id, SESSION_LEN);
	return true;
}

void
save_session()
{
	char fname[SESSION_ID_PATH_LEN];
	FILE *fp = NULL;

	snprintf(fname, SESSION_ID_PATH_LEN, SESSION_ID_PATH_FORMAT, session);

	fp = fopen(fname, "w");
	if (fp) {
		fprintf(fp, "username = %s\n", username);
		fprintf(fp, "expiry_time = %ld\n", expiry_time);
		fprintf(fp, "auth_pid = %d\n", auth_pid);
		fprintf(fp, "auth_state = %d\n", auth_state);
		fprintf(fp, "session_len = %ld\n", session_len);
		fprintf(fp, "autorefresh = %s\n", (
				autorefresh ? "true" : "false"));
		fprintf(fp, "src_ip = %s\n", src_ip);
		fprintf(fp, "src_port = %ld\n", src_port);
		fprintf(fp, "session_start = %ld\n", session_start);
		if (session_key) {
			/* session_key is an optional field used by the Web UI for CSRF tokens */
			fprintf(fp, "session_key = %s\n", session_key);
		}
		fclose(fp);
	}
}

const char *
get_username()
{
	return username;
}

bool
get_auth_success()
{
	return auth_state == STATE_AUTH_SUCCESS && is_initialised == true;
}

char *
get_session_hash(const char *usage)
{
	SHA512_CTX ctx;
	SHA512_Init(&ctx);

	// concatenate the sessionId with the usage key
	SHA512_Update(&ctx, session, SESSION_LEN);
	SHA512_Update(&ctx, usage, strlen(usage));

	unsigned char result[SHA256_DIGEST_LENGTH];
	SHA512_Final(result, &ctx);

	// convert result into a hex string
	char *hex_result = calloc(SHA256_DIGEST_LENGTH * 2, sizeof(char));
	size_t i;
	for (i = 0; i < SHA256_DIGEST_LENGTH; i++) {
		static const char digit[] = "0123456789abcdef";
		hex_result[i * 2] = digit[(result[i] >> 4) & 0xf];
		hex_result[i * 2 + 1] = digit[result[i] & 0xf];
	}

	return hex_result;
}

void
init_session(const char *cookie)
{
	char **session_ret = NULL;
	int i;

	reset_session();

	session_ret = get_session_cookie(cookie);
	if (session_ret != NULL) {
		is_initialised = load_session(session_ret);

		for (i = 0; session_ret[i] != NULL; i++) {
			free(session_ret[i]);
		}
		free(session_ret);
	}

	if (get_auth_success() && autorefresh == true) {
		expiry_time = time(NULL) + (session_len * 60);
		save_session();
	}
}
