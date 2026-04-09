// followup.c

#include "followup.h"
#include "logger.h"

/* ================= GLOBAL STRUCTURES ================= */

// Linked List (Active Patients)
 Node* head = NULL;

// Stack (Completed Visits)
 Node* completed_stack[MAX_COMPLETED];
int top = -1;

// Queue (Missed Visits)
 Node* missed_queue[MAX_MISSED];
int front = 0, rear = 0, count = 0;

// Mutex for thread safety
pthread_mutex_t followup_lock;

/* ================= INTERNAL HELPERS ================= */

static int append_output(char* out, size_t out_size, const char* text) {
    size_t used;
    size_t remaining;
    int written;

    if (!out || out_size == 0 || !text) return 0;

    used = strlen(out);
    if (used >= out_size - 1) return 0;

    remaining = out_size - used;
    written = snprintf(out + used, remaining, "%s", text);

    return (written >= 0 && (size_t)written < remaining);
}

static void append_truncation_notice(char* out, size_t out_size) {
    const char* notice = "... output truncated ...\n";
    size_t notice_len = strlen(notice);

    if (!out || out_size == 0) return;

    if (out_size <= notice_len) {
        snprintf(out, out_size, "%s", notice);
        return;
    }

    size_t start = strlen(out);
    if (start > out_size - notice_len - 1)
        start = out_size - notice_len - 1;

    snprintf(out + start, out_size - start, "%s", notice);
}

static Node* create_node(int id, const char* pname, const char* dname, int day) {
    Node* n = (Node*)malloc(sizeof(Node));
    if (!n) return NULL;

    n->patient_id = id;
    safe_strcpy(n->patient_name, pname, MAX_NAME_LEN);
    safe_strcpy(n->doctor_name, dname, MAX_NAME_LEN);
    n->followup_day = day;
    n->next = NULL;

    return n;
}

static Node* remove_from_list(int patient_id) {
    Node *curr = head, *prev = NULL;

    while (curr) {
        if (curr->patient_id == patient_id) {
            if (prev == NULL)
                head = curr->next;
            else
                prev->next = curr->next;

            curr->next = NULL;
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

/* ================= INITIALIZATION ================= */

void init_followup_system(void) {
    pthread_mutex_init(&followup_lock, NULL);
    head = NULL;
    top = -1;
    front = rear = count = 0;
}

void cleanup_followup_system(void) {
    pthread_mutex_lock(&followup_lock);

    Node* curr = head;
    while (curr) {
        Node* tmp = curr;
        curr = curr->next;
        free(tmp);
    }

    for (int i = 0; i <= top; i++)
        free(completed_stack[i]);

    for (int i = 0; i < count; i++) {
        int idx = (front + i) % MAX_MISSED;
        free(missed_queue[idx]);
    }

    pthread_mutex_unlock(&followup_lock);
    pthread_mutex_destroy(&followup_lock);
}

/* ================= CORE OPERATIONS ================= */

int add_followup(int id, const char* pname, const char* dname, int day) {
    pthread_mutex_lock(&followup_lock);

    Node* n = create_node(id, pname, dname, day);
    if (!n) {
        pthread_mutex_unlock(&followup_lock);
        return FAILURE;
    }

    n->next = head;
    head = n;

    log_info("Patient added to followup");

    pthread_mutex_unlock(&followup_lock);
    return SUCCESS;
}

int complete_visit(int patient_id) {
    pthread_mutex_lock(&followup_lock);

    Node* n = remove_from_list(patient_id);
    if (!n || top >= MAX_COMPLETED - 1) {
        pthread_mutex_unlock(&followup_lock);
        return FAILURE;
    }

    completed_stack[++top] = n;

    log_info("Visit completed");

    pthread_mutex_unlock(&followup_lock);
    return SUCCESS;
}

int miss_visit(int patient_id) {
    pthread_mutex_lock(&followup_lock);

    Node* n = remove_from_list(patient_id);
    if (!n || count >= MAX_MISSED) {
        pthread_mutex_unlock(&followup_lock);
        return FAILURE;
    }

    missed_queue[rear] = n;
    rear = (rear + 1) % MAX_MISSED;
    count++;

    log_warn("Visit missed");

    pthread_mutex_unlock(&followup_lock);
    return SUCCESS;
}

int handle_missed(void) {
    pthread_mutex_lock(&followup_lock);

    if (count == 0) {
        pthread_mutex_unlock(&followup_lock);
        return FAILURE;
    }

    Node* n = missed_queue[front];
    front = (front + 1) % MAX_MISSED;
    count--;

    n->next = head;
    head = n;

    log_info("Missed patient rescheduled");

    pthread_mutex_unlock(&followup_lock);
    return SUCCESS;
}

int undo_complete(void) {
    pthread_mutex_lock(&followup_lock);

    if (top < 0) {
        pthread_mutex_unlock(&followup_lock);
        return FAILURE;
    }

    Node* n = completed_stack[top--];

    n->next = head;
    head = n;

    log_info("Undo complete visit");

    pthread_mutex_unlock(&followup_lock);
    return SUCCESS;
}

/* ================= DISPLAY ================= */

int show_schedule(char* out, size_t out_size) {
    pthread_mutex_lock(&followup_lock);

    Node* curr = head;
    char temp[128];

    if (!curr) {
        snprintf(out, out_size, "No active patients\n");
        pthread_mutex_unlock(&followup_lock);
        return SUCCESS;
    }

    out[0] = '\0';

    while (curr) {
        snprintf(temp, sizeof(temp),
                 "ID:%d Name:%s Doctor:%s Day:%d\n",
                 curr->patient_id,
                 curr->patient_name,
                 curr->doctor_name,
                 curr->followup_day);

        if (!append_output(out, out_size, temp)) {
            append_truncation_notice(out, out_size);
            break;
        }
        curr = curr->next;
    }

    pthread_mutex_unlock(&followup_lock);
    return SUCCESS;
}

int show_missed(char* out, size_t out_size) {
    pthread_mutex_lock(&followup_lock);

    char temp[128];
    out[0] = '\0';

    if (count == 0) {
        snprintf(out, out_size, "No missed patients\n");
        pthread_mutex_unlock(&followup_lock);
        return SUCCESS;
    }

    for (int i = 0; i < count; i++) {
        int idx = (front + i) % MAX_MISSED;
        Node* n = missed_queue[idx];

        snprintf(temp, sizeof(temp),
                 "ID:%d Name:%s Doctor:%s Day:%d\n",
                 n->patient_id,
                 n->patient_name,
                 n->doctor_name,
                 n->followup_day);

        if (!append_output(out, out_size, temp)) {
            append_truncation_notice(out, out_size);
            break;
        }
    }

    pthread_mutex_unlock(&followup_lock);
    return SUCCESS;
}

int show_history(char* out, size_t out_size) {
    pthread_mutex_lock(&followup_lock);

    char temp[128];
    out[0] = '\0';

    if (top < 0) {
        snprintf(out, out_size, "No completed visits\n");
        pthread_mutex_unlock(&followup_lock);
        return SUCCESS;
    }

    for (int i = top; i >= 0; i--) {
        Node* n = completed_stack[i];

        snprintf(temp, sizeof(temp),
                 "ID:%d Name:%s Doctor:%s Day:%d\n",
                 n->patient_id,
                 n->patient_name,
                 n->doctor_name,
                 n->followup_day);

        if (!append_output(out, out_size, temp)) {
            append_truncation_notice(out, out_size);
            break;
        }
    }

    pthread_mutex_unlock(&followup_lock);
    return SUCCESS;
}
