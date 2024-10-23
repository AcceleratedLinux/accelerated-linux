/****************************************************************************/

/*
 *	http.c -- get FLASH data from a http address.
 *
 *	(C) Copyright 1999-2000, Greg Ungerer (gerg@snapgear.com)
 */

/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "base64.h"

/****************************************************************************/

int httpreadline(int fd, char *buf, int len)
{
	char	c;
	int	size;

	for (len--, size = 0; (size < len); size++) {
		if (read(fd, &c, sizeof(c)) != sizeof(c))
			break;
		if (c == '\n')
			break;
		*buf++ = c;
	}
	*buf = 0;
	return(size);
}

/****************************************************************************/

int openhttp(char *url)
{
	struct addrinfo		*ai, *ais;
	const struct addrinfo	ai_hint = { .ai_socktype = SOCK_STREAM };
	char			*up, *sp, *ap;
	char			urlip[256];
	char			urlport[32] = "80";
	char			urlfile[256];
	char 			authbuf[256];
	char			enc_authbuf[b64_strlen(256)];
	char			buf[256];
	char			relocurl[512];
	int			fd, relocated, auth;
	int			bracketed;
	int			error;
	const char		*host;
	int			hostlen;

	fd = -1;
	up = url;
	auth = 0;

	do {
		/* Strip http protocol name from url */
		if (strncmp(up, "http://", 7) == 0)
				up += 7;

		/* See if we have a username and password 
		 * We determine this by looking for an @ before
		 * the first slash. After the slash, it could a file,
		 * and is disregarded
		 */
		ap = up;
		while (ap && (*ap != '/')) {
			if (*ap == '@') auth = 1;
			
			ap++;
		}
		
		if (auth) {
			/* Get the username and password pair */
			/* Note: this is a really dumb parser, so don't put slashes or @'s in the password */
			ap = up;
			for (sp = &authbuf[0]; (*ap != '@'); ap++) {
				*sp++ = *ap;
				if (sp >= &authbuf[sizeof(authbuf)-1])
					return(-1);
			}
			/* Set url pointer to just beyond the '@' */
			up = ap + 1;

			/* Generate our Base64 encoded string to present to the server */
			if (b64_ntop((const unsigned char *)authbuf, (sp - authbuf), 
							enc_authbuf, sizeof(enc_authbuf) -1) == -1)
				return(-1);
		}
		
		/* Get system name (or IP address) from url */
		host = up;
		bracketed = (*up == '[');
		if (bracketed)
			up++;
		for (sp = &urlip[0];
		     bracketed ? *up != ']' : ((*up != ':') && (*up != '/'));
		     up++) {
			unsigned int hex;
			if (*up == 0)
				return(-1);
			if (sscanf(up, "%%%2x", &hex) == 1) {
				up += 2;
				*sp++ = hex; /* URL-encoded %dd */
			} else {
				*sp++ = *up;
			}
			if (sp >= &urlip[sizeof(urlip)-1])
				return(-1);
		}
		*sp = 0;
		if (bracketed && *up == ']')
			up++;

		/* Get port number if supplied */
		if (*up == ':') {
			for (up++, sp = &urlport[0]; (*up != 0); up++) {
				if (*up == '/')
					break;
				*sp++ = *up;
				if (sp >= &urlport[sizeof(urlport)-1])
					return(-1);
			}
			*sp = 0;
		}
		hostlen = up - host;	/* host[:port] for the Host header */

		/* Get file path */
		for (sp = &urlfile[0]; (*up != 0); up++) {
			*sp++ = *up;
			if (sp >= &urlfile[sizeof(urlfile)-1])
				return(-1);
		}
		*sp = 0;

		/* Connect to host:port using first working address */
		if ((error = getaddrinfo(urlip, urlport, &ai_hint, &ais)) != 0) {
			fprintf(stderr, "%s %s: %s\n", urlip, urlport,
				gai_strerror(error));
			return(-1);
		}
		fd = -1;
		for (ai = ais; ai; ai = ai->ai_next) {
			if ((fd = socket(ai->ai_family, ai->ai_socktype,
					 ai->ai_protocol)) == -1)
				continue;
			if (connect(fd, ai->ai_addr, ai->ai_addrlen) != -1)
				break;
			close(fd);
			fd = -1;
		}
		freeaddrinfo(ais);
		if (fd == -1)
			return(-1);

		if (bracketed && strchr(urlip, '%')) {
			/* Fixup scopes in IPv6 hosts. RFC 3986 requires
			 * removing %scope component from Host headers.
			 * See https://redmine.lighttpd.net/issues/2442 */
			static char fixedhost[256];
			int urliplen = strchr(urlip, '%') - urlip;
			hostlen = snprintf(fixedhost, sizeof fixedhost,
				"[%.*s]:%s", urliplen, urlip, urlport);
			host = fixedhost;
		}

		/* Send GET request to server */
		if (!auth) {
			if (snprintf(buf, sizeof(buf), "GET %s HTTP/1.0\r\n"
				"User-Agent: netflash/100\r\n"
				"Host: %.*s\r\n"
				"\r\n",
				urlfile, hostlen, host) >= sizeof(buf)) {
					close(fd);
					return(-1);
			}
		} else {
			if (snprintf(buf, sizeof(buf), "GET %s HTTP/1.0\r\n"
				"User-Agent: netflash/100\r\n"
				"Host: %.*s\r\n"
				"Authorization: Basic %s\r\n"
				"\r\n",
				urlfile, hostlen, host, enc_authbuf)  >= sizeof(buf)) {
						close(fd);
						return(-1);
			}
		}

		if (write(fd, buf, strlen(buf)) < 0) {
			close(fd);
			return(-1);
		}

		if (httpreadline(fd, buf, sizeof(buf)) < 0) {
			close(fd);
			return(-1);
		}

		relocated = 0;
		if ((sp = strchr(buf, ' '))) {
			switch (sp[1]) {
			case '3':
				relocated++;
				break;
			case '2':
				break;
			default:
				close(fd);
				return(-1);
			}
		}

		for (;;) {
			if (httpreadline(fd, buf, sizeof(buf)) < 0) {
				close(fd);
				return(-1);
			}
			if ((buf[0] == '\n') || (buf[0] == '\r'))
				break;
			if (strncmp(buf, "Location:", 9) == 0) {
				strncpy(relocurl, buf+10, sizeof(relocurl));
				up = relocurl;
			}
		}
	} while (relocated);

	return(fd);
}

/****************************************************************************/
