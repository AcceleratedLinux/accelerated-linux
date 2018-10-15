/*
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Google Inc.
 *  Copyright (C) 2018  Digi International Inc.
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#define _GNU_SOURCE  /* for asprintf */

#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/wait.h>
#include <unistd.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"
#include "lib/l2cap.h"
#include "lib/uuid.h"

#include "src/shared/mainloop.h"
#include "src/shared/util.h"
#include "src/shared/att.h"
#include "src/shared/queue.h"
#include "src/shared/timeout.h"
#include "src/shared/gatt-db.h"
#include "src/shared/gatt-server.h"

#include "subprocess.h"

#define UUID_GAP			0x1800
#define UUID_GATT			0x1801
#define UUID_DEVICE_PROVISINING		0x02DB // Digi ID

#define ATT_CID 4
#define NOTIFICATION_TIMEOUT_MS	1000
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define ETHERNET	0x00
#define CELLULAR	0x01
#define WIFI		0x02

#define DISCONNECTED	0x00
#define CONNECTED	0x01

#define DRM_ENABLED	0x01
#define DRM_DISABLED	0x00

#define PROVISIONING_SERVICE_VERSION	0x02

static bool verbose = false;
static bool keep_running = true;

struct server {
	int fd;
	struct bt_att *att;
	struct gatt_db *db;
	struct bt_gatt_server *gatt;

	bool svc_chngd_enabled;

	uint8_t def_conn_state_cached;
	uint8_t drm_connected_cached;
	uint16_t cell_handle;

	bool is_unlocked;

	int *timeout_handlers;

	int n_characteristics;
};

struct notify_cb_t {
	uint16_t handle;
	struct server *server;
};

void destroy_notify_data(void *user_data) {
	free(user_data);
}

static char *get_cmd_output(char *cmd)
{
	FILE *in;
	char buff[512], *pbuf = NULL;
	int ret_value;

	in = popen(cmd, "r");
	if (!in) {
		syslog(LOG_WARNING, "execution of cmd '%s' failed\n", cmd);
		return NULL;
	}

	pbuf = fgets(buff, ARRAY_SIZE(buff), in);
	if (pbuf)
		buff[strlen(buff) - 1] = '\0';

	ret_value = pclose(in);

	syslog(LOG_DEBUG, "cmd '%s' returned: '%s' (return value: %d)\n", cmd, buff, ret_value);
	if (pbuf)
		return strdup(buff);

	return pbuf;
}

static void debug_cb(const char *str, void *user_data)
{
	const char *prefix = user_data;

	syslog(LOG_DEBUG ,"%s%s\n", prefix, str);
}

static void att_disconnect_cb(int err, void *user_data)
{
	syslog(LOG_NOTICE, "Device disconnected: %s\n", strerror(err));

	mainloop_quit();
}

static void gap_device_name_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	uint8_t error = 0;
	size_t len = 0;
	uint8_t *value = NULL;
	char *name = NULL;

	name = get_cmd_output("bluetooth name");
	if (!name) {
		error = BT_ATT_ERROR_INVALID_HANDLE;
		goto done;
	}

	len = strlen(name) + 1;
	if (offset > len) {
		error = BT_ATT_ERROR_INVALID_HANDLE;
		goto done;
	}

	len -= offset;
	value = len ? (uint8_t *) &name[offset] : NULL;

done:
	syslog(LOG_DEBUG, "%s called, return: %s (error: 0x%x)\n", __func__, value, error);
	gatt_db_attribute_read_result(attrib, id, error, value, len);
	free(name);
}

static void gatt_service_changed_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	uint8_t error = 0;
	struct server *server = user_data;

	if (!server->is_unlocked) {
		error = BT_ATT_ERROR_READ_NOT_PERMITTED;
		goto done;
	}

	syslog(LOG_DEBUG, "Service Changed Read called\n");

done:
	gatt_db_attribute_read_result(attrib, id, error, NULL, 0);
}

static void gatt_svc_chngd_ccc_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	struct server *server = user_data;
	uint8_t error = 0;
	uint8_t value[2];

	if (!server->is_unlocked) {
		error = BT_ATT_ERROR_READ_NOT_PERMITTED;
		goto done;
	}

	syslog(LOG_DEBUG, "Service Changed CCC Read called\n");

	value[0] = server->svc_chngd_enabled ? 0x02 : 0x00;
	value[1] = 0x00;

done:
	gatt_db_attribute_read_result(attrib, id, error, value, sizeof(value));
}

static void gatt_svc_chngd_ccc_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	struct server *server = user_data;
	uint8_t error = 0;

	if (!server->is_unlocked) {
		error = BT_ATT_ERROR_WRITE_NOT_PERMITTED;
		goto done;
	}

	syslog(LOG_DEBUG, "Service Changed CCC Write called\n");

	if (!value || len != 2) {
		error = BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN;
		goto done;
	}

	if (offset) {
		error = BT_ATT_ERROR_INVALID_OFFSET;
		goto done;
	}

	if (value[0] == 0x00)
		server->svc_chngd_enabled = false;
	else if (value[0] == 0x02)
		server->svc_chngd_enabled = true;
	else
		error = BT_ATT_ERROR_INVALID_HANDLE;

	syslog(LOG_DEBUG, "Service Changed Enabled: %s\n",
				server->svc_chngd_enabled ? "true" : "false");

done:
	gatt_db_attribute_write_result(attrib, id, error);
}

static void provisioning_service_version_read_cb(struct gatt_db_attribute *attrib,
                                       unsigned int id, uint16_t offset,
                                       uint8_t opcode, struct bt_att *att,
                                       void *user_data)
{
	uint8_t value;

	value = PROVISIONING_SERVICE_VERSION;

	syslog(LOG_DEBUG, "%s called, return: 0x%x\n", __func__, value);

	gatt_db_attribute_read_result(attrib, id, 0, &value, sizeof(value));
}

static void drm_device_id_read_cb(struct gatt_db_attribute *attrib,
                                       unsigned int id, uint16_t offset,
                                       uint8_t opcode, struct bt_att *att,
                                       void *user_data)
{
	uint8_t error;
	uint8_t *value = NULL;
	size_t len = 0;
	char *device_id = NULL;
	struct server *server = user_data;

	if (!server->is_unlocked) {
		error = BT_ATT_ERROR_READ_NOT_PERMITTED;
		goto done;
	}

	device_id = get_cmd_output("runt get drm.device_id");
	if (!device_id) {
		error = BT_ATT_ERROR_INVALID_HANDLE;
		goto done;
	}

	len = strlen(device_id) + 1;

	if (offset > len) {
		error = BT_ATT_ERROR_INVALID_OFFSET;
		goto done;
	}

	len -= offset;
	value = len ? (uint8_t *) &device_id[offset] : NULL;
	error = 0;

done:
	syslog(LOG_DEBUG, "%s called, return: %s (error: 0x%x)\n", __func__, value, error);
	gatt_db_attribute_read_result(attrib, id, error, value, len);
	free(device_id);
}

#define SIM_LOCK_STATUS_UNKNOWN 	100
static struct {
	uint8_t value;
	const char *description;
} sim_lock_status[] = {
	/* Override 0 to mean 'OK' (no lock) */
	{ 0  , "none" },
	/* The rest of values are CME error codes */
	{ 11 , "sim-pin" },
	{ 17 , "sim-pin2" },
	{ 12 , "sim-puk" },
	{ 18 , "sim-puk2" },
	{ 44 , "ph-sp-pin" },
	{ 45 , "ph-sp-puk" },
	{ 40 , "ph-net-pin" },
	{ 41 , "ph-net-puk" },
	{ 5  , "ph-sim-pin" },
	{ 46 , "ph-corp-pin" },
	{ 47 , "ph-corp-puk" },
	{ 6  , "ph-fsim-pin" },
	{ 7  , "ph-fsim-puk" },
	{ 42 , "ph-netsub-pin" },
	{ 43 , "ph-netsub-puk" },
	{ SIM_LOCK_STATUS_UNKNOWN, "unknown" },
};

static void sim_lock_status_read_cb(struct gatt_db_attribute *attrib,
                                       unsigned int id, uint16_t offset,
                                       uint8_t opcode, struct bt_att *att,
                                       void *user_data)
{
	struct server *server = user_data;
	char *mm_lock_status = NULL;
	const char *str_value = NULL;
	uint8_t error = 0;
	uint8_t value = SIM_LOCK_STATUS_UNKNOWN;
	int i;

	if (!server->is_unlocked) {
		error = BT_ATT_ERROR_READ_NOT_PERMITTED;
		goto done;
	}

	mm_lock_status = get_cmd_output("mmcli -m $(modem idx) | sed -ne \"s,.*lock: '\\([^']\\+\\)'$,\\1,g;T;p\"");
	printf("mm_lock_status: %s\n", mm_lock_status);
	if (!mm_lock_status)
		goto done;

	for (i = 0; i < ARRAY_SIZE(sim_lock_status); i++) {
		if (!strcmp(mm_lock_status, sim_lock_status[i].description)) {
			value = sim_lock_status[i].value;
			str_value = sim_lock_status[i].description;
			break;
		}
	}

done:
	if (value == SIM_LOCK_STATUS_UNKNOWN)
		error = BT_ATT_ERROR_INVALID_HANDLE;

	syslog(LOG_DEBUG, "%s called, return: 0x%x (%s) (error: 0x%x)\n", __func__, value, str_value, error);
	gatt_db_attribute_read_result(attrib, id, error, &value, sizeof(value));
	free(mm_lock_status);
}


static void default_connect_type_read_cb(struct gatt_db_attribute *attrib,
                                       unsigned int id, uint16_t offset,
                                       uint8_t opcode, struct bt_att *att,
                                       void *user_data)
{
	struct server *server = user_data;
	uint8_t value;
	uint8_t error = 0;

	if (!server->is_unlocked) {
		error = BT_ATT_ERROR_READ_NOT_PERMITTED;
		goto done;
	}

	syslog(LOG_DEBUG, "Default Connection type read\n");

	/* Currently the only WWAN supported is Cellular */
	value = CELLULAR;

done:
	gatt_db_attribute_read_result(attrib, id, error, &value, sizeof(value));
}

static void default_connect_state_read_cb(struct gatt_db_attribute *attrib,
                                       unsigned int id, uint16_t offset,
                                       uint8_t opcode, struct bt_att *att,
                                       void *user_data)
{
	struct server *server = user_data;
	uint8_t error = 0;

	if (!server->is_unlocked) {
		error = BT_ATT_ERROR_READ_NOT_PERMITTED;
		goto done;
	}

done:
	syslog(LOG_DEBUG, "%s called, return: 0x%x (error: 0x%x)\n", __func__, server->def_conn_state_cached, error);
	gatt_db_attribute_read_result(attrib, id, error, &server->def_conn_state_cached, sizeof(server->def_conn_state_cached));
}

static void default_connect_type_write_cb(struct gatt_db_attribute *attrib,
				unsigned int id, uint16_t offset,
				const uint8_t *value, size_t len,
				uint8_t opcode, struct bt_att *att,
				void *user_data)
{
	struct server *server = user_data;
	uint8_t error = 0;

	if (!server->is_unlocked) {
		error = BT_ATT_ERROR_WRITE_NOT_PERMITTED;
		goto done;
	}

	if (!value || len != 1) {
		error = BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN;
		goto done;
	}

	if (offset) {
		error = BT_ATT_ERROR_INVALID_OFFSET;
		goto done;
	}

	/* Currently the only WWAN supported is Cellular */
	if (*value != CELLULAR) {
		error = BT_ATT_ERROR_INVALID_HANDLE;
		goto done;
	}

done:
	syslog(LOG_DEBUG, "%s called, return: 0x%x (error: 0x%x)\n", __func__, value ? *value : -1, error);
	gatt_db_attribute_write_result(attrib, id, error);
}

static int get_modem_index()
{
	char *output = get_cmd_output("modem idx");
	char *endptr = NULL;
	int index;

	if (output == NULL)
		return -1;

	index = strtol(output, &endptr, 10);
	free(output);

	if (*endptr != '\0')
		return -1;

	return index;
}

static bool default_connect_state_notify_cb(void *user_data)
{
	struct notify_cb_t *notify_data = user_data;
	struct server *server = notify_data->server;
	uint8_t notify = DISCONNECTED;
	char *value = NULL;
	char *cmd = NULL;
	int index;

	index = get_modem_index();

	if (index >= 0 && asprintf(&cmd, "runt get mm.modem.%d.status.state", index) >= 0) {
		value = get_cmd_output(cmd);
		free(cmd);

		if (value && !strcmp(value, "connected")) {
			notify = CONNECTED;
		} else {
			notify = DISCONNECTED;
		}
	}

	/* If the Connection state changed send notification */
	if (server->is_unlocked && server->def_conn_state_cached != notify) {
		syslog(LOG_DEBUG, "%s sending notify, new value: 0x%x\n", __func__, notify);
		bt_gatt_server_send_notification(server->gatt,
						notify_data->handle, &notify, sizeof(notify));
	}

	server->def_conn_state_cached = notify;

	free(value);

	return true;
}


static void drm_connect_state_read_cb(struct gatt_db_attribute *attrib,
                                       unsigned int id, uint16_t offset,
                                       uint8_t opcode, struct bt_att *att,
                                       void *user_data)
{
	struct server *server = user_data;
	uint8_t error = 0;

	if (!server->is_unlocked) {
		error = BT_ATT_ERROR_READ_NOT_PERMITTED;
		goto done;
	}

done:
	syslog(LOG_DEBUG, "%s called, return: 0x%x (error: 0x%x)\n", __func__, server->drm_connected_cached, error);
	gatt_db_attribute_read_result(attrib, id, error, &server->drm_connected_cached, sizeof(server->drm_connected_cached));
}

static bool drm_connect_state_notify_cb(void *user_data)
{
	struct notify_cb_t *notify_data = user_data;
	struct server *server = notify_data->server;
	char *drm_connected;
	uint8_t drm_connected_value;

	if (!server->is_unlocked)
		goto done;

	drm_connected = get_cmd_output("runt get drm.connected");
	drm_connected_value = (drm_connected && !strcmp(drm_connected, "true")) ? CONNECTED : DISCONNECTED;
	free(drm_connected);

	/* If the DRM Connection state changed send notification */
	if (server->is_unlocked && drm_connected_value != server->drm_connected_cached) {
		syslog(LOG_DEBUG, "%s sending notify, new value: 0x%x\n", __func__, drm_connected_value);
		bt_gatt_server_send_notification(server->gatt,
						 notify_data->handle,
				                 &drm_connected_value,
						sizeof(drm_connected_value));
	}

	server->drm_connected_cached = drm_connected_value;

done:
	return true;
}

static void drm_enable_read_cb(struct gatt_db_attribute *attrib,
                                       unsigned int id, uint16_t offset,
                                       uint8_t opcode, struct bt_att *att,
                                       void *user_data)
{
	struct server *server = user_data;
	char *config_service = NULL;
	char *config_enable = NULL;
	uint8_t value = 0xff;
	uint8_t error = 0;

	if (!server->is_unlocked) {
		error = BT_ATT_ERROR_READ_NOT_PERMITTED;
		goto done;
	}

	config_service = get_cmd_output("config get config.service");
	config_enable = get_cmd_output("config get config.enable");
	value =
		(
			(config_service && !strcmp(config_service, "drm"))  &&
			(config_enable && !strcmp(config_enable, "1"))
		) ? DRM_ENABLED : DRM_DISABLED;


done:
	syslog(LOG_DEBUG, "%s called, return: 0x%x (error: 0x%x)\n", __func__, value, error);
	gatt_db_attribute_read_result(attrib, id, error, &value, sizeof(value));
	free(config_enable);
	free(config_service);
}

static void drm_enable_write_cb(struct gatt_db_attribute *attrib,
				unsigned int id, uint16_t offset,
				const uint8_t *value, size_t len,
				uint8_t opcode, struct bt_att *att,
				void *user_data)
{
	struct server *server = user_data;
	uint8_t error = 0;
	bool request_enable;
	char *cmd = NULL;
	char *config_service = NULL;
	int status = 0;

	if (!server->is_unlocked) {
		error = BT_ATT_ERROR_WRITE_NOT_PERMITTED;
		goto done;
	}

	if (!value || len != 1) {
		error = BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN;
		goto done;
	}

	if (offset) {
		error = BT_ATT_ERROR_INVALID_OFFSET;
		goto done;
	}

	if (*value == DRM_DISABLED)
		request_enable = false;
	else if (*value == DRM_ENABLED)
		request_enable = true;
	else
		error = BT_ATT_ERROR_INVALID_HANDLE;

	if (!error) {
		config_service = get_cmd_output("config get config.service");
		/* When enabling, we also need to establish DRM (no matter previous config.service) */
		if (request_enable) {
			if (safe_execute("config set config.service drm", &status) < 0 || status) {
				syslog(LOG_ERR, "Failed to set DRM cnonection, error(%d)\n", status);
				error = BT_ATT_ERROR_INVALID_HANDLE;
				goto done;
			}
		}

		if (!request_enable && (!config_service || strcmp(config_service, "drm"))) {
			/*
			 * In this specific case:
			 *    - We are requested to disable the DRM service
			 *    - config.service is not drm
			 *
			 * So take no actions regarding the config.enable setting
			 */
		} else {
			if (asprintf(&cmd, "config set config.enable %s", request_enable ? "1" : "0" ) < 0  ||
			    safe_execute(cmd, &status) < 0                                                  ||
			    status) {
				syslog(LOG_ERR, "Failed to set DRM cnonection, error(%d)\n", status);
				error = BT_ATT_ERROR_INVALID_HANDLE;
			}
			free(cmd);
		}
	}

done:
	syslog(LOG_DEBUG, "%s called, received: 0x%x (error: 0x%x)\n", __func__, value ? *value : -1, error);
	gatt_db_attribute_write_result(attrib, id, error);
	free(config_service);
}

static void cellular_apn_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	uint8_t error;
	uint8_t *value = NULL;
	size_t len = 0;
	char *apn = NULL;
	struct server *server = user_data;

	if (!server->is_unlocked) {
		error = BT_ATT_ERROR_READ_NOT_PERMITTED;
		goto done;
	}

	apn = get_cmd_output("config get modem.modem.apn.0.apn");

	if (!apn) {
		error = BT_ATT_ERROR_INVALID_HANDLE;
		goto done;
	}

	len = strlen(apn) + 1;

	if (offset > len) {
		error = BT_ATT_ERROR_INVALID_OFFSET;
		goto done;
	}

	len -= offset;
	value = len ? (uint8_t *) &apn[offset] : NULL;
	error = 0;

done:
	syslog(LOG_DEBUG, "%s called, return: %s (error: 0x%x)\n", __func__, value, error);
	gatt_db_attribute_read_result(attrib, id, error, value, len);
	free(apn);
}

static void cellular_apn_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	uint8_t error = 0;
	int status = 0;
	char *new_apn = NULL;
	char *cmd = NULL;
	struct server *server = user_data;

	if (!server->is_unlocked) {
		error = BT_ATT_ERROR_WRITE_NOT_PERMITTED;
		goto done;
	}

	/* Do not allow to partially write the APN (offset must be 0) */
	if (offset) {
		error = BT_ATT_ERROR_INVALID_OFFSET;
		goto done;
	}

	/*
	 * We need to terminate the string, unfortunately 'value' is read only,
	 * so copy it all over.
	 */
	new_apn = malloc(len + 1);
	if (!new_apn) {
		error = BT_ATT_ERROR_INVALID_HANDLE;
		goto done;
	}
	memcpy(new_apn, value, len);
	new_apn[len] = '\0';

	if (asprintf(&cmd, "config set modem.modem.apn.0.apn %s", new_apn) < 0  ||
	    safe_execute(cmd, &status) < 0                                      ||
	    status) {
		syslog(LOG_ERR, "Failed to set APN, error(%d)\n", status);
		error = BT_ATT_ERROR_INVALID_HANDLE;
	}

done:
	syslog(LOG_DEBUG, "%s called, received: %s (error: 0x%x)\n", __func__, new_apn, error);
	gatt_db_attribute_write_result(attrib, id, error);
	free(cmd);
	free(new_apn);
}

static void password_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	struct server *server = user_data;
	uint8_t error = 0;
	char *expected_hash = NULL;
	char *received_hash = malloc(len + 1);

	if (!received_hash) {
		error = BT_ATT_ERROR_INVALID_HANDLE;
		goto done;
	}
	memcpy(received_hash, value, len);
	received_hash[len] = '\0';

	expected_hash = get_cmd_output("fw_printenv -n upasswd | tr -d '\n' | openssl dgst -sha256 | cut -d' ' -f2");
	if (!expected_hash) {
		error = BT_ATT_ERROR_INVALID_HANDLE;
		goto done;
	}

	if (!strcmp(received_hash, expected_hash)) {
		syslog(LOG_NOTICE, "Client authenticated, server unlocked");
		server->is_unlocked = true;
	} else {
		syslog(LOG_NOTICE, "Client tried incorrect password");
		server->is_unlocked = false;
		error = BT_ATT_ERROR_INVALID_HANDLE;
	}

done:
	syslog(LOG_DEBUG, "%s called, error: 0x%x\n", __func__, error);
	gatt_db_attribute_write_result(attrib, id, error);
	free(received_hash);
	free(expected_hash);
}

static void cellular_pin_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	struct server *server = user_data;
	uint8_t error = 0;
	int status = 0;
	char *new_pin = NULL;
	char *digits = NULL;
	char *cmd = NULL;

	if (!server->is_unlocked) {
		error = BT_ATT_ERROR_WRITE_NOT_PERMITTED;
		goto done;
	}

	/* Do not allow to partially write the PIN (offset must be 0) */
	if (offset) {
		error = BT_ATT_ERROR_INVALID_OFFSET;
		goto done;
	}

	/*
	 * We need to terminate the string, unfortunately 'value' is read only,
	 * so copy it all over.
	 */
	new_pin = malloc(len + 1);
	if (!new_pin) {
		error = BT_ATT_ERROR_INVALID_HANDLE;
		goto done;
	}
	memcpy(new_pin, value, len);
	new_pin[len] = '\0';

	/* Check that the PIN is numeric */
	strtol(new_pin, &digits, 10);
	if (*digits != '\0') {
		error = BT_ATT_ERROR_INVALID_HANDLE;
		goto done;
	}

	if (asprintf(&cmd, "modem pin_unlock %s", new_pin) < 0 ||
	    safe_execute(cmd, &status)                          ||
	    status) {
		syslog(LOG_ERR, "Failed to unlock SIM, error (%d)\n", status);
		error = BT_ATT_ERROR_INVALID_HANDLE;
		goto done;
	}
	free(cmd);
	cmd = NULL;

	if (asprintf(&cmd, "config set modem.modem.pin %s", new_pin) < 0 ||
	    safe_execute(cmd, &status) < 0                               ||
	    status) {
		syslog(LOG_ERR, "Failed to set PIN, error (%d)\n", status);
		error = BT_ATT_ERROR_INVALID_HANDLE;
	}

done:
	syslog(LOG_DEBUG, "%s called, (error: 0x%x)\n", __func__, error);
	gatt_db_attribute_write_result(attrib, id, error);
	free(cmd);
	free(new_pin);
}

static void confirm_write(struct gatt_db_attribute *attr, int err,
							void *user_data)
{
	if (!err)
		return;

	syslog(LOG_DEBUG, "Error caching attribute %p - err: %d\n", attr, err);
}

static void populate_gap_service(struct server *server)
{
	bt_uuid_t uuid;
	struct gatt_db_attribute *service, *tmp;
	uint16_t appearance;

	/* Add the GAP service */
	bt_uuid16_create(&uuid, UUID_GAP);
	service = gatt_db_add_service(server->db, &uuid, true, 6);

	/*
	 * Device Name characteristic. Make the value dynamically read and
	 * written via callbacks.
	 */
	bt_uuid16_create(&uuid, GATT_CHARAC_DEVICE_NAME);
	gatt_db_service_add_characteristic(service, &uuid,
					BT_ATT_PERM_READ,
					BT_GATT_CHRC_PROP_READ |
					BT_GATT_CHRC_PROP_EXT_PROP,
					gap_device_name_read_cb,
					NULL,
					server);

	bt_uuid16_create(&uuid, GATT_CHARAC_EXT_PROPER_UUID);

	/*
	 * Appearance characteristic. Reads and writes should obtain the value
	 * from the database.
	 */
	bt_uuid16_create(&uuid, GATT_CHARAC_APPEARANCE);
	tmp = gatt_db_service_add_characteristic(service, &uuid,
							BT_ATT_PERM_READ,
							BT_GATT_CHRC_PROP_READ,
							NULL, NULL, server);

	/*
	 * Write the appearance value to the database, since we're not using a
	 * callback.
	 */
	put_le16(128, &appearance);
	gatt_db_attribute_write(tmp, 0, (void *) &appearance,
							sizeof(appearance),
							BT_ATT_OP_WRITE_REQ,
							NULL, confirm_write,
							NULL);

	gatt_db_service_set_active(service, true);
}

static void populate_gatt_service(struct server *server)
{
	bt_uuid_t uuid;
	struct gatt_db_attribute *service, *svc_chngd;

	/* Add the GATT service */
	bt_uuid16_create(&uuid, UUID_GATT);
	service = gatt_db_add_service(server->db, &uuid, true, 4);

	bt_uuid16_create(&uuid, GATT_CHARAC_SERVICE_CHANGED);
	svc_chngd = gatt_db_service_add_characteristic(service, &uuid,
			BT_ATT_PERM_READ,
			BT_GATT_CHRC_PROP_READ | BT_GATT_CHRC_PROP_INDICATE,
			gatt_service_changed_cb,
			NULL, server);
	gatt_db_attribute_get_handle(svc_chngd);

	bt_uuid16_create(&uuid, GATT_CLIENT_CHARAC_CFG_UUID);
	gatt_db_service_add_descriptor(service, &uuid,
				BT_ATT_PERM_READ | BT_ATT_PERM_WRITE,
				gatt_svc_chngd_ccc_read_cb,
				gatt_svc_chngd_ccc_write_cb, server);

	gatt_db_service_set_active(service, true);
}

/*
 * For each characteristic we create 4 uuids:
 *   - Characteristic itself
 *   - Description
 *   - Description format
 *   - Characteristic format
 */
#define HANDLES_PER_CHARACTERISTIC 	4

enum uuids {
	UUID_DEVICE_CELLULAR_PIN 	= 0x2e24,
	UUID_DEFAULT_CONNECTION_TYPE	= 0x2e25,
	UUID_DEFAULT_CONNECTION_STATE 	= 0x2e26,
	UUID_DRM_ENABLE			= 0x2e27,
	UUID_DEVICE_SERVICE_VER		= 0x2e28,
	UUID_DEVICE_CELLULAR_APN	= 0x2e29,
	UUID_DRM_CONNECTION_STATE	= 0x2e2A,
	UUID_DRM_DEVICE_ID		= 0x2e2B,
	UUID_SIM_LOCK_STATUS		= 0x2e2C,
	UUID_DEVICE_PASSWORD		= 0x2eFF,
};

/* See BT specification 4.2, section 3.3.3.5 */
#define FMT_BYTES	7
const uint8_t FORMAT_8_BIT_INT[] = {
	0x04, 0x00, 0x00, 0x30, 0x01, 0x00, 0x00
};

const uint8_t FORMAT_UTF_8[] = {
	0x19, 0x00, 0x00, 0x30, 0x01, 0x00, 0x00
};

struct {
	uint16_t uuid;
	const char *description;
	const uint8_t *format;
	gatt_db_read_t read_cb;
	gatt_db_write_t write_cb;
	timeout_func_t notify_cb;
} const bt_characteristics[] = {
	{
		UUID_DEVICE_CELLULAR_PIN,
		"SIM Pin",
		FORMAT_UTF_8,
		NULL,
		cellular_pin_write_cb,
		NULL
	},
	{
		UUID_DEFAULT_CONNECTION_TYPE,
		"Default Connection Type",
		FORMAT_8_BIT_INT,
		default_connect_type_read_cb,
		default_connect_type_write_cb,
		NULL
	},
	{
		UUID_DEFAULT_CONNECTION_STATE,
		"Default Connection State",
		FORMAT_8_BIT_INT,
		default_connect_state_read_cb,
		NULL,
		default_connect_state_notify_cb
	},
	{
		UUID_DRM_ENABLE,
		"DRM Enabled",
		FORMAT_8_BIT_INT,
		drm_enable_read_cb,
		drm_enable_write_cb,
		NULL
	},
	{
		UUID_DEVICE_SERVICE_VER,
		"Digi Provisioning Service Version",
		FORMAT_8_BIT_INT,
		provisioning_service_version_read_cb,
		NULL,
		NULL
	},
	{
		UUID_DEVICE_CELLULAR_APN,
		"Default APN",
		FORMAT_UTF_8,
		cellular_apn_read_cb,
		cellular_apn_write_cb,
		NULL
	},
	{
		UUID_DRM_CONNECTION_STATE,
		"DRM Connection State",
		FORMAT_8_BIT_INT,
		drm_connect_state_read_cb,
		NULL,
		drm_connect_state_notify_cb
	},
	{
		UUID_DRM_DEVICE_ID,
		"Device ID",
		FORMAT_UTF_8,
		drm_device_id_read_cb,
		NULL,
		NULL
	},
	{
		UUID_SIM_LOCK_STATUS,
		"SIM lock status",
		FORMAT_8_BIT_INT,
		sim_lock_status_read_cb,
		NULL,
		NULL
	},
	{
		UUID_DEVICE_PASSWORD,
		"Password",
		FORMAT_UTF_8,
		NULL,
		password_write_cb,
		NULL
	},
};

static void populate_provisioning_service(struct server *server)
{
	bt_uuid_t uuid;
	struct gatt_db_attribute *service;
	int i;

	/* Add Provisioning Service */
	server->n_characteristics = ARRAY_SIZE(bt_characteristics);
	bt_uuid16_create(&uuid, UUID_DEVICE_PROVISINING);
	service = gatt_db_add_service(server->db, &uuid, true, server->n_characteristics * HANDLES_PER_CHARACTERISTIC);

	server->cell_handle = gatt_db_attribute_get_handle(service);
	server->timeout_handlers = calloc(server->n_characteristics, sizeof(server->timeout_handlers[0]));
	if (!server->timeout_handlers) {
		syslog(LOG_ERR, "Out of memory, calloc failed!");
		exit(2);
	}

	/* Add all characteristics */
	for (i = 0; i < server->n_characteristics; i++) {
		uint32_t permissions = 0;
		uint8_t properties = 0;
		struct gatt_db_attribute *desc_att, *gatt_handle;

		bt_uuid16_create(&uuid, bt_characteristics[i].uuid);

		if (bt_characteristics[i].read_cb)
			properties |= BT_GATT_CHRC_PROP_READ;

		if (bt_characteristics[i].write_cb) {
			properties |= BT_GATT_CHRC_PROP_WRITE;
			permissions |= BT_ATT_PERM_WRITE;
		}

		if (bt_characteristics[i].notify_cb)
			properties |= BT_GATT_CHRC_PROP_NOTIFY;

		permissions |= BT_ATT_PERM_READ;

		/* Characteristic */
		gatt_handle = gatt_db_service_add_characteristic(
						service,
						&uuid,
						permissions,
						properties,
						bt_characteristics[i].read_cb,
						bt_characteristics[i].write_cb,
						server);

		/* Characteristic descriptor */
		bt_uuid16_create(&uuid, GATT_CHARAC_USER_DESC_UUID);
		desc_att = gatt_db_service_add_descriptor(
						service,
						&uuid,
						BT_ATT_PERM_READ,
						NULL,
						NULL,
						server);
		gatt_db_attribute_write(desc_att,
					0,
					(const uint8_t *) bt_characteristics[i].description,
					strlen(bt_characteristics[i].description) + 1,
					BT_ATT_OP_READ_REQ,
					NULL,
					confirm_write,
					NULL);

		/* Charateristic description format */
		bt_uuid16_create(&uuid, GATT_CHARAC_FMT_UUID);
		desc_att = gatt_db_service_add_descriptor(service, &uuid,
						  BT_ATT_PERM_READ, NULL, NULL, server);

		/* Characteristic format */
		gatt_db_attribute_write(desc_att,
					0,
					bt_characteristics[i].format,
					FMT_BYTES,
					BT_ATT_OP_READ_REQ,
					NULL,
					confirm_write,
					NULL);

		/* Notification timed function */
		if (bt_characteristics[i].notify_cb) {
			struct notify_cb_t *notify_data = malloc(sizeof(*notify_data));

			if (!notify_data) {
				syslog(LOG_ERR, "Out of memory, calloc failed!");
				exit(2);
			}
			notify_data->server = server;
			notify_data->handle = gatt_db_attribute_get_handle(gatt_handle);

			server->timeout_handlers[i] = timeout_add(NOTIFICATION_TIMEOUT_MS, bt_characteristics[i].notify_cb, notify_data, destroy_notify_data);

			/* Manually call the notify once to cache a first value */
			bt_characteristics[i].notify_cb(notify_data);
		}
	}

	gatt_db_service_set_active(service, true);
}

static void populate_db(struct server *server)
{
	populate_gap_service(server);
	populate_gatt_service(server);
	populate_provisioning_service(server);
}

static struct server *server_create(int fd, uint16_t mtu)
{
	struct server *server;

	server = new0(struct server, 1);
	if (!server) {
		syslog(LOG_ERR, "Failed to allocate memory for server\n");
		return NULL;
	}

	server->att = bt_att_new(fd, false);
	if (!server->att) {
		syslog(LOG_ERR, "Failed to initialze ATT transport layer\n");
		goto fail;
	}

	if (!bt_att_set_close_on_unref(server->att, true)) {
		syslog(LOG_ERR, "Failed to set up ATT transport layer\n");
		goto fail;
	}

	if (!bt_att_register_disconnect(server->att, att_disconnect_cb, NULL,
									NULL)) 	{
		syslog(LOG_ERR, "Failed to set ATT disconnect handler\n");
		goto fail;
	}

	server->fd = fd;
	server->db = gatt_db_new();
	if (!server->db) {
		syslog(LOG_ERR, "Failed to create GATT database\n");
		goto fail;
	}

	server->gatt = bt_gatt_server_new(server->db, server->att, mtu);
	if (!server->gatt) {
		syslog(LOG_ERR, "Failed to create GATT server\n");
		goto fail;
	}

	if (verbose) {
		bt_att_set_debug(server->att, debug_cb, "att: ", NULL);
		bt_gatt_server_set_debug(server->gatt, debug_cb,
							"server: ", NULL);
	}

	/* bt_gatt_server already holds a reference */
	populate_db(server);

	return server;

fail:
	gatt_db_unref(server->db);
	bt_att_unref(server->att);
	free(server);

	return NULL;
}

static void server_destroy(struct server *server)
{
	int i;

	for (i = 0; i < server->n_characteristics; i++)
		if (server->timeout_handlers[i])
			timeout_remove(server->timeout_handlers[i]);

	free(server->timeout_handlers);

	bt_gatt_server_unref(server->gatt);
	gatt_db_unref(server->db);
	bt_att_unref(server->att);
	free(server);
}

static void usage(void)
{
	printf("ble-provisioning-app\n");
	printf("Usage:\n\tble-provisioning-app [options]\n");

	printf("Options:\n"
		"\t-i, --index <id>\t\tSpecify adapter index, e.g. hci0\n"
		"\t-m, --mtu <mtu>\t\t\tThe ATT MTU to use\n"
		"\t-v, --verbose\t\t\tEnable extra logging\n"
		"\t-h, --help\t\t\tDisplay help\n");
}

static struct option main_options[] = {
	{ "index",		1, 0, 'i' },
	{ "mtu",		1, 0, 'm' },
	{ "verbose",		0, 0, 'v' },
	{ "help",		0, 0, 'h' },
	{ }
};

static int l2cap_le_att_listen_and_accept(bdaddr_t *src, int sec,
							uint8_t src_type)
{
	int sk, nsk;
	struct sockaddr_l2 srcaddr, addr;
	socklen_t optlen;
	struct bt_security btsec;
	char ba[18];

	sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (sk < 0) {
		syslog(LOG_ERR, "Failed to create L2CAP socket");
		return -1;
	}

	/* Set up source address */
	memset(&srcaddr, 0, sizeof(srcaddr));
	srcaddr.l2_family = AF_BLUETOOTH;
	srcaddr.l2_cid = htobs(ATT_CID);
	srcaddr.l2_bdaddr_type = src_type;
	bacpy(&srcaddr.l2_bdaddr, src);

	if (bind(sk, (struct sockaddr *) &srcaddr, sizeof(srcaddr)) < 0) {
		syslog(LOG_ERR, "Failed to bind L2CAP socket");
		goto fail;
	}

	/* Set the security level */
	memset(&btsec, 0, sizeof(btsec));
	btsec.level = sec;
	if (setsockopt(sk, SOL_BLUETOOTH, BT_SECURITY, &btsec,
							sizeof(btsec)) != 0) {
		syslog(LOG_ERR, "Failed to set L2CAP security level\n");
		goto fail;
	}

	if (listen(sk, 10) < 0) {
		syslog(LOG_ERR, "Listening on socket failed");
		goto fail;
	}
	syslog(LOG_NOTICE,"Started listening on ATT channel. Waiting for connections\n");

	memset(&addr, 0, sizeof(addr));
	optlen = sizeof(addr);
	nsk = accept(sk, (struct sockaddr *) &addr, &optlen);
	if (nsk < 0) {
		syslog(LOG_ERR, "Accept failed");
		goto fail;
	}

	ba2str(&addr.l2_bdaddr, ba);
	syslog(LOG_NOTICE,"Connect from %s\n", ba);
	close(sk);

	return nsk;

fail:
	close(sk);
	return -1;
}

static void signal_cb(int signum, void *user_data)
{
	switch (signum) {
	case SIGINT:
	case SIGTERM:
		keep_running = false;
		mainloop_quit();
		break;
	default:
		break;
	}
}

int main(int argc, char *argv[])
{
	const int sec = BT_SECURITY_LOW;
	const uint8_t src_type = BDADDR_LE_PUBLIC;
	int opt;
	bdaddr_t src_addr;
	int dev_id = -1;
	uint16_t mtu = 0;
	sigset_t mask;

	setlogmask (LOG_UPTO (LOG_NOTICE));

	while ((opt = getopt_long(argc, argv, "+hvrm:i:",
						main_options, NULL)) != -1) {
		switch (opt) {
		case 'h':
			usage();
			return EXIT_SUCCESS;
		case 'v':
			verbose = true;
			setlogmask (LOG_UPTO (LOG_DEBUG));
			break;
		case 'm': {
			int arg;

			arg = atoi(optarg);
			if (arg <= 0) {
				syslog(LOG_ERR, "Invalid MTU: %d\n", arg);
				return EXIT_FAILURE;
			}

			if (arg > UINT16_MAX) {
				syslog(LOG_ERR, "MTU too large: %d\n", arg);
				return EXIT_FAILURE;
			}

			mtu = (uint16_t) arg;
			break;
		}
		case 'i':
			dev_id = hci_devid(optarg);
			if (dev_id < 0) {
				syslog(LOG_ERR, "Invalid adapter");
				return EXIT_FAILURE;
			}

			break;
		default:
			syslog(LOG_ERR, "Invalid option: %c\n", opt);
			return EXIT_FAILURE;
		}
	}

	argc -= optind;
	argv -= optind;
	optind = 0;

	openlog("ble-provisioning-app", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	syslog(LOG_NOTICE, "Running GATT server\n");

	if (argc) {
		usage();
		return EXIT_SUCCESS;
	}

	while (keep_running == true) {
		int fd;
		struct server *server;

		if (dev_id == -1)
			bacpy(&src_addr, BDADDR_ANY);
		else if (hci_devba(dev_id, &src_addr) < 0) {
			syslog(LOG_ERR, "Adapter not available");
			return EXIT_FAILURE;
		}

		fd = l2cap_le_att_listen_and_accept(&src_addr, sec, src_type);
		if (fd < 0) {
			syslog(LOG_ERR, "Failed to accept L2CAP ATT connection\n");
			return EXIT_FAILURE;
		}

		mainloop_init();

		server = server_create(fd, mtu);
		if (!server) {
			close(fd);
			return EXIT_FAILURE;
		}

		bt_gatt_server_send_notification(server->gatt,
						 server->cell_handle,
						 0,
						 0);

		sigemptyset(&mask);
		sigaddset(&mask, SIGINT);
		sigaddset(&mask, SIGTERM);

		mainloop_set_signal(&mask, signal_cb, NULL, NULL);

		mainloop_run();

		/* remove the application signals */
		sigdelset(&mask, SIGTERM);
		sigdelset(&mask, SIGINT);
		sigemptyset(&mask);

		server_destroy(server);

		close(fd);
	}

	return EXIT_SUCCESS;
}
