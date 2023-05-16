#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <scew/scew.h>

#include <opengear/xml.h>
#include <opengear/xmldb.h>
#include <opengear/serial.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

struct xmldb {
	scew_tree *tree;
	scew_parser *parser;
	scew_element *root;
	void (*error)(const char *, ...);
};

static void
noerror(const char *fmt, ...)
{
	fmt = NULL;
}

xmldb_t *
xmldb_open(void (*error)(const char *, ...))
{
	xmldb_t *db = NULL;

	db = xmldb_create();
	if (db == NULL) {
		return NULL;
	}
	db->error = error ? error : noerror;
	if (xmldb_load(db, XMLDB_DEFAULT_CONFIG) < 0) {
		free(db);
		return NULL;
	}
	return db;
}

xmldb_t *
xmldb_create(void)
{
	xmldb_t *db = calloc(1, sizeof(*db));
	if (db != NULL) {
		db->error = noerror;
	}
	return db;
}

int
xmldb_load(xmldb_t *db, const char *path)
{
	void (*error)(const char *, ...) = db->error;
	int err;
	scew_reader *reader = NULL;

	db->parser = scew_parser_create();
	if (db->parser == NULL) {
		error("Failed to create XML parser");
		goto fail;
	}
	scew_parser_ignore_whitespaces(db->parser, 1);
	reader = scew_reader_file_create(path);
	if (reader == NULL) {
		error("Could not load from config file: %s", path);
		goto fail;
	}
	db->tree = scew_parser_load(db->parser, reader);
	if (db->tree == NULL) {
		error("No XML tree generated from config file: %s",
		    scew_xml_error(db->parser));
		goto fail;
	}
	db->root = scew_tree_root(db->tree);
	if (db->root == NULL) {
		error("Cannot find root XML element: %s",
		    scew_xml_error(db->parser));
		goto fail;
	}
	scew_reader_free(reader);
	return 0;
fail:
	err = errno;
	if (db->parser != NULL) {
		scew_parser_free(db->parser);
		db->parser = NULL;
	}
	if (db->tree != NULL) {
		scew_tree_free(db->tree);
		db->tree = NULL;
	}
	if (reader != NULL) {
		scew_reader_free(reader);
	}
	errno = err;
	return -1;
}

xmldb_t *
xmldb_open_buffer(char const* xml_buf, unsigned int xml_buf_size, void (*error)(const char *, ...)) {
	int err;

    scew_reader* reader = NULL;
	xmldb_t *db = xmldb_create();
	if (!db) {
		error("Failed to allocate XML DB: %m");
		goto fail;
	}
	db->error = error;
	db->parser = scew_parser_create();
	scew_parser_ignore_whitespaces(db->parser, 1);
    reader = scew_reader_buffer_create(xml_buf, xml_buf_size);
    if (!reader) {
        error("Could not create scew buffer reader");
        goto fail;
    }

	db->tree = scew_parser_load(db->parser, reader);
	if (db->tree == NULL) {
		error("No XML tree generated from config file: %s",
		    scew_xml_error(db->parser));
		goto fail;
	}
	db->root = scew_tree_root(db->tree);
	if (db->root == NULL) {
		error("Cannot find root XML element: %s",
		    scew_xml_error(db->parser));
		goto fail;
	}

    scew_reader_free(reader);
	return db;
fail:
	err = errno;
	if (reader != NULL)
        scew_reader_free(reader);

    if (db->parser != NULL) {
		scew_parser_free(db->parser);
		db->parser = NULL;
	}
	if (db->tree != NULL) {
		scew_tree_free(db->tree);
		db->tree = NULL;
	}
	free(db);
	errno = err;
	return NULL;
}

void
xmldb_close(xmldb_t *db)
{
	if (db != NULL) {
		if (db->tree != NULL) {
			/* This will release db->root as well */
			scew_tree_free(db->tree);
		}
		if (db->parser != NULL) {
			scew_parser_free(db->parser);
			db->parser = NULL;
		}
		free(db);
	}
}

static const char *
xmlid(const char *id)
{
	/* Skip the 'config.' part */
	return strncmp(id, "config.", 7) == 0 ? id + 7 : id;
}

const scew_element *
xmldb_root( xmldb_t * db )
{
	return db->root;
}

const scew_element *
xmldb_subtree(xmldb_t *db, const char *id)
{
	return scew_xml_subtree(db->root, xmlid(id));
}

const scew_element *
xmldb_search(xmldb_t *db, const char *table, const char *fieldid,
    const char *data)
{
	return scew_xml_search(db->root, xmlid(table), fieldid, data);
}

bool
xmldb_getbool(xmldb_t *db, const char *id, const char *field)
{
	char xid[200];
	snprintf(xid, sizeof(xid), "%s.%s", id, field);
	return scew_xml_bool(xmldb_subtree(db, xid));
}

int
xmldb_getint(xmldb_t *db, const char *id, const char *field)
{
	char xid[200];
	snprintf(xid, sizeof(xid), "%s.%s", id, field);
	return scew_xml_int(xmldb_subtree(db, xid));
}

size_t
xmldb_getsize(xmldb_t *db, const char *id, const char *field)
{
	size_t ret = 0;
	int tmp = xmldb_getint(db, id, field);
	if (tmp >= 0) {
		ret = (size_t) tmp;
	}
	return ret;
}

const char *
xmldb_getstring(xmldb_t *db, const char *id, const char *field)
{
	char xid[200];
	snprintf(xid, sizeof(xid), "%s.%s", id, field);
	return scew_xml_string(xmldb_subtree(db, xid));
}

int
xmldb_getchar(xmldb_t *db, const char *id, const char *field)
{
	char xid[200];
	const char *s = NULL;
	snprintf(xid, sizeof(xid), "%s.%s", id, field);
	s = scew_xml_string(xmldb_subtree(db, xid));
	if (s != NULL && s[0] != '\0') {
		return s[0];
	}
	return -1;
}

bool *
xmldb_portlist(xmldb_t *db, const char *id, int *num_ports)
{
	const scew_element *parent = NULL, *elem = NULL;
	const scew_element *tmp = NULL;
	bool *ports;
	size_t port_count;

	parent = xmldb_subtree(db, id);
	if (parent == NULL) {
		*num_ports = 0;
		return NULL;
	}
	tmp = xmldb_root(db);
	port_count = (size_t) opengear_serial_count();
	port_count += (size_t) opengear_cascaded_node_count(tmp);

	ports = calloc(port_count, sizeof(bool));
	if (!ports) {
		db->error("port array allocation failure");
		*num_ports = 0;
		return NULL;
	}

	*num_ports = port_count;
	scew_list *elem_entry = NULL;
	scew_list *elem_list = parent ? scew_element_children(parent) : NULL;
	for (elem_entry = elem_list; elem_entry != NULL; elem_entry = scew_list_next(elem_entry)) {
		elem = (scew_element *) scew_list_data(elem_entry);
		const char *name = scew_element_name(elem);
		unsigned int portnum, len = sizeof("port") - 1;

		if (name == NULL) {
			continue;
		}

		if (strlen(name) < len + 1 ||
				strncasecmp(name, "port", len) != 0) {
			continue;
		}

		if (xmldb_getbool(db, id, name)) {
			if (sscanf(name + len, "%u", &portnum) == 1) {
				if ((portnum - 1) < port_count)
					ports[portnum - 1] = true;
			}
		}
	}
	return (ports);
}

