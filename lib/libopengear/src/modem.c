#define _GNU_SOURCE

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/file.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <termios.h>

#if defined(EMBED) && !defined(NGCS)
#include <config/autoconf.h>
#endif

#include <opengear/serial.h>
#include <opengear/modem.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

typedef struct modem_port {
	const char *data_dev;
	const char *cmd_dev;
	opengear_modem_flags_t flags;
} modem_port_t;

modem_port_t modem_ports[] = {
	{ OG_DEV_SERCON, NULL, OG_MODEM_PORT_DB9 },
	{ OG_MODEM_DEV_INTERNAL, NULL, OG_MODEM_PORT_INTERNAL },
	{ OG_MODEM_DEV_PCMCIA, NULL, OG_MODEM_PORT_PCMCIA },
	{ OG_MODEM_DEV_USB, NULL, OG_MODEM_PORT_USB },
	{ OG_MODEM_DEV_CELL, OG_COMMAND_DEV_CELL, OG_MODEM_PORT_CELL },
	{ NULL, NULL, 0 }
};

#define MODEM_EXTERNAL_CELL "/var/run/.modem/cellmodem_is_external"
#define MODEM_INTERNAL_CELL "/var/run/.modem/cellmodem_is_internal"

#define MODEM_CELL_IFACE "/var/run/.modem/.cellmodem_iface"
#define MODEM_CELL_IFACE_DEFAULT "wwan0"

const char *
opengear_modem_title(const opengear_modem_t *modem)
{
	struct stat statbuf;

	if (modem->m_flags & OG_MODEM_PORT_DB9) {
#ifdef CONFIG_PRODUCT
	        if (strcmp(CONFIG_PRODUCT, "IM72xx") == 0) {
			return ("Serial Console");
		}
#endif
		return ("Serial DB9 Port");
	} else if (modem->m_flags & OG_MODEM_PORT_INTERNAL) {
		return ("Internal Modem");
	} else if (modem->m_flags & OG_MODEM_PORT_PCMCIA) {
		return ("PC Card Modem");
	} else if (modem->m_flags & OG_MODEM_PORT_USB) {
		return ("External USB Modem");
	} else if (modem->m_flags & OG_MODEM_PORT_CELL) {
		if (stat(MODEM_EXTERNAL_CELL, &statbuf) != 0) {
			return ("Internal Cellular Modem");
		} else {
			return ("Cellular Modem");
		}
	}
	return ("Unknown");
}

static bool
is_char_device(const char *device)
{
	struct stat st;

	if (stat(device, &st) == 0 && S_ISCHR(st.st_mode)) {
		return true;
	}
	return false;
}

static bool
file_exists(const char *file)
{
	struct stat st;

	if (stat(file, &st) == 0) {
		return true;
	}
	return false;
}

int
opengear_modem_getmodems(opengear_modem_t *modems, size_t len)
{
	modem_port_t *m_port = NULL;
	size_t count = 0;
	size_t i;

	for (i = 0; modem_ports[i].data_dev != NULL; i++) {

		if (count >= len) {
			fprintf(stderr, "Exceeded modem buffer limit\n");
			break;
		}

		m_port = &modem_ports[i];

		if (!is_char_device(m_port->data_dev)) {
			/* If device has an internal cell modem but it is not
			detected, create it anyway. It could be disconnected
			due to upgrading firmware, or restarting via watchdog,
			but we still want to display it in the UI. */
			if (strncmp(m_port->data_dev, OG_MODEM_DEV_CELL,
				sizeof(OG_MODEM_DEV_CELL)) == 0) {
				if (!file_exists(MODEM_INTERNAL_CELL)) {
					continue;
				}
			} else {
				continue;
			}
		}

		strncpy(modems[count].m_data_dev,
			m_port->data_dev + strlen("/dev/"),
			sizeof(modems[count].m_data_dev));

		if (m_port->cmd_dev && is_char_device(m_port->cmd_dev)) {
			strncpy(modems[count].m_cmd_dev,
				m_port->cmd_dev + strlen("/dev/"),
				sizeof(modems[count].m_cmd_dev));
		} else {
			modems[count].m_cmd_dev[0] = '\0';
		}

		modems[count].m_flags = m_port->flags;
		count++;
	}

	return count;
}

int
opengear_modem_count(void)
{
	int count = 0;
	opengear_modem_t *tmp = calloc(OG_MODEM_MAX, sizeof(*tmp));
	if (tmp == NULL) {
		return (0);
	}
	count = opengear_modem_getmodems(tmp, OG_MODEM_MAX);
	free(tmp);
	return (count);
}

#define AT_RECV_RETRIES 20

static int
flock_timeout(int fd, int operation, time_t *timeout)
{
	int ret;

	ret = flock(fd, operation | LOCK_NB);
	while (ret == -1 && errno == EWOULDBLOCK && *timeout > 0) {
	    sleep(1);
	    --*timeout;
	    ret = flock(fd, operation | LOCK_NB);
	}
	return ret;
}

int
opengear_modem_getatresponse(
		const char *command, const char *expect,
		char *buf, size_t len, time_t timeout)
{
	char timestring[sizeof("600")];
	struct termios savetio, tio;
	int ttyfd = -1;
	bool rv = false;
	int pipefds[2];
	pid_t pid;
	size_t readlen = 0;
	char *line = NULL;
	char *p = NULL;
	ssize_t nread = 0;
	int status = 0;
	FILE *childfp = NULL;
	size_t bufcount = 0;

	memset(buf, '\0', len);

	if (timeout > 600) {
		syslog(LOG_ERR, "Timeout must be < 600 seconds");
		return (-1);
	}
	memset(timestring, '\0', sizeof(timestring));
	snprintf(timestring, sizeof(timestring), "%u", (unsigned int) timeout);

	syslog(LOG_DEBUG, "Sending command: '%s' to '%s'",
			command, OG_COMMAND_DEV_CELL);
	ttyfd = open(OG_COMMAND_DEV_CELL, O_RDWR | O_NOCTTY | O_NDELAY);
	if (ttyfd < 0) {
		syslog(LOG_ERR, "open(%s) failed: %s",
			OG_COMMAND_DEV_CELL, strerror(errno));
		return (-1);
	}
	if (flock_timeout(ttyfd, LOCK_EX, &timeout) == -1) {
		syslog(LOG_ERR, "flock(%s, EX) failed: %s",
			OG_COMMAND_DEV_CELL, strerror(errno));
		close(ttyfd);
		return (-1);
	}
	fcntl(ttyfd, F_SETFL, 0);

	if (tcgetattr(ttyfd, &savetio) < 0) {
		syslog(LOG_ERR, "tcgetattr failed: %s", strerror(errno));
		close(ttyfd);
		return (-1);
	}

	cfsetispeed(&tio, B115200);
	cfsetospeed(&tio, B115200);
	cfmakeraw(&tio);
	tcsetattr(ttyfd, TCSANOW, &tio);
	tcflush(ttyfd, TCIFLUSH);

	if (pipe(pipefds) < 0) {
		syslog(LOG_ERR, "pipe failed: %s", strerror(errno));
		goto done;
	}

	pid = fork();
	if (pid == 0) {
		const char *chatscript = "/etc/scripts/command-chat";
		char ecommand[sizeof("COMMAND=") + 64];
		char eexpect[sizeof("RESPONSE=") + 128];
		const char *env[] = { NULL, NULL, NULL };

		close(pipefds[0]);

		dup2(ttyfd, STDIN_FILENO);
		dup2(ttyfd, STDOUT_FILENO);

		dup2(pipefds[1], STDERR_FILENO);
		snprintf(ecommand, sizeof(ecommand), "COMMAND=%s", command);
		snprintf(eexpect, sizeof(eexpect), "RESPONSE=%s", expect);

		env[0] = ecommand;
		env[1] = eexpect;

		execle("/bin/chat",
			"chat", "-V", "-E", "-t", timestring, "-f",
			chatscript, (char *) NULL,
			env);
		_exit(1);

	} else if (pid < 0) {
		syslog(LOG_ERR, "fork failed: %s", strerror(errno));
		goto done;
	}

	close(pipefds[1]);
	childfp = fdopen(pipefds[0], "r");
	if (childfp == NULL) {
		syslog(LOG_ERR, "fdopen(r) failed: %s", strerror(errno));
		goto done;
	}

	while ((nread = getline(&line, &readlen, childfp)) > 0) {

		syslog(LOG_DEBUG, "AT OUTPUT: %s", line);

		if (strncmp(command, line, strlen(command)) == 0) {
			continue;
		}
		if (strncmp(expect, line, strlen(expect)) == 0) {
			continue;
		}
		if (*line == '\n') {
			continue;
		}
		// Convert ^I to tab
		for (p = line; nread--; p++) {
			if (*p == '^' && *(p + 1) == 'I') {
				if (bufcount < len) {
					buf[bufcount] = '\t';
					bufcount++;
				}
				++p;
				--nread;
			} else {
				if (bufcount < len) {
					buf[bufcount] = *p;
					bufcount++;
				}
			}
		}
	}
	if (line != NULL) {
		free(line);
	}

        waitpid(pid, &status, 0);
	if (WEXITSTATUS(status) != 0) {
		syslog(LOG_DEBUG, "child returned: %d", WEXITSTATUS(status));
		goto done;
	}

	rv = 0;

done:
	if (childfp != NULL) {
		fclose(childfp);
	}

	if (ttyfd != -1) {
		tcsetattr(ttyfd, TCSANOW, &savetio);
		close(ttyfd);
	}

	return (rv);
}

#if 0
{

	fd_set in;
	struct termios oldtio;
	struct termios newtio;
	struct timeval tv = { timeout, 0 };
	ssize_t nread = 0;
	int result = 0;
	int total = 0;
	int retries = 0;
	int fd = -1;
	int rv = -1;

	memset(buf, '\0', len);

	/* Initialise AT Command Channel */
	/* Open device in RW exclusive mode. */
	fd = open(OG_COMMAND_DEV_CELL, O_RDWR | O_EXCL | O_NONBLOCK | O_NOCTTY);
	if (fd < 0) {
		fprintf(stderr, "Failed to open: " OG_COMMAND_DEV_CELL ": %s\n",
			strerror(errno));
		goto onexit;
	}
	if (ioctl(fd, TIOCEXCL) != 0) {
		fprintf(stderr, "Failed to lock: " OG_COMMAND_DEV_CELL "\n");
        	close(fd);
		goto onexit;
	}

	/* Save old device settings. */
	if (ioctl(fd, TCGETA, &oldtio) != 0) {
		fprintf(stderr, "Failed saving old termios: %s (%d) %s\n",
			OG_COMMAND_DEV_CELL, errno, strerror(errno));
		close(fd);
		fd = -1;
		goto onexit;
	}

	bzero(&newtio, sizeof(newtio));
        newtio.c_cflag = B9600 | CRTSCTS | CS8 | CLOCAL | CREAD;
        newtio.c_iflag = IGNPAR;
        newtio.c_oflag = 0;

        tcflush(fd, TCIFLUSH);
        tcsetattr(fd, TCSANOW, &newtio);

	FD_ZERO(&in);
	FD_SET(fd, &in);
	result = select(fd + 1, &in, NULL, NULL, &tv);
	if (result == -1) {
		fprintf(stderr, "Failed to select on: "
			OG_COMMAND_DEV_CELL ": %s\n", strerror(errno));
		goto onexit;
	}

	if (result != 1 || !FD_ISSET(fd, &in)) {
		fprintf(stderr, "select() failed: "
			OG_COMMAND_DEV_CELL " result: %d\n", result);
		goto onexit;
	}

	do {
		usleep(100);
		errno = 0;
		nread = read(fd, &buf[total], (len - total));
		if ((nread == 0) || (errno == EAGAIN)) {
			if (retries > AT_RECV_RETRIES) {
				fprintf(stderr, "Exceeded read retries: "
						OG_COMMAND_DEV_CELL "\n");
				break;
			}
			retries++;
			continue;
		} else if (nread > 0) {
			total += nread;
			continue;
		} else {
			fprintf(stderr, "Read Error on %s: %s\n",
				OG_COMMAND_DEV_CELL, strerror(errno));
			goto onexit;
		}
	} while (total < len && retries < AT_RECV_RETRIES);

        fprintf(stdout, "Got %d bytes from %s retries: %d\n",
			total, OG_COMMAND_DEV_CELL, retries);
	rv = 0;

onexit:

	if (fd != -1) {
		tcsetattr(fd, TCSANOW, &oldtio);
		close(fd);
	}

	return (rv);
}

#endif

/* Reads first line from filename into the buffer, trims trailing newline.
 * Returns length of the buffer on success, or -1 on error. */
int opengear_read_file_line(const char *filename, char *buffer, size_t buffersz)
{
        FILE *f;
        int len = -1;

        f = fopen(filename, "r");
        if (!f)
                goto out;
        if (!fgets(buffer, buffersz, f))
                goto out;
        len = strlen(buffer);
        if (len && buffer[len - 1] == '\n')
                buffer[--len] = '\0';
out:
        if (f)
                fclose(f);
        return (len);
}

const char *opengear_modem_getcelliface(void)
{
        static char iface[sizeof("wwan65535")];

	memset(iface, '\0', sizeof(iface));

        if (opengear_read_file_line(MODEM_CELL_IFACE, iface, sizeof(iface)) == -1) {
                if (errno != ENOENT)
                        perror(MODEM_CELL_IFACE);
                return MODEM_CELL_IFACE_DEFAULT;
        }
        return iface;
}

/*****************************************************************************/
/*
 * this __MUST__ be at the VERY end of the file - do NOT move!!
 *
 * Local Variables:
 * c-basic-offset: 8
 * tab-width: 8
 * end:
 * vi: tabstop=8 shiftwidth=8 textwidth=79 noexpandtab
 */
