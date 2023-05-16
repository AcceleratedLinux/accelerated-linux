#ifndef _OPENGEAR_LOG_H_
#define _OPENGEAR_LOG_H_

#pragma GCC system_header

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <log4c.h>

int log_init();

extern log4c_category_t *logger;
static inline void LOG(int pri, const char *fmt, ...)
	__attribute__((format(printf, 2, 3))); /* 2 = format, 3 = params */

static inline void
LOG(int pri, const char *fmt, ...)
{
#if 1
	va_list va;
	int err;
	if (!log4c_category_is_priority_enabled(logger, pri)) {
		return;
	}

	err = errno;
	va_start(va, fmt);
	log4c_category_vlog(logger, pri, fmt, va);
	va_end(va);
	fflush(NULL);
	errno = err;
#else
	return;
#endif
}

#define LOG_IS_ENABLED(pri) \
	log4c_category_is_priority_enabled(logger, pri)

#define LOG_FATAL(fmt, a...) LOG(LOG4C_PRIORITY_FATAL, fmt, ##a)
#define LOG_ALERT(fmt, a...) LOG(LOG4C_PRIORITY_ALERT, fmt, ##a)
#define LOG_CRIT(fmt, a...) LOG(LOG4C_PRIORITY_CRIT, fmt, ##a)
#define LOG_ERROR(fmt, a...) LOG(LOG4C_PRIORITY_ERROR, fmt, ##a)
#define LOG_WARN(fmt, a...) LOG(LOG4C_PRIORITY_WARN, fmt, ##a)
#define LOG_NOTICE(fmt, a...) LOG(LOG4C_PRIORITY_NOTICE, fmt, ##a)
#define LOG_INFO(fmt, a...) LOG(LOG4C_PRIORITY_INFO, fmt, ##a)
#define LOG_DEBUG(fmt, a...) LOG(LOG4C_PRIORITY_DEBUG, fmt, ##a)
#define LOG_TRACE(fmt, a...) \
	LOG(LOG4C_PRIORITY_TRACE, "%s:%d " fmt, __func__, __LINE__, ##a)

#define LOG_FATAL_ENABLED() LOG_IS_ENABLED(LOG4C_PRIORITY_FATAL)
#define LOG_ALERT_ENABLED() LOG_IS_ENABLED(LOG4C_PRIORITY_ALERT)
#define LOG_CRIT_ENABLED() LOG_IS_ENABLED(LOG4C_PRIORITY_CRIT)
#define LOG_ERROR_ENABLED() LOG_IS_ENABLED(LOG4C_PRIORITY_ERROR)
#define LOG_WARN_ENABLED() LOG_IS_ENABLED(LOG4C_PRIORITY_WARN)
#define LOG_NOTICE_ENABLED() LOG_IS_ENABLED(LOG4C_PRIORITY_NOTICE)
#define LOG_INFO_ENABLED() LOG_IS_ENABLED(LOG4C_PRIORITY_INFO)
#define LOG_DEBUG_ENABLED() LOG_IS_ENABLED(LOG4C_PRIORITY_DEBUG)
#define LOG_TRACE_ENABLED() LOG_IS_ENABLED(LOG4C_PRIORITY_TRACE)

#endif /* _OPENGEAR_LOG_H_ */

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
