#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <err.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <inttypes.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>

#include "vfile.h"
#include "multipart.h"

#define streq(a,b) (strcmp(a,b) == 0)

static struct options {
	unsigned force : 1;		/*  -F */
	unsigned reboot : 1;		/* !-b */
	unsigned compare_only : 1;	/*  -C */
	unsigned erase_blanks : 1;	/* !-e */
	unsigned config_init : 1;	/*  -E */
	unsigned erase_all : 1;		/*  -s/-S */
	unsigned check_product : 1;	/* !-H */
	unsigned check_version : 1;	/* !-i */
	unsigned check_checksum : 1;	/* !-n */
	unsigned check_hash : 1;	/* !-N */
	unsigned unlock_before : 1;	/*  -u */
	unsigned lock_after : 1;	/*  -l */
	unsigned preserve : 1;		/*  -p */
	unsigned output_plain : 1;	/*  -R */
	unsigned test_only : 1;		/*  -t */
	unsigned pat_watchdog : 1;	/* !-w */
	unsigned decompress : 1;	/*  -z */
	unsigned idempotent_check : 1;	/*  -Z */
	unsigned error : 1;		/* options processing error */
	const char *console;		/*  -C */
	const char *proto;		/*  -f */
	const char *server;
	char *filename;
	const char *srcaddr;		/* -I */
	const char *output;		/* -r/-R */
	unsigned int delay;		/* -d */
	unsigned int write_offset;	/* -o */
	enum { KILL_NONE, KILL_ALL, KILL_SOME } kill; /* -k, -K */
} Opt;

extern const char netflash_version[];
extern const char netflash_product[];
extern const char netflash_vendor[];

static void process_options(int argc, char *argv[]);

struct trailer {
	int length;	/* length of SnapGear trailer in bytes */
	char version[20];
	char vendor[64];
	char product[64];
	uint32_t crc;
};

static void
usage(const char *arg0)
{
	fprintf(stderr, "usage: %s [-abCeEfFhHijkKlnNpsStuvwzZ]"
			" [-c console]"
			" [-d delay]"
			" [-I address]"
			" [-o ]"
			" [-r device]"
			" [-R file]"
			" [net-server]"
			" filename\n",
			arg0);
}

static int
startswith(const char *s, const char *prefix)
{
	return strncmp(s, prefix, strlen(prefix)) == 0;
}

/* Return s as an integer, or exit(1) */
static unsigned int
to_uint(const char *s)
{
	char *end = NULL;
	unsigned long v;

	v = strtoul(s, &end, 0);
	if (end == s) errx(1, "invalid number '%s'", s);
	if (end && *end) errx(1, "invalid character '%c' in '%s'", *end, s);
	if (v > UINT_MAX) errx(1, "number too big");
	return v;
}

/* Tests if s starts with "<scheme>://" */
static int
is_url(const char *s)
{
	if (!isalpha(*s)) return 0;
	while (*s && isalpha(*s)) s++;
	if (*s++ != ':') return 0;
	if (*s++ != '/') return 0;
	if (*s++ != '/') return 0;
	return 1;
}

static void
print_progress(void *d, int n, int max)
{
	const char *filename = d;

	/* Throttle partial updates to 1 Hz */
	if (n != max && n != -1) {
		static time_t last_update;
		time_t now = time(0);
		if (now == last_update)
			return;
		last_update = now;
	}

	if (max == -1)
		printf("%6d kB %.50s\r", n / 1024, filename);
	else
		printf("%6d/%d kB %.50s\r", n / 1024, max / 1024, filename);
	if (max >= 0 && n == max)
		putchar('\n');
	fflush(stdout);
}

/**
 * Handle an invocation from netflash.cgi. CGI parameters are
 * read from the environment, and the form data is read from #stdin.
 * @returns valid vfile because error paths end up at exit(1).
 */
static struct vfile *
load_cgi(char *filename)
{
	char dataname[64];
	char optionsname[64];
	struct cgi_req req = {
		.request_method = getenv("REQUEST_METHOD"),
		.content_length = getenv("CONTENT_LENGTH"),
		.content_type = getenv("CONTENT_TYPE"),
	};
	struct multipart *m;
	char name[64];
	struct vfile *flashfile = NULL;
	struct vfile *f;

	if (sscanf(filename, "cgi://%63[^,],%63[^,]", dataname, optionsname) != 2)
		errx(1, "bad cgi url");
	req.content = vfile_membuf(vfile_fd(STDIN_FILENO)),
	m = multipart_new(req);
	if (!m)
		errx(1, "bad multipart document");

	while ((f = multipart_part(m, name))) {
		if (strcmp(name, dataname) == 0) {
			vfile_decref(flashfile);
			flashfile = f;
		} else if (strcmp(name, optionsname) == 0) {
			static char options[1024];
			static char *argv[64] = { 0 };
			int argc = 0;
			int len = vfile_read(f, options, sizeof options - 1);
			char *p;
			char *arg;

			options[len] = '\0';
			argv[argc++] = (char *)"netflash.cgi";
			p = options;
			while ((arg = strsep(&p, " ")))
				if (*arg && argc < 63)
					argv[argc++] = arg;
			argv[argc] = NULL;
			process_options(argc, argv);
			vfile_decref(f);
			if (Opt.error)
				errx(1, "error processing options");
		} else
			vfile_decref(f);
	}
	multipart_free(m);

	return flashfile;
}

/* Loads a file or URL */
static struct vfile *
load_file(char *filename)
{
	struct vfile *src;

	if (startswith(filename, "cgi://")) {
		src = load_cgi(filename);
	} else if (is_url(filename)) {
		char *argv[] = { "curl", "-Ls", "--url", filename, 0 };
		src = vfile_proc("curl", argv);
	} else if (strcmp(filename, "-") == 0) {
		src = vfile_fd(STDIN_FILENO);
	} else {
		int fd = open(filename, O_RDONLY);
		if (fd == -1)
			err(1, "%s", filename);
		src = vfile_fd(fd);
	}

	return src;
}

static int
is_qcow2(struct vfile *f)
{
	char magic[4];

	if (vfile_seek(f, 0) != 0)
		return 0;
	if (vfile_read(f, magic, sizeof magic) != sizeof magic)
		return 0;
	return memcmp(magic, "QFI\xfb", sizeof magic) == 0;
}

/* Test if dotted version a <= b.
 * Missing numeric components treated as 0.
 * Components may be separated by (b|.|u|p). */
static int
version_le(const char *a, const char *b)
{
	while (*a && *b) {
		unsigned int aval = 0, bval = 0;
		static const char prefix[] = "b.up";
		const char *apfx, *bpfx;

		if (!isdigit(*a) || !isdigit(*b))
			break; /* incomparable */
		while (isdigit(*a))
			aval = 10 * aval + (*a++ - '0');
		while (isdigit(*b))
			bval = 10 * bval + (*b++ - '0');
		if (aval < bval) return 1; /* a < b */
		if (aval > bval) return 0; /* a > b */

		if (!*a && !*b) return 1; /* a == b */
		if (!*a) a = ".0";
		if (!*b) b = ".0";

		apfx = strchr(prefix, *a++);
		bpfx = strchr(prefix, *b++);
		if (!apfx || !bpfx) break; /* incomparable */
		if (apfx < bpfx) return 1; /* a < b */
		if (apfx > bpfx) return 0; /* a > b */
	}
	return strcmp(a, b) <= 0;
}

#if defined(SELFTEST)
#include <assert.h>
__attribute__((constructor)) static void
test_version_le() {
	assert(version_le("0.0.0", "0.0.0"));
	assert(version_le("0.0.0", "0.0.1"));
	assert(!version_le("0.0.1", "0.0.0"));
	assert(version_le("9", "10"));
	assert(version_le("1.9", "1.10"));
	assert(version_le("1.9.2", "1.10.1"));
	assert(version_le("1b2", "1.1"));
	assert(version_le("1.2", "1.2u0"));
	assert(version_le("1.2", "1.2p3"));
}
#endif

/** Reads the file's SnapGear trailer.
 * "\04.10.0\0OpenGear\0ACM700x\x00\x00\x3b\x64"
 *    <vers>  <vendor>  <prdct><----checksum-->
 * @arg f               the file to read the trailer from
 * @arg end		end position of the file (where the trailer ends)
 * @arg trailer_return  storage for holding the read trailer data
 * @return 0 on success, -1 on error
 */
static int
read_trailer(struct vfile *f, int end, struct trailer *trailer_return)
{
	struct trailer *t = trailer_return;
	unsigned char buf[256];
	int m;
	int n;

	if (end < sizeof buf)
		return -1; /* too short */
	if (vfile_seek(f, end - sizeof buf) == -1)
		return -1;
	m = vfile_read(f, buf, sizeof buf);
	if (m == -1)
		return -1;

	n = m;
	if (n < 7)
		return -1;
	n -= 4;
	t->crc = (buf[n + 3] << 0) |	/* big-endian */
	         (buf[n + 2] << 8) |
	         (buf[n + 1] << 16) |
	         (buf[n + 0] << 24);

	/* Reads a string from the trailer ending at buf[n-1].
	 * Moves n backwards to the NUL lead byte and sets src
	 * to the string just skipped. */
#	define scan_part(dst) do { \
		int e = n; \
		while (n && dst[n - 1]) \
			n--; \
		if (!n || e - n >= sizeof dst) \
			return -1; \
		memcpy(dst, &buf[n], e - n); \
		dst[e - n] = '\0'; \
		n--; \
		/* assert(buf[n] == '\0') */ \
	} while (0)

	scan_part(t->product);
	scan_part(t->vendor);
	scan_part(t->version);

	t->length = m - n;

	return 0;
}

static uint32_t
fsk_checksum(struct vfile *f, int len)
{
	uint32_t crc = 0;
	static unsigned char buf[512];

	if (vfile_seek(f, 0) == -1)
		return 0;
	while (len) {
		int i;
		int n = vfile_read(f, buf, len < sizeof buf ? len : sizeof buf);
		if (n == -1)
			return 0;
		for (i = 0; i < n; n++)
			crc += buf[i];
		len -= n;
	}
	crc = (crc & 0xffff) + (crc >> 16);
	crc = (crc & 0xffff) + (crc >> 16);
	return crc;
}

static void
write_to_file(const char *filename, int wfd, struct vfile *src, int max)
{
	int n;
	int total = 0;
	static char buf[4096];
	static char rbuf[4096];

	print_progress((void *)filename, -1, max);

	vfile_seek(src, 0);
	while ((n = vfile_read(src, buf, sizeof buf)) > 0) {
		char *b = rbuf;
		int rn = n;

		while (rn) {
			int m = read(wfd, b, rn);
			if (m == -1)
				err(1, "read %s", filename);
			rn -= m;
			b += m;
		}
		if (memcmp(rbuf, buf, n) == 0)
			continue;

		if (lseek(wfd, -n, SEEK_CUR) == -1)
			err(1, "seek %s", filename);
		b = buf;
		while (n) {
			int m = write(wfd, b, n);
			if (m == -1)
				err(1, "write %s", filename);
			n -= m;
			b += m;
			total += m;
			print_progress((void *)filename, total, max);
		}
	}
	if (n == -1)
		errx(1, "read");
	if (close(wfd) == -1)
		err(1, "%s", filename);
}

static void
attach_console(const char *console)
{
	int cons;
	int tty;
	pid_t pid;

	fflush(stdin);
	fflush(stdout);
	fflush(stderr);

	/* daemon() */
	signal(SIGCHLD, SIG_IGN);
	pid = fork();
	if (pid == -1)
		err(1, "fork");
	if (pid != 0)
		_exit(0);
	if (setsid() == -1)
		err(1, "setsid");

	/* Detach from ctty */
	tty = open("/dev/tty", O_RDWR, 0666);
	if (tty == -1)
		err(1, "/dev/tty");
	ioctl(tty, TIOCNOTTY);
	close(tty);

	/* Redirect stdio to console */
	cons = open(console, O_RDWR, 0666);
	if (cons == -1)
		err(1, "%s", console);
	dup2(cons, 0);
	dup2(cons, 1);
	dup2(cons, 2);
	if (cons > 2)
		close(cons);
}

static void
process_options(int argc, char *argv[])
{
	int ch;
	extern char *optarg;

	while ((ch = getopt(argc, argv, "abc:Cd:eEfFhHiI:jkKlnNo:pr:R:sStuvwzZ")) != -1)
		switch (ch) {
		case 'a': break; /* ignored */
		case 'b': Opt.reboot = 0; break;
		case 'c': Opt.console = optarg; break;
		case 'C': Opt.compare_only = 1; break;
		case 'd': Opt.delay = to_uint(optarg); break;
		case 'e': Opt.erase_blanks = 0; break;
		case 'E': Opt.config_init = 1; break;
		case 'f': Opt.proto = "ftp"; break;
		case 'F': Opt.force = 1;
		case 'h': usage(argv[0]); exit(0); break;
		case 'H': Opt.check_product = 0; break;
		case 'i': Opt.check_version = 0; break;
		case 'I': Opt.srcaddr = optarg; break;
		case 'j': /* XXX */ break;
		case 'k': Opt.kill = KILL_NONE; break;
		case 'K': Opt.kill = KILL_SOME; break;
		case 'l': Opt.lock_after = 1; break;
		case 'n': Opt.check_checksum = 0; break;
		case 'N': Opt.check_hash = 0; break;
		case 'o': Opt.write_offset = to_uint(optarg); break;
		case 'p': Opt.preserve = 1; break;
		case 'r': Opt.output = optarg; break;
		case 'R': Opt.output = optarg; Opt.output_plain = 1; break;
		case 's': Opt.erase_all = 0; break;
		case 'S': Opt.erase_all = 1; break;
		case 't': Opt.test_only = 1; break;
		case 'u': Opt.unlock_before = 1; break;
		case 'v': puts(netflash_version); exit(0);
		case 'w': Opt.pat_watchdog = 0; break;
		case 'z': Opt.decompress = 1; break;
		case 'Z': Opt.idempotent_check = 1; break;
		default:
			Opt.error = 1;
		}
}

int
main(int argc, char *argv[])
{
	extern char *optarg;
	extern int optind;
	struct vfile *src;
	int size;
	struct trailer trailer;
	int wfd;
	int flags;

	Opt.reboot = 1;
	Opt.kill = KILL_ALL;
	Opt.erase_blanks = 1;
	Opt.check_product = 1;
	Opt.check_version = 1;
	Opt.check_checksum = 1;
	Opt.check_hash = 1;
	Opt.proto = "tftp";
	Opt.output = "/dev/flash/image";
	Opt.pat_watchdog = 1;

	process_options(argc, argv);
	if (optind + 1 < argc)			/* [server] filename */
		Opt.server = argv[optind++];
	if (optind < argc)			/* filename */
		Opt.filename = argv[optind++];
	else
		Opt.error = 1;	/* missing required argument */
	if (optind < argc)
		Opt.error = 1;	/* too many arguments */
	if (Opt.error) {
		usage(argv[0]);
		exit(2); /* usage error */
	}

	/* Convert [-f] [server] <filename> into an URL */
	if (Opt.server) {
		int urlsz = strlen(Opt.proto) + strlen("://") +
			strlen(Opt.server) + 1 + strlen(Opt.filename) + 1;
		char *url = malloc(urlsz);
		if (!url) err(1, "malloc");
		snprintf(url, urlsz, "%s://%s%s%s",
			Opt.proto, Opt.server, *Opt.filename == '/' ? "" : "/",
			Opt.filename);
		Opt.filename = url;
		Opt.server = NULL;
	}

	src = load_file(Opt.filename);

	if (Opt.console)
		attach_console(Opt.console);

	if (!startswith(Opt.filename, "cgi://")) {
		src = vfile_progress(src, print_progress, Opt.filename);
		src = vfile_membuf(src);
	}

	/* Trigger a full load into memory */
	vfile_getsize(src);

	if (Opt.test_only)
		wfd = -1;
	else {
		/* Open the output device early */
		flags = Opt.compare_only ? O_RDONLY : O_RDWR;
		if (!startswith(Opt.output, "/dev/") || Opt.force)
			flags |= O_CREAT; /* Avoid creating new files under /dev */
		wfd = open(Opt.output, flags, 0666);
		if (wfd == -1)
			err(1, "%s", Opt.output);
	}

	/* For QCOW2 images we narrow ourselves to partition 2 */
	if (is_qcow2(src)) {
		src = vfile_qcow2(src);
		src = vfile_dospart(src, 2);
		if (!src)
			errx(1, "DOS partition not found");
	}

	/* Read the SnapGear trailer */
	size = vfile_getsize(src);
	if (read_trailer(src, size, &trailer) == -1)
		errx(1, "missing trailer");

	printf("trailer len %d version <%s> vendor <%s> product <%s> crc %08x\n",
		trailer.length, trailer.version, trailer.vendor,
		trailer.product, trailer.crc);

	if (Opt.check_product) {
		if (!streq(trailer.vendor, netflash_vendor))
			errx(1, "image vendor mismatch '%s'", trailer.vendor);
		if (!streq(trailer.product, netflash_product))
			errx(1, "image product mismatch '%s'", trailer.product);
	}
	if (Opt.check_version) {
		if (version_le(trailer.version, netflash_version))
			errx(1, "image version too old '%s'", trailer.version);
	}
	if (Opt.check_checksum) {
		uint32_t crc = fsk_checksum(src, size - trailer.length);
		if (crc != trailer.crc)
			errx(1, "bad checksum %x != %x", crc, trailer.crc);
	}

	if (!Opt.test_only)
		write_to_file(Opt.output, wfd, src, size);

	vfile_decref(src);

	if (Opt.reboot) {
		sync();
		if (reboot(RB_AUTOBOOT) == -1)
			err(1, "reboot");
		errx(1, "reboot returned");
	}

	exit(0);
}
