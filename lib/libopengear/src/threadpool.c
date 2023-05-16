#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <paths.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>

#ifdef EMBED
#include <config/autoconf.h>
#endif

#include <pthread.h>

#include <opengear/queue.h>
#include <opengear/threadpool.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/** A thread pool task */
struct og_threadtask {
	og_threadtaskfunc_t 		*func;
	void 				*arg1;
	void 				*arg2;
	void 				*arg3;
	TAILQ_ENTRY(og_threadtask) 	list;
};

/**
 * A thread pool task
 */
struct og_thread {
	pthread_t 		id;
	TAILQ_ENTRY(og_thread) list;
};

struct og_threadpool {

	/* A queue of threads. */
	TAILQ_HEAD(, og_thread) threads;

	/** A queue of tasks. */
	TAILQ_HEAD(, og_threadtask) tasks;

	/** The mutually exclusive lock. */
	pthread_mutex_t lock;

	/** The current exiting status of the thread pool. */
	bool exiting;

	/** The current number of running tasks. */
	size_t runningtasks;

};

/** The concurrent task pool. */
og_threadpool_t *og_pool = NULL;

/** The time delay between checking for tasks in the thread-pool. */
static const size_t og_threadpool_delay = 500;

#if 1
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

int
og_threadpool_create(og_threadpool_t **poolp, size_t len)
{
	og_threadpool_t *pool = calloc(1, sizeof(*pool));
	size_t i;

	if (pool == NULL) {
		LOG_FATAL("Failed to allocate thread pool: %s",
			strerror(errno));
		return (-1);
	}

	TAILQ_INIT(&pool->threads);
	TAILQ_INIT(&pool->tasks);
	pthread_mutex_init(&pool->lock, NULL);
	pool->exiting = false;
	pool->runningtasks = 0;

	*poolp = pool;
	for (i = 0; i < len; ++i) {
		struct og_thread *thread = calloc(1, sizeof(*thread));
		if (thread == NULL) {
			LOG_FATAL("Failed to allocate thread: %s",
				strerror(errno));
			return (-1);
		}
		pthread_create(
			&thread->id, NULL, og_thread_run,
			(void *) pool);
		TAILQ_INSERT_TAIL(&pool->threads, thread, list);
	}

	return (0);
}

void *
og_thread_run(void *arg)
{
	og_threadpool_t *pool = (og_threadpool_t *) arg;
//	og_threadpool_t *pool = og_pool;

	LOG_DEBUG("Thread starting");
	while (!pool->exiting) {

		pthread_mutex_lock(&pool->lock);
		if (!TAILQ_EMPTY(&pool->tasks)) {

			// Pull a new task from the queue
			og_threadtask_t *task = TAILQ_FIRST(&pool->tasks);
			if (task != NULL) {
				TAILQ_REMOVE(&pool->tasks, task, list);
				pool->runningtasks++;
				pthread_mutex_unlock(&pool->lock);

				// Run the task
				(void) task->func(task->arg1, task->arg2, task->arg3);
				pool->runningtasks--;

				if (task->arg1 != NULL) {
					free(task->arg1);
				}
				if (task->arg2 != NULL) {
					free(task->arg2);
				}
				if (task->arg3 != NULL) {
					free(task->arg3);
				}
				free(task);
			}

		} else {
			pthread_mutex_unlock(&pool->lock);
		}

		// Force a sleep.
		usleep(og_threadpool_delay);
	}
	LOG_DEBUG("Thread exiting");
	return (NULL);
}

void
og_threadpool_wait(og_threadpool_t *pool)
{

	LOG_INFO("Entering realtime thread loop");
	while (!pool->exiting) {

		LOG_DEBUG("Waiting on pool %p: Exiting: %d, Queue Empty: %s, "
			"Running: %d", pool, pool->exiting,
			TAILQ_EMPTY(&pool->tasks) ? "true" : "false",
			pool->runningtasks);
		if (TAILQ_EMPTY(&pool->tasks) && pool->runningtasks == 0) {
			break;

		}


		usleep(og_threadpool_delay);
	}
	LOG_INFO("Leaving realtime thread loop");
}

void
og_threadpool_destroy(og_threadpool_t **poolp)
{
	og_threadpool_t *pool = *poolp;
	void *exitstatus;

	if (pool != NULL) {
		pool->exiting = true;
		sleep(1);

		while (!TAILQ_EMPTY(&pool->threads)) {
			struct og_thread *thread =
				TAILQ_FIRST(&pool->threads);
			if (thread != NULL) {
				TAILQ_REMOVE(&pool->threads, thread, list);
				pthread_join(thread->id, &exitstatus);
				free(thread);
			}
		}

		while (!TAILQ_EMPTY(&pool->tasks)) {
			og_threadtask_t *task = TAILQ_FIRST(&pool->tasks);
			if (task != NULL) {
				TAILQ_REMOVE(&pool->tasks, task, list);
				free(task);
			}
		}
		pthread_mutex_destroy(&pool->lock);
		free(pool);
	}

	*poolp = NULL;
}

int
og_threadpool_addtask(
		og_threadpool_t *pool,
		og_threadtaskfunc_t *func,
		const char *arg1, const char *arg2, const char *arg3)
{
	og_threadtask_t *task = calloc(1, sizeof(*task));
	if (task == NULL) {
		LOG_FATAL("Out of memory");
		return (-1);
	}

	LOG_DEBUG("Adding func: %p arg1: %s arg2: %s arg3: %s",
			func, arg1, arg2, arg3);

	task->func = func;
	if (arg1 != NULL) {
		task->arg1 = strdup(arg1);
	}
	if (arg2 != NULL) {
		task->arg2 = strdup(arg2);
	}
	if (arg3 != NULL) {
		task->arg3 = strdup(arg3);
	}

	pthread_mutex_lock(&pool->lock);
	TAILQ_INSERT_TAIL(&pool->tasks, task, list);
	pthread_mutex_unlock(&pool->lock);
	return (0);
}

void
og_threadpool_exit(og_threadpool_t *pool)
{
	pool->exiting = true;
}

bool
og_threadpool_isexiting(og_threadpool_t *pool)
{
	return pool->exiting;
}

void
og_threadpool_lock(og_threadpool_t *pool)
{
	pthread_mutex_lock(&pool->lock);
}

void
og_threadpool_unlock(og_threadpool_t *pool)
{
	pthread_mutex_unlock(&pool->lock);
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
