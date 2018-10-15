/*
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995 Rick Sladkey <jrs@world.std.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	$Id$
 */

	{ 0,	0,	sys_restart_syscall,	"restart_syscall" }, /* 0 */
	{ 1,	TP,	sys_exit,		"_exit", SYS_exit }, /* 1 */
	{ 0,	TP,	sys_fork,		"fork", SYS_fork }, /* 2 */
	{ 3,	0,	sys_read,		"read", SYS_read }, /* 3 */
	{ 3,	0,	sys_write,		"write", SYS_write }, /* 4 */
	{ 3,	TF,	sys_open,		"open"		}, /* 5 */
	{ 1,	0,	sys_close,		"close"		}, /* 6 */
	{ 3,	TP,	sys_waitpid,		"waitpid", SYS_wait4 }, /* 7 */
	{ 2,	TF,	sys_creat,		"creat"		}, /* 8 */
	{ 2,	TF,	sys_link,		"link"		}, /* 9 */
	{ 1,	TF,	sys_unlink,		"unlink"	}, /* 10 */
	{ 3,	TF|TP,	sys_execve,		"execve", SYS_execve }, /* 11 */
	{ 1,	TF,	sys_chdir,		"chdir"		}, /* 12 */
	{ 1,	0,	sys_time,		"time"		}, /* 13 */
	{ 3,	TF,	sys_mknod,		"mknod"		}, /* 14 */
	{ 2,	TF,	sys_chmod,		"chmod"		}, /* 15 */
	{ 3,	TF,	sys_chown,		"chown"		}, /* 16 */
	{ 0,	0,	sys_break,		"break"		}, /* 17 */
	{ 2,	TF,	sys_oldstat,		"oldstat"	}, /* 18 */
	{ 3,	0,	sys_lseek,		"lseek"		}, /* 19 */
	{ 0,	0,	sys_getpid,		"getpid"	}, /* 20 */
	{ 5,	TF,	sys_mount,		"mount"		}, /* 21 */
	{ 1,	TF,	sys_umount,		"oldumount"	}, /* 22 */
	{ 1,	0,	sys_setuid,		"setuid"	}, /* 23 */
	{ 0,	0,	sys_getuid,		"getuid"	}, /* 24 */
	{ 1,	0,	sys_stime,		"stime"		}, /* 25 */
	{ 4,	0,	sys_ptrace,		"ptrace"	}, /* 26 */
	{ 1,	0,	sys_alarm,		"alarm"		}, /* 27 */
	{ 2,	0,	sys_oldfstat,		"oldfstat"	}, /* 28 */
	{ 0,	TS,	sys_pause,		"pause"		}, /* 29 */
	{ 2,	TF,	sys_utime,		"utime"		}, /* 30 */
	{ 2,	0,	sys_stty,		"stty"		}, /* 31 */
	{ 2,	0,	sys_gtty,		"gtty"		}, /* 32 */
	{ 2,	TF,	sys_access,		"access"	}, /* 33 */
	{ 1,	0,	sys_nice,		"nice"		}, /* 34 */
	{ 0,	0,	sys_ftime,		"ftime"		}, /* 35 */
	{ 0,	0,	sys_sync,		"sync"		}, /* 36 */
	{ 2,	TS,	sys_kill,		"kill"		}, /* 37 */
	{ 2,	TF,	sys_rename,		"rename"	}, /* 38 */
	{ 2,	TF,	sys_mkdir,		"mkdir"		}, /* 39 */
	{ 1,	TF,	sys_rmdir,		"rmdir"		}, /* 40 */
	{ 1,	0,	sys_dup,		"dup"		}, /* 41 */
	{ 1,	0,	sys_pipe,		"pipe"		}, /* 42 */
	{ 1,	0,	sys_times,		"times"		}, /* 43 */
	{ 0,	0,	sys_prof,		"prof"		}, /* 44 */
	{ 1,	0,	sys_brk,		"brk"		}, /* 45 */
	{ 1,	0,	sys_setgid,		"setgid"	}, /* 46 */
	{ 0,	0,	sys_getgid,		"getgid"	}, /* 47 */
	{ 3,	TS,	sys_signal,		"signal"	}, /* 48 */
	{ 0,	0,	sys_geteuid,		"geteuid"	}, /* 49 */
	{ 0,	0,	sys_getegid,		"getegid"	}, /* 50 */
	{ 1,	TF,	sys_acct,		"acct"		}, /* 51 */
	{ 2,	TF,	sys_umount2,		"umount"	}, /* 52 */
	{ 0,	0,	sys_lock,		"lock"		}, /* 53 */
	{ 3,	0,	sys_ioctl,		"ioctl"		}, /* 54 */
	{ 3,	0,	sys_fcntl,		"fcntl"		}, /* 55 */
	{ 0,	0,	sys_mpx,		"mpx"		}, /* 56 */
	{ 2,	0,	sys_setpgid,		"setpgid"	}, /* 57 */
	{ 2,	0,	sys_ulimit,		"ulimit"	}, /* 58 */
	{ 1,	0,	sys_oldolduname,	"oldolduname"	}, /* 59 */
	{ 1,	0,	sys_umask,		"umask"		}, /* 60 */
	{ 1,	TF,	sys_chroot,		"chroot"	}, /* 61 */
	{ 2,	0,	sys_ustat,		"ustat"		}, /* 62 */
	{ 2,	0,	sys_dup2,		"dup2"		}, /* 63 */
	{ 0,	0,	sys_getppid,		"getppid"	}, /* 64 */
	{ 0,	0,	sys_getpgrp,		"getpgrp"	}, /* 65 */
	{ 0,	0,	sys_setsid,		"setsid"	}, /* 66 */
	{ 3,	TS,	sys_sigaction,		"sigaction"	}, /* 67 */
	{ 0,	TS,	sys_siggetmask,		"siggetmask"	}, /* 68 */
	{ 1,	TS,	sys_sigsetmask,		"sigsetmask"	}, /* 69 */
	{ 2,	0,	sys_setreuid,		"setreuid"	}, /* 70 */
	{ 2,	0,	sys_setregid,		"setregid"	}, /* 71 */
	{ 3,	TS,	sys_sigsuspend,		"sigsuspend"	}, /* 72 */
	{ 1,	TS,	sys_sigpending,		"sigpending"	}, /* 73 */
	{ 2,	0,	sys_sethostname,	"sethostname"	}, /* 74 */
	{ 2,	0,	sys_setrlimit,		"setrlimit"	}, /* 75 */
	{ 2,	0,	sys_getrlimit,		"old_getrlimit"	}, /* 76 */
	{ 2,	0,	sys_getrusage,		"getrusage"	}, /* 77 */
	{ 2,	0,	sys_gettimeofday,	"gettimeofday"	}, /* 78 */
	{ 2,	0,	sys_settimeofday,	"settimeofday"	}, /* 79 */
	{ 2,	0,	sys_getgroups,		"getgroups"	}, /* 80 */
	{ 2,	0,	sys_setgroups,		"setgroups"	}, /* 81 */
	{ 1,	0,	sys_oldselect,		"oldselect"	}, /* 82 */
	{ 2,	TF,	sys_symlink,		"symlink"	}, /* 83 */
	{ 2,	TF,	sys_oldlstat,		"oldlstat"	}, /* 84 */
	{ 3,	TF,	sys_readlink,		"readlink"	}, /* 85 */
	{ 1,	TF,	sys_uselib,		"uselib"	}, /* 86 */
	{ 1,	TF,	sys_swapon,		"swapon"	}, /* 87 */
	{ 3,	0,	sys_reboot,		"reboot"	}, /* 88 */
	{ 3,	0,	sys_readdir,		"readdir"	}, /* 89 */
	{ 6,	0,	sys_old_mmap,		"old_mmap"	}, /* 90 */
	{ 2,	0,	sys_munmap,		"munmap"	}, /* 91 */
	{ 2,	TF,	sys_truncate,		"truncate"	}, /* 92 */
	{ 2,	0,	sys_ftruncate,		"ftruncate"	}, /* 93 */
	{ 2,	0,	sys_fchmod,		"fchmod"	}, /* 94 */
	{ 3,	0,	sys_fchown,		"fchown"	}, /* 95 */
	{ 2,	0,	sys_getpriority,	"getpriority"	}, /* 96 */
	{ 3,	0,	sys_setpriority,	"setpriority"	}, /* 97 */
	{ 4,	0,	sys_profil,		"profil"	}, /* 98 */
	{ 2,	TF,	sys_statfs,		"statfs"	}, /* 99 */
	{ 2,	0,	sys_fstatfs,		"fstatfs"	}, /* 100 */
	{ 3,	0,	sys_ioperm,		"ioperm"	}, /* 101 */
	{ 2,	0,	sys_socketcall,		"socketcall", SYS_socketcall }, /* 102 */
	{ 3,	0,	sys_syslog,		"syslog"	}, /* 103 */
	{ 3,	0,	sys_setitimer,		"setitimer"	}, /* 104 */
	{ 2,	0,	sys_getitimer,		"getitimer"	}, /* 105 */
	{ 2,	TF,	sys_stat,		"stat"		}, /* 106 */
	{ 2,	TF,	sys_lstat,		"lstat"		}, /* 107 */
	{ 2,	0,	sys_fstat,		"fstat"		}, /* 108 */
	{ 1,	0,	sys_olduname,		"olduname"	}, /* 109 */
	{ 1,	0,	sys_iopl,		"iopl"		}, /* 110 */
	{ 0,	0,	sys_vhangup,		"vhangup"	}, /* 111 */
	{ 0,	0,	sys_idle,		"idle"		}, /* 112 */
	{ 1,	0,	sys_vm86old,		"vm86old"	}, /* 113 */
	{ 4,	TP,	sys_wait4,		"wait4", SYS_wait4 }, /* 114 */
	{ 1,	0,	sys_swapoff,		"swapoff"	}, /* 115 */
	{ 1,	0,	sys_sysinfo,		"sysinfo"	}, /* 116 */
	{ 6,	0,	sys_ipc,		"ipc", SYS_ipc }, /* 117 */
	{ 1,	0,	sys_fsync,		"fsync"		}, /* 118 */
	{ 1,	TS,	sys_sigreturn,		"sigreturn"	}, /* 119 */
	{ 5,	TP,	sys_clone,		"clone", SYS_clone }, /* 120 */
	{ 2,	0,	sys_setdomainname,	"setdomainname"	}, /* 121 */
	{ 1,	0,	sys_uname,		"uname"		}, /* 122 */
	{ 4,	0,	sys_cacheflush,		"cacheflush"	}, /* 123 */
	{ 1,	0,	sys_adjtimex,		"adjtimex"	}, /* 124 */
	{ 3,	0,	sys_mprotect,		"mprotect"	}, /* 125 */
	{ 3,	TS,	sys_sigprocmask,	"sigprocmask"	}, /* 126 */
	{ 2,	0,	sys_create_module,	"create_module"	}, /* 127 */
	{ 3,	0,	sys_init_module,	"init_module"	}, /* 128 */
	{ 2,	0,	sys_delete_module,	"delete_module"	}, /* 129 */
	{ 1,	0,	sys_get_kernel_syms,	"get_kernel_syms"}, /* 130 */
	{ 4,	0,	sys_quotactl,		"quotactl"	}, /* 131 */
	{ 1,	0,	sys_getpgid,		"getpgid"	}, /* 132 */
	{ 1,	0,	sys_fchdir,		"fchdir"	}, /* 133 */
	{ 0,	0,	sys_bdflush,		"bdflush"	}, /* 134 */
	{ 3,	0,	sys_sysfs,		"sysfs", SYS_sysfs }, /* 135 */
	{ 1,	0,	sys_personality,	"personality"	}, /* 136 */
	{ 5,	0,	sys_afs_syscall,	"afs_syscall"	}, /* 137 */
	{ 1,	0,	sys_setfsuid,		"setfsuid"	}, /* 138 */
	{ 1,	0,	sys_setfsgid,		"setfsgid"	}, /* 139 */
	{ 5,	0,	sys_llseek,		"_llseek"	}, /* 140 */
	{ 3,	0,	sys_getdents,		"getdents"	}, /* 141 */
	{ 5,	0,	sys_select,		"select"	}, /* 142 */
	{ 2,	0,	sys_flock,		"flock"		}, /* 143 */
	{ 3,	0,	sys_msync,		"msync"		}, /* 144 */
	{ 3,	0,	sys_readv,		"readv", SYS_readv }, /* 145 */
	{ 3,	0,	sys_writev,		"writev", SYS_writev }, /* 146 */
	{ 1,	0,	sys_getsid,		"getsid"	}, /* 147 */
	{ 1,	0,	sys_fdatasync,		"fdatasync"	}, /* 148 */
	{ 1,	0,	sys_sysctl,		"_sysctl"	}, /* 149 */
	{ 2,	0,	sys_mlock,		"mlock"		}, /* 150 */
	{ 2,	0,	sys_munlock,		"munlock"	}, /* 151 */
	{ 2,	0,	sys_mlockall,		"mlockall"	}, /* 152 */
	{ 0,	0,	sys_munlockall,		"munlockall"	}, /* 153 */
	{ 0,	0,	sys_sched_setparam,	"sched_setparam"}, /* 154 */
	{ 2,	0,	sys_sched_getparam,	"sched_getparam"}, /* 155 */
	{ 3,	0,	sys_sched_setscheduler,	"sched_setscheduler"}, /* 156 */
	{ 1,	0,	sys_sched_getscheduler,	"sched_getscheduler"}, /* 157 */
	{ 0,	0,	sys_sched_yield,	"sched_yield"}, /* 158 */
	{ 1,	0,	sys_sched_get_priority_max,"sched_get_priority_max"}, /* 159 */
	{ 1,	0,	sys_sched_get_priority_min,"sched_get_priority_min"}, /* 160 */
	{ 2,	0,	sys_sched_rr_get_interval,"sched_rr_get_interval"}, /* 161 */
	{ 2,	0,	sys_nanosleep,		"nanosleep"	}, /* 162 */
	{ 4,	0,	sys_mremap,		"mremap"	}, /* 163 */
	{ 3,	0,	sys_setresuid,		"setresuid"	}, /* 164 */
	{ 3,	0,	sys_getresuid,		"getresuid"	}, /* 165 */
	{ 5,	0,	printargs,		"getpagesize"	}, /* 166 */
	{ 5,	0,	sys_query_module,	"query_module"	}, /* 167 */
	{ 3,	0,	sys_poll,		"poll"		}, /* 168 */
	{ 3,	0,	printargs,		"nfsservctl"	}, /* 169 */
	{ 3,	0,	sys_setresgid,		"setresgid"	}, /* 170 */
	{ 3,	0,	sys_getresgid,		"getresgid"	}, /* 171 */
	{ 5,	0,	sys_prctl,		"prctl"		}, /* 172 */
	{ 1,	TS,	printargs,		"rt_sigreturn"	}, /* 173 */
	{ 4,	TS,	sys_rt_sigaction,	"rt_sigaction"  }, /* 174 */
	{ 4,	TS,	sys_rt_sigprocmask,	"rt_sigprocmask"}, /* 175 */
	{ 2,	TS,	sys_rt_sigpending,	"rt_sigpending"	}, /* 176 */
	{ 4,	TS,	sys_rt_sigtimedwait,	"rt_sigtimedwait"}, /* 177 */
	{ 3,	TS,	sys_rt_sigqueueinfo,    "rt_sigqueueinfo"}, /* 178 */
	{ 2,	TS,	sys_rt_sigsuspend,	"rt_sigsuspend"	}, /* 179 */
	{ 5,	TF,	sys_pread,		"pread64", SYS_read }, /* 180 */
	{ 5,	TF,	sys_pwrite,		"pwrite64", SYS_write }, /* 181 */
	{ 3,	TF,	sys_chown,		"lchown"	}, /* 182 */
	{ 2,	TF,	sys_getcwd,		"getcwd"	}, /* 183 */
	{ 2,	0,	sys_capget,		"capget"	}, /* 184 */
	{ 2,	0,	sys_capset,		"capset"	}, /* 185 */
	{ 2,	TS,	sys_sigaltstack,	"sigaltstack"	}, /* 186 */
	{ 4,	TD|TN,	sys_sendfile,		"sendfile"	}, /* 187 */
	{ 5,	0,	sys_getpmsg,		"getpmsg"	}, /* 188 */
	{ 5,	0,	sys_putpmsg,		"putpmsg"	}, /* 189 */
	{ 0,	TP,	sys_vfork,		"vfork", SYS_vfork }, /* 190 */
	{ 2,	0,	sys_getrlimit,		"getrlimit"	}, /* 191 */
	{ 6,	0,	sys_mmap,		"mmap2"		}, /* 192 */
	{ 3,	TF,	sys_truncate64,		"truncate64"	}, /* 193 */
	{ 3,	TF,	sys_ftruncate64,	"ftruncate64"	}, /* 194 */
	{ 2,	TF,	sys_stat64,		"stat64"	}, /* 195 */
	{ 2,	TF,	sys_lstat64,		"lstat64"	}, /* 196 */
	{ 2,	TF,	sys_fstat64,		"fstat64"	}, /* 197 */
	{ 3,	TF,	sys_chown,		"chown32"	}, /* 198 */
	{ 0,	0,	sys_getuid,		"getuid32"	}, /* 199 */
	{ 0,	0,	sys_getgid,		"getgid32"	}, /* 200 */
	{ 0,	0,	sys_geteuid,		"geteuid32"	}, /* 201 */
	{ 0,	0,	sys_geteuid,		"getegid32"	}, /* 202 */
	{ 2,	0,	sys_setreuid,		"setreuid32"	}, /* 203 */
	{ 2,	0,	sys_setregid,		"setregid32"	}, /* 204 */
	{ 2,	0,	sys_getgroups32,	"getgroups32"	}, /* 205 */
	{ 2,	0,	sys_setgroups32,	"setgroups32"	}, /* 206 */
	{ 3,	0,	sys_fchown,		"fchown32"	}, /* 207 */
	{ 3,	0,	sys_setresuid,		"setresuid32"	}, /* 208 */
	{ 3,	0,	sys_getresuid,		"getresuid32"	}, /* 209 */
	{ 3,	0,	sys_setresgid,		"setresgid32"	}, /* 210 */
	{ 3,	0,	sys_getresgid,		"getresgid32"	}, /* 211 */
	{ 3,	TF,	sys_chown,		"lchown32"	}, /* 212 */
	{ 1,	0,	sys_setuid,		"setuid32"	}, /* 213 */
	{ 1,	0,	sys_setgid,		"setgid32"	}, /* 214 */
	{ 1,	0,	sys_setfsuid,		"setfsuid32"	}, /* 215 */
	{ 1,	0,	sys_setfsgid,		"setfsgid32"	}, /* 216 */
	{ 2,	TF,	sys_pivotroot,		"pivot_root"	}, /* 217 */
	{ 5,	0,	printargs,		"SYS_218"	}, /* 218 */
	{ 5,	0,	printargs,		"SYS_219"	}, /* 219 */
	{ 3,	TD,	sys_getdents64,		"getdents64"	}, /* 220 */
	{ 0,	0,	printargs,		"gettid"	}, /* 221 */
	{ 2,	TS,	sys_kill,		"tkill"		}, /* 222 */
	{ 5,	TF,	sys_setxattr,		"setxattr"	}, /* 223 */
	{ 5,	TF,	sys_setxattr,		"lsetxattr"	}, /* 224 */
	{ 5,	0,	sys_fsetxattr,		"fsetxattr"	}, /* 225 */
	{ 4,	TF,	sys_getxattr,		"getxattr"	}, /* 226 */
	{ 4,	TF,	sys_getxattr,		"lgetxattr"	}, /* 227 */
	{ 4,	0,	sys_fgetxattr,		"fgetxattr"	}, /* 228 */
	{ 3,	TF,	sys_listxattr,		"listxattr"	}, /* 229 */
	{ 3,	TF,	sys_listxattr,		"llistxattr"	}, /* 230 */
	{ 3,	0,	sys_flistxattr,		"flistxattr"	}, /* 231 */
	{ 2,	TF,	sys_removexattr,	"removexattr"	}, /* 232 */
	{ 2,	TF,	sys_removexattr,	"lremovexattr"	}, /* 233 */
	{ 2,	0,	sys_fremovexattr,	"fremovexattr"	}, /* 234 */
	{ 6,	0,	sys_futex,		"futex"		}, /* 235 */
	{ 4,	TF,	sys_sendfile64,		"sendfile64"	}, /* 236 */
	{ 3,	0,	sys_mincore,		"mincore"	}, /* 237 */
	{ 3,	0,	sys_madvise,		"madvise"	}, /* 238 */
	{ 3,	0,	sys_fcntl,		"fcntl64"	}, /* 239 */
	{ 4,	0,	sys_readahead,		"readahead"	}, /* 240 */
	{ 2,	0,	sys_io_setup,		"io_setup"	}, /* 241 */
	{ 1,	0,	sys_io_destroy,		"io_destroy"	}, /* 242 */
	{ 5,	0,	sys_io_getevents,	"io_getevents"	}, /* 243 */
	{ 3,	0,	sys_io_submit,		"io_submit"	}, /* 244 */
	{ 3,	0,	sys_io_cancel,		"io_cancel"	}, /* 245 */
	{ 5,	TF,	sys_fadvise64,		"fadvise64"	}, /* 246 */
	{ 1,	TP,	sys_exit,		"exit_group", __NR_exit_group }, /* 247 */
	{ 4,	0,	printargs,		"lookup_dcookie"}, /* 248 */
	{ 1,	0,	sys_epoll_create,	"epoll_create"	}, /* 249 */
	{ 4,	0,	sys_epoll_ctl,		"epoll_ctl"	}, /* 250 */
	{ 4,	0,	sys_epoll_wait,		"epoll_wait"	}, /* 251 */
	{ 5,	0,	sys_remap_file_pages,	"remap_file_pages"}, /* 252 */
	{ 1,	0,	printargs,		"set_tid_address"}, /* 253 */
	{ 3,	0,	sys_timer_create,	"timer_create"	}, /* 254 */
	{ 4,	0,	sys_timer_settime,	"timer_settime"	}, /* 255 */
	{ 2,	0,	sys_timer_gettime,	"timer_gettime"	}, /* 256 */
	{ 1,	0,	sys_timer_getoverrun,	"timer_getoverrun"}, /* 257 */
	{ 1,	0,	sys_timer_delete,	"timer_delete"	}, /* 258 */
	{ 2,	0,	sys_clock_settime,	"clock_settime"	}, /* 259 */
	{ 2,	0,	sys_clock_gettime,	"clock_gettime"	}, /* 260 */
	{ 2,	0,	sys_clock_getres,	"clock_getres"	}, /* 261 */
	{ 4,	0,	sys_clock_nanosleep,	"clock_nanosleep"}, /* 262 */
	{ 3,	TF,	sys_statfs64,		"statfs64"	}, /* 263 */
	{ 3,	TF,	sys_fstatfs64,		"fstatfs64"	}, /* 264 */
	{ 3,	TS,	sys_tgkill,		"tgkill"	}, /* 265 */
	{ 2,	TF,	sys_utimes,		"utimes"	}, /* 266 */
	{ 6,	TF,	sys_fadvise64_64,	"fadvise64_64"	}, /* 267 */
	{ 6,	0,	sys_mbind,		"mbind"		}, /* 268 */
	{ 5,	0,	sys_get_mempolicy,	"get_mempolicy"	}, /* 269 */
	{ 3,	0,	sys_set_mempolicy,	"set_mempolicy"	}, /* 270 */
	{ 4,	0,	sys_mq_open,		"mq_open"	}, /* 271 */
	{ 1,	0,	sys_mq_unlink,		"mq_unlink"	}, /* 272 */
	{ 5,	0,	sys_mq_timedsend,	"mq_timedsend"	}, /* 273 */
	{ 5,	0,	sys_mq_timedreceive,	"mq_timedreceive" }, /* 274 */
	{ 2,	0,	sys_mq_notify,		"mq_notify"	}, /* 275 */
	{ 3,	0,	sys_mq_getsetattr,	"mq_getsetattr"	}, /* 276 */
	{ 5,	TP,	sys_waitid,		"waitid", SYS_waitid }, /* 277 */
	{ 5,	0,	printargs,		"vserver"	}, /* 278 */
	{ 5,	0,	printargs,		"add_key"	}, /* 279 */
	{ 5,	0,	printargs,		"request_key"	}, /* 280 */
	{ 5,	0,	printargs,		"keyctl"	}, /* 281 */

	{ 5,	0,	printargs,		"SYS_282"	}, /* 282 */
	{ 5,	0,	printargs,		"SYS_283"	}, /* 283 */
	{ 5,	0,	printargs,		"SYS_284"	}, /* 284 */
	{ 5,	0,	printargs,		"SYS_285"	}, /* 285 */
	{ 5,	0,	printargs,		"SYS_286"	}, /* 286 */
	{ 5,	0,	printargs,		"SYS_287"	}, /* 287 */
	{ 5,	0,	printargs,		"SYS_288"	}, /* 288 */
	{ 5,	0,	printargs,		"SYS_289"	}, /* 289 */
	{ 5,	0,	printargs,		"SYS_290"	}, /* 290 */
	{ 5,	0,	printargs,		"SYS_291"	}, /* 291 */
	{ 5,	0,	printargs,		"SYS_292"	}, /* 292 */
	{ 5,	0,	printargs,		"SYS_293"	}, /* 293 */
	{ 5,	0,	printargs,		"SYS_294"	}, /* 294 */
	{ 5,	0,	printargs,		"SYS_295"	}, /* 295 */
	{ 5,	0,	printargs,		"SYS_296"	}, /* 296 */
	{ 5,	0,	printargs,		"SYS_297"	}, /* 297 */
	{ 5,	0,	printargs,		"SYS_298"	}, /* 298 */
	{ 5,	0,	printargs,		"SYS_299"	}, /* 299 */
	{ 5,	0,	printargs,		"SYS_300"	}, /* 300 */
	{ 5,	0,	printargs,		"SYS_301"	}, /* 301 */
	{ 5,	0,	printargs,		"SYS_302"	}, /* 302 */
	{ 5,	0,	printargs,		"SYS_303"	}, /* 303 */
	{ 5,	0,	printargs,		"SYS_304"	}, /* 304 */
	{ 5,	0,	printargs,		"SYS_305"	}, /* 305 */
	{ 5,	0,	printargs,		"SYS_306"	}, /* 306 */
	{ 5,	0,	printargs,		"SYS_307"	}, /* 307 */
	{ 5,	0,	printargs,		"SYS_308"	}, /* 308 */
	{ 5,	0,	printargs,		"SYS_309"	}, /* 309 */
	{ 6,	0,	sys_move_pages,		"move_pages"	}, /* 310 */
	{ 5,	0,	printargs,		"SYS_311"	}, /* 311 */
	{ 5,	0,	printargs,		"SYS_312"	}, /* 312 */
	{ 5,	0,	printargs,		"SYS_313"	}, /* 313 */
	{ 5,	0,	printargs,		"SYS_314"	}, /* 314 */
	{ 5,	0,	printargs,		"SYS_315"	}, /* 315 */
	{ 5,	0,	printargs,		"SYS_316"	}, /* 316 */
	{ 5,	0,	printargs,		"SYS_317"	}, /* 317 */
	{ 5,	0,	printargs,		"SYS_318"	}, /* 318 */
	{ 5,	0,	printargs,		"SYS_319"	}, /* 319 */
	{ 5,	0,	printargs,		"SYS_320"	}, /* 320 */
	{ 5,	0,	printargs,		"SYS_321"	}, /* 321 */
	{ 5,	0,	printargs,		"SYS_322"	}, /* 322 */
	{ 5,	0,	printargs,		"SYS_323"	}, /* 323 */
	{ 5,	0,	printargs,		"SYS_324"	}, /* 324 */
	{ 5,	0,	printargs,		"SYS_325"	}, /* 325 */
	{ 5,	0,	printargs,		"SYS_326"	}, /* 326 */
	{ 5,	0,	printargs,		"SYS_327"	}, /* 327 */
	{ 5,	0,	printargs,		"SYS_328"	}, /* 328 */
	{ 5,	0,	printargs,		"SYS_329"	}, /* 329 */
	{ 5,	0,	printargs,		"SYS_330"	}, /* 330 */
	{ 5,	0,	printargs,		"SYS_331"	}, /* 331 */
	{ 5,	0,	printargs,		"SYS_332"	}, /* 332 */
	{ 5,	0,	printargs,		"SYS_333"	}, /* 333 */
	{ 5,	0,	printargs,		"SYS_334"	}, /* 334 */
	{ 5,	0,	printargs,		"SYS_335"	}, /* 335 */
	{ 5,	0,	printargs,		"SYS_336"	}, /* 336 */
	{ 5,	0,	printargs,		"SYS_337"	}, /* 337 */
	{ 5,	0,	printargs,		"SYS_338"	}, /* 338 */
	{ 5,	0,	printargs,		"SYS_339"	}, /* 339 */
	{ 5,	0,	printargs,		"SYS_340"	}, /* 340 */
	{ 5,	0,	printargs,		"SYS_341"	}, /* 341 */
	{ 5,	0,	printargs,		"SYS_342"	}, /* 342 */
	{ 5,	0,	printargs,		"SYS_343"	}, /* 343 */
	{ 5,	0,	printargs,		"SYS_344"	}, /* 344 */
	{ 5,	0,	printargs,		"SYS_345"	}, /* 345 */
	{ 5,	0,	printargs,		"SYS_346"	}, /* 346 */
	{ 5,	0,	printargs,		"SYS_347"	}, /* 347 */
	{ 5,	0,	printargs,		"SYS_348"	}, /* 348 */
	{ 5,	0,	printargs,		"SYS_349"	}, /* 349 */
	{ 5,	0,	printargs,		"SYS_350"	}, /* 350 */
	{ 5,	0,	printargs,		"SYS_351"	}, /* 351 */
	{ 5,	0,	printargs,		"SYS_352"	}, /* 352 */
	{ 5,	0,	printargs,		"SYS_353"	}, /* 353 */
	{ 5,	0,	printargs,		"SYS_354"	}, /* 354 */
	{ 5,	0,	printargs,		"SYS_355"	}, /* 355 */
	{ 5,	0,	printargs,		"SYS_356"	}, /* 356 */
	{ 5,	0,	printargs,		"SYS_357"	}, /* 357 */
	{ 5,	0,	printargs,		"SYS_358"	}, /* 358 */
	{ 5,	0,	printargs,		"SYS_359"	}, /* 359 */
	{ 5,	0,	printargs,		"SYS_360"	}, /* 360 */
	{ 5,	0,	printargs,		"SYS_361"	}, /* 361 */
	{ 5,	0,	printargs,		"SYS_362"	}, /* 362 */
	{ 5,	0,	printargs,		"SYS_363"	}, /* 363 */
	{ 5,	0,	printargs,		"SYS_364"	}, /* 364 */
	{ 5,	0,	printargs,		"SYS_365"	}, /* 365 */
	{ 5,	0,	printargs,		"SYS_366"	}, /* 366 */
	{ 5,	0,	printargs,		"SYS_367"	}, /* 367 */
	{ 5,	0,	printargs,		"SYS_368"	}, /* 368 */
	{ 5,	0,	printargs,		"SYS_369"	}, /* 369 */
	{ 5,	0,	printargs,		"SYS_370"	}, /* 370 */
	{ 5,	0,	printargs,		"SYS_371"	}, /* 371 */
	{ 5,	0,	printargs,		"SYS_372"	}, /* 372 */
	{ 5,	0,	printargs,		"SYS_373"	}, /* 373 */
	{ 5,	0,	printargs,		"SYS_374"	}, /* 374 */
	{ 5,	0,	printargs,		"SYS_375"	}, /* 375 */
	{ 5,	0,	printargs,		"SYS_376"	}, /* 376 */
	{ 5,	0,	printargs,		"SYS_377"	}, /* 377 */
	{ 5,	0,	printargs,		"SYS_378"	}, /* 378 */
	{ 5,	0,	printargs,		"SYS_379"	}, /* 379 */
	{ 5,	0,	printargs,		"SYS_380"	}, /* 380 */
	{ 5,	0,	printargs,		"SYS_381"	}, /* 381 */
	{ 5,	0,	printargs,		"SYS_382"	}, /* 382 */
	{ 5,	0,	printargs,		"SYS_383"	}, /* 383 */
	{ 5,	0,	printargs,		"SYS_384"	}, /* 384 */
	{ 5,	0,	printargs,		"SYS_385"	}, /* 385 */
	{ 5,	0,	printargs,		"SYS_386"	}, /* 386 */
	{ 5,	0,	printargs,		"SYS_387"	}, /* 387 */
	{ 5,	0,	printargs,		"SYS_388"	}, /* 388 */
	{ 5,	0,	printargs,		"SYS_389"	}, /* 389 */
	{ 5,	0,	printargs,		"SYS_390"	}, /* 390 */
	{ 5,	0,	printargs,		"SYS_391"	}, /* 391 */
	{ 5,	0,	printargs,		"SYS_392"	}, /* 392 */
	{ 5,	0,	printargs,		"SYS_393"	}, /* 393 */
	{ 5,	0,	printargs,		"SYS_394"	}, /* 394 */
	{ 5,	0,	printargs,		"SYS_395"	}, /* 395 */
	{ 5,	0,	printargs,		"SYS_396"	}, /* 396 */
	{ 5,	0,	printargs,		"SYS_397"	}, /* 397 */
	{ 5,	0,	printargs,		"SYS_398"	}, /* 398 */
	{ 5,	0,	printargs,		"SYS_399"	}, /* 399 */

#if SYS_socket_subcall != 400
 #error fix me
#endif
	{ 8,	0,	printargs,		"socket_subcall"}, /* 400 */
	{ 3,	TN,	sys_socket,		"socket"	}, /* 401 */
	{ 3,	TN,	sys_bind,		"bind"		}, /* 402 */
	{ 3,	TN,	sys_connect,		"connect"	}, /* 403 */
	{ 2,	TN,	sys_listen,		"listen"	}, /* 404 */
	{ 3,	TN,	sys_accept,		"accept"	}, /* 405 */
	{ 3,	TN,	sys_getsockname,	"getsockname"	}, /* 406 */
	{ 3,	TN,	sys_getpeername,	"getpeername"	}, /* 407 */
	{ 4,	TN,	sys_socketpair,		"socketpair"	}, /* 408 */
	{ 4,	TN,	sys_send,		"send", SYS_sub_send }, /* 409 */
	{ 4,	TN,	sys_recv,		"recv", SYS_sub_recv }, /* 410 */
	{ 6,	TN,	sys_sendto,		"sendto", SYS_sub_sendto }, /* 411 */
	{ 6,	TN,	sys_recvfrom,		"recvfrom", SYS_sub_recvfrom }, /* 412 */
	{ 2,	TN,	sys_shutdown,		"shutdown"	}, /* 413 */
	{ 5,	TN,	sys_setsockopt,		"setsockopt"	}, /* 414 */
	{ 5,	TN,	sys_getsockopt,		"getsockopt"	}, /* 415 */
	{ 5,	TN,	sys_sendmsg,		"sendmsg"	}, /* 416 */
	{ 5,	TN,	sys_recvmsg,		"recvmsg"	}, /* 417 */
	{ 4,	TN,	sys_accept4,		"accept4"	}, /* 418 */

#if SYS_ipc_subcall != 419
 #error fix me
#endif
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 419 */
	{ 4,	TI,	sys_semop,		"semop"		}, /* 420 */
	{ 4,	TI,	sys_semget,		"semget"	}, /* 421 */
	{ 4,	TI,	sys_semctl,		"semctl"	}, /* 422 */
	{ 5,	TI,	sys_semtimedop,		"semtimedop"	}, /* 423 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 424 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 425 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 426 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 427 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 428 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 429 */
	{ 4,	TI,	sys_msgsnd,		"msgsnd"	}, /* 430 */
	{ 4,	TI,	sys_msgrcv,		"msgrcv"	}, /* 431 */
	{ 4,	TI,	sys_msgget,		"msgget"	}, /* 432 */
	{ 4,	TI,	sys_msgctl,		"msgctl"	}, /* 433 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 434 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 435 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 436 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 437 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 438 */
	{ 4,	0,	printargs,		"ipc_subcall"	}, /* 439 */
	{ 4,	TI,	sys_shmat,		"shmat"		}, /* 440 */
	{ 4,	TI,	sys_shmdt,		"shmdt"		}, /* 441 */
	{ 4,	TI,	sys_shmget,		"shmget"	}, /* 442 */
	{ 4,	TI,	sys_shmctl,		"shmctl"	}, /* 443 */
