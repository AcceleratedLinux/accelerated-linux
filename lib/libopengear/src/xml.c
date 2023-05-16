#ifndef __USE_GNU
# define __USE_GNU 1
#endif
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <syslog.h>

#include <scew/scew.h>
#include <opengear/xml.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

const char *
scew_xml_error(scew_parser *parser)
{
	const char *errstr;
	scew_error error = scew_error_code();
	if (error == scew_error_expat) {
		error = scew_error_expat_code(parser);
		errstr = scew_error_expat_string(error);
	} else {
		errstr = scew_error_string(scew_error_code());
	}
	return errstr;
}

const char *
scew_xml_string(const scew_element *e)
{
	if (e == NULL || scew_element_contents(e) == NULL) {
		return "";
	}
	return (scew_element_contents(e));
}

bool
scew_xml_bool(const scew_element *elem)
{
	const char *val = scew_xml_string(elem);
	return strcasecmp(val, "on") == 0 || strcasecmp(val, "true") == 0 ||
	    strcmp(val, "1") == 0;
}

int
scew_xml_int(const scew_element *elem)
{
	const char *val = scew_xml_string(elem);
	return strtol(val, NULL, 10);
}

const scew_element *
scew_xml_search(const scew_element *root, const char *table, const char *id,
    const char *data)
{
	const scew_element *elem;
	const scew_element *top = scew_xml_subtree(root, table);
	if (top == NULL) {
		return NULL;
	}
	elem = NULL;
	scew_list *elem_entry = NULL;
	scew_list *elem_list = top ? scew_element_children(top) : NULL;
	for (elem_entry = elem_list; elem_entry != NULL; elem_entry = scew_list_next(elem_entry)) {
		elem = (scew_element *) scew_list_data(elem_entry);
		scew_element *field = scew_element_by_name(elem, id);
		if (field == NULL) {
			continue;
		}
		if (data == NULL ||
		    strcasecmp(data, scew_xml_string(field)) == 0) {
			return elem;
		}
	}
	return NULL;
}

const scew_element *
scew_xml_subtree(const scew_element *root, const char *id)
{
	const scew_element *elem = NULL;
	char name[200] = { 0 };
	char *nextid = NULL, *cp = name;

	strncpy(name, id, sizeof(name) - 1);

	/*
	 * Traverse from the root down for each element of the ID
	 */
	elem = root;
	while (elem != NULL && (nextid = strsep(&cp, ".")) != NULL) {
		elem = scew_element_by_name(elem, nextid);
	}
	return elem;
}

scew_element*
xml_element_add_pair(scew_element* node, const char* name, const char* contents)
{
	char escaped[1024];
	scew_element *child;

	memset(escaped, '\0', sizeof(escaped));
	xml_escape(contents, escaped, sizeof(escaped));

	child = scew_element_add(node, name);
	scew_element_set_contents(child, escaped);

	return child;
}

void
xml_element_attribute(scew_element *elem, const char *name, const char *value)
{
	char escaped[1024];

	memset(escaped, '\0', sizeof(escaped));
	xml_escape(value, escaped, sizeof(escaped));

	scew_element_add_attribute_pair(elem, name, escaped);
}

scew_element*
xml_element_add_int(scew_element* node, const char* name, int value)
{
	char buf[100];
	scew_element *child;

	memset(buf, '\0', sizeof(buf));
	snprintf(buf, sizeof(buf), "%d", value);

	child = scew_element_add(node, name);
	scew_element_set_contents(child, buf);

	return child;
}

int
xml_escape(const char *src, char* dst, size_t dstlen)
{
	FILE *fp;
	int result = 0;

	fp = fmemopen(dst, dstlen, "w");
	if (fp == NULL) {
		return -1;
	}
	while (*src) {
		switch (*src) {
		case '&':
			fputs("&amp;", fp);
			break;
		case '<':
			fputs("&lt;", fp);
			break;
		case '>':
			fputs("&gt;", fp);
			break;
		case '"':
			fputs("&quot;", fp);
			break;
		case '\'':
			fputs("&apos;", fp);
			break;
		default:
			fputc(*src, fp);
			break;
		}
		++src;
	}
	result = ferror(fp) ? -1 : 0;
	fflush(fp);
	fclose(fp);

	return result;
}

static int
replace(const char *s, const char *old, const char *new, char *buf, size_t len)
{
	int i, count = 0;
	size_t newlen = strlen(new);
	size_t oldlen = strlen(old);

	memset(buf, '\0', len);

	for (i = 0; s[i] != '\0'; i++) {
		if (strstr(&s[i], old) == &s[i]) {
			count++;
			i += oldlen - 1;
		}
	}

	/* Ensure there is enough buffer. */
	if (len < (i + count * (newlen - oldlen))) {
		return (-1);
	}

	i = 0;
	while (*s) {
		if (strstr(s, old) == s) {
			strcpy(&buf[i], new);
			i += newlen;
			s += oldlen;
		} else {
			buf[i++] = *s++;
		}
	}

	return (0);
}

struct xmltok {
	const char *key;
	const char *val;
};

static struct xmltok xml_tokens[] = {
	{ "&amp;", "&" },
	{ "&lt;", "<" },
	{ "&gt;", ">" },
	{ "&quot;", "\"" },
	{ "&apos;", "'" },
	{ NULL, '\0'  }
};

int
xml_unescape(const char *src, char *buf, size_t len)
{
	int rv = 0;
	size_t i;

	if (strlen(src) + 1 > len) {
		return (-1);
	}

	strncpy(buf, src, strlen(src));

	for (i = 0; xml_tokens[i].key != NULL; ++i) {
		char *swap = strdup(buf);
		if (swap == NULL) {
			return (-1);
		}

		if (replace(swap, xml_tokens[i].key, xml_tokens[i].val,
					buf, len) != 0) {
			rv = -1;
		}
		free(swap);
	}
	return (rv);
}

bool
xml_get_int(const scew_element* node, XML_Char const* name, int* value)
{
	const scew_element* child;

	child = scew_element_by_name(node, name);
	if (child == NULL) {
		return false;
	}
	*value = scew_xml_int(child);

	return true;
}

bool
xml_get_uint(const scew_element* node, XML_Char const* name, uint32_t* value)
{
	const scew_element* child;
	const char *val = NULL;
	long int tmp = LONG_MAX;

	child = scew_element_by_name(node, name);
	if (child == NULL) {
		return false;
	}
	val = scew_xml_string(child);
	tmp = strtol(val, NULL, 10);
	if (tmp == LONG_MIN || tmp == LONG_MAX || tmp < 0) {
		return false;
	}
	*value = (uint32_t) tmp;

	return true;
}

bool
xml_get_string(const scew_element* node, XML_Char const* name,
    const char** value)
{
	const scew_element* child;

	child = scew_element_by_name(node, name);
	if (child == NULL) {
		return false;
	}

	*value = scew_xml_string(child);

	return true;
}

bool
xml_copy_string(const scew_element* node, XML_Char const* name, char** value)
{
	const char* tmp;
	if (!xml_get_string(node, name, &tmp)) {
		return false;
	}
	*value = (tmp != NULL) ? strdup(tmp) : NULL;
	return true;
}
