// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024 neov5

#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdarg.h>
#include "nes.h"

typedef enum {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
} log_level_t;

typedef struct {
  va_list ap;
  const char *fmt;
  FILE* output;
  log_level_t level;
  u64 cpu_cycle;
  u64 ppu_cycle;
} log_event_t;

#if defined(LOG_TRACE)
#define log_trace(...) log_log(LOG_TRACE, __VA_ARGS__)
#else
#define log_trace(...) do {} while (0)
#endif

#if defined(LOG_DEBUG) || defined(LOG_TRACE)
#define log_debug(...) log_log(DEBUG, __VA_ARGS__)
#else
#define log_debug(...) do {} while (0)
#endif

#if defined(LOG_INFO) || defined(LOG_DEBUG) || defined(LOG_TRACE)
#define log_info(...) log_log(INFO, __VA_ARGS__)
#else
#define log_info(...) do {} while (0)
#endif

#if defined(LOG_WARN) || defined(LOG_INFO) || defined(LOG_DEBUG) || defined(LOG_TRACE)
#define log_warn(...) log_log(WARN, __VA_ARGS__)
#else
#define log_warn(...) do {} while (0)
#endif

#if defined(LOG_ERROR) || defined(LOG_WARN) || defined(LOG_INFO) || defined(LOG_DEBUG) || defined(LOG_TRACE)
#define log_error(...) log_log(ERROR, __VA_ARGS__)
#else
#define log_error(...) do {} while (0)
#endif

#if defined(LOG_FATAL) || defined(LOG_ERROR) || defined(LOG_WARN) || defined(LOG_INFO) || defined(LOG_DEBUG) || defined(LOG_TRACE)
#define log_fatal(...) log_log(FATAL, __VA_ARGS__)
#else
#define log_fatal(...) do {} while (0)
#endif

void log_log(log_level_t level, const char* fmt, ...);
int log_add_fp(FILE *fp);
void log_to_console(bool should_log);

#endif
