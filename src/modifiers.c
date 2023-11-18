#include "sim/subscriptions.h"

#include <math.h>

#undef MODIFIER_PRINTS

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

SIM_SUBSCRIBE(ALL_YEAR, ALL_MONTH, ALL_DAY, ADD, adult_modifier, POWER, SIM_PRIO0);
SIM_SUBSCRIBE(ALL_YEAR, ALL_MONTH, ALL_DAY, ADD, children_modifier, POWER, SIM_PRIO0);
SIM_SUBSCRIBE(ALL_YEAR, ALL_MONTH, ALL_DAY, ADD, housesize_modifier, POWER, SIM_PRIO0);
SIM_SUBSCRIBE(ALL_YEAR, ALL_MONTH, ALL_DAY, ADD, housesize_modifier, HEAT, SIM_PRIO0);



double seasonal_modifier(const house_data_t * const house_data, convinient_time_t  * time, sim_subscription_t * sim_subscription, unsigned int seed) {

    // Scaling factor to control the amplitude of the sinusoidal wave
    double amplitude = 0.5;

    // Frequency of the sinusoidal wave (one cycle per year)
    /* By some mathematical magic a frequency of 57.823 makes the wave repeat once a year,
        I have no idea how this value correlates with 365. */
    double frequency = 57.823;

    // Phase shift to control the starting point of the wave (peak or low)
    /* frequency * -1.6 makes winter the peak, +1.6 makes it the low. */
    double phaseShift = frequency * -1.6;

    double verticalShift = 1;

    // Calculate the sinusoidal value
    double sinusoidalValue = amplitude * sin((time->timeinfo.tm_yday - phaseShift)/frequency) + verticalShift;

#ifdef MODIFIER_PRINTS
    static int last_day = 0;
    if (house_data->id == 1 && last_day != time->timeinfo.tm_yday) {
        int num_spaces = 10 * (sinusoidalValue + 1);
        last_day = time->timeinfo.tm_yday;
        printf("year=%d month=%d\tday=%d\tsinusoidal_multiplier=%f\t", 1900+time->timeinfo.tm_year, time->timeinfo.tm_mon, time->timeinfo.tm_yday, sinusoidalValue);
        for (size_t i = 0; i < num_spaces; i++) {
            printf(" ");
        }
        printf(".\n");
        
    }
#endif
    return sinusoidalValue;
}



SIM_SUBSCRIBE(ALL_YEAR, ALL_MONTH, ALL_DAY, MULTIPLY, seasonal_modifier, POWER, SIM_PRIO2);
SIM_SUBSCRIBE(ALL_YEAR, ALL_MONTH, ALL_DAY, MULTIPLY, seasonal_modifier, HEAT, SIM_PRIO2);
