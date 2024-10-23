#pragma once
#include <stdio.h>

#define JSON_WORD 'w'

/*
 * Reads the next JSON token from file f. Returns one of
 *  [ ] { } : , " EOF or JSON_WORD
 * When JSON_WORD is returned, the word is stored in buf[] as UTF-8.
 * When '"' is returned, the decoded string is stored in buf[] as UTF-8.
 * buf[] may be quietly truncated but is always nul-terminated.
 */
int json_read_token(FILE *f, char *buf, size_t bufsz);

/* Encodes s as a JSON string into buf[].
 * Returns buf */
char *json_str(const char *s, char *buf, size_t bufsz);

/* Reads and discards the next JSON value. Returns 0 iff JSON parse error. */
int json_skip_value(FILE *f);
