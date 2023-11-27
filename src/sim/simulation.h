#pragma once

#include <time.h>

#include "subscriptions.h"

typedef struct {

    double power_usage;
    double water_usage;
    double heat_usage;

    // size_t delta_seconds;
    time_t unix_timestamp_seconds;
    house_data_t * house_data;
} usage_line_t;

void seed_sim(unsigned int seed);

void simulation_step(house_data_t * const house_data, const time_t unix_timestamp_seconds, usage_line_t * usage_line_out);
