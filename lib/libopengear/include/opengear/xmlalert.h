#ifndef _OPENGEAR_XMLALERT_H_
#define _OPENGEAR_XMLALERT_H_

#include <scew/scew.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

int
xml_alert_create(const char *alertType, const char* xmlId,
		 scew_tree **treep, scew_element **alertNode);

int
xml_alert_save(scew_tree *tree, const char* alertName, int XMLIDNumber);

#ifdef __cplusplus
}
#endif

#endif /* _OPENGEAR_XMLALERT_H_ */
