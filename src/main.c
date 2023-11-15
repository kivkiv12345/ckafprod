#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cJSON.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>

#include "utils.h"
#include "house_worker.h"

#define HOUSES_CLEANUP() \
    cJSON_Delete(houses_json); \
    free(houses_str); \
    houses_str = NULL;

#define verbose 1

void sigintHandler(int signal) {
    printf("Received SIGINT. Stopping house simulations...\n");

    stop_house_simulations();

    // TODO Kevin: What should be done here???
}

int main(void) {

#if 1
    if (signal(SIGINT, sigintHandler) == SIG_ERR) {
        exit(-1);
    }
#else
    struct sigaction sigterm_action;
    memset(&sigterm_action, 0, sizeof(sigterm_action));
    sigterm_action.sa_handler = &sigintHandler;
    sigterm_action.sa_flags = 0;
#endif

    char * houses_str = read_entire_house("houses.json");

    if (houses_str == NULL) {
        /* Error should've been printed by read_entire_house() */
        exit(1);
    }

    cJSON * houses_json = cJSON_Parse(houses_str);
    if (houses_json == NULL) {

        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        /* We may use HOUSES_CLEANUP even though (houses_json == NULL) */
        HOUSES_CLEANUP();
        exit(2);
    }

    if (!cJSON_IsArray(houses_json)) {
        fprintf(stderr, "Root JSON object should be an array of house objects");
        HOUSES_CLEANUP();
        exit(3);
    }

    unsigned int num_houses = cJSON_GetArraySize(houses_json);
    pthread_t house_thread_pool[num_houses];
    house_data_t house_data[num_houses];

    if (verbose)
        printf("Found %d houses\n", num_houses);

    int i = 0;  // TODO Kevin: If we're forking cJSON anyway, we could add a cJSON_ArrayForEachEnumerate();
    cJSON * house_json = NULL;
    cJSON_ArrayForEach(house_json, houses_json) {

        cJSON *house_id = cJSON_GetObjectItemCaseSensitive(house_json, "id");
        cJSON *num_adults = cJSON_GetObjectItemCaseSensitive(house_json, "no_adults");
        cJSON *num_children = cJSON_GetObjectItemCaseSensitive(house_json, "no_children");
        cJSON *house_size_m2 = cJSON_GetObjectItemCaseSensitive(house_json, "house_size_m2");
        cJSON *no_electric_cars = cJSON_GetObjectItemCaseSensitive(house_json, "no_electric_cars");

#if 1  /* Check house for valid fields/types */
        if (!cJSON_IsNumber(house_id)) {
            HOUSES_CLEANUP();
            exit(4);
        }

        if (!cJSON_IsNumber(num_adults)) {
            HOUSES_CLEANUP();
            exit(5);
        }

        if (!cJSON_IsNumber(num_children)) {
            HOUSES_CLEANUP();
            exit(6);
        }

        if (!cJSON_IsNumber(house_size_m2)) {
            HOUSES_CLEANUP();
            exit(7);
        }
        
        if (!cJSON_IsNumber(no_electric_cars)) {
            HOUSES_CLEANUP();
            exit(8);
        }
#endif

        assert(i < num_houses);

        /* Start house thread */
        house_data[i] = (house_data_t){
            .id = (unsigned int)cJSON_GetNumberValue(house_id),
            .num_adults = (unsigned int)cJSON_GetNumberValue(num_adults),
            .num_children = (unsigned int)cJSON_GetNumberValue(num_children),
            .house_size_m2 = (unsigned int)cJSON_GetNumberValue(house_size_m2),
            .no_electric_cars = (unsigned int)cJSON_GetNumberValue(no_electric_cars),
        };
        if ((pthread_create(&house_thread_pool[i], NULL, houseworker_thread, &house_data[i])) != 0) {
            HOUSES_CLEANUP();
            exit(9);
        }
        i++;
    }

    // Wait for all threads to finish
    for (int i = 0; i < num_houses; ++i) {
        pthread_join(house_thread_pool[i], NULL);
    }

    // Cleanup
#if 0
    for (int i = 0; i < num_houses; ++i) {
        free(house_thread_pool[i]);
    }
#endif

    return 0;

}