#ifndef PMXML_ALERT_H
#define PMXML_ALERT_H
/*
 To use these routines, do something like:
  
    pmxml_alert_t** alerts;
    int count, i;
    alerts = pmxml_alert_list_load(PMXML_SIGNAL, &count);
    printf("loaded %d signal alerts\n", count);
    for (i = 0; i < count; ++i) {
	    pmxml_alert_t* alert = alerts[i];
	    printf("portnum=%d\n", alert->portnum);
	    printf("device=%s\n", alert->device);
	    switch (alert->type) {
	    case PMXML_SIGNAL:
		    printf("signal=%s state=%s\n",
			   alert->data.signal.name,
			   (alert->data.signal.state == PMXML_SIGNAL_ON)
			   ? "on" : "off");
	    }
    }
    pmxml_alert_list_free(alerts);

    alerts = pmxml_alert_list_load(PMXML_USER, &count);
    printf("loaded %d user alerts\n", count);
    for (i = 0; i < count; ++i) {
	    pmxml_alert_t* alert = alerts[i];
	    printf("portnum=%d\n", alert->portnum);
	    printf("device=%s\n", alert->device);
	    switch (alert->type) {
	    case PMXML_USER:
		    printf("username=%s event=%s\n",
			   alert->data.user.name,
			   (alert->data.user.event == PMXML_USER_LOGIN)
			   ? "login" : "logout");
	    }
    }

 */
enum {
	PMXML_SIGNAL = 1,
	PMXML_USER,
	PMXML_PATTERN
};

enum {
	PMXML_USER_LOGIN = 1,
	PMXML_USER_LOGOUT,
};

enum {
	PMXML_SIGNAL_OFF = 0,
	PMXML_SIGNAL_ON = 1,
	PMXML_SIGNAL_INVALID = 2
};

typedef struct {
	int type;
	int portnum;
	char* xmlid;
	char* portname;
	char* device;
	union {
		struct {
			char* name;
			int state;
		} signal;
		struct {
			char* name;
			int event;
		} user;
		struct {
			char* pattern;
			char* match;
		} pattern;
	} data;
} pmxml_alert_t;


pmxml_alert_t** pmxml_alert_list_load(int type, int* count);
void pmxml_alert_list_free(pmxml_alert_t **list);

pmxml_alert_t* pmxml_alert_load_file(const char* path);
void pmxml_alert_free(pmxml_alert_t* alert);

#endif /* PMXML_ALERT_H */
