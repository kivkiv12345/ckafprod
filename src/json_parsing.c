#include "json_parsing.h"

#include <cJSON.h>

#include "sim/subscriptions.h"


int housejson_parse(const cJSON * house_json, house_data_t * house_data) {

    const cJSON *house_id = cJSON_GetObjectItemCaseSensitive(house_json, "id");
    const cJSON *num_adults = cJSON_GetObjectItemCaseSensitive(house_json, "no_adults");
    const cJSON *num_children = cJSON_GetObjectItemCaseSensitive(house_json, "no_children");
    const cJSON *house_size_m2 = cJSON_GetObjectItemCaseSensitive(house_json, "house_size_m2");
    const cJSON *no_electric_cars = cJSON_GetObjectItemCaseSensitive(house_json, "no_electric_cars");

#if 1  /* Check house for valid fields/types */
    if (!cJSON_IsNumber(house_id)) {
        return 4;
    }

    if (!cJSON_IsNumber(num_adults)) {
        return 5;
    }

    if (!cJSON_IsNumber(num_children)) {
        return 6;
    }

    if (!cJSON_IsNumber(house_size_m2)) {
        return 7;
    }
    
    if (!cJSON_IsNumber(no_electric_cars)) {
        return 8;
    }

    /* TODO Kevin: This is where we would check for unknown fields */
#endif

    /* Start house thread */
    *house_data = (house_data_t){
        .id = (unsigned int)cJSON_GetNumberValue(house_id),
        .num_adults = (unsigned int)cJSON_GetNumberValue(num_adults),
        .num_children = (unsigned int)cJSON_GetNumberValue(num_children),
        .house_size_m2 = (unsigned int)cJSON_GetNumberValue(house_size_m2),
        .no_electric_cars = (unsigned int)cJSON_GetNumberValue(no_electric_cars),
    };

    return 0;
}