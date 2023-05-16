/**
 * @file xmldb.h
 *
 * An interface to a simple XML database implementation where elements can be
 * referenced via a dot notation.
 */
#ifndef _OPENGEAR_XMLDB_H_
#define _OPENGEAR_XMLDB_H_

#include <stdint.h>
#include <stdbool.h>
#include <scew/scew.h>

#define XMLDB_DEFAULT_CONFIG "/etc/config/config.xml"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * An opaque type which handles an XML database.
 */
typedef struct xmldb xmldb_t;

/**
 * Open a new XML database.
 * @param error The callback function to handle error events.
 * @return A pointer to the database handler otherwise NULL.
 */
xmldb_t *xmldb_open(void (*error)(const char *, ...));

/**
 * Create a new empty XML database.
 * @return A pointer to the database handler otherwise NULL.
 */
xmldb_t *xmldb_create(void);

/**
 * Load XML data into the XML database.
 * @param path The filename of the XML file to load
 * @return 0 on success, -1 on error
 */
int xmldb_load(xmldb_t *db, const char *path);

/**
 * Load XML data into the XML database.
 * @param xml_buf Character data representation of XML in a memory buffer.
 * @param xml_buf_size Size of the xml_buf memory buffer.
 * @param error The callback function to handle error events.
 * @return 0 on success, -1 on error
 */
xmldb_t * xmldb_open_buffer(char const* xml_buf, unsigned int xml_buf_size, void (*error)(const char *, ...));

/**
 * Close the given XML database.
 * @param db The database to close.
 */
void xmldb_close(xmldb_t *db);

/**
 * Retrieve a subtree from the XML database.
 * @param db The database to use.
 * @param id The elements ID to start the subtree from.
 * @return The subtree identified by ID otherwise NULL.
 */
const scew_element *xmldb_subtree(xmldb_t *db, const char *id);

/**
 * Search a database table/array for elements matching the given field name, and
 * optionally also the given data.
 *
 * For example, given a table of "config.groups", a fieldid of "name", and data of
 * "users", it would search the tree for the first element under config.groups that
 * has a 'name' field set to 'users'. (eg. it would find the first instance where
 * config.groups.*.name=users). It would then return the subtree that contains the field.
 * for example, the above might return a scew_element for config.groups.group2.
 *
 * If the data parameter is NULL, the value is not checked: the first element with a
 * field called 'fieldid' will be returned.
 *
 * @param db The database to use.
 * @param table The subtree to search.
 * @param fieldid The field to match.
 * @param data The value the above field should be set to (NULL for anything).
 * @return The matching elements.
 */
const scew_element *xmldb_search(xmldb_t *db, const char *table,
    const char *fieldid, const char *data);

/**
 * Return the root elelemnt of a database
 * @param sb The database to use
 * @return The root element
 */
const scew_element *xmldb_root(xmldb_t *db);
/**
 * Retrieve a boolean value from the XML database.
 * @param db The database to use.
 * @param id The ID of the element that contains the field
 * @param field The name of the sub-element that contains the value
 * @return true or false
 */
bool xmldb_getbool(xmldb_t *db, const char *id, const char *field);

/**
 * Retrieve an integer value from the XML database.
 * @param db The database to use.
 * @param id The ID of the element that contains the field containing the value
 * @param field The name of the sub-element that contains the value
 * @return integer value of field
 */
int xmldb_getint(xmldb_t *db, const char *id, const char *field);

/**
 * Retrieve an size type value from the XML database.
 * @param db The database to use.
 * @param id The ID of the element that contains the field containing the value
 * @param field The name of the sub-element that contains the value
 * @return size value of field
 */
size_t xmldb_getsize(xmldb_t *db, const char *id, const char *field);

/**
 * Retrieve a string value from the XML database.
 * @param db The database to use.
 * @param id The ID of the element that contains the field containing the value
 * @param field The name of the sub-element that contains the value
 * @return string contents of field
 */
const char *xmldb_getstring(xmldb_t *db, const char *id, const char *field);

/**
 * Retrieve a character value from the XML database.
 * @param db The database to use.
 * @param id The ID of the element that contains the field containing the value
 * @param field The name of the sub-element that contains the value
 * @return first character of field
 */
int xmldb_getchar(xmldb_t *db, const char *id, const char *field);

/**
 * Return an array bools indicating enabled ports from XML subtree.
 * This array is the callers responsibility to free. It is a single allocation
 * @param db The database to use.
 * @param id The ID of the portlist parent element.
 * @param num_ports A pointer to return the number of ports in the port array
 * @return The array of bools indicating enabled ports.
 */
bool * xmldb_portlist(xmldb_t *db, const char *id, int *num_ports);

#ifdef __cplusplus
}
#endif

#endif /* _OPENGEAR_XMLDB_H_ */
