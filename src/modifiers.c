/* Currently some very basic examples of modifiers. */

#include "sim/subscriptions.h"

#define MODIFIER_PRINTS

#ifdef MODIFIER_PRINTS
#include <stdio.h>
#endif

double adult_modifier(const house_data_t * const house_data, convinient_time_t  * time, sim_subscription_t * sim_subscription, unsigned int seed) {
    return 100 * house_data->num_adults;
}

double children_modifier(const house_data_t * const house_data, convinient_time_t  * time, sim_subscription_t * sim_subscription, unsigned int seed) {
    return 50 * house_data->num_children;
}

double housesize_modifier(const house_data_t * const house_data, convinient_time_t  * time, sim_subscription_t * sim_subscription, unsigned int seed) {
    return house_data->house_size_m2;
}

SIM_SUBSCRIBE(ALL_YEAR, ALL_MONTH, ALL_DAY, ADD, adult_modifier, SIM_PRIO0);
SIM_SUBSCRIBE(ALL_YEAR, ALL_MONTH, ALL_DAY, ADD, children_modifier, SIM_PRIO0);
SIM_SUBSCRIBE(ALL_YEAR, ALL_MONTH, ALL_DAY, ADD, housesize_modifier, SIM_PRIO0);



double winter_modifier(const house_data_t * const house_data, convinient_time_t  * time, sim_subscription_t * sim_subscription, unsigned int seed) {
#ifdef MODIFIER_PRINTS
    static int last_day = 0;
    if (house_data->id == 1 && last_day != time->timeinfo.tm_mday)
        printf("Winter year=%d month=%d\tday=%d\n", 1900+time->timeinfo.tm_year, time->timeinfo.tm_mon, (last_day = time->timeinfo.tm_mday));
#endif
    return 1.5;
}

double spring_modifier(const house_data_t * const house_data, convinient_time_t  * time, sim_subscription_t * sim_subscription, unsigned int seed) {
#ifdef MODIFIER_PRINTS
    static int last_day = 0;
    if (house_data->id == 1 && last_day != time->timeinfo.tm_mday)
        printf("Spring year=%d month=%d\tday=%d\n", 1900+time->timeinfo.tm_year, time->timeinfo.tm_mon, (last_day = time->timeinfo.tm_mday));
#endif
    return 1;
}

double summer_modifier(const house_data_t * const house_data, convinient_time_t  * time, sim_subscription_t * sim_subscription, unsigned int seed) {
#ifdef MODIFIER_PRINTS
    static int last_day = 0;
    if (house_data->id == 1 && last_day != time->timeinfo.tm_mday)
        printf("Summer year=%d month=%d\tday=%d\n", 1900+time->timeinfo.tm_year, time->timeinfo.tm_mon, (last_day = time->timeinfo.tm_mday));
#endif
    return 0.7;
}

double autumn_modifier(const house_data_t * const house_data, convinient_time_t  * time, sim_subscription_t * sim_subscription, unsigned int seed) {
#ifdef MODIFIER_PRINTS
    static int last_day = 0;
    if (house_data->id == 1 && last_day != time->timeinfo.tm_mday)
        printf("Autumn year=%d month=%d\tday=%d\n", 1900+time->timeinfo.tm_year, time->timeinfo.tm_mon, (last_day = time->timeinfo.tm_mday));
#endif
    return 1.2;
}


SIM_SUBSCRIBE(WINTER, ALL_MONTH, ALL_DAY, MULTIPLY, winter_modifier, SIM_PRIO2);
SIM_SUBSCRIBE(SPRING, ALL_MONTH, ALL_DAY, MULTIPLY, spring_modifier, SIM_PRIO2);
SIM_SUBSCRIBE(SUMMER, ALL_MONTH, ALL_DAY, MULTIPLY, summer_modifier, SIM_PRIO2);
SIM_SUBSCRIBE(AUTUMN, ALL_MONTH, ALL_DAY, MULTIPLY, autumn_modifier, SIM_PRIO2);
