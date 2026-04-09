// command.h

#ifndef COMMAND_H
#define COMMAND_H

#include "common.h"

/* ================= CONSTANTS ================= */

#define MAX_COMMAND_LEN 256
#define MAX_TOKENS 10

/* ================= COMMAND TYPES ================= */

typedef enum {
    CMD_ADD = 0,
    CMD_COMPLETE,
    CMD_MISS,
    CMD_HANDLE_MISSED,
    CMD_UNDO,
    CMD_SHOW,
    CMD_SHOW_MISSED,
    CMD_SHOW_HISTORY,
    CMD_EXIT,
    CMD_INVALID
} command_type_t;

/* ================= CORE FUNCTION ================= */

// Main command processor
// input  → raw command string
// output → response to send back to client
// role   → user role (for authorization)
int process_command(const char* input, char* output, const char* role);

/* ================= PARSING ================= */

// Tokenize input command
int tokenize_command(char* input, char* tokens[], int max_tokens);

// Identify command type
command_type_t get_command_type(const char* cmd);

/* ================= VALIDATION ================= */

// Validate command syntax
int validate_command(command_type_t type, char* tokens[], int token_count);

// Authorization check (uses auth module internally)
int check_permission(const char* role, command_type_t type);

/* ================= EXECUTION ================= */

// Execute parsed command
int execute_command(command_type_t type,
                    char* tokens[],
                    int token_count,
                    char* output);

/* ================= UTILITIES ================= */

// Convert command type to string (for logging/debug)
const char* command_to_string(command_type_t type);

#endif