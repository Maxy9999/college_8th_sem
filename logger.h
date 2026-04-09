// logger.h

#ifndef LOGGER_H
#define LOGGER_H

#include "common.h"

/* ================= LOG CONFIG ================= */

#define MAX_LOG_LINE 1024

/* ================= INITIALIZATION ================= */

// Initialize logger with file path (e.g., "server.log")
int init_logger(const char* filename);

// Close logger (flush + close file)
void close_logger(void);

/* ================= LOG FUNCTIONS ================= */

// Generic log function
void log_event(const char* level, const char* message);

// Convenience wrappers
void log_info(const char* message);
void log_warn(const char* message);
void log_error(const char* message);

/* ================= ADVANCED ================= */

// Log with formatted string (like printf)
void log_format(const char* level, const char* fmt, ...);

// Enable/disable logging dynamically
void set_log_enabled(int enabled);

// Set minimum log level (INFO/WARN/ERROR)
void set_log_level(const char* level);

/* ================= INTERNAL STATE ================= */

// Global log file pointer (defined in logger.c)
extern FILE* log_file;

// Mutex for thread-safe logging
extern pthread_mutex_t log_lock;

#endif