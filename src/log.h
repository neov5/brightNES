#ifndef __LOG_H__
#define __LOG_H__

typedef enum {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
} log_level_t;

#if defined(LOG_TRACE)
#define log_trace(...) log_log(LOG_TRACE, __VA_ARGS__)
#else
#define log_trace(...) do {} while (0)
#endif

#if defined(LOG_DEBUG) || defined(LOG_TRACE)
#define log_debug(...) log_log(LOG_DEBUG, __VA_ARGS__)
#else
#define log_debug(...) do {} while (0)
#endif

#if defined(LOG_INFO) || defined(LOG_DEBUG) || defined(LOG_TRACE)
#define log_info(...) log_log(LOG_INFO, __VA_ARGS__)
#else
#define log_info(...) do {} while (0)
#endif

#if defined(LOG_WARN) || defined(LOG_INFO) || defined(LOG_DEBUG) || defined(LOG_TRACE)
#define log_warn(...) log_log(LOG_WARN, __VA_ARGS__)
#else
#define log_warn(...) do {} while (0)
#endif

#if defined(LOG_ERROR) || defined(LOG_WARN) || defined(LOG_INFO) || defined(LOG_DEBUG) || defined(LOG_TRACE)
#define log_error(...) log_log(LOG_ERROR, __VA_ARGS__)
#else
#define log_error(...) do {} while (0)
#endif

#if defined(LOG_FATAL) || defined(LOG_ERROR) || defined(LOG_WARN) || defined(LOG_INFO) || defined(LOG_DEBUG) || defined(LOG_TRACE)
#define log_fatal(...) log_log(LOG_FATAL, __VA_ARGS__)
#else
#define log_fatal(...) do {} while (0)
#endif

void log_log(log_level_t level, const char* fmt, ...);

#endif
