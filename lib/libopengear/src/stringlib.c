#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/* This function will replace any instances of old_str in str with rep_str
 * str is assumed to be a malloc'ed string, and will be resized with realloc
 * if rep_str is larger than old_str. A pointer to str will be returned.
 */
char *strrepl(char *str, char *old_str, char *rep_str) {
	char *str_ptr = str;
	int offset = 0;
	char *ptr = NULL;
	int old_str_len = strlen(old_str);
	int new_str_len = strlen(rep_str);

	while ((ptr = strstr(str_ptr, old_str)) != NULL) {
		offset = (ptr - str_ptr);
		/* Expand the string if required */
		if (old_str_len < new_str_len) {
			str_ptr = realloc(str_ptr, (strlen(str_ptr) + (new_str_len - old_str_len) + 1));
			if (str_ptr == NULL) {
				return NULL;
			}
			ptr = str_ptr + offset;
		}
		memmove(ptr + new_str_len, ptr + old_str_len, strlen(ptr + old_str_len) + 1);
		memcpy(ptr, rep_str, new_str_len);
	}
	return str_ptr;
}

char *strappend(char *dest, const char *src, const char *delim) {
	size_t newlen  = 0;
	char *ret;
	if (dest != NULL) {
		newlen += strlen(dest);
	}
	if (src != NULL) {
		newlen += strlen(src);
	}
	if (delim != NULL) {
		newlen += strlen(delim);
	}
	if (newlen == 0)
		return NULL;

	ret = realloc(dest, newlen + 1);
	if (!ret)
		return NULL;

	if (!dest)
		memset(ret, 0, newlen + 1);

	if (delim)
		strcat(ret, delim);

	if (src)
		strcat(ret, src);

	return ret;
}


char *strtoupper(const char *orig) {
	char *ptr;
	char *ret = strdup(orig);
	if (!ret)
		return NULL;

	ptr = ret;
	while ((*ptr) != '\0') {
		(*ptr) = toupper(*ptr);
		ptr++;
	}
	return ret;
}

char *strtolower(const char *orig) {
	char *ptr;
	char *ret = strdup(orig);
	if (!ret)
		return NULL;

	ptr = ret;
	while ((*ptr) != '\0') {
		(*ptr) = tolower(*ptr);
		ptr++;
	}
	return ret;
}

size_t trimwhitespace(char *out, size_t len, const char *str)
{
	if(len == 0)
		return 0;

	const char *end;
	size_t out_size;

	// Trim leading space
	while(isspace(*str)) str++;

	if(*str == 0)  // All spaces?
	{
		*out = 0;
		return 1;
	}

	// Trim trailing space
	end = str + strlen(str) - 1;
	while(end > str && isspace(*end)) end--;
	end++;

	// Set output size to minimum of trimmed string length and buffer size minus 1
	out_size = (end - str) < len-1 ? (end - str) : len-1;

	// Copy trimmed string and add null terminator
	memcpy(out, str, out_size);
	out[out_size] = 0;

	return out_size;
}

char *trimwhitespace_inplace(char *str)
{
	char *end;

	// Trim leading space
	while(isspace(*str)) str++;

	if(*str == 0)  // All spaces?
		return str;

	// Trim trailing space
	end = str + strlen(str) - 1;
	while(end > str && isspace(*end)) end--;

	// Write new null terminator
	*(end+1) = 0;

	return str;
}

