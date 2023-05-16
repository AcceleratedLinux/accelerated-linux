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

#include <opengear/sdtxmlalert.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#ifdef EMBED
#  define ALERT_DIR "/var/run/alerts"
#else
#  define ALERT_DIR "."
#endif

sdtxml_alert_t**
sdtxml_alert_list_load(int* count)
{
	glob_t gl;
	char path[1024];
	sdtxml_alert_t** alerts;
	int i;

	snprintf(path, sizeof(path), "%s/sdt-login-alert.xml", ALERT_DIR);
	if (glob(path, 0, NULL, &gl) != 0) {
		return NULL;
	}

	alerts = calloc(gl.gl_pathc + 1, sizeof(*alerts));
	if (alerts == NULL) {
		goto error;
	}

	for (i = 0; i < (int) gl.gl_pathc; ++i) {
		alerts[i] = sdtxml_alert_load_file(gl.gl_pathv[i]);
	}
	*count = gl.gl_pathc;
	globfree(&gl);

	return alerts;
 error:
	globfree(&gl);
	return NULL;
}

void
sdtxml_alert_list_free(sdtxml_alert_t **list)
{
	sdtxml_alert_t* alert;

	if (list != NULL) {
		sdtxml_alert_t **l = list;
		while ((alert = *l++) != NULL) {
			sdtxml_alert_free(alert);
		}

		free(list);
	}
}

sdtxml_alert_t*
sdtxml_alert_load_file(const char* path)
{
	scew_element *root, *alertElem, *sdtElem;
	scew_tree* tree;
	scew_parser* parser = NULL;
	scew_reader *reader = NULL;
	sdtxml_alert_t* alert = NULL;

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

	sdtElem = scew_element_by_name(alertElem, "sdt");
	if (sdtElem == NULL) {
		goto error;
	}
	if (!xml_get_int(sdtElem, "port", &alert->port)) {
		alert->port = -1;
	}
	if (!xml_copy_string(sdtElem, "username", &alert->username)) {
		alert->username = strdup("");
	}
	if (!xml_copy_string(sdtElem, "host", &alert->host)) {
		alert->host = strdup("");
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
		sdtxml_alert_free(alert);
	}
	return NULL;
}

void
sdtxml_alert_free(sdtxml_alert_t* alert)
{
	if (alert) {
		if (alert->username != NULL) {
			free(alert->username);
		}
		if (alert->host != NULL) {
			free(alert->host);
		}
		if (alert->xmlid != NULL) {
			free(alert->xmlid);
		}
	}
	free(alert);
}
