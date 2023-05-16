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

#include <opengear/pmxmlalert.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#ifdef EMBED
#  define ALERT_DIR "/var/run/alerts"
#else
#  define ALERT_DIR "."
#endif

pmxml_alert_t**
pmxml_alert_list_load(int type, int* count)
{
	glob_t gl;
	char path[1024];
	pmxml_alert_t** alerts;
	const char* typeString;
	int i;

	switch (type) {
	case PMXML_SIGNAL:
		typeString = "signal";
		break;
	case PMXML_USER:
		typeString = "login";
		break;
	case PMXML_PATTERN:
		typeString = "pattern";
		break;
	default:
		return NULL;
	}

	snprintf(path, sizeof(path), "%s/port*-%s-alert*.xml", ALERT_DIR,
	    typeString);
	if (glob(path, 0, NULL, &gl) != 0) {
		return NULL;
	}

	alerts = calloc(gl.gl_pathc + 1, sizeof(*alerts));
	if (alerts == NULL) {
		goto error;
	}

	for (i = 0; i < (int) gl.gl_pathc; ++i) {
		alerts[i] = pmxml_alert_load_file(gl.gl_pathv[i]);
	}
	*count = gl.gl_pathc;
	globfree(&gl);

	return alerts;
 error:
	globfree(&gl);
	return NULL;
}

void
pmxml_alert_list_free(pmxml_alert_t **list)
{
	pmxml_alert_t* alert = NULL;

	if (list != NULL) {
		pmxml_alert_t **l = list;
		while ((alert = *l++) != NULL) {
			pmxml_alert_free(alert);
		}

		free(list);
	}
}

void
pmxml_alert_free(pmxml_alert_t* alert)
{
	if (alert != NULL) {
		switch (alert->type) {
		case PMXML_SIGNAL:
			if (alert->data.signal.name != NULL) {
				free(alert->data.signal.name);
			}
			break;
		case PMXML_USER:
			if (alert->data.user.name != NULL) {
				free(alert->data.user.name);
			}
			break;
		case PMXML_PATTERN:
			if (alert->data.pattern.pattern != NULL) {
				free(alert->data.pattern.pattern);
			}
			if (alert->data.pattern.match != NULL) {
				free(alert->data.pattern.match);
			}
			break;
		}
		free(alert);
	}
}

pmxml_alert_t*
pmxml_alert_load_file(const char* path)
{
	scew_element *root, *typeElem, *alertElem, *portElem, *elem;
	scew_tree* tree = NULL;
	scew_parser* parser = NULL;
	scew_reader *reader = NULL;
	pmxml_alert_t* alert = NULL;
	const char* type;
	const char* str;

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
	typeElem = scew_element_by_name(alertElem, "type");
	if (typeElem == NULL) {
		goto error;
	}
	type = scew_xml_string(typeElem);
	if (type == NULL) {
		goto error;
	}
	if (strcmp(type, "signal") == 0) {
		alert->type = PMXML_SIGNAL;
	} else if (strcmp(type, "user") == 0) {
		alert->type = PMXML_USER;
	} else if (strcmp(type, "pattern") == 0) {
		alert->type = PMXML_PATTERN;
	}

	portElem = scew_element_by_name(alertElem, "port");
	if (portElem == NULL) {
		goto error;
	}
	if (!xml_get_int(portElem, "number", &alert->portnum)) {
		alert->portnum = -1;
	}
	if (!xml_copy_string(portElem, "name", &alert->portname)) {
		alert->portname = strdup("");
	}
	if (!xml_copy_string(portElem, "device", &alert->device)) {
		alert->device = strdup("");
	}

	switch (alert->type) {
	case PMXML_SIGNAL:
		if ((elem = scew_element_by_name(alertElem, "signal"))== NULL) {
			goto error;
		}
		if (!xml_copy_string(elem, "name",
				   &alert->data.signal.name)) {
			alert->data.signal.name = strdup("");
		}
		if (!xml_get_string(elem, "state", &str)) {
			goto error;
		}
		if (strcmp(str, "on") == 0) {
			alert->data.signal.state = PMXML_SIGNAL_ON;
		} else if (strcmp(str, "off") == 0) {
			alert->data.signal.state = PMXML_SIGNAL_OFF;
		} else {
			alert->data.signal.state = PMXML_SIGNAL_INVALID;
		}
		break;

	case PMXML_USER:
		if ((elem = scew_element_by_name(alertElem, "user"))== NULL) {
			goto error;
		}
		if (!xml_copy_string(elem, "name",
				   &alert->data.user.name)) {
			alert->data.user.name = strdup("");
		}
		if (!xml_get_string(elem, "event", &str)
		    && !xml_get_string(alertElem, "event", &str)) {
			goto error;
		}
		if (strcmp(str, "login") == 0) {
			alert->data.user.event = PMXML_USER_LOGIN;
		} else if (strcmp(str, "logout") == 0) {
			alert->data.user.event = PMXML_USER_LOGOUT;
		}
		break;

	case PMXML_PATTERN:
		if ((elem = scew_element_by_name(alertElem, "pattern"))==NULL) {
			goto error;
		}
		if (!xml_copy_string(elem, "pattern",
				   &alert->data.pattern.pattern)) {
			alert->data.pattern.pattern = strdup("");
		}
		if (!xml_copy_string(elem, "match",
				   &alert->data.pattern.match)) {
			alert->data.pattern.match = strdup("");
		}
		break;
	default:
		goto error;
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
		pmxml_alert_free(alert);
	}
	return NULL;
}
