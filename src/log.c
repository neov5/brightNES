#include "log.h"
#include "nes.h"
#include <stdio.h>

#define MAX_FILES 32

extern nes_state_t state;

static FILE *_fps[32];
static int _n_fps = 0;
static bool _log_stderr = true;

static const char* level_strings[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif

int log_add_fp(FILE *fp) {
    if (_n_fps == MAX_FILES) return -1;
    _fps[_n_fps++] = fp;
    return 0;
}

void log_to_console(bool should_log) {
    _log_stderr = should_log;
}

static void _log_event(log_event_t *ev) {
#ifdef LOG_USE_COLOR
    if (ev->output == stderr || ev->output == stdout) {
        fprintf(
            ev->output, "(cpu: %7llu) (ppu: %7llu) %s%-5s\x1b[0m ",
            ev->cpu_cycle, ev->ppu_cycle, 
            level_colors[ev->level], level_strings[ev->level]);
    }
    else {
        fprintf(
            ev->output, "(cpu: %-8llu) (ppu: %-8llu) %-5s ",
            state.cpu_cycle, state.ppu_cycle, 
            level_strings[ev->level]);
    }
#else
    fprintf(
        ev->output, "(cpu: %-8llu) (ppu: %-8llu) %-5s ",
        state.cpu_cycle, state.ppu_cycle, 
        level_strings[ev->level]);
#endif
    vfprintf(ev->output, ev->fmt, ev->ap);
    fprintf(ev->output, "\n");
    fflush(ev->output);
}

void log_log(log_level_t level, const char* fmt, ...) {
    log_event_t evt = {
        .fmt   = fmt,
        .cpu_cycle = state.cpu_cycle,
        .ppu_cycle = state.ppu_cycle,
        .output = stderr,
        .level = level,
    };

    if (_log_stderr) {
        va_start(evt.ap, fmt);
        _log_event(&evt);
        va_end(evt.ap);
    }

    for (int i=0; i<_n_fps; i++) {
        evt.output = _fps[i];
        va_start(evt.ap, fmt);
        _log_event(&evt);
        va_end(evt.ap);
    }
}
