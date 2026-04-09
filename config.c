// config.c

#include "config.h"
#include "logger.h"

/* ================= GLOBAL CONFIG ================= */

config_t global_config;

/* ================= INTERNAL ================= */

static void set_defaults(void) {
    global_config.port = DEFAULT_PORT;
    global_config.max_clients = DEFAULT_MAX_CLIENTS;
    global_config.doctor_threads = DEFAULT_DOCTOR_THREADS;
    safe_strcpy(global_config.log_level, DEFAULT_LOG_LEVEL, MAX_ROLE_LEN);
}

/* ================= PARSER ================= */

int parse_config_line(const char* line) {
    char key[64], value[64];

    if (sscanf(line, "%63[^=]=%63s", key, value) != 2)
        return FAILURE;

    if (strcmp(key, "PORT") == 0) {
        global_config.port = atoi(value);
    }
    else if (strcmp(key, "MAX_CLIENTS") == 0) {
        global_config.max_clients = atoi(value);
    }
    else if (strcmp(key, "DOCTOR_THREADS") == 0) {
        global_config.doctor_threads = atoi(value);
    }
    else if (strcmp(key, "LOG_LEVEL") == 0) {
        safe_strcpy(global_config.log_level, value, MAX_ROLE_LEN);
    }

    return SUCCESS;
}

/* ================= LOAD ================= */

int load_config(const char* filename) {
    FILE* f = fopen(filename, "r");

    set_defaults();

    if (!f) {
        printf("Config file not found, using defaults\n");
        return SUCCESS;
    }

    char line[128];

    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);

        // skip empty lines or comments
        if (strlen(line) == 0 || line[0] == '#')
            continue;

        parse_config_line(line);
    }

    fclose(f);

    // Apply log level after loading
    set_log_level(global_config.log_level);

    log_info("Configuration loaded");

    return SUCCESS;
}

int reload_config(void) {
    return load_config(CONFIG_FILE);
}

/* ================= GETTERS ================= */

int get_config_port(void) {
    return global_config.port;
}

int get_config_max_clients(void) {
    return global_config.max_clients;
}

int get_config_doctor_threads(void) {
    return global_config.doctor_threads;
}

const char* get_config_log_level(void) {
    return global_config.log_level;
}