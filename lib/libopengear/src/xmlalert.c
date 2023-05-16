#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <scew/scew.h>
#include <scew/tree.h>
#include <scew/element.h>

#include <opengear/xml.h>
#include <opengear/xmlalert.h>
#include <syslog.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static int
check_dir(const char* path, int mode)
{
    struct stat st;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
	return 0;
    }

    unlink(path);
    return mkdir(path, mode);
}

int
xml_alert_create(const char *alertType, const char* xmlId,
	       scew_tree **treep, scew_element **alertNode)
{
	scew_tree *tree;
	scew_element *root;

	tree = scew_tree_create();
	root = scew_tree_set_root(tree, "alerts");
	*alertNode = scew_element_add(root, "alert");
	xml_element_add_pair(*alertNode, "type", alertType);

	if (xmlId != NULL) {

	    xml_element_add_pair(*alertNode, "xmlid", xmlId);
	}

	*treep = tree;

	return 0;
}

#ifdef EMBED
#  define ALERT_DIR "/var/run/alerts"
#else
#  define ALERT_DIR "."
#endif

int
xml_alert_save(scew_tree *tree, const char* alertName, int XMLIDNumber )
{
	char fileName[1024], tempFileName[1024];
	FILE* fp;


	check_dir(ALERT_DIR, 0700);

	snprintf(fileName, sizeof(fileName),
		 "%s/%s-alert%d.xml", ALERT_DIR, alertName, XMLIDNumber);
	snprintf(tempFileName, sizeof(tempFileName),
		 "%s/%s-alert%d.tmp", ALERT_DIR, alertName, XMLIDNumber);

	fp = fopen(tempFileName, "w");
	if (fp == NULL) {
	    return -1;
	}

	scew_writer *writer = scew_writer_fp_create(fp);
	scew_printer *printer = scew_printer_create(writer);
	if (writer == NULL || printer == NULL) {
		goto fail;
	}
	if (!scew_printer_print_tree(printer, tree)) {
		goto fail;
	}
	scew_printer_free(printer);
	scew_writer_free(writer);

	rename(tempFileName, fileName);

	return 0;

fail:
	if (writer != NULL) {
		scew_writer_free(writer);
		fp = NULL;
	}
	if (printer != NULL) {
		scew_printer_free(printer);
	}
	if (fp != NULL) {
		fclose(fp);
	}
	return -1;
}
