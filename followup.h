// followup.h

#ifndef FOLLOWUP_H
#define FOLLOWUP_H

#include "common.h"

/* ================= CONSTANTS ================= */

#define MAX_NAME_LEN 50
#define MAX_COMPLETED 100
#define MAX_MISSED 100

/* ================= DATA STRUCTURES ================= */

// Patient Node (Linked List)
typedef struct Node {
    int patient_id;
    char patient_name[MAX_NAME_LEN];
    char doctor_name[MAX_NAME_LEN];
    int followup_day;
    struct Node* next;
} Node;

/* ================= CORE OPERATIONS ================= */

// Add new patient follow-up
int add_followup(int patient_id, const char* patient_name,
                 const char* doctor_name, int followup_day);

// Mark visit as completed (moves to stack)
int complete_visit(int patient_id);

// Mark visit as missed (moves to queue)
int miss_visit(int patient_id);

// Handle missed visit (re-add to active list)
int handle_missed(void);

// Undo last completed visit (pop from stack)
int undo_complete(void);

/* ================= DISPLAY FUNCTIONS ================= */

// Show active follow-ups (linked list)
int show_schedule(char* output_buffer, size_t output_size);

// Show missed follow-ups (queue)
int show_missed(char* output_buffer, size_t output_size);

// Show completed history (stack)
int show_history(char* output_buffer, size_t output_size);

/* ================= INITIALIZATION ================= */

// Initialize data structures
void init_followup_system(void);

// Cleanup memory
void cleanup_followup_system(void);

/* ================= SYNCHRONIZATION ================= */

// Lock for thread-safe operations
extern pthread_mutex_t followup_lock;

#endif
