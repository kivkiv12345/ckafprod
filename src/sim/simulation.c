#include "simulation.h"

#include <time.h>
#include <stdlib.h>

#include "subscriptions.h"

static unsigned int user_seed = 0;
void seed_sim(unsigned int seed) {
    user_seed = seed;
}

static unsigned int house_to_seed(const house_data_t  * house_data) {
    unsigned int sum = 0;

    // Iterate over the memory of the struct
    for (size_t i = 0; i < sizeof(house_data_t); ++i) {
        sum += ((unsigned char *)house_data)[i];
    }

    return sum;
}

void simulation_step(const house_data_t * const house_data, const time_t unix_timestamp_seconds) {
    
    struct tm* timeinfo = localtime(&unix_timestamp_seconds);
    int month = timeinfo->tm_mon; // tm_mon is zero-based

    /**
	 * GNU Linker symbols. These will be autogenerate by GCC when using
	 * __attribute__((section("sim_subscriptions"))
	 */
	__attribute__((weak)) extern sim_subscription_t __start_sim_subscriptions;
	__attribute__((weak)) extern sim_subscription_t __stop_sim_subscriptions;

    /* Check if there are no functions in the section */
    /* Both __start_displayfuncs and __stop_displayfuncs 
        should be NULL in this case. */
    if (&__start_sim_subscriptions == &__stop_sim_subscriptions && &__start_sim_subscriptions == NULL)
        return;

    unsigned int sim_seed = user_seed + unix_timestamp_seconds + house_to_seed(house_data);
    int random = rand_r(&sim_seed);

    for (sim_subscription_t * sim_subscription = &__start_sim_subscriptions; sim_subscription >= &__stop_sim_subscriptions; sim_subscription++) {
        
    }
    
#if 0
    static sim_subscription_t* current_displayfunc = &__start_sim_subscriptions;

    /* TODO Kevin: Do stuff with sim_subscription */

    // Move to the next display function
    current_displayfunc++;

    // Check if we've reached the end of the section and wrap around
    if (current_displayfunc >= &__stop_sim_subscriptions)
        current_displayfunc = &__start_sim_subscriptions;
#endif

}