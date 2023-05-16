#ifndef LIBINFOD_H
#define LIBINFOD_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Infod Client interaction types and functions */

/* ** Please note that all infod message components are delimited by the
   ** 'record separator' ASCII character (0x1E).  As it is non-printable,
   ** these examples use the colon in its place (':').   */

/* infod messages have the following format
 * len(4 bytes network order):infod_op:arguments\n
 * Get message
 * TX  INFOD_OP_GET:config.ipsec.tunnels_up\n
 * RX  INFOD_OP_RESULT:config.ipsec.tunnels_up:timestamp:INFOD_TYPE_DATAPOINT:3\nINFOD_OP_RESULT_FINISHED\n
 *
 * Subscribe Request
 * TX INFOD_OP_SUBSCRIBE_ADD:subscribername:config.ipsec\n
 * RX INFOD_OP_OK\n
 * RX INFOD_SUBSCRIBED_DATA:config.ipsec.tunnels_up:timestamp:INFOD_TYPE_DATAPOINT:3\n
 * RX INFOD_SUBSCRIBED_DATA:config.ipsec.tunnels.tunnel3.status:timestamp:INFOD_TYPE_EVENT:Negotiating Phase 2\n
 * RX INFOD_SUBSCRIBED_DATA:config.ipsec.tunnels.tunnel3.status:timestamp:INFOD_TYPE_EVENT:Up\n
 *
 * Put Request
 * TX INFOD_OP_PUT:config.ipsec.tunnels_up:INFOD_TYPE_DATAPOINT:4\n
 * RX INFOD_OP_OK
 */
/*  infod message types */
enum infod_op {
	/* Get an entry based on a prefix (1 arg, prefix)*/
	INFOD_OP_GET,
	/* Put an entry (3 args, prefix, type, data) */
	INFOD_OP_PUT,
	/* Subscribe to a prefix */
	INFOD_OP_SUBSCRIBE_ADD,
	/* Remove a subscription to a prefix */
	INFOD_OP_SUBSCRIBE_REMOVE,
	/* Answer to a basic subscription or push request to indicate that the daemon has received and processed it */
	INFOD_OP_OK,
	/* Answer to a basic subscription or push request to indicate that an error occurred */
	INFOD_OP_ERROR,
	/* Answer to a get request */
	INFOD_OP_RESULT,
	/* Trailer to indicate no more results from get request */
	INFOD_OP_RESULT_FINISHED,
	/* Indicates that this is a pushed message from a subscription request */
	INFOD_OP_SUBSCRIBED_DATA,
	/* Remove all entries with the given prefix */
	INFOD_OP_DELETE
};
/*
 * Type of message that is getting sent through
 */
enum infod_type {
	/* Log Line - this is plain log data - ie, serial port logs */
	INFOD_TYPE_LOGLINE,
	/* 
	 * Data Point - this is a graphable/alertable scalar quantity
	 * ie: temperature, voltage, humidty, load, number of tunnels, packet count, concurrent users
	 */
	INFOD_TYPE_DATAPOINT,
	/* 
	 * Event - this is a discrete event that occurs
	 * ie: user logs in, logs out, user switches outlet on/off/cycle, serial signals, failover occurs,
	 * digital input changes state
	 */
	INFOD_TYPE_EVENT,
	/*
	 * This is a processed event that is pushed back in via an alerting system.
	 * ie. Temperature/humidity/load is greater/less than threshold x, serial pattern match hit
	 */
	INFOD_TYPE_ALERTEVENT
};

/* Max message length, just to catch badness */
#define INFOD_MAX_MESSAGE 4096

/* Sleep increment in seconds for sync */
#define SYNC_INCREMENT 10

/* infod push functions */
/* one off push functions - ie from scripts - dialup up, dialup down */
int infod_push_text(const char *prefix, enum infod_type type, const char *message);
int infod_push_long(const char *prefix, enum infod_type type, long number);
/* multiple push functions - from longer running processes, like emd, upsd, portmanager */
int infod_push_multiple_start(int *fd);
int infod_push_multiple_text(int fd, const char *prefix, enum infod_type type, const char *message);
int infod_push_multiple_long(int fd, const char *prefix, enum infod_type type, long number);
int infod_push_multiple_end(int fd);

/* infod sync function */
int infod_sync(int timeout);
/* infod get function */
int infod_get(const char *prefix, char **ret);
int infod_get_extended(const char *prefix, char **result_data, char **result_prefix, int *result_type, unsigned long long *result_timestamp);
/* infod multiple get */
int infod_get_multiple_start(int *fd);
int infod_get_multiple(int fd, const char *prefix, char **ret);
int infod_get_multiple_extended(int fd, const char *prefix, char **result_data, char **result_prefix, int *result_type, unsigned long long *result_timestamp);
int infod_get_multiple_end(int fd);
int infod_delete(const char *prefix);

/* infod array functions */
int __construct_array(const char *msg, char ***array, size_t *num_elements);
int infod_multiple_get_array(int fd, const char *prefix, char ***array, size_t *num_elements);
int infod_multiple_push_array(int fd, const char *prefix, enum infod_type type, const char **array, size_t num_elements);

#define UNIX_SOCK_PATH "/var/run/infod-sock"
int infod_multiple_start_unix(int *fd);

#ifdef __cplusplus
}
#endif
#endif /* LIBINFOD_H */
