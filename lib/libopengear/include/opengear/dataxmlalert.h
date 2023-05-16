#ifndef OPENGEAR_DATAXML_ALERT_H
#define OPENGEAR_DATAXML_ALERT_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/**
 * The information which makes up a Data Alert. */
typedef struct {
	/** The config prefix for the rule which triggered the alert. */
	char *xmlid;

	/** The device name pertaining to this alert. */
	char *device;

	/** The alert rule number. */
	size_t number;

	/** The actual number of bytes which triggered this alert. */
	size_t bytes;

	/** The maximum number of bytes allowed in the given seconds. */
	size_t limit;

	/** The number of seconds to sample over. */
	time_t seconds;

	/** The current alrm state 0 == off, 1 == on. */
	bool state;

} dataxml_alert_t;

/**
 * Create the current data alerts.
 * @param count A user provided buffer to fill in with the total alerts.
 * @return A pointer to a pointer to the Data Alert structure or NULL.
 */
dataxml_alert_t** dataxml_alert_list_load(int* count);

/**
 * Destroy the current data alert list.
 * @param list A pointer to a pointer to the current list.
 */
void dataxml_alert_list_free(dataxml_alert_t **list);

/**
 * Create a Data Alert entry from the given file.
 * @param path The file to create an alert from.
 * @return The new Data Alert or NULL.
 */
dataxml_alert_t* dataxml_alert_load_file(const char* path);

/**
 * Destroy an existing Data Alert entry.
 * @param alert The Data Alert to destroy.
 */
void dataxml_alert_free(dataxml_alert_t* alert);

#endif /* OPENGEAR_DATAXML_ALERT_H */
