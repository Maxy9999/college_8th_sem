// auth.c

#include "auth.h"
#include "db.h"
#include "logger.h"

/* ================= GLOBALS ================= */

static user_t users[MAX_USERS];
static int user_count = 0;

/* ================= INIT ================= */

int init_auth_system(void) {
    if (load_users(users, &user_count) != SUCCESS) {
        log_error("Failed to load users");
        return FAILURE;
    }

    log_info("Authentication system initialized");
    return SUCCESS;
}

int reload_users(void) {
    return init_auth_system();
}

void cleanup_auth_system(void) {
    user_count = 0;
}

/* ================= CORE AUTH ================= */

int validate_credentials(const char* username,
                         const char* password,
                         char* role_out) {

    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0) {
            
            safe_strcpy(role_out, users[i].role, MAX_ROLE_LEN);
            return SUCCESS;
        }
    }

    return FAILURE;
}

/*
Expected login command format:
LOGIN <username> <password>
*/
int authenticate_user(const char* login_cmd, char* role_out) {
    char cmd[16], username[MAX_NAME_LEN], password[MAX_NAME_LEN];

    if (sscanf(login_cmd, "%s %s %s", cmd, username, password) != 3) {
        log_warn("Invalid login format");
        return FAILURE;
    }

    if (strcmp(cmd, "LOGIN") != 0) {
        log_warn("Invalid login command keyword");
        return FAILURE;
    }

    if (validate_credentials(username, password, role_out) != SUCCESS) {
        log_warn("Invalid username/password");
        return FAILURE;
    }

    log_info("User authenticated successfully");
    return SUCCESS;
}

/* ================= ROLE CHECK ================= */

int is_valid_role(const char* role) {
    return (strcmp(role, ROLE_ADMIN) == 0 ||
            strcmp(role, ROLE_DOCTOR) == 0 ||
            strcmp(role, ROLE_RECEPTION) == 0);
}

int has_role(const char* role, const char* required_role) {
    return strcmp(role, required_role) == 0;
}

/*
Basic permission mapping:
ADMIN → all
DOCTOR → COMPLETE, MISS, SHOW, SHOW_HISTORY
RECEPTION → ADD, SHOW, SHOW_MISSED
*/
int is_authorized(const char* role, const char* command) {

    if (has_role(role, ROLE_ADMIN)) return 1;

    if (has_role(role, ROLE_DOCTOR)) {
        if (strcmp(command, "COMPLETE") == 0 ||
            strcmp(command, "MISS") == 0 ||
            strcmp(command, "SHOW") == 0 ||
            strcmp(command, "SHOW_HISTORY") == 0)
            return 1;
    }

    if (has_role(role, ROLE_RECEPTION)) {
        if (strcmp(command, "ADD") == 0 ||
            strcmp(command, "SHOW") == 0 ||
            strcmp(command, "SHOW_MISSED") == 0)
            return 1;
    }

    return 0;
}

/* ================= SESSION HELPERS ================= */

void init_auth_ctx(auth_ctx_t* ctx) {
    if (!ctx) return;

    ctx->is_authenticated = 0;
    ctx->username[0] = '\0';
    ctx->role[0] = '\0';
}

void set_authenticated(auth_ctx_t* ctx,
                       const char* username,
                       const char* role) {
    if (!ctx) return;

    ctx->is_authenticated = 1;
    safe_strcpy(ctx->username, username, MAX_NAME_LEN);
    safe_strcpy(ctx->role, role, MAX_ROLE_LEN);
}

void clear_auth_ctx(auth_ctx_t* ctx) {
    if (!ctx) return;

    ctx->is_authenticated = 0;
    ctx->username[0] = '\0';
    ctx->role[0] = '\0';
}
