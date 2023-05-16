#ifndef _OPENGEAR_USERS_H_
#define _OPENGEAR_USERS_H_

#include <sys/types.h>
#include <grp.h>
#include <pwd.h>

#include <stdbool.h>

#include <scew/scew.h>

#include <opengear/xmldb.h>
#include <opengear/og_config.h>

#define OG_ALLOW_ALL (size_t)-1

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Determine if the given user is a member of the given group.
 * @param pw A passwd account entry.
 * @param gr The group entry user may belong to.
 * @return true if user is a primary or secondary member otherwise false.
 */
bool opengear_users_ispwingroup(struct passwd *pw, struct group *gr);

/**
 * Handle for users permissions preloaded data.
 */
struct opengear_users_permissions;
typedef struct opengear_users_permissions* opengear_users_permissions_t;

/**
 * Preload users permissions to speed up iterating over lists and subsequent queries.
 * When multiple permissions queries are expected, this is faster than loading the
 * permissions info each time. Partial support has been added where this optimisation
 * is needed. Alternative functions that make use of this permissions data are provided
 * with the extension '_with_perms'.
 * Returns a handle to the permissions data to use in subsequent calls to the '_with_perms' functions.
 * This handle must be destroyed with opengear_users_destroy_permissions_data()
 */
opengear_users_permissions_t opengear_users_load_permissions(xmldb_t *db, uid_t uid, gid_t gid);

/**
 * Destroy data created by opengear_users_load_permissions()
 */
void opengear_users_destroy_permissions_data(opengear_users_permissions_t perms_h);

/**
 * Get the allowed ports for a user
 * @param db A handle to the configuration database
 * @param ports A zero-indexed array to record the port permissions
 * @param numports The number of indices in ports
 * @param uid The User-ID of the user attempting access.
 * @param gid The Group-ID of the user attempting access.
 */
void opengear_users_getallowedports(xmldb_t *db, bool ports[], int numports, uid_t uid, gid_t gid);
void opengear_users_getallowedports_with_perms(bool ports[], int numports,
	opengear_users_permissions_t perms_h);

/**
 * Determine if the given user can access the specified port.
 * @param db A handle to the configuration database.
 * @param port The serial port in question (1 - NPORTS)
 * @param uid The User-ID if the user attempting access.
 * @param gid The Group-ID of the user attempting access.
 * @return true if permitted otherwise false.
 */
bool opengear_users_isportallowed(xmldb_t *db, int port, uid_t uid, gid_t gid);
bool opengear_users_isportallowed_with_perms(int port, opengear_users_permissions_t perms_h);
bool opengear_groups_isportallowed(xmldb_t *db, int port, gid_t gid);
bool opengear_users_isportvisible(xmldb_t *db, int port, uid_t uid, gid_t gid);
bool opengear_groups_isportvisible(xmldb_t *db, int port, gid_t gid);

/**
 * Determine if the given user is a port administrator
 * @param db A handle to the configuration database.
 * @param uid The User-ID if the user attempting access.
 * @param gid The Group-ID of the user attempting access.
 * @return true if permitted otherwise false.
 */
bool opengear_users_isportadmin(xmldb_t *db, uid_t uid, gid_t gid);
bool opengear_users_isportadmin_with_perms(opengear_users_permissions_t perms_h);
bool opengear_groups_isportadmin(xmldb_t *db, gid_t gid);


/**
 * Determine if the given user can access the specified host.
 * @param db A handle to the configuration database.
 * @param address The unique host address
 * @param uid The User-ID if the user attempting access.
 * @param gid The Group-ID of the user attempting access.
 * @return true if permitted otherwise false.
 */
bool opengear_users_ishostallowed(
	xmldb_t *db, const char *address, uid_t uid, gid_t gid);
bool opengear_users_ishostallowed_with_perms(
	const char *address, opengear_users_permissions_t perms_h);
bool opengear_groups_ishostallowed(
	xmldb_t *db, const char *address, gid_t gid);
bool opengear_users_ishostvisible(
	xmldb_t *db, const char *address, uid_t uid, gid_t gid);
bool opengear_groups_ishostvisible(
	xmldb_t *db, const char *address, gid_t gid);

/**
 * Determine if the given user can access the specified RPC outlet.
 * @param db A handle to the configuration database.
 * @param configpath The unique outlet config path.
 * @param uid The User-ID if the user attempting access.
 * @param gid The Group-ID of the user attempting access.
 * @return true if permitted otherwise false.
 */
bool opengear_users_isoutletallowed(
	xmldb_t *db, const char *configpath, uid_t uid, gid_t gid);
bool opengear_users_isoutletallowed_with_perms(
	const char *configpath, opengear_users_permissions_t perms_h);
bool opengear_groups_isoutletallowed(
	xmldb_t *db, const char *configpath, gid_t gid);
bool opengear_users_isoutletvisible(
	xmldb_t *db, const char *configpath, uid_t uid, gid_t gid);
bool opengear_groups_isoutletvisible(
	xmldb_t *db, const char *configpath, gid_t gid);

/**
 * Determine if the given user can access the specified network RPC outlet.
 * @param db A handle to the configuration database.
 * @param address The RPC network address.
 * @param outletnum Index of the outlet.
 * @param uid The User-ID if the user attempting access.
 * @param gid The Group-ID of the user attempting access.
 * @return true if permitted otherwise false.
 */
bool opengear_users_isoutletallowedbyaddress(
	xmldb_t *db, const char *address, unsigned int outletnum,
	uid_t uid, gid_t gid);
bool opengear_users_isoutletvisiblebyaddress(
	xmldb_t *db, const char *address, unsigned int outletnum,
	uid_t uid, gid_t gid);

/**
 * Determine if the given user can access the specified serial RPC outlet.
 * @param db A handle to the configuration database.
 * @param device RPC serial port device node.
 * @param outletnum Index of the outlet.
 * @param uid The User-ID if the user attempting access.
 * @param gid The Group-ID of the user attempting access.
 * @return true if permitted otherwise false.
 */
bool opengear_users_isoutletallowedbydevice(
	xmldb_t *db, const char *device, unsigned int outletnum,
	uid_t uid, gid_t gid);
bool opengear_users_isoutletvisiblebydevice(
	xmldb_t *db, const char *device, unsigned int outletnum,
	uid_t uid, gid_t gid);

/**
 * Determine if the given user can access the specified serial RPC outlet.
 * @param db A handle to the configuration database.
 * @param portnum Index of the serial port.
 * @param outletnum Index of the outlet.
 * @param uid The User-ID if the user attempting access.
 * @param gid The Group-ID of the user attempting access.
 * @return true if permitted otherwise false.
 */
bool opengear_users_isoutletallowedbyportnum(
	xmldb_t *db, unsigned int portnum, unsigned int outletnum,
	uid_t uid, gid_t gid);
bool opengear_users_isoutletvisiblebyportnum(
	xmldb_t *db, unsigned int portnum, unsigned int outletnum,
	uid_t uid, gid_t gid);

/**
 * Determine if the given user can access one or more outlets on
 * the specified RPC.
 * @param db A handle to the configuration database.
 * @param configpath The unique RPC config path.
 * @param uid The User-ID if the user attempting access.
 * @param gid The Group-ID of the user attempting access.
 * @return true if permitted otherwise false.
 */
bool opengear_users_isrpcallowed(
	xmldb_t *db, const char *configpath, uid_t uid, gid_t gid);
bool opengear_users_isrpcallowed_with_perms(
	const char *configpath, opengear_users_permissions_t perms_h);
bool opengear_groups_isrpcallowed(
	xmldb_t *db, const char *configpath, gid_t gid);
bool opengear_users_isrpcvisible(
	xmldb_t *db, const char *configpath, uid_t uid, gid_t gid);
bool opengear_groups_isrpcvisible(
	xmldb_t *db, const char *configpath, gid_t gid);

/**
 * Determine the next available uid
 * @return the expected next value for uid
 */
int getNextUid();

/**
 * Get a list of local groups that the user is a member of.
 * This is primarily used by pam_adduser. Caller must free the array, and the contents.
 */
char **opengear_users_getlocalgroups(xmldb_t *db, const char *username, size_t *num_groups);

#ifdef __cplusplus
}
#endif

#endif /* _OPENGEAR_USERS_H_ */
