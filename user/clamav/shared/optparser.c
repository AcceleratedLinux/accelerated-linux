/*
 *  Copyright (C) 2008-2009 Sourcefire, Inc.
 *
 *  Author: Tomasz Kojm <tkojm@clamav.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301, USA.
 */

/*
 * TODO:
 * - clamconf, milter
 * - clamconf: generation/verification/updating of config files and man page entries
 * - automatically generate --help pages (use the first line from the description)
 */

#if HAVE_CONFIG_H
#include "clamav-config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <ctype.h>

#include "shared/optparser.h"
#include "shared/misc.h"

#include "libclamav/regex/regex.h"
#include "libclamav/default.h"

#include "getopt.h"

#define MAXCMDOPTS  100
#define MAX(a,b) (a > b ? a : b)

#define MATCH_NUMBER "^[0-9]+$"
#define MATCH_SIZE "^[0-9]+[KM]?$"
#define MATCH_BOOL "^(yes|true|1|no|false|0)$"

#define FLAG_MULTIPLE	1 /* option can be used multiple times */
#define FLAG_REQUIRED	2 /* arg is required, even if there's a default value */
#define FLAG_HIDDEN	4 /* don't print in clamconf --generate-config */
#define FLAG_REG_CASE	8 /* case-sensitive regex matching */

const struct clam_option clam_options[] = {
    /* name,   longopt, sopt, argtype, regex, num, str, flags, owner, description, suggested */

    /* cmdline only */
    { NULL, "help", 'h', TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD | OPT_FRESHCLAM | OPT_CLAMSCAN | OPT_CLAMDSCAN | OPT_SIGTOOL | OPT_MILTER | OPT_CLAMCONF | OPT_CLAMDTOP, "", "" },
    { NULL, "config-file", 'c', TYPE_STRING, NULL, 0, CONFDIR"/clamd.conf", FLAG_REQUIRED, OPT_CLAMD | OPT_CLAMDSCAN | OPT_CLAMDTOP, "", "" },
    { NULL, "config-file", 0, TYPE_STRING, NULL, 0, CONFDIR"/freshclam.conf", FLAG_REQUIRED, OPT_FRESHCLAM, "", "" },
    { NULL, "config-file", 'c', TYPE_STRING, NULL, 0, CONFDIR"/clamav-milter.conf", FLAG_REQUIRED, OPT_MILTER, "", "" },
    { NULL, "version", 'V', TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD | OPT_FRESHCLAM | OPT_CLAMSCAN | OPT_CLAMDSCAN | OPT_SIGTOOL | OPT_MILTER | OPT_CLAMCONF | OPT_CLAMDTOP, "", "" },
    { NULL, "debug", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD | OPT_FRESHCLAM | OPT_CLAMSCAN | OPT_SIGTOOL, "", "" },
    { NULL, "verbose", 'v', TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_FRESHCLAM | OPT_CLAMSCAN | OPT_CLAMDSCAN | OPT_SIGTOOL, "", "" },
    { NULL, "quiet", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_FRESHCLAM | OPT_CLAMSCAN | OPT_CLAMDSCAN | OPT_SIGTOOL, "", "" },
    { NULL, "leave-temps", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN, "", "" },
    { NULL, "no-warnings", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_FRESHCLAM, "", "" },
    { NULL, "stdout", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_FRESHCLAM | OPT_CLAMSCAN | OPT_CLAMDSCAN | OPT_SIGTOOL, "", "" },
    { NULL, "daemon", 'd', TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_FRESHCLAM, "", "" },
    { NULL, "no-dns", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_FRESHCLAM, "", "" },
    { NULL, "list-mirrors", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_FRESHCLAM, "", "" },
    { NULL, "submit-stats", 0, TYPE_STRING, NULL, 0, CONFDIR"/clamd.conf", 0, OPT_FRESHCLAM, "", "" }, /* Don't merge this one with SubmitDetectionStats */
    { NULL, "reload", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMDSCAN, "", "" },
    { NULL, "multiscan", 'm', TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMDSCAN, "", "" },
    { NULL, "fdpass", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMDSCAN, "", "" },
    { NULL, "stream", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMDSCAN, "", "" },
    { NULL, "database", 'd', TYPE_STRING, NULL, -1, DATADIR, FLAG_REQUIRED, OPT_CLAMSCAN, "", "" }, /* merge it with DatabaseDirectory (and fix conflict with --datadir */
    { NULL, "recursive", 'r', TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN, "", "" },
    { NULL, "bell", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN, "", "" },
    { NULL, "no-summary", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN | OPT_CLAMDSCAN, "", "" },
    { NULL, "file-list", 'f', TYPE_STRING, NULL, -1, NULL, 0, OPT_CLAMSCAN | OPT_CLAMDSCAN, "", "" },
    { NULL, "infected", 'i', TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN | OPT_CLAMDSCAN, "", "" },
    { NULL, "move", 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_CLAMSCAN | OPT_CLAMDSCAN, "", "" },
    { NULL, "copy", 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_CLAMSCAN | OPT_CLAMDSCAN, "", "" },
    { NULL, "remove", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN | OPT_CLAMDSCAN, "", "" },
    { NULL, "exclude", 0, TYPE_STRING, NULL, -1, NULL, FLAG_MULTIPLE, OPT_CLAMSCAN, "", "" },
    { NULL, "exclude-dir", 0, TYPE_STRING, NULL, -1, NULL, FLAG_MULTIPLE, OPT_CLAMSCAN, "", "" },
    { NULL, "include", 0, TYPE_STRING, NULL, -1, NULL, FLAG_MULTIPLE, OPT_CLAMSCAN, "", "" },
    { NULL, "include-dir", 0, TYPE_STRING, NULL, -1, NULL, FLAG_MULTIPLE, OPT_CLAMSCAN, "", "" },
    { NULL, "structured-ssn-format", 0, TYPE_NUMBER, MATCH_NUMBER, 0, NULL, 0, OPT_CLAMSCAN, "", "" },
    { NULL, "hex-dump", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_SIGTOOL, "", "" },
    { NULL, "md5", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_SIGTOOL, "", "" },
    { NULL, "mdb", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_SIGTOOL, "", "" },
    { NULL, "html-normalise", 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_SIGTOOL, "", "" },
    { NULL, "utf16-decode", 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_SIGTOOL, "", "" },
    { NULL, "build", 'b', TYPE_STRING, NULL, -1, NULL, 0, OPT_SIGTOOL, "", "" },
    { NULL, "no-cdiff", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_SIGTOOL, "", "" },
    { NULL, "server", 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_SIGTOOL, "", "" },
    { NULL, "unpack", 'u', TYPE_STRING, NULL, -1, NULL, 0, OPT_SIGTOOL, "", "" },
    { NULL, "unpack-current", 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_SIGTOOL, "", "" },
    { NULL, "info", 'i', TYPE_STRING, NULL, -1, NULL, 0, OPT_SIGTOOL, "", "" },
    { NULL, "list-sigs", 'l', TYPE_STRING, NULL, -1, DATADIR, 0, OPT_SIGTOOL, "", "" },
    { NULL, "vba", 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_SIGTOOL, "", "" },
    { NULL, "vba-hex", 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_SIGTOOL, "", "" },
    { NULL, "diff", 'd', TYPE_STRING, NULL, -1, NULL, 0, OPT_SIGTOOL, "", "" },
    { NULL, "run-cdiff", 'r', TYPE_STRING, NULL, -1, NULL, 0, OPT_SIGTOOL, "", "" },
    { NULL, "verify-cdiff", 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_SIGTOOL, "", "" },
    { NULL, "defaultcolors", 'd', TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMDTOP, "", "" },

    { NULL, "config-dir", 'c', TYPE_STRING, NULL, 0, CONFDIR, FLAG_REQUIRED, OPT_CLAMCONF, "", "" },
    { NULL, "non-default", 'n', TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMCONF, "", "" },
    { NULL, "generate-config", 'g', TYPE_STRING, NULL, -1, NULL, 0, OPT_CLAMCONF, "", "" },

    /* cmdline only - deprecated */
    { NULL, "http-proxy", 0, TYPE_STRING, NULL, 0, NULL, 0, OPT_FRESHCLAM | OPT_DEPRECATED, "", "" },
    { NULL, "proxy-user", 0, TYPE_STRING, NULL, 0, NULL, 0, OPT_FRESHCLAM | OPT_DEPRECATED, "", "" },
    { NULL, "log-verbose", 0, TYPE_BOOL, NULL, 0, NULL, 0, OPT_FRESHCLAM | OPT_DEPRECATED, "", "" },
    { NULL, "force", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "disable-summary", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN | OPT_CLAMDSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "disable-archive", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "no-archive", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "no-pe", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "no-elf", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "no-ole2", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "no-pdf", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "no-html", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "no-mail", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "no-phishing-sigs", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "no-phishing-scan-urls", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "no-algorithmic", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "no-phishing-restrictedscan", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "max-ratio", 0, TYPE_NUMBER, MATCH_NUMBER, 0, NULL, 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "max-space", 0, TYPE_SIZE, MATCH_SIZE, 0, NULL, 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "block-max", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "unzip", 0, TYPE_STRING, NULL, -1, "foo", 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "unrar", 0, TYPE_STRING, NULL, -1, "foo", 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "arj", 0, TYPE_STRING, NULL, -1, "foo", 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "unzoo", 0, TYPE_STRING, NULL, -1, "foo", 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "lha", 0, TYPE_STRING, NULL, -1, "foo", 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "jar", 0, TYPE_STRING, NULL, -1, "foo", 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "tar", 0, TYPE_STRING, NULL, -1, "foo", 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "tgz", 0, TYPE_STRING, NULL, -1, "foo", 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },
    { NULL, "deb", 0, TYPE_STRING, NULL, -1, "foo", 0, OPT_CLAMSCAN | OPT_DEPRECATED, "", "" },

    /* config file/cmdline options */
    { "LogFile", "log", 'l', TYPE_STRING, NULL, -1, NULL, 0, OPT_CLAMD | OPT_MILTER | OPT_CLAMSCAN | OPT_CLAMDSCAN, "Save all reports to a log file.", "/tmp/clamav.log" },

    { "LogFileUnlock", NULL, 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD | OPT_MILTER, "By default the log file is locked for writing and only a single\ndaemon process can write to it. This option disables the lock.", "yes" },

    { "LogFileMaxSize", NULL, 0, TYPE_SIZE, MATCH_SIZE, 1048576, NULL, 0, OPT_CLAMD | OPT_FRESHCLAM | OPT_MILTER, "Maximum size of the log file.\nValue of 0 disables the limit.", "5M" },

    { "LogTime", NULL, 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD | OPT_FRESHCLAM | OPT_MILTER, "Log time with each message.", "yes" },

    { "LogClean", NULL, 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD, "Log all clean files.\nUseful in debugging but drastically increases the log size.", "yes" },

    { "LogSyslog", NULL, 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD | OPT_FRESHCLAM | OPT_MILTER, "Use the system logger (can work together with LogFile).", "yes" },

    { "LogFacility", NULL, 0, TYPE_STRING, NULL, -1, "LOG_LOCAL6", FLAG_REQUIRED, OPT_CLAMD | OPT_FRESHCLAM | OPT_MILTER, "Type of syslog messages.\nPlease refer to 'man syslog' for the facility names.", "LOG_MAIL" },

    { "LogVerbose", NULL, 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD | OPT_FRESHCLAM | OPT_MILTER, "Enable verbose logging.", "yes" },

    { "PidFile", "pid", 'p', TYPE_STRING, NULL, -1, NULL, 0, OPT_CLAMD | OPT_FRESHCLAM | OPT_MILTER, "Save the process ID to a file.", "/var/run/clam.pid" },

    { "TemporaryDirectory", "tempdir", 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_CLAMD | OPT_MILTER | OPT_CLAMSCAN | OPT_SIGTOOL, "This option allows you to change the default temporary directory.", "/tmp" },

    { "DatabaseDirectory", "datadir", 0, TYPE_STRING, NULL, -1, DATADIR, 0, OPT_CLAMD | OPT_FRESHCLAM, "This option allows you to change the default database directory.\nIf you enable it, please make sure it points to the same directory in\nboth clamd and freshclam.", "/var/lib/clamav" },

    { "LocalSocket", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_CLAMD, "Path to a local socket file the daemon will listen on.", "/tmp/clamd.socket" },

    { "FixStaleSocket", NULL, 0, TYPE_BOOL, MATCH_BOOL, 1, NULL, 0, OPT_CLAMD | OPT_MILTER, "Remove a stale socket after unclean shutdown", "yes" },

    { "TCPSocket", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, -1, NULL, 0, OPT_CLAMD, "A TCP port number the daemon will listen on.", "3310" },

    /* FIXME: add a regex for IP addr */
    { "TCPAddr", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_CLAMD, "By default clamd binds to INADDR_ANY.\nThis option allows you to restrict the TCP address and provide\nsome degree of protection from the outside world.", "127.0.0.1" },

    { "MaxConnectionQueueLength", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, 15, NULL, 0, OPT_CLAMD, "Maximum length the queue of pending connections may grow to.", "30" },

    { "StreamMaxLength", NULL, 0, TYPE_SIZE, MATCH_SIZE, CLI_DEFAULT_MAXFILESIZE, NULL, 0, OPT_CLAMD, "Close the STREAM session when the data size limit is exceeded.\nThe value should match your MTA's limit for the maximum attachment size.", "25M" },

    { "StreamMinPort", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, 1024, NULL, 0, OPT_CLAMD, "The STREAM command uses an FTP-like protocol.\nThis option sets the lower boundary for the port range.", "1024" },

    { "StreamMaxPort", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, 2048, NULL, 0, OPT_CLAMD, "This option sets the upper boundary for the port range.", "2048" },

    { "MaxThreads", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, 10, NULL, 0, OPT_CLAMD | OPT_MILTER, "Maximum number of threads running at the same time.", "20" },

    { "ReadTimeout", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, 120, NULL, 0, OPT_CLAMD, "This option specifies the time (in seconds) after which clamd should\ntimeout if a client doesn't provide any data.", "120" },

    { "CommandReadTimeout", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, 5, NULL, 0, OPT_CLAMD, "This option specifies the time (in seconds) after which clamd should\ntimeout if a client doesn't provide any initial command after connecting.", "5" },

    { "SendBufTimeout", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, 500, NULL, 0, OPT_CLAMD, "This option specifies how long to wait (in miliseconds) if the send buffer is full. Keep this value low to prevent clamd hanging\n", "200"},

    { "ReadTimeout", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, 120, NULL, 0, OPT_MILTER, "Waiting for data from clamd will timeout after this time (seconds).\nValue of 0 disables the timeout.", "300" },

    { "MaxQueue", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, 100, NULL, 0, OPT_CLAMD, "Maximum number of queued items (including those being processed by MaxThreads threads)\nIt is recommended to have this value at least twice MaxThreads if possible.\nWARNING: you shouldn't increase this too much to avoid running out  of file descriptors,\n the following condition should hold:\n MaxThreads*MaxRecursion + MaxQueue - MaxThreads  + 6 < RLIMIT_NOFILE (usual max is 1024)\n", "200" },

    { "IdleTimeout", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, 30, NULL, 0, OPT_CLAMD, "This option specifies how long (in seconds) the process should wait\nfor a new job.", "60" },

    { "ExcludePath", NULL, 0, TYPE_STRING, NULL, -1, NULL, FLAG_MULTIPLE, OPT_CLAMD, "Don't scan files/directories whose names match the provided\nregular expression. This option can be specified multiple times.", "^/proc/\n^/sys/" },

    { "MaxDirectoryRecursion", "max-dir-recursion", 0, TYPE_NUMBER, MATCH_NUMBER, 15, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "Maximum depth the directories are scanned at.", "15" },

    { "FollowDirectorySymlinks", NULL, 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD, "Follow directory symlinks.", "no" },

    { "FollowFileSymlinks", NULL, 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD, "Follow symlinks to regular files.", "no" },

    { "SelfCheck", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, 600, NULL, 0, OPT_CLAMD, "This option specifies the time intervals (in seconds) in which clamd\nshould perform a database check.", "600" },

    { "VirusEvent", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_CLAMD, "Execute a command when a virus is found. In the command string %v will be\nreplaced with the virus name. Additionally, two environment variables will\nbe defined: $CLAM_VIRUSEVENT_FILENAME and $CLAM_VIRUSEVENT_VIRUSNAME.", "/usr/bin/mailx -s \"ClamAV VIRUS ALERT: %v\" alert < /dev/null" },

    { "ExitOnOOM", NULL, 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD, "Stop the daemon when libclamav reports an out of memory condition.", "yes" },

    { "Foreground", NULL, 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD | OPT_FRESHCLAM | OPT_MILTER, "Don't fork into background.", "no" },

    { "Debug", NULL, 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD | OPT_FRESHCLAM, "Enable debug messages in libclamav.", "no" },

    { "LeaveTemporaryFiles", NULL, 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD, "Don't remove temporary files (for debugging purposes).", "no" },

    { "User", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_CLAMD | OPT_MILTER, "Run the daemon as a specified user (the process must be started by root).", "clamav" },

    { "AllowSupplementaryGroups", NULL, 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD | OPT_FRESHCLAM | OPT_MILTER, "Initialize a supplementary group access (the process must be started by root).", "no" },

    /* Scan options */

    { "DetectPUA", "detect-pua", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "Detect Potentially Unwanted Applications.", "yes" },

    { "ExcludePUA", "exclude-pua", 0, TYPE_STRING, NULL, -1, NULL, FLAG_MULTIPLE, OPT_CLAMD | OPT_CLAMSCAN, "Exclude a specific PUA category. This directive can be used multiple times.\nSee http://www.clamav.net/support/pua for the complete list of PUA\ncategories.", "NetTool\nPWTool" },

    { "IncludePUA", "include-pua", 0, TYPE_STRING, NULL, -1, NULL, FLAG_MULTIPLE, OPT_CLAMD | OPT_CLAMSCAN, "Only include a specific PUA category. This directive can be used multiple\ntimes.", "Spy\nScanner\nRAT" },

    { "AlgorithmicDetection", "algorithmic-detection", 0, TYPE_BOOL, MATCH_BOOL, 1, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "In some cases (eg. complex malware, exploits in graphic files, and others),\nClamAV uses special algorithms to provide accurate detection. This option\ncontrols the algorithmic detection.", "yes" },

    { "ScanPE", "scan-pe", 0, TYPE_BOOL, MATCH_BOOL, 1, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "PE stands for Portable Executable - it's an executable file format used\nin all 32- and 64-bit versions of Windows operating systems. This option\nallows ClamAV to perform a deeper analysis of executable files and it's also\nrequired for decompression of popular executable packers such as UPX or FSG.", "yes" },

    { "ScanELF", "scan-elf", 0, TYPE_BOOL, MATCH_BOOL, 1, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "Executable and Linking Format is a standard format for UN*X executables.\nThis option allows you to control the scanning of ELF files.", "yes" },

    { "DetectBrokenExecutables", "detect-broken", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "With this option enabled clamav will try to detect broken executables\n(both PE and ELF) and mark them as Broken.Executable.", "yes" },

    { "ScanMail", "scan-mail", 0, TYPE_BOOL, MATCH_BOOL, 1, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "Enable the built in email scanner.", "yes" },

    { "MailFollowURLs", "mail-follow-urls", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "If an email contains URLs ClamAV can download and scan them.\nWARNING: This option may open your system to a DoS attack. Please don't use\nthis feature on highly loaded servers.", "no" },

    { "ScanPartialMessages", NULL, 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD, "Scan RFC1341 messages split over many emails. You will need to\nperiodically clean up $TemporaryDirectory/clamav-partial directory.\nWARNING: This option may open your system to a DoS attack. Please don't use\nthis feature on highly loaded servers.", "no" },

    { "PhishingSignatures", "phishing-sigs", 0, TYPE_BOOL, MATCH_BOOL, 1, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "With this option enabled ClamAV will try to detect phishing attempts by using\nsignatures.", "yes" },

    { "PhishingScanURLs", "phishing-scan-urls", 0, TYPE_BOOL, MATCH_BOOL, 1, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "Scan URLs found in mails for phishing attempts using heuristics.", "yes" },

    { "PhishingAlwaysBlockCloak", "phishing-cloak", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "Always block cloaked URLs, even if they're not in the database.\nThis feature can lead to false positives.", "no" },

    { "PhishingAlwaysBlockSSLMismatch", "phishing-ssl", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "Always block SSL mismatches in URLs, even if they're not in the database.\nThis feature can lead to false positives.", "" },

    { "HeuristicScanPrecedence", "heuristic-scan-precedence", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "Allow heuristic match to take precedence.\nWhen enabled, if a heuristic scan (such as phishingScan) detects\na possible virus/phish it will stop scan immediately. Recommended, saves CPU\nscan-time.\nWhen disabled, virus/phish detected by heuristic scans will be reported only\nat the end of a scan. If an archive contains both a heuristically detected\nvirus/phish, and a real malware, the real malware will be reported.\nKeep this disabled if you intend to handle \"*.Heuristics.*\" viruses\ndifferently from \"real\" malware.\nIf a non-heuristically-detected virus (signature-based) is found first,\nthe scan is interrupted immediately, regardless of this config option.", "yes" },

    { "StructuredDataDetection", "detect-structured", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "Enable the Data Loss Prevention module.", "no" },

    { "StructuredMinCreditCardCount", "structured-cc-count", 0, TYPE_NUMBER, MATCH_NUMBER, CLI_DEFAULT_MIN_CC_COUNT, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "This option sets the lowest number of Credit Card numbers found in a file\nto generate a detect.", "5" },

    { "StructuredMinSSNCount", "structured-ssn-count", 0, TYPE_NUMBER, MATCH_NUMBER, CLI_DEFAULT_MIN_SSN_COUNT, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "This option sets the lowest number of Social Security Numbers found\nin a file to generate a detect.", "5" },

    { "StructuredSSNFormatNormal", NULL, 0, TYPE_BOOL, MATCH_BOOL, 1, NULL, 0, OPT_CLAMD, "With this option enabled the DLP module will search for valid\nSSNs formatted as xxx-yy-zzzz.", "yes" },

    { "StructuredSSNFormatStripped", NULL, 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD, "With this option enabled the DLP module will search for valid\nSSNs formatted as xxxyyzzzz", "no" },

    { "ScanHTML", "scan-html", 0, TYPE_BOOL, MATCH_BOOL, 1, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "Perform HTML/JavaScript/ScriptEncoder normalisation and decryption.", "yes" },

    { "ScanOLE2", "scan-ole2", 0, TYPE_BOOL, MATCH_BOOL, 1, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "This option enables scanning of OLE2 files, such as Microsoft Office\ndocuments and .msi files.", "yes" },

    { "ScanPDF", "scan-pdf", 0, TYPE_BOOL, MATCH_BOOL, 1, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "This option enables scanning within PDF files.", "yes" },

    { "ScanArchive", "scan-archive", 0, TYPE_BOOL, MATCH_BOOL, 1, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "Scan within archives and compressed files.", "yes" },

    { "ArchiveBlockEncrypted", "block-encrypted", 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "Mark encrypted archives as viruses (Encrypted.Zip, Encrypted.RAR).", "no" },

    { "MaxScanSize", "max-scansize", 0, TYPE_SIZE, MATCH_SIZE, CLI_DEFAULT_MAXSCANSIZE, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "This option sets the maximum amount of data to be scanned for each input file.\nArchives and other containers are recursively extracted and scanned up to this\nvalue.\nThe value of 0 disables the limit.\nWARNING: disabling this limit or setting it too high may result in severe\ndamage.", "100M" },

    { "MaxFileSize", "max-filesize", 0, TYPE_SIZE, MATCH_SIZE, CLI_DEFAULT_MAXFILESIZE, NULL, 0, OPT_CLAMD | OPT_MILTER | OPT_CLAMSCAN, "Files/messages larger than this limit won't be scanned. Affects the input\nfile itself as well as files contained inside it (when the input file is\nan archive, a document or some other kind of container).\nThe value of 0 disables the limit.\nWARNING: disabling this limit or setting it too high may result in severe\ndamage to the system.", "25M" },

    { "MaxRecursion", "max-recursion", 0, TYPE_NUMBER, MATCH_NUMBER, CLI_DEFAULT_MAXRECLEVEL, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "Nested archives are scanned recursively, e.g. if a Zip archive contains a RAR\nfile, all files within it will also be scanned. This option specifies how\ndeeply the process should be continued.\nThe value of 0 disables the limit.\nWARNING: disabling this limit or setting it too high may result in severe\ndamage to the system.", "16" },

    { "MaxFiles", "max-files", 0, TYPE_NUMBER, MATCH_NUMBER, CLI_DEFAULT_MAXFILES, NULL, 0, OPT_CLAMD | OPT_CLAMSCAN, "Number of files to be scanned within an archive, a document, or any other\ncontainer file.\nThe value of 0 disables the limit.\nWARNING: disabling this limit or setting it too high may result in severe\ndamage to the system.", "10000" },

    { "ClamukoScanOnAccess", NULL, 0, TYPE_BOOL, MATCH_BOOL, -1, NULL, 0, OPT_CLAMD, "This option enables Clamuko. Dazuko needs to be already configured and\nrunning.", "no" },

    { "ClamukoScanOnOpen", NULL, 0, TYPE_BOOL, MATCH_BOOL, -1, NULL, 0, OPT_CLAMD, "Scan files when they get opened by the system.", "yes" },

    { "ClamukoScanOnClose", NULL, 0, TYPE_BOOL, MATCH_BOOL, -1, NULL, 0, OPT_CLAMD, "Scan files when they get closed by the system.", "yes" },

    { "ClamukoScanOnExec", NULL, 0, TYPE_BOOL, MATCH_BOOL, -1, NULL, 0, OPT_CLAMD, "Scan files when they get executed by the system.", "yes" },

    { "ClamukoIncludePath", NULL, 0, TYPE_STRING, NULL, -1, NULL, FLAG_MULTIPLE, OPT_CLAMD, "This option specifies a directory (together will all files and directories\ninside this directory) which should be scanned on-access. This option can\nbe used multiple times.", "/home\n/students" },

    { "ClamukoExcludePath", NULL, 0, TYPE_STRING, NULL, -1, NULL, FLAG_MULTIPLE, OPT_CLAMD, "This option allows excluding directories from on-access scanning. It can\nbe used multiple times.", "/home/bofh\n/root" },

    { "ClamukoMaxFileSize", NULL, 0, TYPE_SIZE, MATCH_SIZE, 5242880, NULL, 0, OPT_CLAMD, "Files larger than this value will not be scanned.", "5M" },

    /* FIXME: mark these as private and don't output into clamd.conf/man */
    { "DevACOnly", "dev-ac-only", 0, TYPE_BOOL, MATCH_BOOL, -1, NULL, FLAG_HIDDEN, OPT_CLAMD | OPT_CLAMSCAN, "", "" },

    { "DevACDepth", "dev-ac-depth", 0, TYPE_NUMBER, MATCH_NUMBER, -1, NULL, FLAG_HIDDEN, OPT_CLAMD | OPT_CLAMSCAN, "", "" },

    /* Freshclam-only entries */

    /* FIXME: drop this entry and use LogFile */
    { "UpdateLogFile", "log", 'l', TYPE_STRING, NULL, -1, NULL, 0, OPT_FRESHCLAM, "Save all reports to a log file.", "/var/log/freshclam.log" },

    { "DatabaseOwner", "user", 'u', TYPE_STRING, NULL, -1, CLAMAVUSER, FLAG_REQUIRED, OPT_FRESHCLAM, "When started by root freshclam will drop privileges and switch to the user\ndefined in this option.", CLAMAVUSER },

    { "Checks", "checks", 'c', TYPE_NUMBER, MATCH_NUMBER, 12, NULL, 0, OPT_FRESHCLAM, "This option defined how many times daily freshclam should check for\na database update.", "24" },

    { "DNSDatabaseInfo", NULL, 0, TYPE_STRING, NULL, -1, "current.cvd.clamav.net", FLAG_REQUIRED, OPT_FRESHCLAM, "Use DNS to verify the virus database version. Freshclam uses DNS TXT records\nto verify the versions of the database and software itself. With this\ndirective you can change the database verification domain.\nWARNING: Please don't change it unless you're configuring freshclam to use\nyour own database verification domain.", "current.cvd.clamav.net" },

    { "DatabaseMirror", NULL, 0, TYPE_STRING, NULL, -1, NULL, FLAG_MULTIPLE, OPT_FRESHCLAM, "DatabaseMirror specifies to which mirror(s) freshclam should connect.\nYou should have at least two entries: db.XY.clamav.net and\ndatabase.clamav.net (in this order). Please replace XY with your country\ncode (see http://www.iana.org/cctld/cctld-whois.htm). database.clamav.net\nis a round-robin record which points to our most reliable mirrors. It's used\nas a fall back in case db.XY.clamav.net is not working.", "db.XY.clamav.net\ndatabase.clamav.net" },

    { "MaxAttempts", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, 3, NULL, 0, OPT_FRESHCLAM, "This option defines how many attempts freshclam should make before giving up.", "5" },

    { "ScriptedUpdates", NULL, 0, TYPE_BOOL, MATCH_BOOL, 1, NULL, 0, OPT_FRESHCLAM, "With this option you can control scripted updates. It's highly recommended to keep them enabled.", "yes" },

    { "CompressLocalDatabase", NULL, 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_FRESHCLAM, "By default freshclam will keep the local databases (.cld) uncompressed to\nmake their handling faster. With this option you can enable the compression.\nThe change will take effect with the next database update.", "" },

    { "HTTPProxyServer", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_FRESHCLAM, "If you're behind a proxy, please enter its address here.", "your-proxy" },

    { "HTTPProxyPort", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, -1, NULL, 0, OPT_FRESHCLAM, "HTTP proxy's port", "8080" },

    { "HTTPProxyUsername", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_FRESHCLAM, "A user name for the HTTP proxy authentication.", "username" },

    { "HTTPProxyPassword", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_FRESHCLAM, "A password for the HTTP proxy authentication.", "pass" },

    { "HTTPUserAgent", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_FRESHCLAM, "If your servers are behind a firewall/proxy which does a User-Agent\nfiltering you can use this option to force the use of a different\nUser-Agent header.", "default" },

    { "NotifyClamd", "daemon-notify", 0, TYPE_STRING, NULL, -1, CONFDIR"/clamd.conf", 0, OPT_FRESHCLAM, "Send the RELOAD command to clamd after a successful update.", "yes" },

    { "OnUpdateExecute", "on-update-execute", 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_FRESHCLAM, "Run a command after a successful database update.", "command" },

    { "OnErrorExecute", "on-error-execute", 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_FRESHCLAM, "Run a command when a database update error occurs.", "command" },

    { "OnOutdatedExecute", "on-outdated-execute", 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_FRESHCLAM, "Run a command when freshclam reports an outdated version.\nIn the command string %v will be replaced with the new version number.", "command" },

    /* FIXME: MATCH_IPADDR */
    { "LocalIPAddress", "local-address", 'a', TYPE_STRING, NULL, -1, NULL, 0, OPT_FRESHCLAM, "With this option you can provide a client address for the database downlading.\nUseful for multi-homed systems.", "aaa.bbb.ccc.ddd" },

    { "ConnectTimeout", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, 30, NULL, 0, OPT_FRESHCLAM, "Timeout in seconds when connecting to database server.", "30" },

    { "ReceiveTimeout", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, 30, NULL, 0, OPT_FRESHCLAM, "Timeout in seconds when reading from database server.", "30" },

    { "SubmitDetectionStats", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_FRESHCLAM, "When enabled freshclam will submit statistics to the ClamAV Project about\nthe latest virus detections in your environment. The ClamAV maintainers\nwill then use this data to determine what types of malware are the most\ndetected in the field and in what geographic area they are.\nThis feature requires LogTime and LogFile to be enabled in clamd.conf.", "/path/to/clamd.conf" },

    { "DetectionStatsCountry", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_FRESHCLAM, "Country of origin of malware/detection statistics (for statistical\npurposes only). The statistics collector at ClamAV.net will look up\nyour IP address to determine the geographical origin of the malware\nreported by your installation. If this installation is mainly used to\nscan data which comes from a different location, please enable this\noption and enter a two-letter code (see http://www.iana.org/domains/root/db/)\nof the country of origin.", "country-code" },

    { "SafeBrowsing", NULL, 0, TYPE_BOOL, MATCH_BOOL, 0, NULL, 0, OPT_FRESHCLAM, "This option enables support for Google Safe Browsing. When activated for\nthe first time, freshclam will download a new database file (safebrowsing.cvd)\nwhich will be automatically loaded by clamd and clamscan during the next\nreload, provided that the heuristic phishing detection is turned on. This\ndatabase includes information about websites that may be phishing sites or\npossible sources of malware. When using this option, it's mandatory to run\nfreshclam at least every 30 minutes.\nFreshclam uses the ClamAV's mirror infrastructure to distribute the\ndatabase and its updates but all the contents are provided under Google's\nterms of use. See http://code.google.com/support/bin/answer.py?answer=70015\nand http://safebrowsing.clamav.net for more information.", "yes" },

    /* Deprecated options */

    { "MailMaxRecursion", NULL, 0, TYPE_NUMBER, NULL, -1, NULL, 0, OPT_CLAMD | OPT_DEPRECATED, "", "" },
    { "ArchiveMaxScanSize", NULL, 0, TYPE_SIZE, NULL, -1, NULL, 0, OPT_CLAMD | OPT_DEPRECATED, "", "" },
    { "ArchiveMaxRecursion", NULL, 0, TYPE_NUMBER, NULL, -1, NULL, 0, OPT_CLAMD | OPT_DEPRECATED, "", "" },
    { "ArchiveMaxFiles", NULL, 0, TYPE_NUMBER, NULL, -1, NULL, 0, OPT_CLAMD | OPT_DEPRECATED, "", "" },
    { "ArchiveMaxCompressionRatio", NULL, 0, TYPE_NUMBER, NULL, -1, NULL, 0, OPT_CLAMD | OPT_DEPRECATED, "", "" },
    { "ArchiveBlockMax", NULL, 0, TYPE_BOOL, MATCH_BOOL, -1, NULL, 0, OPT_CLAMD | OPT_DEPRECATED, "", "" },
    { "ArchiveLimitMemoryUsage", NULL, 0, TYPE_BOOL, MATCH_BOOL, -1, NULL, 0, OPT_CLAMD | OPT_DEPRECATED, "", "" },

    /* Milter specific options */

    { "ClamdSocket", NULL, 0, TYPE_STRING, NULL, -1, NULL, FLAG_MULTIPLE, OPT_MILTER, "Define the clamd socket to connect to for scanning.\nThis option is mandatory! Syntax:\n  ClamdSocket unix:path\n  ClamdSocket tcp:host:port\nThe first syntax specifies a local unix socket (needs an absolute path) e.g.:\n  ClamdSocket unix:/var/run/clamd/clamd.socket\nThe second syntax specifies a tcp local or remote tcp socket: the\nhost can be a hostname or an ip address; the \":port\" field is only required\nfor IPv6 addresses, otherwise it defaults to 3310\n  ClamdSocket tcp:192.168.0.1\nThis option can be repeated several times with different sockets or even\nwith the same socket: clamd servers will be selected in a round-robin fashion.", "tcp:scanner.mydomain:7357" },

    { "MilterSocket",NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_MILTER, "Define the interface through which we communicate with sendmail.\nThis option is mandatory! Possible formats are:\n[[unix|local]:]/path/to/file - to specify a unix domain socket;\ninet:port@[hostname|ip-address] - to specify an ipv4 socket;\ninet6:port@[hostname|ip-address] - to specify an ipv6 socket.", "/tmp/clamav-milter.socket\ninet:7357" },

    { "LocalNet", NULL, 0, TYPE_STRING, NULL, -1, NULL, FLAG_MULTIPLE, OPT_MILTER, "Messages originating from these hosts/networks will not be scanned\nThis option takes a host(name)/mask pair in CIRD notation and can be\nrepeated several times. If \"/mask\" is omitted, a host is assumed.\nTo specify a locally orignated, non-smtp, email use the keyword \"local\".", "local\n192.168.0.0/24\n1111:2222:3333::/48" },

    { "OnClean", NULL, 0, TYPE_STRING, "^(Accept|Reject|Defer|Blackhole|Quarantine)$", -1, "Accept", 0, OPT_MILTER, "Action to be performed on clean messages (mostly useful for testing).\nThe following actions are available:\nAccept: the message is accepted for delievery\nReject: immediately refuse delievery (a 5xx error is returned to the peer)\nDefer: return a temporary failure message (4xx) to the peer\nBlackhole: like Accept but the message is sent to oblivion\nQuarantine: like Accept but message is quarantined instead of being delivered", "Accept" },

    { "OnInfected", NULL, 0, TYPE_STRING, "^(Accept|Reject|Defer|Blackhole|Quarantine)$", -1, "Quarantine", 0, OPT_MILTER, "Action to be performed on clean messages (mostly useful for testing).\nThe following actions are available:\nAccept: the message is accepted for delievery\nReject: immediately refuse delievery (a 5xx error is returned to the peer)\nDefer: return a temporary failure message (4xx) to the peer\nBlackhole: like Accept but the message is sent to oblivion\nQuarantine: like Accept but message is quarantined instead of being delivered", "Quarantine" },

    { "OnFail", NULL, 0, TYPE_STRING, "^(Accept|Reject|Defer)$", -1, "Defer", 0, OPT_MILTER, "Action to be performed on error conditions (this includes failure to\nallocate data structures, no scanners available, network timeouts, unknown\nscanner replies and the like.\nThe following actions are available:\nAccept: the message is accepted for delievery;\nReject: immediately refuse delievery (a 5xx error is returned to the peer);\nDefer: return a temporary failure message (4xx) to the peer.", "Defer" },

    { "RejectMsg", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_MILTER, "This option allows to set a specific rejection reason for infected messages\nand it's therefore only useful together with \"OnInfected Reject\"\nThe string \"%v\", if present, will be replaced with the virus name.", "MTA specific" },

    { "AddHeader", NULL, 0, TYPE_STRING, "^(No|Replace|Yes|Add)$", -1, "no", 0, OPT_MILTER, "If this option is set to \"Replace\" (or \"Yes\"), an \"X-Virus-Scanned\" and an\n\"X-Virus-Status\" headers will be attached to each processed message, possibly\nreplacing existing headers.\nIf it is set to Add, the X-Virus headers are added possibly on top of the\nexisting ones.\nNote that while \"Replace\" can potentially break DKIM signatures, \"Add\" may\nconfuse procmail and similar filters.", "Replace" },

    { "Chroot", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_MILTER, "Chroot to the specified directory.\nChrooting is performed just after reading the config file and before\ndropping privileges.", "/newroot" },

    { "Whitelist", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_MILTER, "This option specifies a file which contains a list of basic POSIX regular\nexpressions. Addresses (sent to or from - see below) matching these regexes\nwill not be scanned.  Optionally each line can start with the string \"From:\"\nor \"To:\" (note: no whitespace after the colon) indicating if it is,\nrespectively, the sender or recipient that is to be whitelisted.\nIf the field is missing, \"To:\" is assumed.\nLines starting with #, : or ! are ignored.", "/etc/whitelisted_addresses" },

    { "SkipAuthenticated", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_MILTER, "Messages from authenticated SMTP users matching this extended POSIX\nregular expression (egrep-like) will not be scanned.\nNote: this is the AUTH login name!", "SkipAuthenticated ^(tom|dick|henry)$" },

    { "LogInfected", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_MILTER, "This option allows to tune what is logged when a message is infected.\nPossible values are Off (the default - nothing is logged),\nBasic (minimal info logged), Full (verbose info logged)\nNote:\nFor this to work properly in sendmail, make sure the msg_id, mail_addr,\nrcpt_addr and i macroes are available in eom. In other words add a line like:\nMilter.macros.eom={msg_id}, {mail_addr}, {rcpt_addr}, i\nto your .cf file. Alternatively use the macro:\ndefine(`confMILTER_MACROS_EOM', `{msg_id}, {mail_addr}, {rcpt_addr}, i')\nPostfix should be working fine with the default settings.", "Basic" },

    /* Deprecated milter options */

    { "ArchiveBlockEncrypted", NULL, 0, TYPE_BOOL, MATCH_BOOL, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "DatabaseDirectory", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "Debug", NULL, 0, TYPE_BOOL, MATCH_BOOL, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "DetectBrokenExecutables", NULL, 0, TYPE_BOOL, MATCH_BOOL, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "LeaveTemporaryFiles", NULL, 0, TYPE_BOOL, MATCH_BOOL, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "LocalSocket", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "MailFollowURLs", NULL, 0, TYPE_BOOL, MATCH_BOOL, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "MaxScanSize", NULL, 0, TYPE_SIZE, MATCH_SIZE, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "MaxFiles", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "MaxRecursion", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "PhishingSignatures", NULL, 0, TYPE_BOOL, MATCH_BOOL, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "ScanArchive", NULL, 0, TYPE_BOOL, MATCH_BOOL, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "ScanHTML", NULL, 0, TYPE_BOOL, MATCH_BOOL, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "ScanMail", NULL, 0, TYPE_BOOL, MATCH_BOOL, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "ScanOLE2", NULL, 0, TYPE_BOOL, MATCH_BOOL, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "ScanPE", NULL, 0, TYPE_BOOL, MATCH_BOOL, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "StreamMaxLength", NULL, 0, TYPE_SIZE, MATCH_SIZE, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "TCPAddr", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "TCPSocket", NULL, 0, TYPE_NUMBER, MATCH_NUMBER, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },
    { "TemporaryDirectory", NULL, 0, TYPE_STRING, NULL, -1, NULL, 0, OPT_MILTER | OPT_DEPRECATED, "", "" },

    { NULL, NULL, 0, 0, NULL, 0, NULL, 0, 0, NULL, NULL }
};

const struct optstruct *optget(const struct optstruct *opts, const char *name)
{
    while(opts) {
	if((opts->name && !strcmp(opts->name, name)) || (opts->cmd && !strcmp(opts->cmd, name)))
	    return opts;
	opts = opts->next;
    }
    return NULL;
}

static struct optstruct *optget_i(struct optstruct *opts, const char *name)
{
    while(opts) {
	if((opts->name && !strcmp(opts->name, name)) || (opts->cmd && !strcmp(opts->cmd, name)))
	    return opts;
	opts = opts->next;
    }
    return NULL;
}

/*
static void optprint(const struct optstruct *opts)
{
	const struct optstruct *h;

    printf("\nOPTIONS:\n\n");

    while(opts) {
	printf("OPT_NAME: %s\n", opts->name);
	printf("OPT_CMD: %s\n", opts->cmd);
	printf("OPT_STRARG: %s\n", opts->strarg ? opts->strarg : "NONE");
	printf("OPT_NUMARG: %d\n", opts->numarg);
	h = opts;
	while((h = h->nextarg)) {
	    printf("SUBARG_OPT_STRARG: %s\n", h->strarg ? h->strarg : "NONE");
	    printf("SUBARG_OPT_NUMARG: %d\n", h->numarg);
	}
	printf("----------------\n");
	opts = opts->next;
    }
}
*/

static int optadd(struct optstruct **opts, struct optstruct **opts_last, const char *name, const char *cmd, const char *strarg, long long numarg, int flags, int idx)
{
	struct optstruct *newnode;


    newnode = (struct optstruct *) malloc(sizeof(struct optstruct));

    if(!newnode)
	return -1;

    if(name) {
	newnode->name = strdup(name);
	if(!newnode->name) {
	    free(newnode);
	    return -1;
	}
    } else {
	newnode->name = NULL;
    }

    if(cmd) {
	newnode->cmd = strdup(cmd);
	if(!newnode->cmd) {
	    free(newnode->name);
	    free(newnode);
	    return -1;
	}
    } else {
	newnode->cmd = NULL;
    }

    if(strarg) {
	newnode->strarg = strdup(strarg);
	if(!newnode->strarg) {
	    free(newnode->cmd);
	    free(newnode->name);
	    free(newnode);
	    return -1;
	}
	newnode->enabled = 1;
    } else {
	newnode->strarg = NULL;
	newnode->enabled = 0;
    }
    newnode->numarg = numarg;
    if(numarg && numarg != -1)
	newnode->enabled = 1;
    newnode->nextarg = NULL;
    newnode->next = NULL;
    newnode->active = 0;
    newnode->flags = flags;
    newnode->idx = idx;
    newnode->filename = NULL;

    if(!*opts_last) {
	newnode->next = *opts;
	*opts = newnode;
	*opts_last = *opts;
    } else {
	(*opts_last)->next = newnode;
	*opts_last = newnode;
    }
    return 0;
}

static int optaddarg(struct optstruct *opts, const char *name, const char *strarg, long long numarg)
{
	struct optstruct *pt, *h, *new;


    if(!(pt = optget_i(opts, name))) {
	fprintf(stderr, "ERROR: optaddarg: Unregistered option %s\n", name);
	return -1;
    }

    if(pt->flags & FLAG_MULTIPLE) {
	if(!pt->active) {
	    if(strarg) {
		free(pt->strarg);
		pt->strarg = strdup(strarg);
	 	if(!pt->strarg) {
		    fprintf(stderr, "ERROR: optaddarg: strdup() failed\n");
		    return -1;
		}
	    }
	    pt->numarg = numarg;
	} else {
	    new = (struct optstruct *) calloc(1, sizeof(struct optstruct));
	    if(!new) {
		fprintf(stderr, "ERROR: optaddarg: malloc() failed\n");
		return -1;
	    }
	    if(strarg) {
		new->strarg = strdup(strarg);
	 	if(!new->strarg) {
		    fprintf(stderr, "ERROR: optaddarg: strdup() failed\n");
		    free(new);
		    return -1;
		}
	    }
	    new->numarg = numarg;
	    h = pt;
	    while(h->nextarg)
		h = h->nextarg;
	    h->nextarg = new;
	}
    } else {
	if(pt->active)
	    return 0;

	if(strarg) {
	    free(pt->strarg);
	    pt->strarg = strdup(strarg);
	    if(!pt->strarg) {
		fprintf(stderr, "ERROR: optaddarg: strdup() failed\n");
		return -1;
	    }
	}
	pt->numarg = numarg;
    }

    pt->active = 1;
    if(pt->strarg || (pt->numarg && pt->numarg != -1))
	pt->enabled = 1;
    else
	pt->enabled = 0;

    return 0;
}

void optfree(struct optstruct *opts)
{
    	struct optstruct *h, *a;
	int i;

    if(opts && opts->filename) {
	for(i = 0; opts->filename[i]; i++)
	    free(opts->filename[i]);
	free(opts->filename);
    }

    while(opts) {
	a = opts->nextarg;
	while(a) {
	    if(a->strarg) {
		free(a->name);
		free(a->cmd);
		free(a->strarg);
		h = a;
		a = a->nextarg;
		free(h);
	    } else {
		a = a->nextarg;
	    }
	}
	free(opts->name);
	free(opts->cmd);
	free(opts->strarg);
	h = opts;
	opts = opts->next;
	free(h);
    }
    return;
}

struct optstruct *optparse(const char *cfgfile, int argc, char **argv, int verbose, int toolmask, int ignore, struct optstruct *oldopts)
{
	FILE *fs = NULL;
	const struct clam_option *optentry;
	char *pt;
	const char *name = NULL, *arg;
	int i, err = 0, lc = 0, sc = 0, opt_index, line = 0, ret;
	struct optstruct *opts = NULL, *opts_last = NULL, *opt;
	char buffer[512], *buff;
	struct option longopts[MAXCMDOPTS];
	char shortopts[MAXCMDOPTS];
	regex_t regex;
	long long numarg, lnumarg;
	int regflags = REG_EXTENDED | REG_NOSUB;


    if(oldopts)
	opts = oldopts;

    shortopts[sc++] = ':';
    for(i = 0; ; i++) {
	optentry = &clam_options[i];
	if(!optentry->name && !optentry->longopt)
	    break;

	if(((optentry->owner & toolmask) && ((optentry->owner & toolmask) != OPT_DEPRECATED)) || (ignore && (optentry->owner & ignore))) {
	    if(!oldopts && optadd(&opts, &opts_last, optentry->name, optentry->longopt, optentry->strarg, optentry->numarg, optentry->flags, i) < 0) {
		fprintf(stderr, "ERROR: optparse: Can't register new option (not enough memory)\n");
		optfree(opts);
		return NULL;
	    }

	    if(!cfgfile) {
		if(optentry->longopt) {
		    if(lc >= MAXCMDOPTS) {
			fprintf(stderr, "ERROR: optparse: longopts[] is too small\n");
			optfree(opts);
			return NULL;
		    }
		    longopts[lc].name = optentry->longopt;
		    if(!(optentry->flags & FLAG_REQUIRED) && (optentry->argtype == TYPE_BOOL || optentry->strarg))
			longopts[lc].has_arg = 2;
		    else
			longopts[lc].has_arg = 1;
		    longopts[lc].flag = NULL;
		    longopts[lc++].val = optentry->shortopt;
		}
		if(optentry->shortopt) {
		    if(sc + 2 >= MAXCMDOPTS) {
			fprintf(stderr, "ERROR: optparse: shortopts[] is too small\n");
			optfree(opts);
			return NULL;
		    }
		    shortopts[sc++] = optentry->shortopt;
		    if(optentry->argtype != TYPE_BOOL) {
			shortopts[sc++] = ':';
			if(!(optentry->flags & FLAG_REQUIRED) && optentry->strarg)
			    shortopts[sc++] = ':';
		    }
		}
	    }
	}
    }

    if(cfgfile) {
	if((fs = fopen(cfgfile, "rb")) == NULL) {
	    /* don't print error messages here! */
	    optfree(opts);
	    return NULL;
	}
    } else {
	if(MAX(sc, lc) > MAXCMDOPTS) {
	    fprintf(stderr, "ERROR: optparse: (short|long)opts[] is too small\n");
	    optfree(opts);
	    return NULL;
	}
	shortopts[sc] = 0;
	longopts[lc].name = NULL;
	longopts[lc].flag = NULL;
	longopts[lc].has_arg = longopts[lc].val = 0;
    }

    while(1) {

	if(cfgfile) {
	    if(!fgets(buffer, sizeof(buffer), fs))
		break;

	    buff = buffer;
	    for(i = 0; i < (int) strlen(buff) - 1 && (buff[i] == ' ' || buff[i] == '\t'); i++);
	    buff += i;
	    line++;
	    if(strlen(buff) <= 2 || buff[0] == '#')
		continue;

	    if(!strncmp("Example", buff, 7)) {
		if(verbose)
		    fprintf(stderr, "ERROR: Please edit the example config file %s\n", cfgfile);
		err = 1;
		break;
	    }

	    if(!(pt = strpbrk(buff, " \t"))) {
		if(verbose)
		    fprintf(stderr, "ERROR: Missing argument for option at line %d\n", line);
		err = 1;
		break;
	    }
	    name = buff;
	    *pt++ = 0;
	    for(i = 0; i < (int) strlen(pt) - 1 && (pt[i] == ' ' || pt[i] == '\t'); i++);
	    pt += i;
	    for(i = strlen(pt); i >= 1 && (pt[i - 1] == ' ' || pt[i - 1] == '\t' || pt[i - 1] == '\n'); i--);
	    if(!i) {
		if(verbose)
		    fprintf(stderr, "ERROR: Missing argument for option at line %d\n", line);
		err = 1;
		break;
	    }
	    pt[i] = 0;
	    arg = pt;
	    if(*arg == '"') {
		arg++; pt++;
		pt = strrchr(pt, '"');
		if(!pt) {
		    if(verbose)
			fprintf(stderr, "ERROR: Missing closing parenthesis in option %s at line %d\n", name, line);
		    err = 1;
		    break;
		}
		*pt = 0;
		if(!strlen(arg)) {
		    if(verbose)
			fprintf(stderr, "ERROR: Empty argument for option %s at line %d\n", name, line);
		    err = 1;
		    break;
		}
	    }

	} else {
	    opt_index = 0;
	    ret = my_getopt_long(argc, argv, shortopts, longopts, &opt_index);
	    if(ret == -1)
		break;

	    if(ret == ':') {
		fprintf(stderr, "ERROR: Incomplete option passed (missing argument)\n");
		err = 1;
		break;
	    } else if(!ret || strchr(shortopts, ret)) {
		name = NULL;
		if(ret) {
		    for(i = 0; i < lc; i++) {
			if(ret == longopts[i].val) {
			    name = longopts[i].name;
			    break;
			}
		    }
		} else {
		    name = longopts[opt_index].name;
		}
		if(!name) {
		    fprintf(stderr, "ERROR: optparse: No corresponding long name for option '-%c'\n", (char) ret);
		    err = 1;
		    break;
		}
		optarg ? (arg = optarg) : (arg = NULL);
	    } else {
		fprintf(stderr, "ERROR: Unknown option passed\n");
		err = 1;
		break;
	    }
	}

	if(!name) {
	    fprintf(stderr, "ERROR: Problem parsing options (name == NULL)\n");
	    err = 1;
	    break;
	}

	opt = optget_i(opts, name);
	if(!opt) {
	    if(cfgfile) {
		if(verbose)
		    fprintf(stderr, "ERROR: Parse error at line %d: Unknown option %s\n", line, name);
	    }
	    err = 1;
	    break;
	}
	optentry = &clam_options[opt->idx];

	if(ignore && (optentry->owner & ignore) && !(optentry->owner & toolmask)) {
	    if(cfgfile) {
		if(verbose)
		    fprintf(stderr, "WARNING: Ignoring unsupported option %s at line %u\n", opt->name, line);
	    } else {
		if(verbose) {
		    if(optentry->shortopt)
			fprintf(stderr, "WARNING: Ignoring unsupported option --%s (-%c)\n", optentry->longopt, optentry->shortopt);
		    else
			fprintf(stderr, "WARNING: Ignoring unsupported option --%s\n", optentry->longopt);
		}
	    }
	    continue;
	}

	if(optentry->owner & OPT_DEPRECATED) {
	    if(toolmask & OPT_DEPRECATED) {
		if(optaddarg(opts, name, "foo", 1) < 0) {
		    if(cfgfile)
			fprintf(stderr, "ERROR: Can't register argument for option %s\n", name);
		    else
			fprintf(stderr, "ERROR: Can't register argument for option --%s\n", optentry->longopt);
		    err = 1;
		    break;
		}
	    } else {
		if(cfgfile) {
		    if(verbose)
			fprintf(stderr, "WARNING: Ignoring deprecated option %s at line %u\n", opt->name, line);
		} else {
		    if(verbose) {
			if(optentry->shortopt)
			    fprintf(stderr, "WARNING: Ignoring deprecated option --%s (-%c)\n", optentry->longopt, optentry->shortopt);
			else
			    fprintf(stderr, "WARNING: Ignoring deprecated option --%s\n", optentry->longopt);
		    }
		}
	    }
	    continue;
	}

	if(!cfgfile && !arg && optentry->argtype == TYPE_BOOL) {
	    arg = "yes"; /* default to yes */
	} else if(optentry->regex) {
	    if(!(optentry->flags & FLAG_REG_CASE))
		regflags |= REG_ICASE;

	    if(cli_regcomp(&regex, optentry->regex, regflags)) {
		fprintf(stderr, "ERROR: optparse: Can't compile regular expression %s for option %s\n", optentry->regex, name);
		err = 1;
		break;
	    }
	    ret = cli_regexec(&regex, arg, 0, NULL, 0);
	    cli_regfree(&regex);
	    if(ret == REG_NOMATCH) {
		if(cfgfile) {
		    fprintf(stderr, "ERROR: Incorrect argument format for option %s\n", name);
		} else {
		    if(optentry->shortopt)
			fprintf(stderr, "ERROR: Incorrect argument format for option --%s (-%c)\n", optentry->longopt, optentry->shortopt);
		    else
			fprintf(stderr, "ERROR: Incorrect argument format for option --%s\n", optentry->longopt);
		}
		err = 1;
		break;
	    }
	}

	numarg = -1;
	switch(optentry->argtype) {
	    case TYPE_STRING:
		if(!arg)
		    arg = optentry->strarg;
		if(!cfgfile && !strlen(arg)) {
		    if(optentry->shortopt)
			fprintf(stderr, "ERROR: Option --%s (-%c) requires a non-empty string argument\n", optentry->longopt, optentry->shortopt);
		    else
			fprintf(stderr, "ERROR: Option --%s requires a non-empty string argument\n", optentry->longopt);
		    err = 1;
		    break;
		}
		break;

	    case TYPE_NUMBER:
		numarg = atoi(arg);
		arg = NULL;
		break;

	    case TYPE_SIZE:
		errno = 0;
		lnumarg = strtoul(arg, &buff, 0);
		if(errno != ERANGE) {
		    switch(*buff) {
		    case 'M':
		    case 'm':
			if(lnumarg <= UINT_MAX/(1024*1024)) lnumarg *= 1024*1024;
			else errno = ERANGE;
			break;
		    case 'K':
		    case 'k':
			if(lnumarg <= UINT_MAX/1024) lnumarg *= 1024;
			else errno = ERANGE;
			break;
		    case '\0':
			break;
		    default:
			if(cfgfile) {
			    fprintf(stderr, "ERROR: Can't parse numerical argument for option %s\n", name);
			} else {
			    if(optentry->shortopt)
				fprintf(stderr, "ERROR: Can't parse numerical argument for option --%s (-%c)\n", optentry->longopt, optentry->shortopt);
			    else
				fprintf(stderr, "ERROR: Can't parse numerical argument for option --%s\n", optentry->longopt);
			}
			err = 1;
		    }
		}

		arg = NULL;
		if(err) break;

		if(sizeof(lnumarg) > sizeof(numarg) && (lnumarg >> (sizeof(numarg)<<3)) )
		    errno = ERANGE;

		if(errno == ERANGE) {
		    if(cfgfile) {
			fprintf(stderr, "WARNING: Numerical value for option %s too high, resetting to 4G\n", name);
		    } else {
			if(optentry->shortopt)
			    fprintf(stderr, "WARNING: Numerical value for option --%s (-%c) too high, resetting to 4G\n", optentry->longopt, optentry->shortopt);
			else
			    fprintf(stderr, "WARNING: Numerical value for option %s too high, resetting to 4G\n", optentry->longopt);
		    }
		    lnumarg = UINT_MAX;
		}

		numarg = lnumarg;
		break;

	    case TYPE_BOOL:
                if(!strcasecmp(arg, "yes") || !strcmp(arg, "1") || !strcasecmp(arg, "true"))
		    numarg = 1;
		else
		    numarg = 0;

		arg = NULL;
		break;
	}

	if(err)
	    break;

	if(optaddarg(opts, name, arg, numarg) < 0) {
	    if(cfgfile)
		fprintf(stderr, "ERROR: Can't register argument for option %s\n", name);
	    else
		fprintf(stderr, "ERROR: Can't register argument for option --%s\n", optentry->longopt);
	    err = 1;
	    break;
	}
    }

    if(fs)
	fclose(fs);

    if(err) {
	optfree(opts);
	return NULL;
    }

    if(!cfgfile && (optind < argc)) {
	opts->filename = (char **) calloc(argc - optind + 1, sizeof(char *));
	if(!opts->filename) {
	    fprintf(stderr, "ERROR: optparse: calloc failed\n");
	    optfree(opts);
	    return NULL;
	}
        for(i = optind; i < argc; i++) {
	    opts->filename[i - optind] = strdup(argv[i]);
	    if(!opts->filename[i - optind]) {
		fprintf(stderr, "ERROR: optparse: strdup failed\n");
		optfree(opts);
		return NULL;
	    }
	}
    }

    /* optprint(opts); */

    return opts;
}
