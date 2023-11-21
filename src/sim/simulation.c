#include "simulation.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "timeutils.h"
#include "subscriptions.h"

/* TODO Kevin: If we can find a good way to expose house_to_seed() and user_seed,
    we can consider removing the "seed" parameter from modifier functions. */

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
    /* NOTE: We currently assume that all fields are immutable,
        should this ever no longer be the case,
        the .id field should suffice. */
    for (size_t i = 0; i < sizeof(house_data_t); ++i) {
        sum += ((unsigned char *)house_data)[i];
    }

    return sum;
}

void simulation_step(const house_data_t * const house_data, const time_t unix_timestamp_seconds) {
    
    // struct tm* timeinfo = localtime(&unix_timestamp_seconds);

    struct tm *localtime_r(const time_t *timep, struct tm *result);

    convinient_time_t time = {
        .unix_timestamp_sec = unix_timestamp_seconds,
        /* localtime_r() is used to initialize the 'time' struct,
            so we must consider it okay to cast away the ‘const’ qualifier. */
        /* I'm quite surprised &time can be an argument while initializing &time */
        .timeinfo = *localtime_r(&unix_timestamp_seconds, (struct tm *)&time.timeinfo),
    };
    int month = time.timeinfo.tm_mon; // tm_mon is zero-indexed
    int day = time.timeinfo.tm_mday - 1; // tm_mday is one-indexed
    int hour = time.timeinfo.tm_hour; // tm_hour is zero-indexed

    /* Any randomization included in the simulation must be kept deterministic,
        as this allows us to keep the output consistent when rerunning the simulation. */
    /* The initial seed is (user_seed + house_to_seed(house_data)),
        the modifier may then add in any field(s) of the provided convinient_time_t, 
        this should allow us to run multiple threads for the same house, but with different start times.
        If any thread(s) were to catch up to the start time of any other thread, they should generate the same output. */
    /* Modifiers can decide whether the randomized value should change based on .tm_year, .tm_mon, .tm_yday, (etc).
        By adding said field(s) unto the seed, they can make random decisions on a yearly basis, for example. */
    /* Modifiers may want to perform their own further mutations on the seed,
        so they will stand apart from each other. */
    unsigned int sim_seed = user_seed + house_to_seed(house_data);

    double power_usage_sum = 0;
    double water_usage_sum = 0;
    double heat_usage_sum = 0;

    /**
     * @brief Applies modifiers that are subscribed to the current simulation time.
     * 
     * Most of the would-be arguments to this function are found in the scope of the outer wrapping function,
     * thanks to the magic of GCC.
     * 
     * @param sim_subscription Current subscription in the for-loop in SUBSCRIPTION_FOREACH_PRIO().
     */
    void inline apply_valid_modifier(sim_subscription_t * sim_subscription, double * _sumptr) {

        if (!in_timeframe(month, sim_subscription->month_range.start_month, sim_subscription->month_range.end_month))
            return;  // We are not subscribed to the current month.

        if (!in_timeframe(day, sim_subscription->day_range.start_day, sim_subscription->day_range.end_day))
            return;  // We are not subscribed to the current day.

        if (!in_timeframe(hour, sim_subscription->hour_range.start_hour, sim_subscription->hour_range.end_hour))
            return;  // We are not subscribed to the current hour.

        switch (sim_subscription->operation) {
            case ADD:
                *_sumptr += sim_subscription->modifier_func(house_data, &time, sim_subscription, sim_seed);
                break;
            case MULTIPLY:
                *_sumptr *= sim_subscription->modifier_func(house_data, &time, sim_subscription, sim_seed);
                break;
            default:
                /* TODO Kevin: Invalid operator, error handling goes here */
                break;
        }
    }

#define SUBSCRIPTION_FOREACH_PRIO(_prio, _metric, _sumbuf) \
    /** \
	 * GNU Linker symbols. These will be autogenerate by GCC when using \
	 * __attribute__((section("sim_subscriptions")) \
	 */ \
	__attribute__((weak)) extern sim_subscription_t __start_sim_subscriptions##_prio##_metric; \
	__attribute__((weak)) extern sim_subscription_t __stop_sim_subscriptions##_prio##_metric; \
 \
    /* TODO Kevin: We must assert that every subscription priority only contains 1 type of operator, \
            this is because sections are unordered and therefore doesn't follow the order of operations. */ \
    /* TODO Kevin: Our use of sections also complicate the possible future feature that is dynamic linking with expansion modules, \
        I suspect that such an expansion may then be best implemented with a linked list of subscription_t sections. */ \
 \
    /* For some reason this 'if' is needed, even though the for-loop condition should be sufficient. \
        It can't be replaced with a guard clause, as we must allow the other priorities to run.*/ \
    if (&__start_sim_subscriptions##_prio##_metric != &__stop_sim_subscriptions##_prio##_metric && &__start_sim_subscriptions##_prio##_metric != NULL) { \
        /* printf("UUU %d %p %p\n", _prio, &__start_sim_subscriptions##_prio, &__stop_sim_subscriptions##_prio); */ \
        for (sim_subscription_t * sim_subscription = &__start_sim_subscriptions##_prio##_metric; sim_subscription < &__stop_sim_subscriptions##_prio##_metric; sim_subscription++) { \
            apply_valid_modifier(sim_subscription, &_sumbuf); \
        } \
    }
    
    /* NOTE: Any priorities added to subscriptions.h must also be added here. */
    /* NOTE: Currently our sections are named as sim_subscriptionsSIM_PRIO0 when using SIM_SUBSCRIBE() (instead of sim_subscriptions0),
        It is therfore important that SUBSCRIPTION_FOREACH_PRIO() is used with the SIM_PRIO* #defines. */
    SUBSCRIPTION_FOREACH_PRIO(SIM_PRIO0, POWER, power_usage_sum);
    SUBSCRIPTION_FOREACH_PRIO(SIM_PRIO1, POWER, power_usage_sum);
    SUBSCRIPTION_FOREACH_PRIO(SIM_PRIO2, POWER, power_usage_sum);
    SUBSCRIPTION_FOREACH_PRIO(SIM_PRIO0, WATER, water_usage_sum);
    SUBSCRIPTION_FOREACH_PRIO(SIM_PRIO1, WATER, water_usage_sum);
    SUBSCRIPTION_FOREACH_PRIO(SIM_PRIO2, WATER, water_usage_sum);
    SUBSCRIPTION_FOREACH_PRIO(SIM_PRIO0, HEAT, heat_usage_sum);
    SUBSCRIPTION_FOREACH_PRIO(SIM_PRIO1, HEAT, heat_usage_sum);
    SUBSCRIPTION_FOREACH_PRIO(SIM_PRIO2, HEAT, heat_usage_sum);

#undef SUMPRINT
#ifdef SUMPRINT
    if (house_data->id == 1) {
        printf("month=%d \thour=%d\tpower_sum=%f\theat_sum=%f\n", month, hour, power_usage_sum, heat_usage_sum);
    }
#endif // SUMPRINT

}