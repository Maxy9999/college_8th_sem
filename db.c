// db.c

#include "db.h"
#include "logger.h"
#include "auth.h"
/* ================= GLOBAL STATE ================= */

// NOTE: These should ideally be fetched from followup.c via getters,
// but for simplicity (and tight integration), we re-use structures here.

extern Node* head;                // active patients (LL)
extern Node* completed_stack[];  // stack
extern int top;

extern Node* missed_queue[];     // queue
extern int front, rear, count;

/* ================= CORE ================= */

int load_database(void) {
    log_info("Loading database...");

    load_patients(&head);
    load_history(completed_stack, &top);
    load_missed(missed_queue, &front, &rear, &count);

    log_info("Database loaded");

    return SUCCESS;
}

int save_database(void) {
    log_info("Saving database...");

    save_patients(head);
    save_history(completed_stack, top);
    save_missed(missed_queue, front, rear, count);

    log_info("Database saved");

    return SUCCESS;
}

void close_database(void) {
    save_database();
}

/* ================= PATIENTS ================= */

int save_patients(Node* head_ref) {
    FILE* f = fopen(PATIENTS_DB_FILE, "wb");
    if (!f) return FAILURE;

    Node* curr = head_ref;

    while (curr) {
        fwrite(curr, sizeof(Node), 1, f);
        curr = curr->next;
    }

    fclose(f);
    return SUCCESS;
}

int load_patients(Node** head_ref) {
    FILE* f = fopen(PATIENTS_DB_FILE, "rb");
    if (!f) return FAILURE;

    Node* temp;
    Node* last = NULL;

    while (1) {
        temp = (Node*)malloc(sizeof(Node));
        if (fread(temp, sizeof(Node), 1, f) != 1) {
            free(temp);
            break;
        }

        temp->next = NULL;

        if (*head_ref == NULL)
            *head_ref = temp;
        else
            last->next = temp;

        last = temp;
    }

    fclose(f);
    return SUCCESS;
}

/* ================= HISTORY (STACK) ================= */

int save_history(Node* stack[], int top_idx) {
    FILE* f = fopen(HISTORY_DB_FILE, "wb");
    if (!f) return FAILURE;

    fwrite(&top_idx, sizeof(int), 1, f);

    for (int i = 0; i <= top_idx; i++) {
        fwrite(stack[i], sizeof(Node), 1, f);
    }

    fclose(f);
    return SUCCESS;
}

int load_history(Node* stack[], int* top_idx) {
    FILE* f = fopen(HISTORY_DB_FILE, "rb");
    if (!f) return FAILURE;

    fread(top_idx, sizeof(int), 1, f);

    for (int i = 0; i <= *top_idx; i++) {
        stack[i] = (Node*)malloc(sizeof(Node));
        fread(stack[i], sizeof(Node), 1, f);
        stack[i]->next = NULL;
    }

    fclose(f);
    return SUCCESS;
}

/* ================= MISSED (QUEUE) ================= */

int save_missed(Node* queue[], int f_idx, int r_idx, int cnt) {
    FILE* f = fopen(MISSED_DB_FILE, "wb");
    if (!f) return FAILURE;

    fwrite(&f_idx, sizeof(int), 1, f);
    fwrite(&r_idx, sizeof(int), 1, f);
    fwrite(&cnt, sizeof(int), 1, f);

    for (int i = 0; i < cnt; i++) {
        int idx = (f_idx + i) % MAX_MISSED;
        fwrite(queue[idx], sizeof(Node), 1, f);
    }

    fclose(f);
    return SUCCESS;
}

int load_missed(Node* queue[], int* f_idx, int* r_idx, int* cnt) {
    FILE* f = fopen(MISSED_DB_FILE, "rb");
    if (!f) return FAILURE;

    fread(f_idx, sizeof(int), 1, f);
    fread(r_idx, sizeof(int), 1, f);
    fread(cnt, sizeof(int), 1, f);

    for (int i = 0; i < *cnt; i++) {
        int idx = (*f_idx + i) % MAX_MISSED;

        queue[idx] = (Node*)malloc(sizeof(Node));
        fread(queue[idx], sizeof(Node), 1, f);
        queue[idx]->next = NULL;
    }

    fclose(f);
    return SUCCESS;
}

/* ================= USERS ================= */

int load_users(user_t* users, int* user_count) {
    FILE* f = fopen(USERS_DB_FILE, "r");
    if (!f) return FAILURE;

    *user_count = 0;

    while (fscanf(f, "%s %s %s",
                  users[*user_count].username,
                  users[*user_count].password,
                  users[*user_count].role) == 3) {

        (*user_count)++;
        if (*user_count >= MAX_USERS) break;
    }

    fclose(f);
    return SUCCESS;
}

int validate_user(const char* username, const char* password, char* role_out) {
    user_t users[MAX_USERS];
    int count = 0;

    if (load_users(users, &count) != SUCCESS)
        return FAILURE;

    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0) {

            safe_strcpy(role_out, users[i].role, MAX_ROLE_LEN);
            return SUCCESS;
        }
    }

    return FAILURE;
}