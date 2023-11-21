#pragma once

#include <time.h>
#include <stdbool.h>


bool in_timeframe(const unsigned int time, const unsigned int start_time, const unsigned int end_time);

/**
 * @brief Stores both a UNIX timestamp and a 'datetime' representing the same timestamp.
 * 
 * We are generating both anyway, this is just a convenient way to pass both to modifiers,
 *  so they can pick which one they want.
 */
typedef const struct convinient_time_s {
    const time_t unix_timestamp_sec;
    const struct tm timeinfo;
} convinient_time_t;

typedef struct month_range_s {
    unsigned short start_month : 5;
    unsigned short end_month : 5;
} month_range_t;

/* These #defines make it easy to subscribe to specific seasons */
#define SPRING (month_range_t){.start_month = 2, .end_month = 4}
#define SUMMER (month_range_t){.start_month = 5, .end_month = 7}
#define AUTUMN (month_range_t){.start_month = 8, .end_month = 10}
#define WINTER (month_range_t){.start_month = 11, .end_month = 1}
#define ALL_YEAR (month_range_t){.start_month = 0, .end_month = 11}

typedef struct day_range_s {
    unsigned short start_day : 5;
    unsigned short end_day : 5;
} day_range_t;

/* We leave it up to the simulation loop to handle months with fewer days than 32 (i.e -1) */
#define ALL_MONTH (day_range_t){.start_day = 0, .end_day = -1}

typedef struct hour_range_s {
    unsigned short start_hour : 6;
    unsigned short end_hour : 6;
} hour_range_t;

#define ALL_DAY (hour_range_t){.start_hour = 0, .end_hour = 23}

/* Because of macro shenanigans, this macro-function is needed when the user wants to specify a time-range themselves,
    as the comma in the struct initializer will be interpreted as an argument separator.
    so this:
    SIM_SUBSCRIBE(ALL_YEAR, ALL_MONTH, (hour_range_t){.start_hour = 14, .end_hour = 4}, ADD, ev_charging_modifier, POWER, SIM_PRIO0);
    becomes this:
    SIM_SUBSCRIBE(ALL_YEAR, ALL_MONTH, TIME_RANGE((hour_range_t){.start_hour = 14, .end_hour = 4}), ADD, ev_charging_modifier, POWER, SIM_PRIO0); */
#define TIME_RANGE(_start, _stop) _start, _stop

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
    MONDAY = 0,
    TUESDAY = 1,
    WEDNESDAY = 2,
    THURSDAY = 3,
    FRIDAY = 4,
    SATURDAY = 5,
    SUNDAY = 6,
} weekday_e;
