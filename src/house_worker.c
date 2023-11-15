#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "house_worker.h"

#define verbose 1

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

static int event_flag = 0;

void stop_house_simulations(void) {
    pthread_mutex_lock(&mutex);

    // Set the event flag
    event_flag = 1;

    // Signal waiting threads
    pthread_cond_signal(&condition);

    pthread_mutex_unlock(&mutex);
}

void *houseworker_thread(void *house_data) {
    
    
    pthread_mutex_lock(&mutex);

    // Use a loop to periodically check the condition without blocking
    while (event_flag == 0) {
        // Perform some work
        printf("Thread %ld is doing some work while periodically checking for the event...\n", pthread_self());
        // You can add more work here

        // Wait for the condition variable with a timeout of zero
        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);

        // Set timeout to zero for non-blocking behavior
        timeout.tv_sec = 0;
        timeout.tv_nsec = 0;

        int result = pthread_cond_timedwait(&condition, &mutex, &timeout);

        // Check if the condition is met after the wait
        if (result == 0 && event_flag == 1) {
            // Event occurred, break out of the loop
            break;
        }
    }

    // Event occurred, do something
    printf("Event occurred in thread %ld\n", pthread_self());

    pthread_mutex_unlock(&mutex);

    return NULL;
}