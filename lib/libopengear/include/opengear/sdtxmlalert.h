#ifndef SDTXML_ALERT_H
#define SDTXML_ALERT_H

typedef struct {
	int type;
	char* xmlid;
	char* username;
	char* host;
	int port;
} sdtxml_alert_t;


sdtxml_alert_t** sdtxml_alert_list_load(int* count);
void sdtxml_alert_list_free(sdtxml_alert_t **list);

sdtxml_alert_t* sdtxml_alert_load_file(const char* path);
void sdtxml_alert_free(sdtxml_alert_t* alert);

#endif /* SDTXML_ALERT_H */
