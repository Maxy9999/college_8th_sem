// command.c

#include "command.h"
#include "followup.h"
#include "auth.h"
#include "logger.h"

/* ================= TOKENIZER ================= */

int tokenize_command(char* input, char* tokens[], int max_tokens) {
    int count = 0;
    char* token = strtok(input, " ");

    while (token && count < max_tokens) {
        tokens[count++] = token;
        token = strtok(NULL, " ");
    }

    return count;
}

/* ================= COMMAND TYPE ================= */

command_type_t get_command_type(const char* cmd) {
    if (strcmp(cmd, "ADD") == 0) return CMD_ADD;
    if (strcmp(cmd, "COMPLETE") == 0) return CMD_COMPLETE;
    if (strcmp(cmd, "MISS") == 0) return CMD_MISS;
    if (strcmp(cmd, "HANDLE_MISSED") == 0) return CMD_HANDLE_MISSED;
    if (strcmp(cmd, "UNDO") == 0) return CMD_UNDO;
    if (strcmp(cmd, "SHOW") == 0) return CMD_SHOW;
    if (strcmp(cmd, "SHOW_MISSED") == 0) return CMD_SHOW_MISSED;
    if (strcmp(cmd, "SHOW_HISTORY") == 0) return CMD_SHOW_HISTORY;
    if (strcmp(cmd, "EXIT") == 0) return CMD_EXIT;

    return CMD_INVALID;
}

/* ================= VALIDATION ================= */

int validate_command(command_type_t type, char* tokens[], int token_count) {
    switch (type) {
        case CMD_ADD:
            return (token_count == 5); // ADD id name doctor day

        case CMD_COMPLETE:
        case CMD_MISS:
            return (token_count == 2);

        case CMD_HANDLE_MISSED:
        case CMD_UNDO:
        case CMD_SHOW:
        case CMD_SHOW_MISSED:
        case CMD_SHOW_HISTORY:
        case CMD_EXIT:
            return (token_count == 1);

        default:
            return 0;
    }
}

/* ================= AUTHORIZATION ================= */

int check_permission(const char* role, command_type_t type) {
    const char* cmd_str = command_to_string(type);
    return is_authorized(role, cmd_str);
}

/* ================= EXECUTION ================= */

int execute_command(command_type_t type,
                    char* tokens[],
                    int token_count,
                    char* output) {

    int id, day;

    switch (type) {

        case CMD_ADD:
            id = atoi(tokens[1]);
            day = atoi(tokens[4]);

            if (add_followup(id, tokens[2], tokens[3], day) == SUCCESS)
                sprintf(output, "Patient added\n");
            else
                sprintf(output, "Add failed\n");
            break;

        case CMD_COMPLETE:
            id = atoi(tokens[1]);
            if (complete_visit(id) == SUCCESS)
                sprintf(output, "Visit completed\n");
            else
                sprintf(output, "Complete failed\n");
            break;

        case CMD_MISS:
            id = atoi(tokens[1]);
            if (miss_visit(id) == SUCCESS)
                sprintf(output, "Visit marked missed\n");
            else
                sprintf(output, "Miss failed\n");
            break;

        case CMD_HANDLE_MISSED:
            if (handle_missed() == SUCCESS)
                sprintf(output, "Missed handled\n");
            else
                sprintf(output, "No missed patients\n");
            break;

        case CMD_UNDO:
            if (undo_complete() == SUCCESS)
                sprintf(output, "Undo successful\n");
            else
                sprintf(output, "Undo failed\n");
            break;

        case CMD_SHOW:
            show_schedule(output, MAX_BUFFER);
            break;

        case CMD_SHOW_MISSED:
            show_missed(output, MAX_BUFFER);
            break;

        case CMD_SHOW_HISTORY:
            show_history(output, MAX_BUFFER);
            break;

        case CMD_EXIT:
            sprintf(output, "Goodbye\n");
            break;

        default:
            sprintf(output, "Invalid command\n");
            return FAILURE;
    }

    return SUCCESS;
}

/* ================= MAIN PROCESS ================= */

int process_command(const char* input, char* output, const char* role) {
    char buffer[MAX_COMMAND_LEN];
    char* tokens[MAX_TOKENS];

    safe_strcpy(buffer, input, sizeof(buffer));

    int token_count = tokenize_command(buffer, tokens, MAX_TOKENS);

    if (token_count == 0) {
        sprintf(output, "Empty command\n");
        return FAILURE;
    }

    command_type_t type = get_command_type(tokens[0]);

    if (type == CMD_INVALID) {
        sprintf(output, "Unknown command\n");
        return FAILURE;
    }

    if (!validate_command(type, tokens, token_count)) {
        sprintf(output, "Invalid syntax\n");
        return FAILURE;
    }

    if (!check_permission(role, type)) {
        sprintf(output, "Permission denied\n");
        return FAILURE;
    }

    log_info(tokens[0]);

    return execute_command(type, tokens, token_count, output);
}

/* ================= UTILITY ================= */

const char* command_to_string(command_type_t type) {
    switch (type) {
        case CMD_ADD: return "ADD";
        case CMD_COMPLETE: return "COMPLETE";
        case CMD_MISS: return "MISS";
        case CMD_HANDLE_MISSED: return "HANDLE_MISSED";
        case CMD_UNDO: return "UNDO";
        case CMD_SHOW: return "SHOW";
        case CMD_SHOW_MISSED: return "SHOW_MISSED";
        case CMD_SHOW_HISTORY: return "SHOW_HISTORY";
        case CMD_EXIT: return "EXIT";
        default: return "INVALID";
    }
}
