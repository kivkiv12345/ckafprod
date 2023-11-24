#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "simulation.h"
#include "house_worker.h"
#include "../victoria_metrics.h"

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

void *houseworker_thread(void *houseworker_thread_arg) {
    
    houseworker_thread_args_t * houseworker_thread_args = (houseworker_thread_args_t*)houseworker_thread_arg;
    house_data_t * house_data = (house_data_t*)&(houseworker_thread_args->house_data);
#ifdef USE_VM
    vm_init_args_t * vm_args = (vm_init_args_t*)houseworker_thread_args->vm_args;

    vm_connection_t vm_connection;

    if (vm_init(vm_args, &vm_connection) != CURLE_OK) {
        fprintf(stderr, "House ID %d couldn't connect to Victoria Metrics.\n", house_data->id);
    }
#endif

#undef THREADSAFE_STOPCHECK

#ifdef THREADSAFE_STOPCHECK
    pthread_mutex_lock(&house_mutex);
#endif

    // const time_t unix_timestamp_seconds = time(NULL);
    time_t unix_timestamp_seconds = START_TIMESTAMP;


    // Use a loop to periodically check the house_condition without blocking
    while (stop_simulation_flag == 0) {

        usage_line_t usage_line;
        simulation_step(house_data, unix_timestamp_seconds, &usage_line);
#ifdef USE_VM
        vm_add_usage_line(&usage_line, &vm_connection);
#endif
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

#ifdef USE_VM
    vm_push(&vm_connection);  // Push the current buffer, even if it is only partially filled.
    vm_cleanup(&vm_connection);
#endif

    // Event occurred, do something
    // printf("Event occurred in thread %ld (for house_id %d)\n", pthread_self(), house_data->id);

#ifdef THREADSAFE_STOPCHECK
    pthread_mutex_unlock(&house_mutex);
#endif

    return NULL;
}