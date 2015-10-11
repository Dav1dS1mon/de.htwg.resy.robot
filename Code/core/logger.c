#include "logger.h"
#include "time_utils.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static uint8_t initialised = 0;
static uint8_t measurementPositionsCount = 4;

typedef struct {
    struct timespec timestamp;
    char *name;
} event_t;

event_t *logMin;
event_t *logMax;
uint8_t *logInitialised;
struct timespec *eventStartTimestamp;


void logEventStart(uint8_t posNo) {
    if (posNo >= measurementPositionsCount) {
        fprintf(stderr, "Cannot log measurement position #%d as there are only %d positions configured (index starts at 0).\n", posNo, measurementPositionsCount);
    } else if (!initialised) {
        fprintf(stderr, "Cannot log measurement, because the logger wasn't initialised yet! Please make sure to call the initLogger() function first.\n");
    } else {
        struct timespec timestamp;
        if (clock_gettime(CLOCK_MONOTONIC, &timestamp)) {
            perror("logger.c: logEvent: clock_gettime failed.");
            exit(EXIT_FAILURE);
        }

        eventStartTimestamp[posNo] = timestamp;
    }
}


void logEventEnd(uint8_t posNo, const char *eventName) {
    struct timespec endTimestamp;
    if (clock_gettime(CLOCK_MONOTONIC, &endTimestamp)) {
        perror("logger.c: logEvent: clock_gettime failed.");
        exit(EXIT_FAILURE);
    }

    struct timespec runtime = diffTime(eventStartTimestamp[posNo], endTimestamp);

    event_t event;
    event.timestamp = runtime;
    event.name = strdup(eventName);


    if (logInitialised[posNo]) {
        event_t minEvent = logMin[posNo];
        if (compareTime(runtime, minEvent.timestamp) == 2) {
            // if minEvent.timestamp is bigger than runtime, we found a new
            // minimum candidate
            logMin[posNo] = event;
        }

        event_t maxEvent = logMax[posNo];
        if (compareTime(runtime, maxEvent.timestamp) == 1) {
            // if runtime is bigger than maxEvent.timestamp, we found a new
            // maximum candidate
            logMax[posNo] = event;
        }
    } else {
        logMin[posNo] = event;
        logMax[posNo] = event;
        logInitialised[posNo] = 1;
    }
}


void initLogger(uint8_t measurementPositions) {
    measurementPositionsCount = measurementPositions;

    logMin = (event_t*) calloc(measurementPositionsCount, sizeof(event_t));
    logMax = (event_t*) calloc(measurementPositionsCount, sizeof(event_t));

    logInitialised = (uint8_t*) calloc(measurementPositionsCount, sizeof(uint8_t));

    eventStartTimestamp = (struct timespec*) calloc(measurementPositionsCount, sizeof(struct timespec));

    initialised = 1;
    // TODO: Handle case of timer overflow - maybe create a new logfile then?
}


void saveLogToFile(char *path) {
    event_t event;

    FILE *outputFp = fopen(path, "w");
    if (outputFp == NULL) {
        fprintf(stderr, "Cannot write to log output file: \"%s\"!\n", path);
        exit(1);
    }

    // print headings to output csv logfile
    fprintf(outputFp, "Event Name;Event No.;Value;Timestamp (s); Timestamp (ns)\n");

    for (int i = 0; i < measurementPositionsCount; ++i) {
        event = logMin[i];
        fprintf(outputFp, "%s;%d;min;\"%ld\";\"%ld\"\n", event.name, i, event.timestamp.tv_sec, event.timestamp.tv_nsec);
        event = logMax[i];
        fprintf(outputFp, "%s;%d;max;\"%ld\";\"%ld\"\n", event.name, i, event.timestamp.tv_sec, event.timestamp.tv_nsec);
    }

    if (fclose(outputFp)) {
        perror("Could not close log outputfile");
        exit(EXIT_FAILURE);
    }
}
