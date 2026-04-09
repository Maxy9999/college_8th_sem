// db.h

#ifndef DB_H
#define DB_H

#include "common.h"
#include "followup.h"

/* ================= FILE PATHS ================= */

#define PATIENTS_DB_FILE   "data/patients.dat"
#define HISTORY_DB_FILE    "data/history.dat"
#define MISSED_DB_FILE     "data/missed.dat"
#define USERS_DB_FILE      "data/users.dat"

/* ================= DATABASE CORE ================= */

// Load all data into memory at server startup
int load_database(void);

// Save all in-memory data to files
int save_database(void);

// Cleanup DB (if needed before shutdown)
void close_database(void);

/* ================= PATIENT PERSISTENCE ================= */

// Save active patients (linked list)
int save_patients(Node* head);

// Load active patients into linked list
int load_patients(Node** head_ref);

/* ================= HISTORY (STACK) ================= */

// Save completed visits (stack)
int save_history(Node** stack, int top);

// Load completed visits into stack
int load_history(Node** stack, int* top);

/* ================= MISSED (QUEUE) ================= */

// Save missed visits (queue)
int save_missed(Node** queue, int front, int rear, int count);

// Load missed visits into queue
int load_missed(Node** queue, int* front, int* rear, int* count);

/* ================= USER DATABASE ================= */

// Load users for authentication
int load_users(user_t* users, int* user_count);

// Validate user credentials
int validate_user(const char* username, const char* password, char* role_out);

#endif