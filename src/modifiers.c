/* Currently some very basic examples of modifiers. */

#include "sim/subscriptions.h"

double adult_modifier(const house_data_t * const house_data, convinient_time_t  * time, sim_subscription_t * sim_subscription) {
    return 100 * house_data->num_adults;
}

double children_modifier(const house_data_t * const house_data, convinient_time_t  * time, sim_subscription_t * sim_subscription) {
    return 50 * house_data->num_children;
}

double housesize_modifier(const house_data_t * const house_data, convinient_time_t  * time, sim_subscription_t * sim_subscription) {
    return house_data->house_size_m2;
}

double season_modifier(const house_data_t * const house_data, convinient_time_t  * time, sim_subscription_t * sim_subscription) {
    /* TODO Kevin: Do we want to convert from UNIX timestamp in our modifier functions? */
    return 1;
}

SIM_SUBSCRIBE(ALL_YEAR, ALL_MONTH, ALL_DAY, ADD, adult_modifier, SIM_PRIO0);
SIM_SUBSCRIBE(ALL_YEAR, ALL_MONTH, ALL_DAY, ADD, children_modifier, SIM_PRIO0);
SIM_SUBSCRIBE(ALL_YEAR, ALL_MONTH, ALL_DAY, ADD, housesize_modifier, SIM_PRIO0);
SIM_SUBSCRIBE(ALL_YEAR, ALL_MONTH, ALL_DAY, MULTIPLY, season_modifier, SIM_PRIO2);
