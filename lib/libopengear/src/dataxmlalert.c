#include <sys/stat.h>

#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <syslog.h>

#include <scew/tree.h>
#include <scew/parser.h>
#include <scew/element.h>
#include <scew/error.h>

#include <opengear/xml.h>

#include <opengear/dataxmlalert.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#ifdef EMBED
#  define ALERT_DIR "/var/run/alerts"
#else
#  define ALERT_DIR "."
#endif

dataxml_alert_t**
dataxml_alert_list_load(int* count)
{
	glob_t gl;
	dataxml_alert_t** alerts;
	int i;

	if (glob(ALERT_DIR "/datalog-alert*.xml", 0, NULL, &gl) != 0) {
		return NULL;
	}

	alerts = calloc(gl.gl_pathc + 1, sizeof(*alerts));
	if (alerts == NULL) {
		goto error;
	}

	for (i = 0; i < (int) gl.gl_pathc; ++i) {
		alerts[i] = dataxml_alert_load_file(gl.gl_pathv[i]);
	}
	*count = gl.gl_pathc;
	globfree(&gl);

	syslog(LOG_DEBUG, "Loaded %d alerts", *count);
	return alerts;
error:
	globfree(&gl);
	return NULL;
}

void
dataxml_alert_list_free(dataxml_alert_t **list)
{
	dataxml_alert_t* alert;

	if (list != NULL) {
		dataxml_alert_t **l = list;
		while ((alert = *l++) != NULL) {
			dataxml_alert_free(alert);
		}

		free(list);
	}
}

dataxml_alert_t*
dataxml_alert_load_file(const char* path)
{
	scew_element *root, *alertelem, *dataelem;
	scew_tree* tree;
	scew_parser* parser = NULL;
	scew_reader *reader = NULL;
	dataxml_alert_t* alert = NULL;
	const char *str = NULL;
	unsigned int value;

	alert = calloc(1, sizeof(*alert));
	if (alert == NULL) {
		syslog(LOG_CRIT, "Failed to allocate alert buffer");
		goto error;
	}
	parser = scew_parser_create();
	if (parser == NULL) {
		syslog(LOG_ERR, "Failed to create the XML parser");
		goto error;
	}
	reader = scew_reader_file_create(path);
	if (reader == NULL) {
		syslog(LOG_ERR, "Failed to load: %s", path);
		goto error;
	}
	tree = scew_parser_load(parser, reader);
	root = scew_tree_root(tree);
	alertelem = scew_element_by_name(root, "alert");
	if (alertelem == NULL) {
		syslog(LOG_ERR, "Failed to find XML elem: <alert>");
		goto error;
	}

	dataelem = scew_element_by_name(alertelem, "datalog");
	if (dataelem == NULL) {
		syslog(LOG_ERR, "Failed to find XML elem: <datalog>");
		goto error;
	}

	if (!xml_copy_string(dataelem, "xmlid", &alert->xmlid)) {
		syslog(LOG_WARNING, "Failed to find XML elem: <xmlid>");
		alert->xmlid = strdup("");
	}
	if (!xml_copy_string(dataelem, "device", &alert->device)) {
		syslog(LOG_WARNING, "Failed to find XML elem: <device>");
		alert->device = strdup("");
	}
	if (xml_get_uint(dataelem, "number", &value)) {
		alert->number = value;
	} else {
		syslog(LOG_ERR, "Failed to find XML elem: <number>");
	}
	if (xml_get_uint(dataelem, "seconds", &value)) {
		alert->seconds = value;
	} else {
		syslog(LOG_ERR, "Failed to find XML elem: <seconds>");
	}
	if (xml_get_uint(dataelem, "bytes", &value)) {
		alert->bytes = value;
	} else {
		syslog(LOG_ERR, "Failed to find XML elem: <bytes>");
	}
	if (xml_get_uint(dataelem, "limit", &value)) {
		alert->limit = value;
	} else {
		syslog(LOG_ERR, "Failed to find XML elem: <limit>");
	}
	if (xml_get_string(dataelem, "state", &str)) {
		alert->state = (strcmp(str, "on") == 0);
	} else {
		syslog(LOG_ERR, "Failed to find XML elem: <state>");
		goto error;
	}

	scew_parser_free(parser);
	scew_reader_free(reader);
	scew_tree_free(tree);

	syslog(LOG_DEBUG, "Loaded alert: %s", path);
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
		dataxml_alert_free(alert);
	}
	syslog(LOG_ERR, "Failed to parse alert: %s", path);
	return NULL;
}

void
dataxml_alert_free(dataxml_alert_t* alert)
{
	if (alert) {
		if (alert->xmlid != NULL) {
			free(alert->xmlid);
		}
		if (alert->device != NULL) {
			free(alert->device);
		}
	}
	free(alert);
}

/*****************************************************************************/
/*
 * this __MUST__ be at the VERY end of the file - do NOT move!!
 *
 * Local Variables:
 * c-basic-offset: 8
 * tab-width: 8
 * end:
 * vi: tabstop=8 shiftwidth=8 textwidth=79 noexpandtab
 */
