#ifndef _OPENGEAR_PMCTL_H_
#define _OPENGEAR_PMCTL_H_

#include <scew/scew.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct pmctl pmctl_t;

/** 
 * Create a new portmanager control connection.
 * @return The new connection or NULL on error.
 */
pmctl_t *pmctl_open();

/**
 * Destroy an existing portmanager control connection.
 * @param pm The control connection to destroy.
 */
void pmctl_close(pmctl_t *pm);

/**
 * Send a reserve request to portmanager.
 * @param pm The portmanager control connection to use.
 * @param clientname An english description of the requester.
 * @param portname The port to reserve.
 * @return 0 on success otherwise -1.
 */
int pmctl_reserve_port(
	pmctl_t *pm, const char *clientname, const char *portname);

/**
 * Release a previously reserved port.
 * @param pm The portmanager connection to use.
 * @param portname The port to unreserve.
 * @return 0 on success otherwise -1.
 */
int pmctl_unreserve_port(pmctl_t *pm, const char *portname);

/**
 * Send an XML request to portmanager.
 * @param pm The portmanager connection to use.
 * @param request The XML request to send.
 * @param func The function to invoke on completion.
 * @param cbptr Maybe callback this function some doco would have been nice.
 * @return 0 on success or -1 on failure.
 */
int pmctl_xml_request(pmctl_t *pm, const scew_tree *request,
    int (*func)(void *ptr, const scew_tree *response), void *cbptr);

#ifdef __cplusplus
}
#endif

#endif /* _OPENGEAR_PMCTL_H_ */
