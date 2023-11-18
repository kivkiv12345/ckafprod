#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "house_worker.h"
#include "simulation.h"

#include "subscriptions.h"

#define verbose 1

// #define START_TIMESTAMP 1700143038  // Current'ish date
#define START_TIMESTAMP 1704125447  // Start of 2024

// Step size in seconds
#define SIM_STEP_SIZE 3600

static pthread_mutex_t house_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t house_condition = PTHREAD_COND_INITIALIZER;

static volatile int stop_simulation_flag = 0;


void stop_house_simulations(void) {
    pthread_mutex_lock(&house_mutex);

    // Set the event flag
    stop_simulation_flag = 1;

    // Signal waiting threads
    pthread_cond_signal(&house_condition);

    pthread_mutex_unlock(&house_mutex);
}

void *houseworker_thread(void *house_data_arg) {
    
    house_data_t * house_data = (house_data_t*)house_data_arg;
    
#undef THREADSAFE_STOPCHECK

#ifdef THREADSAFE_STOPCHECK
    pthread_mutex_lock(&house_mutex);
#endif

    // const time_t unix_timestamp_seconds = time(NULL);
    time_t unix_timestamp_seconds = START_TIMESTAMP;


    // Use a loop to periodically check the house_condition without blocking
    while (stop_simulation_flag == 0) {

        simulation_step(house_data, unix_timestamp_seconds);

        unix_timestamp_seconds += SIM_STEP_SIZE;

#ifdef THREADSAFE_STOPCHECK
        // Wait for the house_condition variable with a timeout of zero
        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);

        // Set timeout to zero for non-blocking behavior
        timeout.tv_sec = 0;
        timeout.tv_nsec = 0;

        int result = pthread_cond_timedwait(&house_condition, &house_mutex, &timeout);

        // Check if the house_condition is met after the wait
        /* TODO Kevin: Not sure if its okay to skip pthread_cond_timedwait() when only the main thread changes stop_simulation_flag,
            but the program is drastically faster if when it's skipped. */
        if (result == 0 && stop_simulation_flag == 1) {
            // Event occurred, break out of the loop
            break;
        }
#endif
    }

    // Event occurred, do something
    // printf("Event occurred in thread %ld (for house_id %d)\n", pthread_self(), house_data->id);

#ifdef THREADSAFE_STOPCHECK
    pthread_mutex_unlock(&house_mutex);
#endif

    return NULL;
}