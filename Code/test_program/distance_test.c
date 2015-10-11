/* @Name: distance_test.c
 * @Author: Torben Woltjen, David Simon, Dennis Grießer
 * @Date: 14.04.2015
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <wiringPi.h>

// include stdint.h for types like uint8_t
#include <stdint.h>

#define NANOSECONDS_PER_SECOND 1000000000

#define GPIO_TRIGGER 22
#define GPIO_ECHO 27

uint8_t readGPIO(uint8_t number);

struct timespec diffTime(struct timespec before, struct timespec after);

/*
 * The main method initializes the gpios and starts a thread and checks the content
 * of the files.
 */
int main() {
    int res;
    int count, statusBefore, statusAfter, sensorBefore, sensorAfter;
    struct timespec sleeptime, result, sensorResult;
    struct timespec before, measureSensorBefore;
    struct timespec after, measureSensorAfter;
    uint16_t distance;


    sleeptime.tv_sec = 0;
    sleeptime.tv_nsec = 000010000L; // sleep 10µs


    int maxPriority;
    int status;
	// get max available priority
    maxPriority = sched_get_priority_max(SCHED_FIFO);
    if (maxPriority == -1) {
        perror("Could not determine the maximum priority available.");
        exit(EXIT_FAILURE);
    }

    // set scheduler to SCHED_FIFO and priority to max priority
    struct sched_param sched;
    sched.sched_priority = maxPriority;
    status = sched_setscheduler(0, SCHED_FIFO, &sched);

    if (status) {
        perror("ERROR: Could not set real time priority (are you running this as root?)");
        exit(EXIT_FAILURE);
    }


    // TODO: Add comment for the above sleeptime: How much µs / ms whatever is this? -> use human readable unit here!

    /*signal(SIGINT, sigfunc); */


    wiringPiSetupGpio();
    pinMode(GPIO_TRIGGER, OUTPUT);
    pinMode(GPIO_ECHO, INPUT);

    statusBefore = clock_gettime(CLOCK_MONOTONIC, &before);
    
    digitalWrite(GPIO_TRIGGER, 1);
    if(clock_nanosleep(CLOCK_MONOTONIC, 0, &sleeptime, NULL))
    {
        perror("nanosleep failed\n");
        exit(EXIT_FAILURE);
    }
    digitalWrite(GPIO_TRIGGER, 0);

    while(1) {
        if(digitalRead(GPIO_ECHO) == 1) {
    		sensorBefore = clock_gettime(CLOCK_MONOTONIC, &measureSensorBefore);
            break;
        }
    }

    while(1) {
        if(digitalRead(GPIO_ECHO) == 0) {
			sensorAfter = clock_gettime(CLOCK_MONOTONIC, &measureSensorAfter);
			break;
        }
    }


	/* A bit of theory:
     *   Max distance of ultrasonic sensor: about 3m
     *   -> So the max time for the sound to travel to a barrier
     *      and back for this distance is (at 19.2 degree celsius):
     *      6m / 343m/s = 0.017492711s = 17492711ns
	 *   => max number in result.tv_nsec is 17492711ns
     *
     *   17492711 * 343 = 5999999873 ==> this is the max number which must be
     *   stored in the meantime during the calculation.
     *      Max no to be stored: 5999999873
     *      Max no in long     : 4294967295
     *      --> so we need to use an usigned *long long* in the following..
     */
	sensorResult = diffTime(measureSensorBefore, measureSensorAfter);
	distance = (uint64_t) sensorResult.tv_nsec * 343 / 1000000 / 2;
        
        statusAfter = clock_gettime(CLOCK_MONOTONIC, &after);
        
        result = diffTime(before, after);

        printf("seconds: %ld nanoseconds %ld\n", result.tv_sec, result.tv_nsec);

	printf("Result: %hu\n\n", distance);

	delay(20);

    return(EXIT_SUCCESS);
}

/*
 * Read the status (HIGH or LOW) of a given GPIO port.
 * Please note that the given GPIO port must have been initialised
 * before.
 */
uint8_t readGPIO(uint8_t number) {
    FILE *fp;
    /*       26 chars -> /sys/class/gpio/gpio/value
     * + max  3 chars -> GPIO port number, e.g. "23", can max be uint8_t -> 255 -> 3 chars
     * +      1 char  -> terminating '\0' character
     * --> = 30 chars must fit inside the path-buffer!
     */
	char path[30];
	unsigned int gpioState;
	uint8_t result;

	sprintf(path, "/sys/class/gpio/gpio%u/value", number); // %u: u -> unsigned

    fp = fopen(path, "r");
	// error checking
    if(fp == NULL)
    {
        perror(path);
        exit(EXIT_FAILURE);
    }

	/*
     * We only need the first character from the GPIO value file as the file doesn't contain
     * more characters anyway (it can only contain a '0' for LOW or a '1' for HIGH)
     */
	gpioState =	fgetc(fp);

	// convert char '0' to int 0, '1' to 1 etc.
	result = gpioState - '0';

    fclose(fp);

	return result;
}


/*
 * Calculate the difference of two time stamps.
 *
 * This function is inspired by example 4-13 from the book
 * "Moderne Realzeitsysteme kompakt" (Jürgen Quade, Michael Mächtel)
 */
struct timespec diffTime(struct timespec before, struct timespec after)
{
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
