// config.h

#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"

/* ================= CONFIG FILE ================= */

#define CONFIG_FILE "config.txt"

/* ================= DEFAULT VALUES ================= */

#define DEFAULT_PORT 8080
#define DEFAULT_MAX_CLIENTS 50
#define DEFAULT_DOCTOR_THREADS 2
#define DEFAULT_LOG_LEVEL "INFO"

/* ================= CONFIG STRUCT ================= */

typedef struct {
    int port;
    int max_clients;
    int doctor_threads;
    char log_level[MAX_ROLE_LEN];
} config_t;

/* ================= GLOBAL CONFIG ================= */

// Global config instance (defined in config.c)
extern config_t global_config;

/* ================= CORE FUNCTIONS ================= */

// Load configuration from file
int load_config(const char* filename);

// Reload config (optional)
int reload_config(void);

/* ================= GETTERS ================= */

// Get server port
int get_config_port(void);

// Get max clients
int get_config_max_clients(void);

// Get number of doctor threads
int get_config_doctor_threads(void);

// Get log level
const char* get_config_log_level(void);

/* ================= INTERNAL ================= */

// Parse a single key=value line
int parse_config_line(const char* line);

#endif