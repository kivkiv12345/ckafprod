#include "simulation.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

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

    /* Any randomization included in the simulation must be kept deterministic,
        as this allows us to keep the output consistent when rerunning the simulation. */
    /* Using (user_seed + unix_timestamp_seconds + house_to_seed(house_data)) for the seed
        should allow us to run multiple threads for the same house, but with different start times.
        If any thread(s) were to catch up to the start time of any other thread, it should generate the same output. */
    unsigned int sim_seed = user_seed + unix_timestamp_seconds + house_to_seed(house_data);
    int random = rand_r(&sim_seed);

    // void apply_valid_modifier(const house_data_t * const house_data, const time_t unix_timestamp_seconds, sim_subscription_t * sim_subscription) {
    //     // printf("AAA %ld\n", unix_timestamp_seconds);
    // }

    double usage_sum = 0;

#define SUBSCRIPTION_FOREACH_PRIO(_prio) \
    /** \
	 * GNU Linker symbols. These will be autogenerate by GCC when using \
	 * __attribute__((section("sim_subscriptions")) \
	 */ \
	__attribute__((weak)) extern sim_subscription_t __start_sim_subscriptions##_prio; \
	__attribute__((weak)) extern sim_subscription_t __stop_sim_subscriptions##_prio; \
 \
    /* TODO Kevin: We must assert that every subscription priority only contains 1 type of operator, \
            this is because sections are unordered and therefore doesn't follow the order of operations. */ \
    /* TODO Kevin: Our use of sections also complicate the possible future feature that is dynamic linking with expansion modules, \
        I suspect that such an expansion may then be best implemented with a linked list of subscription_t sections. */ \
 \
    /* For some reason this 'if' is needed, even though the for-loop condition should be sufficient. \
        It can't be replaced with a guard clause, as we must allow the other priorities to run.*/ \
    if (&__start_sim_subscriptions##_prio != &__stop_sim_subscriptions##_prio && &__start_sim_subscriptions##_prio != NULL) { \
        /* printf("UUU %d %p %p\n", _prio, &__start_sim_subscriptions##_prio, &__stop_sim_subscriptions##_prio); */ \
        for (sim_subscription_t * sim_subscription = &__start_sim_subscriptions##_prio; sim_subscription < &__stop_sim_subscriptions##_prio; sim_subscription++) { \
            /* apply_valid_modifier(house_data, unix_timestamp_seconds, sim_subscription); */ \
            if (sim_subscription->operation == ADD) { \
                usage_sum += sim_subscription->modifier_func(house_data, unix_timestamp_seconds, sim_subscription); \
            } else if (sim_subscription->operation == MULTIPLY) { \
                usage_sum *= sim_subscription->modifier_func(house_data, unix_timestamp_seconds, sim_subscription); \
            } else { \
                /* TODO Kevin: Invalid operator, error handling goes here */ \
            } \
        } \
    }
    
    /* NOTE: Any priorities added to subscriptions.h must also be added here. */
    /* NOTE: Currently our sections are named as sim_subscriptionsSIM_PRIO0 when using SIM_SUBSCRIBE() (instead of sim_subscriptions0),
        It is therfore important that SUBSCRIPTION_FOREACH_PRIO() is used with the SIM_PRIO* #defines. */
    SUBSCRIPTION_FOREACH_PRIO(SIM_PRIO0);
    SUBSCRIPTION_FOREACH_PRIO(SIM_PRIO1);
    SUBSCRIPTION_FOREACH_PRIO(SIM_PRIO2);

    printf("House->id=%d usage_sum=%f unix_timestamp=%ld\n", house_data->id, usage_sum, unix_timestamp_seconds);

}