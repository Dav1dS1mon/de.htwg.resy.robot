#include "logger.h"

#include <stdlib.h>
#include <time.h>
#include <unistd.h>


void testFunction(int i) {
    sleep(2);
    logEventStart(1);
    sleep(rand() % 10);
    logEventEnd(1, "test event 2 (random)");
}

int main() {
    srand(time(NULL));
    initLogger(2);
    logEventStart(0);
    testFunction(1);
    sleep(1);
    logEventEnd(0, "test event 1");
    testFunction(1);

    saveLogToFile("log.csv");

    return EXIT_SUCCESS;
}
