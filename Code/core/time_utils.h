#ifndef TIME_UTILS_H
#define TIME_UTILS_H
#include <time.h>
#include <stdint.h>

struct timespec diffTime(struct timespec before, struct timespec after);
uint8_t compareTime(struct timespec one, struct timespec two);

#endif
