#pragma once

#include <pthread.h>

#if 0
extern pthread_mutex_t house_mutex;
extern pthread_cond_t house_condition;
extern volatile int stop_simulation_flag;
#endif

void stop_house_simulations(void);

/**
 * @brief Continuously simulates data for the provided house
 * 
 * @param house_data House data of type house_data_t*
 */
void *houseworker_thread(void *house_data_arg);
