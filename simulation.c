// simulation.c

#include "simulation.h"
#include "followup.h"
#include "config.h"
#include "logger.h"

#include <time.h>

/* ================= GLOBALS ================= */

sem_t patient_available;
pthread_mutex_t simulation_lock;

static pthread_t doctor_threads[MAX_DOCTORS];
static pthread_t generator_thread;

int simulation_running = 0;

/* ================= UTIL ================= */

void sim_sleep(int milliseconds) {
    usleep(milliseconds * 1000);
}

/* ================= PATIENT GENERATOR ================= */

void* patient_generator(void* arg) {
    int id_counter = 1000;

    while (simulation_running) {
        int id = id_counter++;
        char pname[MAX_NAME_LEN];
        char dname[MAX_NAME_LEN];

        snprintf(pname, sizeof(pname), "Patient_%d", id);
        snprintf(dname, sizeof(dname), "Doctor_%d", (id % 3) + 1);

        pthread_mutex_lock(&simulation_lock);

        add_followup(id, pname, dname, rand() % 7 + 1);

        log_info("Generated new patient");

        pthread_mutex_unlock(&simulation_lock);

        sem_post(&patient_available);

        sim_sleep(2000); // every 2 seconds
    }

    return NULL;
}

/* ================= DOCTOR THREAD ================= */

void* doctor_worker(void* arg) {
    int doctor_id = *(int*)arg;

    char logmsg[128];

    while (simulation_running) {
        sem_wait(&patient_available);

        pthread_mutex_lock(&simulation_lock);

        // Randomly decide action
        int action = rand() % 2;

        if (action == 0) {
            // Try completing (random ID guess)
            int pid = rand() % 1100;

            if (complete_visit(pid) == SUCCESS) {
                snprintf(logmsg, sizeof(logmsg),
                         "Doctor %d completed patient %d",
                         doctor_id, pid);
                log_info(logmsg);
            }
        } else {
            int pid = rand() % 1100;

            if (miss_visit(pid) == SUCCESS) {
                snprintf(logmsg, sizeof(logmsg),
                         "Doctor %d missed patient %d",
                         doctor_id, pid);
                log_warn(logmsg);
            }
        }

        pthread_mutex_unlock(&simulation_lock);

        sim_sleep(3000);
    }

    return NULL;
}

/* ================= START / STOP ================= */

int start_simulation(void) {
    if (simulation_running) return SUCCESS;

    simulation_running = 1;

    pthread_mutex_init(&simulation_lock, NULL);
    sem_init(&patient_available, 0, 0);

    srand(time(NULL));

    // Start generator thread
    pthread_create(&generator_thread, NULL, patient_generator, NULL);

    // Start doctor threads
    int num_doctors = get_config_doctor_threads();
    if (num_doctors > MAX_DOCTORS) num_doctors = MAX_DOCTORS;

    for (int i = 0; i < num_doctors; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&doctor_threads[i], NULL, doctor_worker, id);
    }

    log_info("Simulation started");

    return SUCCESS;
}

int stop_simulation(void) {
    if (!simulation_running) return SUCCESS;

    simulation_running = 0;

    // Wake up threads
    for (int i = 0; i < MAX_DOCTORS; i++) {
        sem_post(&patient_available);
    }

    pthread_join(generator_thread, NULL);

    int num_doctors = get_config_doctor_threads();
    if (num_doctors > MAX_DOCTORS) num_doctors = MAX_DOCTORS;

    for (int i = 0; i < num_doctors; i++) {
        pthread_join(doctor_threads[i], NULL);
    }

    sem_destroy(&patient_available);
    pthread_mutex_destroy(&simulation_lock);

    log_info("Simulation stopped");

    return SUCCESS;
}