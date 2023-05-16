#ifndef UPSXML_ALERT_H
#define UPSXML_ALERT_H

typedef struct {
	char* xmlid;
	char* name;
	char* host;
	char* status;
	int port;
} upsxml_alert_t;


upsxml_alert_t** upsxml_alert_list_load(int* count);
void upsxml_alert_list_free(upsxml_alert_t **list);

upsxml_alert_t* upsxml_alert_load_file(const char* path);
void upsxml_alert_free(upsxml_alert_t* alert);

#endif /* UPSXML_ALERT_H */
