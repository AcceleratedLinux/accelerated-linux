#ifndef ENVXML_ALERT_H
#define ENVXML_ALERT_H

typedef struct {
	char* xmlid;
	char* device;
	char* sensor;
	char* outlet;
	char* value;
	char* oldValue;
	char* status;
} envxml_alert_t;


envxml_alert_t** envxml_alert_list_load(int* count);
void envxml_alert_list_free(envxml_alert_t **list);

envxml_alert_t* envxml_alert_load_file(const char* path);
void envxml_alert_free(envxml_alert_t* alert);

#endif /* ENVXML_ALERT_H */
