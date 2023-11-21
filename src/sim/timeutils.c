#include "timeutils.h"

#include <stdbool.h>

/**
 * @brief Check if the specified time is within the specified range.
 * Accounts for cases where start_time > end_time, 
 * such as with a year-change, month-change, day-change, etc.
 */
bool in_timeframe(const unsigned int time, const unsigned int start_time, const unsigned int end_time) {
    // Check if the current hour is within the charging time range
    if (start_time <= end_time) {  
        return time >= start_time && time <= end_time;
    } else {  /* The specified range includes a year-change, month-change, day-change, etc. */
        // Handle the case where the charging time range spans across midnight, for example
        return time >= start_time || time <= end_time;
    }
}
