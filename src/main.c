#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cJSON.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <getopt.h>
#include <ctype.h>

#include "utils.h"
#include "json_parsing.h"
#include "sim/simulation.h"
#include "sim/house_worker.h"
#include "sim/subscriptions.h"

#define HOUSES_CLEANUP() \
    cJSON_Delete(houses_json); \
    free(houses_str); \
    houses_str = NULL;

#define verbose 1

static pthread_mutex_t house_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t house_condition = PTHREAD_COND_INITIALIZER;

static volatile int stop_simulation_flag = 0;

static unsigned int parse_seedstr(char * seedstr) {
    /* We trust getopt not to give us an unbounded string */
    const unsigned seedlen = strlen(seedstr);
    unsigned seedsum = 0;
    
    for (size_t i = 0; i < seedlen; i++) {
        seedsum += seedstr[i];
    }
    
    return seedsum;
}

static void sigintHandler(int signal) {
    pthread_mutex_lock(&house_mutex);

    stop_simulation_flag = 1;
    stop_house_simulations();

    // Signal waiting threads
    pthread_cond_signal(&house_condition);

    pthread_mutex_unlock(&house_mutex);

    // TODO Kevin: What more should be done here???
}

int main(int argc, char **argv) {

    int option;
    struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {"seed", required_argument, 0, 's'},
        {0, 0, 0, 0} // Indicates the end of options
    };
    
    unsigned should_exit = 0;

    while ((option = getopt_long(argc, argv, "hvs:", long_options, NULL)) != -1) {
        switch (option) {
            case 'h':
                printf("Help message\n");  // TODO Kevin: Write help message
                should_exit = 1;
                break;
            case 'v':
                /* version_string is generated by meson.build with vcs_tag() */
                extern const char *version_string;
                printf("Version %s\n", version_string);
                should_exit = 1;
                break;
            case 's':
                printf("Seeding simulation with string: %s\n", optarg);
                seed_sim(parse_seedstr(optarg));
                break;
            case '?':
                // Error handling goes here
                if (optopt == 's') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                } else {
                    fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                }
                exit(-1);
                break;
            default:
                break;
        }
    }

    if (should_exit) {
        exit(0);
    }

#if 0  // Positional arguments can be handled here
    // Process positional arguments
    for (int i = optind; i < argc; ++i) {
        printf("Positional argument: %s\n", argv[i]);
    }
#endif

#if 1
    if (signal(SIGINT, sigintHandler) == SIG_ERR) {
        exit(-2);
    }
#else  /* Using sigaction is supposedly better */
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
        if (error_ptr != NULL) {
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

    if (verbose) {
        printf("Found %d houses\n", num_houses);
        printf("Press Ctrl+C to interrupt. Setting up...\n");
    }

    int i = 0;  // TODO Kevin: If we're forking lib/cJSON anyway, we could add a cJSON_ArrayForEachEnumerate();
    cJSON * house_json = NULL;
    cJSON_ArrayForEach(house_json, houses_json) {

        assert(i < num_houses);

        int validation_result = housejson_parse(house_json, &house_data[i]);
        if (validation_result != 0) {
            fprintf(stderr, "House JSON failed validation, result code %d\n", validation_result);
            HOUSES_CLEANUP();
            exit(validation_result);
        }

        if ((pthread_create(&house_thread_pool[i], NULL, houseworker_thread, &house_data[i])) != 0) {
            fprintf(stderr, "Failed to create house thread, number %d\n", i);
            HOUSES_CLEANUP();
            exit(9);
        }
        i++;
    }

    /* TODO Kevin: We should probably support the program ending naturally at some simulated end-date. */
    /* Run continuously while waiting for user interrupt/SIGINT */
    pthread_mutex_lock(&house_mutex);
    while (stop_simulation_flag == 0) {
        pthread_cond_wait(&house_condition, &house_mutex);
    }
    printf("Received SIGINT. Stopping house simulations...\n");
    pthread_mutex_unlock(&house_mutex);

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
