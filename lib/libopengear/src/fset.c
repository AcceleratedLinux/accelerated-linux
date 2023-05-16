#define _GNU_SOURCE /* for strndup */
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <syslog.h>

#include "zlib.h"
#include <opengear/og_config.h>
#include <opengear/fset.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

static vset *head = NULL;

vset *
findValue(const char *value)
{
	vset *vp = head;

	while (vp != NULL) {
		if (!strcmp(value, vp->name)) {
			return vp;
		}
		vp = vp->next;
	}
	return NULL;
}

vset *
firstValue()
{
	return head;
}

static int
_updateflash(const char *flash, off_t offset, const char *write, int save)
{
	vset *var = NULL;
	char *name = NULL;
	char *value = NULL;
	char *eq = strstr(write, "=");
	if (!eq) {
		return 2; /* indicate to display usage() */
	}

	name = strndup(write, eq - write);
	if (name == NULL) {
		return -1;
	}

	value = strdup(eq + 1);
	if (value == NULL) {
		free(name);
		return -1;
	}

	var = findValue(name);
	if (var == NULL) {
		var = calloc(1, sizeof(vset));
		if (var == NULL) {
			syslog(LOG_EMERG, "Failed to malloc: %m");
			exit(2);
		}

		var->name = name;
		var->value = value;
		if (head == NULL) {
			head = var;
		} else {
			vset * link = head;
			while (link->next != NULL) {
				link = link->next;
			}
			link->next = var;
		}
	} else {
		free(var->value);
		var->value = value;
		free(name);
	}
	if (save) {
		saveFlash(flash, offset);
	}

	return 0;
}

int
updateflash(const char * flash, off_t offset, const char *write)
{
	return _updateflash(flash, offset, write, 1);
}

static void
_deleteflash(const char * flash, off_t offset, const char * name, int save)
{

	vset *var = findValue(name) ;
	if (var != NULL) {
		/* Find previous */
		vset *prev = head;
		if (var == head) {
			head = var->next;
		} else {
			while (prev->next != NULL && prev->next != var) {
				prev = prev->next;
			}
			if (prev == NULL) {
				/* bad stuff */
				syslog(LOG_EMERG, "Internal inconsistency - exiting");
				exit(2);
			}
			prev->next = var->next;
		}
		if (var->name != NULL) {
			free(var->name);
		}
		if (var->value != NULL) {
			free(var->value);
		}
		free(var);
	}
	if (save) {
		saveFlash(flash, offset);
	}
}

void
deleteflash(const char *flash, off_t offset, const char *name)
{
	_deleteflash(flash, offset, name, 1);
}

void
freeVars()
{
	vset *f = NULL;

	while ((f = head)) {
		head = f->next;
		free(f->name);
		free(f->value);
		free(f);
	}
}

void
saveFlash(const char *flash, off_t offset)
{
	char cmd[128];
	char bin[FLASHLEN];
	FILE *f = NULL;
	size_t index = 4; /* leave the crc for last */
	int crc = 0;
	vset *var = head;

	while (var != NULL) {
		strncpy(bin + index, var->name, strlen(var->name));
		index += strlen(var->name);

		bin[index] = '=';
		index++;
		strncpy(bin + index, var->value, strlen(var->value));
		index += strlen(var->value);

		bin[index] = '\0';
		index++;
		var = var->next;
	}

	bin[index] = '\0';
	index++;
	bin[index] = '\0';
	index++;
	for (; index < FLASHLEN; index++) {
		bin[index] = (char)0xff;
	}

	crc = crc32(0, (const Bytef *) (bin + 4), FLASHLEN - 4);
	bin[0] = crc & 0x000000ff;
	bin[1] = (crc & 0x0000ff00) >> 8;
	bin[2] = (crc & 0x00ff0000) >> 16;
	bin[3] = (crc & 0xff000000) >> 24;

	snprintf(cmd, sizeof(cmd),
		"/bin/flashw -b -p -F -l -u -o %d -f - %s",
		(int) offset, flash );
	f = popen(cmd, "w");
	fwrite(bin, FLASHLEN, 1, f);
	pclose(f);

}

size_t
readfsetflash(const char *flash, int offset)
{
	char buffer[1024];
	char *eq = NULL;
	long crc32 = 0;
	FILE *f;
	int index = 0;
	int ret = 1;
	vset **last;

	if ((f = fopen(flash, "r")) == NULL) {
		syslog(LOG_ERR,
			"setfset: failed to read factory settings flash: %m");
		goto out;
	}

	if (fseek(f, offset, SEEK_SET) == -1) {
		syslog(LOG_ERR, "setfset: failed to find factory options: %m");
		goto out;
	}

	if (fread(&crc32, sizeof crc32, 1, f) < 1) {
		syslog(LOG_ERR, "setfset: failed to read factory options: %m");
		goto out;
	}

	/* Advance to the last .next pointer of the vset list */
	last = &head;
	while (*last)
		last = &(*last)->next;

	/* TODO end of flash */
	while (1) {
		vset *next = NULL;
		int ch;

		if ((ch = getc(f)) == EOF) {
			syslog(LOG_ERR, "setfset: read %s: %m", flash);
			goto out;
		}
		if (index >= 1024) {
			syslog(LOG_ERR, "setfset: possible corrupted flash");
			goto out;
		}
		buffer[index] = ch;

		if (buffer[index] == '\0') {
			if (index == 0) {
				/* double null - end */
				break;
			}
			eq = strstr(buffer, "=");
			if (!eq) {
				/* log message */
				ret = 2; /* indicate to display usage() */
				goto out;
			}

			*eq = '\0';
			eq++;

			next = calloc(1, sizeof(vset));
			if (!next) {
				/* log message */
				syslog(LOG_EMERG, "Failed to malloc: %m");
				exit(2);
			}

			next->name = strdup(buffer);
			next->value = strdup(eq);
			if (!next->name || !next->value) {
				syslog(LOG_EMERG, "setfset: malloc: %m");
				free(next->value);
				free(next->name);
				free(next);
				exit(2);
			}

			*last = next;
			last = &next->next;

			index = 0;
		} else {
			index++;
		}

	}

	ret = 0; /* success */

out:
	if (f) {
		fclose(f);
	}
	return ret;
}

