#include "time_utils.h"

#define NANOSECONDS_PER_SECOND 1000000000

/*
 * Calculate the difference of two time stamps.
 *
 * This function is inspired by example 4-13 from the book
 * "Moderne Realzeitsysteme kompakt" (Jürgen Quade, Michael Mächtel)
 */
struct timespec diffTime(struct timespec before, struct timespec after) {
    struct timespec result;

    if ((after.tv_sec < before.tv_sec) ||
        ((after.tv_sec == before.tv_sec) &&
         (after.tv_nsec <= before.tv_nsec))) {
        result.tv_sec = result.tv_nsec = 0;
    }
    result.tv_sec = after.tv_sec - before.tv_sec;
    result.tv_nsec = after.tv_nsec - before.tv_nsec;

    if (result.tv_nsec < 0) {
        result.tv_sec--;
        /* result.tv_nsec is negative, therefore we we use "+" */
        result.tv_nsec = NANOSECONDS_PER_SECOND + result.tv_nsec;
    }
    return result;
}

/*
 * Return 1 if timestamp one is bigger than two; if two is bigger than one,
 * return 2; else, if both are the same, return 0;
 */
uint8_t compareTime(struct timespec one, struct timespec two) {
    if (one.tv_sec == two.tv_sec && one.tv_nsec == two.tv_nsec) {
        return 0;
    } else if ((one.tv_sec > two.tv_sec) || (one.tv_sec == two.tv_sec && one.tv_nsec > two.tv_nsec)) {
        return 1;
    } else {
        return 2;
    }
}
