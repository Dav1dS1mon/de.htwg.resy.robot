#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>

void initLogger(uint8_t measurementPositions);
void logEventStart(uint8_t posNo);
void logEventEnd(uint8_t posNo, const char *eventName);
void saveLogToFile(char *path);


#endif /* LOGGER_H */
