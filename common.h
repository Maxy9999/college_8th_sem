// common.h

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <arpa/inet.h>

/* ================= GENERAL CONSTANTS ================= */

#define MAX_BUFFER 1024
#define MAX_CLIENTS 100

#define MAX_NAME_LEN 50
#define MAX_ROLE_LEN 20

/* ================= STATUS CODES ================= */

#define SUCCESS 0
#define FAILURE -1

/* ================= USER ROLES ================= */

#define ROLE_ADMIN "ADMIN"
#define ROLE_DOCTOR "DOCTOR"
#define ROLE_RECEPTION "RECEPTION"

/* ================= BOOKING STATUS ================= */

#define CONFIRMED 0
#define WAITING 1
#define CANCELED 2

/* ================= LOG LEVELS ================= */

#define LOG_INFO  "INFO"
#define LOG_WARN  "WARN"
#define LOG_ERROR "ERROR"

/* ================= COMMON STRUCTS ================= */

// Generic response structure (optional use)
typedef struct {
    int status;
    char message[MAX_BUFFER];
} response_t;

// User structure (for authentication)
typedef struct {
    char username[MAX_NAME_LEN];
    char password[MAX_NAME_LEN];
    char role[MAX_ROLE_LEN];
} user_t;

/* ================= UTILITY FUNCTIONS ================= */

// Safe string copy
static inline void safe_strcpy(char* dest, const char* src, size_t size) {
    strncpy(dest, src, size - 1);
    dest[size - 1] = '\0';
}

// Remove newline from string
static inline void trim_newline(char* str) {
    str[strcspn(str, "\n")] = '\0';
}

#endif