// simulation.h

#ifndef SIMULATION_H
#define SIMULATION_H

#include "common.h"
#include "followup.h"

/* ================= CONFIGURABLE PARAMETERS ================= */

// Default values (can be overridden by config module)
#define MAX_DOCTORS 5
#define MAX_PATIENT_QUEUE 100

/* ================= SIMULATION CONTROL ================= */

// Start all simulation threads
int start_simulation(void);

// Stop simulation gracefully
int stop_simulation(void);

/* ================= THREAD FUNCTIONS ================= */

// Producer thread → simulates incoming patients
void* patient_generator(void* arg);

// Consumer thread → simulates doctors processing patients
void* doctor_worker(void* arg);

/* ================= INTERNAL STATE ================= */

// Semaphore to indicate available patients
extern sem_t patient_available;

// Mutex for shared data protection
extern pthread_mutex_t simulation_lock;

// Flag to control simulation lifecycle
extern int simulation_running;

/* ================= UTILITY ================= */

// Sleep wrapper (milliseconds)
void sim_sleep(int milliseconds);

#endif