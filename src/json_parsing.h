#pragma once

#include <cJSON.h>
#include "sim/subscriptions.h"


/**
 * @brief Parse and validate provided cJSON* object and populate fields of the provided struct.
 * 
 * JSON should be formatted as:
 * {
 *      "id": 1,
 *      "no_adults": 2,
 *      "no_children": 5,
 *      "house_size_m2": 154,
 *      "no_electric_cars": 0
 *  },
 * 
 * @param house_json cJSON* object to parse.
 * @param house_data house_data_t* whose fields should be populated.
 * @return an int depdending on which field failed to parse. Otherwise 0 for success.
 */
int housejson_parse(const cJSON * house_json, house_data_t * house_data);
