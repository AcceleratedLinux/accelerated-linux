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

#include <opengear/envxmlalert.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#ifdef EMBED
#  define ALERT_DIR "/var/run/alerts"
#else
#  define ALERT_DIR "."
#endif

envxml_alert_t**
envxml_alert_list_load(int* count)
{
	glob_t gl;
	char path[1024];
	envxml_alert_t** alerts;
	int i;

	snprintf(path, sizeof(path), "%s/*-environment-alert*.xml", ALERT_DIR);
	if (glob(path, 0, NULL, &gl) != 0) {
		return NULL;
	}

	alerts = calloc(gl.gl_pathc + 1, sizeof(*alerts));
	if (alerts == NULL) {
		goto error;
	}

	for (i = 0; i < (int) gl.gl_pathc; ++i) {
		alerts[i] = envxml_alert_load_file(gl.gl_pathv[i]);
	}
	*count = gl.gl_pathc;
	globfree(&gl);

	return alerts;
 error:
	globfree(&gl);
	return NULL;
}

void
envxml_alert_list_free(envxml_alert_t **list)
{
	envxml_alert_t* alert = NULL;

	if (list != NULL) {
		envxml_alert_t **l = list;
		while ((alert = *l++) != NULL) {
			envxml_alert_free(alert);
		}

		free(list);
	}
}

envxml_alert_t*
envxml_alert_load_file(const char* path)
{
	scew_element *root, *alertElem, *envElem;
	scew_tree* tree;
	scew_parser* parser = NULL;
	scew_reader *reader = NULL;
	envxml_alert_t* alert = NULL;

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

	envElem = scew_element_by_name(alertElem, "environment");
	if (envElem == NULL) {
		goto error;
	}

	if (!xml_copy_string(envElem, "xmlid", &alert->xmlid)) {
		alert->xmlid = strdup("");
	}
	if (!xml_copy_string(envElem, "device", &alert->device)
	    && !xml_copy_string(alertElem, "device", &alert->device)) {
		alert->device = strdup("");
	}
	if (!xml_copy_string(envElem, "sensor", &alert->sensor)) {
		alert->sensor = strdup("");
	}
	if (!xml_copy_string(envElem, "outlet", &alert->outlet)) {
		alert->outlet = strdup("");
	}
	if (!xml_copy_string(envElem, "value", &alert->value)) {
		alert->value = strdup("");
	}
	if (!xml_copy_string(envElem, "oldValue", &alert->oldValue)) {
		alert->oldValue = strdup("");
	}
	if (!xml_copy_string(envElem, "status", &alert->status)) {
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
		envxml_alert_free(alert);
	}
	return NULL;
}

void
envxml_alert_free(envxml_alert_t* alert)
{
	if (alert) {
		if (alert->xmlid != NULL) {
			free(alert->xmlid);
		}
		if (alert->sensor != NULL) {
			free(alert->sensor);
		}
		if (alert->outlet != NULL) {
			free(alert->outlet);
		}
		if (alert->value != NULL) {
			free(alert->value);
		}
		if (alert->oldValue != NULL) {
			free(alert->oldValue);
		}
		if (alert->status != NULL) {
			free(alert->status);
		}
	}
	free(alert);
}
