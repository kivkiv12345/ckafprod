#include "sim/timeutils.h"
#include "sim/subscriptions.h"

#include <math.h>
#include <stdlib.h>
#include <assert.h>

#undef MODIFIER_PRINTS

#ifdef MODIFIER_PRINTS
#include <stdio.h>
#endif

static double adult_modifier(const house_data_t * const house_data, convinient_time_t  * time, sim_subscription_t * sim_subscription, unsigned int seed) {
    return 100 * house_data->num_adults;
}

static double children_modifier(const house_data_t * const house_data, convinient_time_t  * time, sim_subscription_t * sim_subscription, unsigned int seed) {
    return 50 * house_data->num_children;
}

static double housesize_modifier(const house_data_t * const house_data, convinient_time_t  * time, sim_subscription_t * sim_subscription, unsigned int seed) {
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
        int num_spaces = ((sinusoidalValue) * 30);
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


#define EV_EARLIEST_CHARGE_HOUR 14  // What is the absolute earliest an EV can start charging.
#define EV_CHARGE_START_HOUR_RANGE_LEN 5  // Maximum number of hours added unto randomly added unto the earliest charging hour, before charging actually starts.
#define EV_CHARGE_DURATION 8  // 
#define EV_LATEST_CHARGE_HOUR (((EV_EARLIEST_CHARGE_HOUR + EV_CHARGE_START_HOUR_RANGE_LEN) + EV_CHARGE_DURATION) % 24)

static double ev_charging_modifier(const house_data_t * const house_data, convinient_time_t  * time, sim_subscription_t * sim_subscription, unsigned int seed) {

    if (house_data->num_electric_cars == 0)
        return 0;

    const int num_evs = house_data->num_electric_cars;

    /* 1 adult wont drive multiple cars daily. */
    const int num_evs_used_daily = num_evs < house_data->num_adults ? num_evs : house_data->num_adults;

    double power_sum = 0;

    /* This loop makes every EV charge at different times. I.e, adults coming home from work at different times. */
    for (int ev = 0; ev < num_evs_used_daily; ev++) {

        /* NOTE: Currently, any given household changes at what time they charge their EV(s) on a yearly basis */
        seed += time->timeinfo.tm_year + ev;
    
        /* Any applicable EV should start charging between 14:00 and 18:00 (at the time of writing). */
        const int ev_charge_hour_start = rand_r(&seed) % EV_CHARGE_START_HOUR_RANGE_LEN + EV_EARLIEST_CHARGE_HOUR;
        const int ev_charge_hour_end = (ev_charge_hour_start + EV_CHARGE_DURATION) % 24;

        /* Will this car even be charged today? One could imagine it wasn't used. */
        /* NOTE: Charging skip uses a separate seed so we don't modify our "change duration seed" every .tm_yday.*/
        unsigned int skip_charge_seed = seed + time->timeinfo.tm_yday;
        static_assert(EV_LATEST_CHARGE_HOUR < EV_EARLIEST_CHARGE_HOUR);  // The if () below requires a day change
        if (time->timeinfo.tm_hour <= EV_LATEST_CHARGE_HOUR)  // We are charging after midnight.
            skip_charge_seed -= 1;  // Subtract 1 "day" from the seed, so we don't start/stop charging at midnight.
        const int skip_charge_percent = time->timeinfo.tm_wday < SATURDAY ? 10 : 20;  // Car has an increased chance of going unused during weekends.
        if ((rand_r(&skip_charge_seed) % 100) < skip_charge_percent)
            continue;

        if (in_timeframe(time->timeinfo.tm_hour, ev_charge_hour_start, ev_charge_hour_end)) {
#undef EV_PRINT
#ifdef EV_PRINT
            const int print_house = house_data->id == 1;
            if (print_house)
                printf("AAA ev=%d, hour=%d, day=%d\n", ev, time->timeinfo.tm_hour, time->timeinfo.tm_yday);
#endif
            power_sum += 400;
        }
    }

    const int print_house = house_data->id == 1;
    if (print_house)
        printf("%f\n", power_sum);
    return power_sum;
}

/* TODO Kevin: EV charging should probably come after seasonal modifiers, priority-wise. */
/* Assumes EVs are never charged at home between 14:00 and 04:00 */
SIM_SUBSCRIBE(ALL_YEAR, ALL_MONTH, TIME_RANGE((hour_range_t){.start_hour = EV_EARLIEST_CHARGE_HOUR, .end_hour = EV_LATEST_CHARGE_HOUR}), ADD, ev_charging_modifier, POWER, SIM_PRIO0);
