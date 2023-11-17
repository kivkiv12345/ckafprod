#pragma once

#include <time.h>

#include "subscriptions.h"

void seed_sim(unsigned int seed);

void simulation_step(const house_data_t * const house_data, const time_t unix_timestamp_seconds);
