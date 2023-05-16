#ifndef _OPENGEAR_THREADS_H_
#define _OPENGEAR_THREADS_H_

#ifdef __cplusplus
extern "C" {
#endif

/** A task which can be performed in parallel. */
typedef void (og_threadtaskfunc_t)(
	const char *arg1, const char *arg2, const char *arg3);

/** A thread pool task */
typedef struct og_threadtask og_threadtask_t;

/** A thread pool task */
typedef struct og_thread og_thread_t;

/** A thread pool. */
typedef struct og_threadpool og_threadpool_t;

#if 0
// Debug trace functions
#ifdef PMHOSTS_DEBUG
#define LOG_INFO(fmt, a...) \
	fprintf(stdout, fmt, ##a); fprintf(stdout, "\n"); fflush(stdout)
#define LOG_DEBUG(fmt, a...) \
	fprintf(stdout, fmt, ##a); fprintf(stdout, "\n"); fflush(stdout)
#define LOG_WARN(fmt, a...) \
	fprintf(stdout, fmt, ##a); fprintf(stdout, "\n"); fflush(stdout)
#else
#define LOG_INFO(fmt, a...) ({})
#define LOG_DEBUG(fmt, a...) ({})
#define LOG_WARN(fmt, a...) ({})
#endif

#define LOG_NOTICE(fmt, a...) \
	fprintf(stdout, fmt, ##a); fprintf(stdout, "\n"); fflush(stdout)
#define LOG_ERROR(fmt, a...) \
	fprintf(stderr, fmt, ##a); fprintf(stderr, "\n"); fflush(stderr)
#define LOG_FATAL(fmt, a...) \
	fprintf(stderr, fmt, ##a); fprintf(stderr, "\n"); fflush(stderr)
#endif

/**
 * Create a thread pool.
 * @param poolp The thread pool pointer.
 * @param len The size of the thread pool.
 * @return 0 on success otherwise -1.
 */
int og_threadpool_create(og_threadpool_t **poolp, size_t len);

/**
 * Wait for any running tasks to complete.
 * @param pool The thread pool to wait on.
 */
void og_threadpool_wait(og_threadpool_t *pool);

/**
 * A thread pool thread runtime.
 */
void * og_thread_run(void *arg);

/**
 * Cleanup the thread pool.
 * @param poolp The thread pool handle.
 */
void og_threadpool_destroy(
		og_threadpool_t **poolp);

/**
 * Append a new task to the queue for concurrent execution.
 * @param func The function to perform.
 * @param arg1 The 1st argument for the function.
 * @param arg2 The 2nd argument for the function.
 * @param arg3 The 3rd argument for the function.
 * @return 0 on success otherwise -1.
 * */
int og_threadpool_addtask(og_threadpool_t *pool,
	og_threadtaskfunc_t *func,
	const char *arg1, const char *arg2, const char *arg3);

/**
 * Shutdown the thread pool.
 * @param pool The threadpool to close down.
 */
void og_threadpool_exit(og_threadpool_t *pool);

/**
 * Determine if the threadpool is shutting down.
 * @param pool The thread pool to query.
 * @return Whether or not the thread pool is closing down.
 */
bool og_threadpool_isexiting(og_threadpool_t *pool);

/**
 * Lock the threadpool for exclusive use.
 * @param pool The threadpool to lock.
 */
void og_threadpool_lock(og_threadpool_t *pool);

/**
 * Unlock the threadpool for exclusive use.
 * @param pool The threadpool to lock.
 */
void og_threadpool_unlock(og_threadpool_t *pool);

#ifdef __cplusplus
}
#endif

#endif /* _OPENGEAR_THREADS_H_ */
