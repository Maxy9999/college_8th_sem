// auth.h

#ifndef AUTH_H
#define AUTH_H

#include "common.h"

/* ================= CONSTANTS ================= */

#define MAX_USERS 100

/* ================= AUTH CONTEXT ================= */

// Optional: per-client auth context
typedef struct {
    int is_authenticated;
    char username[MAX_NAME_LEN];
    char role[MAX_ROLE_LEN];
} auth_ctx_t;

/* ================= INITIALIZATION ================= */

// Load users from DB (called at server start)
int init_auth_system(void);

// Reload users (optional, for dynamic updates)
int reload_users(void);

// Cleanup (if needed)
void cleanup_auth_system(void);

/* ================= AUTH OPERATIONS ================= */

// Parse and authenticate using command:
// "LOGIN <username> <password>"
// On success, fills role_out and returns SUCCESS (0)
int authenticate_user(const char* login_cmd, char* role_out);

// Direct validation (used internally / by other modules)
int validate_credentials(const char* username,
                         const char* password,
                         char* role_out);

/* ================= ROLE & PERMISSIONS ================= */

// Check if a role is valid
int is_valid_role(const char* role);

// Check if role has permission for a command
// e.g., "ADD", "COMPLETE", "SHOW", etc.
int is_authorized(const char* role, const char* command);

// Utility: role comparison (case-sensitive)
int has_role(const char* role, const char* required_role);

/* ================= SESSION HELPERS ================= */

// Initialize auth context
void init_auth_ctx(auth_ctx_t* ctx);

// Set authenticated session
void set_authenticated(auth_ctx_t* ctx,
                       const char* username,
                       const char* role);

// Reset/clear session
void clear_auth_ctx(auth_ctx_t* ctx);

#endif