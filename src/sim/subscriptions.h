#pragma once

#define NUM_PRIORITIES 2

typedef struct house_data_s {
    unsigned int id;
    unsigned int num_adults;
    unsigned int num_children;
    unsigned int house_size_m2;
    unsigned int no_electric_cars;
} house_data_t;

typedef struct month_range_s {
    unsigned int start_month : 5;
    unsigned int end_month : 5;
} month_range_t;

/* These #defines make it easy to subscribe to specific seasons */
#define SPRING (month_range_t){.start_month = 2, .end_month = 4}
#define SUMMER (month_range_t){.start_month = 5, .end_month = 7}
#define AUTUMN (month_range_t){.start_month = 8, .end_month = 10}
#define WINTER (month_range_t){.start_month = 11, .end_month = 1}
#define ALL_YEAR (month_range_t){.start_month = 0, .end_month = 11}

typedef struct hour_range_s {
    unsigned int start_hour : 6;
    unsigned int end_hour : 6;
} hour_range_t;

#define ALL_DAY (hour_range_t){.start_hour = 0, .end_hour = 23}

/* TODO Kevin: Not sure where we will use these yet, 
    but it fits nicely with our month_range_t */
typedef enum {
    JANUARY = 0,
    FEBRUARY = 1,
    MARCH = 2,
    APRIL = 3,
    MAY = 4,
    JUNE = 5,
    JULY = 6,
    AUGUST = 7,
    SEPTEMBER = 8,
    OCTOBER = 9,
    NOVEMBER = 10,
    DECEMBER = 11
} month_e;

typedef enum {
    ADD = 0,
    MULTIPLY = 1,
} operation_e;

typedef struct __attribute__((section("sim_subscriptions"), used)) sim_subscription_s {

    /* Here we place the criteria for calling the modifier_func */
    month_range_t month_range;
    hour_range_t hour_range;

    /* TODO Kevin: If we want to optimize the simulation,
        we should call the modifier once when entering the period,
        and then once more when exiting (only then with the opposite operator) */
    operation_e operation;

    /**
     * @brief Modifier function called when the simulation falls within the subscribed period.
     * 
     * @param house_data House data of type house_data_t*
     * @param unix_timestamp_sec Exact current timestamp in the simulation, maybe this modifier wants to be very precise.
     * @param sim_subscription Subscription holding the modifier_func, maybe this modifier wants to introspect how far into its period we are.
     * 
     * @return a double representing the amount by which the utility usage should be modifed, operation is determined by sim_subscription_t.operation.
     */
    double (*modifier_func)(const house_data_t * const house_data, unsigned long unix_timestamp_sec, struct sim_subscription_s * sim_subscription);

} sim_subscription_t;

#if 0  // Playing around with different ways to initialize subscriptions, preferably we wouldn't have to name variables ourselves.
#define SOME_MACRO(subscription) \
    ... \

SOME_MACRO((sim_subscription_t){.operation = MULTIPLY, .month_range = ALL_YEAR, .hour_range = ALL_DAY});
double my_modifier(const house_data_t * const house_data, unsigned long unix_timestamp_sec, sim_subscription_t * sim_subscription) {
    return 1 + 0.1 * house_data->num_children;
}

sim_subscription_t test = {
    .month_range = WINTER,
    .hour_range = ALL_DAY,
    .operation = MULTIPLY,
    .modifier_func = my_modifier,
};

#define DISPLAY_FUNC(arg) \
    __attribute__((section("displayfuncs#arg"))) /* */ \
	__attribute__((aligned(1))) \
	__attribute__((used))
#endif
