#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "house_worker.h"
#include "simulation.h"

#include "subscriptions.h"

#define verbose 1

#define START_TIMESTAMP 1700143038

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
    
    pthread_mutex_lock(&house_mutex);

    // const time_t unix_timestamp_seconds = time(NULL);
    time_t unix_timestamp_seconds = START_TIMESTAMP;

    // Use a loop to periodically check the house_condition without blocking
    while (stop_simulation_flag == 0) {

#if 0
        // Perform some work
        printf("Thread %ld (for house_id %d) is doing some work while periodically checking for the event...\n", pthread_self(), house_data->id);
        // You can add more work here
#endif

        simulation_step(house_data, unix_timestamp_seconds);

        unix_timestamp_seconds += SIM_STEP_SIZE;

        // Wait for the house_condition variable with a timeout of zero
        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);

        // Set timeout to zero for non-blocking behavior
        timeout.tv_sec = 0;
        timeout.tv_nsec = 0;

        int result = pthread_cond_timedwait(&house_condition, &house_mutex, &timeout);

        // Check if the house_condition is met after the wait
        if (result == 0 && stop_simulation_flag == 1) {
            // Event occurred, break out of the loop
            break;
        }
    }

    // Event occurred, do something
    printf("Event occurred in thread %ld (for house_id %d)\n", pthread_self(), house_data->id);

    pthread_mutex_unlock(&house_mutex);

    return NULL;
}