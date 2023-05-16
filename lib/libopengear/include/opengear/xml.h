#ifndef _OPENGEAR_XML_H_
#define _OPENGEAR_XML_H_

#include <scew/scew.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

const char *scew_xml_error(scew_parser *);
const char *scew_xml_string(const scew_element *);
bool scew_xml_bool(const scew_element *elem);
int scew_xml_int(const scew_element *elem);


/**
 * Find a subtree with a matching key field.
 * Finds the first subtree "root.table.T" such that
 * "root.table.T.field" == "key".
 *
 * @param root   (optional) an XML subtree
 * @param table  a dot-delimited path into @a root;
 *               @see #scew_xml_subtree() for restrictions.
 * @param field  the key element's tag name
 * @param key    the key value used to match
 * @return NULL if @a root was NULL, or
 *         NULL if a matching element was not found, or
 *         pointer to the matching "root.table.T" element.
 */
const scew_element *scew_xml_search(const scew_element *root,
    const char *table, const char *field, const char *key);

/**
 * Dotted-path lookup under an XML tree.
 * @param root  (optional) an XML subtree
 * @param id    a dot-delimited path under @a root.
 *              Invalid values include: empty string; strings that
 *              begin or end with a "."; strings containg two or more
 *              consecutive dots.
 * @return NULL if @a root was NULL, or
 *         NULL if a matching subtree was not found, or
 *         a pointer to the matching subtree.
 */
const scew_element *scew_xml_subtree(const scew_element *root, const char *id);

/* Adds an element with xml escaped contents */
scew_element* xml_element_add_pair(scew_element* node, const char* name,
				  const char* contents);
scew_element* xml_element_add_int(scew_element* node, const char* name,
				  int value);
/* Sets an element's attribute with xml escaped contents */
void xml_element_attribute(scew_element* elem, const char* name,
				  const char* value);
/* Escapes an xml string. e.g. replacing "<" with "&lt;" */
int xml_escape(const char *src, char* dst, size_t dstlen);

/* Un-does xml_escape. dstlen must be at least strlen(src) */
int xml_unescape(const char *src, char *dst, size_t dstlen);

bool xml_get_int(const scew_element* node, XML_Char const* name, int* value);
bool xml_get_uint(const scew_element* node, XML_Char const* name, uint32_t* value);
bool xml_get_string(const scew_element* node, XML_Char const* name,
			  const char** value);
bool xml_copy_string(const scew_element* node, XML_Char const* name, char** value);

#ifdef __cplusplus
}
#endif

#endif /* _OPENGEAR_PMCTL_H_ */
