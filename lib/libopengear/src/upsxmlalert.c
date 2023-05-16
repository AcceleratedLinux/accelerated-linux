#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <glob.h>

#include <scew/tree.h>
#include <scew/parser.h>
#include <scew/element.h>
#include <scew/error.h>

#include <opengear/xml.h>

#include <opengear/upsxmlalert.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#ifdef EMBED
#  define ALERT_DIR "/var/run/alerts"
#else
#  define ALERT_DIR "."
#endif

upsxml_alert_t**
upsxml_alert_list_load(int* count)
{
	glob_t gl;
	upsxml_alert_t** alerts;
	int i;

	if (glob(ALERT_DIR "/ups-*-alert*.xml", 0, NULL, &gl) != 0) {
		return NULL;
	}

	alerts = calloc(gl.gl_pathc + 1, sizeof(*alerts));
	if (alerts == NULL) {
		goto error;
	}

	for (i = 0; i < (int) gl.gl_pathc; ++i) {
		alerts[i] = upsxml_alert_load_file(gl.gl_pathv[i]);
	}
	*count = gl.gl_pathc;
	globfree(&gl);

	return alerts;
 error:
	globfree(&gl);
	return NULL;
}

void
upsxml_alert_list_free(upsxml_alert_t **list)
{
	upsxml_alert_t* alert;

	if (list != NULL) {
		upsxml_alert_t **l = list;
		while ((alert = *l++) != NULL) {
			upsxml_alert_free(alert);
		}

		free(list);
	}
}

upsxml_alert_t*
upsxml_alert_load_file(const char* path)
{
	scew_element *root, *alertElem, *upsElem;
	scew_tree* tree;
	scew_parser* parser = NULL;
	scew_reader *reader = NULL;
	upsxml_alert_t* alert = NULL;

	alert = calloc(1, sizeof(*alert));
	if (alert == NULL) {
		goto error;
	}
	parser = scew_parser_create();
	if (parser == NULL) {
		goto error;
	}
	reader = scew_reader_file_create(path);
	if (reader == NULL) {
		goto error;
	}
	tree = scew_parser_load(parser, reader);
	root = scew_tree_root(tree);
	alertElem = scew_element_by_name(root, "alert");
	if (alertElem == NULL) {
		goto error;
	}
	if (!xml_copy_string(alertElem, "xmlid", &alert->xmlid)) {
		alert->xmlid = strdup("");
	}

	upsElem = scew_element_by_name(alertElem, "ups");
	if (upsElem == NULL) {
		goto error;
	}
	if (!xml_get_int(upsElem, "port", &alert->port)) {
		alert->port = -1;
	}
	if (!xml_copy_string(upsElem, "name", &alert->name)) {
		alert->name = strdup("");
	}
	if (!xml_copy_string(upsElem, "host", &alert->host)) {
		alert->host = strdup("");
	}
	if (!xml_copy_string(upsElem, "status", &alert->status)) {
		alert->status = strdup("");
	}

	scew_parser_free(parser);
	scew_reader_free(reader);
	scew_tree_free(tree);

	return alert;
error:
	if (parser != NULL) {
		scew_parser_free(parser);
	}
	if (reader != NULL) {
		scew_reader_free(reader);
	}
	if (tree != NULL) {
		scew_tree_free(tree);
	}
	if (alert != NULL) {
		upsxml_alert_free(alert);
	}
	return NULL;
}

void
upsxml_alert_free(upsxml_alert_t* alert)
{
	if (alert) {
		if (alert->xmlid != NULL) {
			free(alert->xmlid);
		}
		if (alert->name != NULL) {
			free(alert->name);
		}
		if (alert->host != NULL) {
			free(alert->host);
		}
		if (alert->status != NULL) {
			free(alert->status);
		}
	}
	free(alert);
}
