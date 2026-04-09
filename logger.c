// logger.c

#include "logger.h"
#include <stdarg.h>

/* ================= GLOBALS ================= */

FILE* log_file = NULL;
pthread_mutex_t log_lock;

static int logging_enabled = 1;
static int current_log_level = 0; // 0=INFO, 1=WARN, 2=ERROR

/* ================= INTERNAL ================= */

static int level_to_int(const char* level) {
    if (strcmp(level, LOG_INFO) == 0) return 0;
    if (strcmp(level, LOG_WARN) == 0) return 1;
    if (strcmp(level, LOG_ERROR) == 0) return 2;
    return 0;
}

static void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

/* ================= INIT ================= */

int init_logger(const char* filename) {
    log_file = fopen(filename, "a");
    if (!log_file) {
        perror("log file");
        return FAILURE;
    }

    pthread_mutex_init(&log_lock, NULL);

    log_info("Logger initialized");
    return SUCCESS;
}

void close_logger(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
    pthread_mutex_destroy(&log_lock);
}

/* ================= CORE LOG ================= */

void log_event(const char* level, const char* message) {
    if (!logging_enabled || !log_file) return;

    int lvl = level_to_int(level);
    if (lvl < current_log_level) return;

    pthread_mutex_lock(&log_lock);

    char timebuf[64];
    get_timestamp(timebuf, sizeof(timebuf));

    fprintf(log_file, "[%s] [%s] %s\n", timebuf, level, message);
    fflush(log_file);

    pthread_mutex_unlock(&log_lock);
}

/* ================= WRAPPERS ================= */

void log_info(const char* message) {
    log_event(LOG_INFO, message);
}

void log_warn(const char* message) {
    log_event(LOG_WARN, message);
}

void log_error(const char* message) {
    log_event(LOG_ERROR, message);
}

/* ================= FORMAT LOG ================= */

void log_format(const char* level, const char* fmt, ...) {
    if (!logging_enabled || !log_file) return;

    int lvl = level_to_int(level);
    if (lvl < current_log_level) return;

    pthread_mutex_lock(&log_lock);

    char timebuf[64];
    char msgbuf[MAX_LOG_LINE];

    get_timestamp(timebuf, sizeof(timebuf));

    va_list args;
    va_start(args, fmt);
    vsnprintf(msgbuf, sizeof(msgbuf), fmt, args);
    va_end(args);

    fprintf(log_file, "[%s] [%s] %s\n", timebuf, level, msgbuf);
    fflush(log_file);

    pthread_mutex_unlock(&log_lock);
}

/* ================= SETTINGS ================= */

void set_log_enabled(int enabled) {
    logging_enabled = enabled;
}

void set_log_level(const char* level) {
    current_log_level = level_to_int(level);
}