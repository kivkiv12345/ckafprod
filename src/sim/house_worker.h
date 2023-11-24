#pragma once

#include "ckafprod_config.h"

#include <pthread.h>

#include "../victoria_metrics.h"

#if 0
extern pthread_mutex_t house_mutex;
extern pthread_cond_t house_condition;
extern volatile int stop_simulation_flag;
#endif

void stop_house_simulations(void);

/* TODO Kevin: Consider whether we wanna go through the hassle and make these const. */
typedef struct {
    house_data_t house_data;
#ifdef USE_VM
    vm_init_args_t * vm_args;
#endif
} houseworker_thread_args_t;

/**
 * @brief Continuously simulates data for the provided house
 * 
 * @param house_data House data of type house_data_t*
 */
void *houseworker_thread(void *houseworker_thread_arg);
