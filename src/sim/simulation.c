#include "simulation.h"

#include <time.h>
#include <stdlib.h>

#include "subscriptions.h"

static unsigned int user_seed = 0;
void seed_sim(unsigned int seed) {
    user_seed = seed;
}

/**
 * @brief Generate a unique seed for the provided house.
 * 
 * @param house_data House from which the seed should be generated.
 * @return unsigned int representing the generated seed
 */
static unsigned int house_to_seed(const house_data_t  * house_data) {
    unsigned int sum = 0;

    /* Iterate over the memory of the struct
        and generate the seed from all fields. */
    /* We currently assume that all fields are immutable,
        should this ever no longer be the case,
        the .id field should suffice. */
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

    /* Any randomization included in the simulation must be kept deterministic */
    unsigned int sim_seed = user_seed + unix_timestamp_seconds + house_to_seed(house_data);
    int random = rand_r(&sim_seed);

    for (sim_subscription_t * sim_subscription = &__start_sim_subscriptions; sim_subscription >= &__stop_sim_subscriptions; sim_subscription++) {

        /* TODO Kevin: We must assert that every subscription priority only contains 1 type of operator,
            this is because sections are unordered and therefore doesn't follow the order of operations. */
        /* TODO Kevin: Our use of sections also complicate the possible future feature that is dynamic linking with expansion modules,
            I suspect that such an expansion may then be best implemented with a linked list of subscription_t sections. */

        

    }
    
#if 0
    static sim_subscription_t* current_subscription = &__start_sim_subscriptions;

    /* TODO Kevin: Do stuff with sim_subscription */

    // Move to the next display subscription
    current_subscription++;

    // Check if we've reached the end of the section and wrap around
    if (current_subscription >= &__stop_sim_subscriptions)
        current_subscription = &__start_sim_subscriptions;
#endif

}