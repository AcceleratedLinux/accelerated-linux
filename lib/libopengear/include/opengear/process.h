#ifndef _OPENGEAR_PROCESS_H_
#define _OPENGEAR_PROCESS_H_

#include <inttypes.h>

#ifdef	__cplusplus
extern "C" {
#endif

typedef enum {
	OG_PROC_NONE	= (2 << 0),
	OG_PROC_DAEMON	= (2 << 1)
} og_procflags_t;

/**
 * Redirect stdin, stdout, stderr to /dev/null
 * @return 0 on success otherwise -1
 */
int opengear_proc_redir_std_null(void);

/**
 * Execute a command / executable with the specified arguments.
 * @param path The path to the executable.
 * @param argv A list of arguments.
 * @param exitcode The buffer to place the exit code in.
 * @param flags options used in deciding how to run the subproc.
 * @return 0 on success otherwise -1
 */
int opengear_proc_exec(
	const char *path, const char * const argv[], int *exitcode, uint32_t flags);

/**
 * Execute a command / executable with the specified arguments,
 * and do not wait for it to finish
 * @param path The path to the executable.
 * @param argv A list of arguments.
 * @param flags options used in deciding how to run the subproc.
 * @return pid of the executed process on success otherwise -1
 */
int opengear_proc_exec_nowait(
	const char *path, const char * const argv[], uint32_t flags);
/**
 * Execute a command / executable with the specified arguments,
 * and do not wait for it to finish
 * @param path The path to the executable.
 * @param argv A list of arguments.
 * @param envp A list of environment variables in key=value form
 * @param flags options used in deciding how to run the subproc.
 * @return pid of the executed process on success otherwise -1
 */
int opengear_proc_exec_env_nowait(
	const char *path, const char * const argv[], const char * const envp[], uint32_t flags);
/**
 * Execute the given string as a command. Will log warnings to syslog if the
 * command fails to execute, or if the command return code does not match the
 * provided expected success code.
 * @param cmd The command to execute.
 * @param exitcode The buffer to place the exit code in.
 * @param success The value the command should return upon succeeding.
 * @return 0 on success otherwise -1
 */
int opengear_system(const char *cmd, int *exitcode, int success);

/**
 * Execute the given string as a command. Does not log any warnings/errors.
 * @param cmd The command to execute.
 * @param exitcode The buffer to place the exit code in.
 * @return 0 on success otherwise -1
 */
int opengear_system_execonly(const char *cmd, int *exitcode);

/**
 * Execute the given command with the specified arguments, returning
 * fd's to stdin and stdout
 */
int opengear_proc_exec_nowait_fds(const char *path, const char * const argv[], int *infd, int *outfd);

/**
 * Execute the given command with the specified arguments, returning
 * fd's to stdin and stdout and stderr
 * Uses PATH to find the executable
 */
int opengear_proc_execp_nowait_fds(const char *path, const char * const argv[], int *infd, int *outfd, int *errfd);

/**
 * Find the pid of the currently running process.
 * @param argv An array of command line strings.
 * @param argc The argv length.
 * @param pidlist The PID buffer array.
 * @param maxpids The size of the PID buffer array
 * @param npids A pointer to the number of pids found on success.
 * @return 0 on success otherwise -1
 */
int opengear_proc_get_pids(const char * const argv[], int argc,
	pid_t *pidlist, size_t maxpids, size_t *npids);

/**
 * Find all the PIDs associated with the given command name
 * @param comm     The command string to match against
 * @param pids     Storage for where to store found PIDs
 * @param maxpids  The length of the pids[] store
 * @return number of pids stored in pids[], or
 *         -1 on system error
 */
int opengear_proc_find_comms(const char *comm, pid_t *pids, int maxpids);

#ifdef __cplusplus
}
#endif

#endif /* _OPENGEAR_NETWORK_H_ */
