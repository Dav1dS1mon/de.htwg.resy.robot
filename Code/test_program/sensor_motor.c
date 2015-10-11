#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <wiringPi.h>
#include <stdint.h>
#include <sched.h>
#include <softPwm.h>
#include "../core/GPIO_defines.h"

#define NANOSECONDS_PER_SECOND 1000000000


uint8_t readGPIO(uint8_t number);
void *sensorMeasurement();
struct timespec diffTime(struct timespec before, struct timespec after);

uint16_t distance[3];
pthread_mutex_t mutex;

// struct for define parameters
typedef struct {
    int trigger;
    int echo;
	int threadNumber;
	uint16_t *distanceThread;
} threadArgs;

// define motor directions
enum MotorDirection_{MOTOR_FORWARD, MOTOR_BACKWARD};
typedef enum MotorDirection_ MotorDirection;

// function prototypes
int setMaxPriority();

void motorGpioSetup();

void motorLeftSetSpeed(uint8_t speed);
void motorRightSetSpeed(uint8_t speed);
void motorSetSpeed(uint8_t pin, uint8_t speed); // private function

void motorLeftSetDirection(MotorDirection direction);
void motorRightSetDirection(MotorDirection direction);
void motorSetDirection(uint8_t pinMotorForward,
                            uint8_t pinMotorBackward,
                            MotorDirection direction); // private function

/*
 * The main method initializes the gpios and starts a thread and checks the content
 * of the files.
 */
int main() {
    pthread_t sensorThreadFront, sensorThreadLeft, sensorThreadRight;
    int res;
    int count;
	int counter = 0;
    struct timespec sleeptime, result;
    struct timespec before;
    struct timespec after;

    threadArgs sensorFront;
    sensorFront.trigger = GPIO_US_FRONT_TRIG;
    sensorFront.echo = GPIO_US_FRONT_ECHO;
	sensorFront.distanceThread = &distance[0];
	sensorFront.threadNumber = 0;

    threadArgs sensorRight;
    sensorRight.trigger = GPIO_US_RIGHT_TRIG;
    sensorRight.echo = GPIO_US_RIGHT_ECHO;
	sensorRight.distanceThread = &distance[1];
	sensorRight.threadNumber = 1;

    threadArgs sensorLeft;
    sensorLeft.trigger = GPIO_US_LEFT_TRIG;
    sensorLeft.echo = GPIO_US_LEFT_ECHO;
	sensorLeft.distanceThread = &distance[2];
	sensorLeft.threadNumber = 2;

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

    res = pthread_mutex_init(&mutex, NULL);
    if(res != 0) {
        perror("initialize a mutex failed!");
        exit(EXIT_FAILURE);
    }

    wiringPiSetupGpio();
    motorGpioSetup();


    res = pthread_create(&sensorThreadFront, NULL, sensorMeasurement, &sensorFront);
    if(res != 0) {
        perror("create a thread failed!");
        exit(EXIT_FAILURE);
    }

    res = pthread_create(&sensorThreadLeft, NULL, sensorMeasurement, &sensorLeft);
    if(res != 0) {
        perror("create a thread failed!");
        exit(EXIT_FAILURE);
    }

    res = pthread_create(&sensorThreadRight, NULL, sensorMeasurement, &sensorRight);
    if(res != 0) {
        perror("create a thread failed!");
        exit(EXIT_FAILURE);
    }

	sleeptime.tv_sec = 1;
    sleeptime.tv_nsec = 000000000L; // sleep 1s

    if(clock_nanosleep(CLOCK_MONOTONIC, 0, &sleeptime, NULL))
    {
    	perror("nanosleep failed\n");
        exit(EXIT_FAILURE);
    }
	
    while(1)
    {
		//printf("1. %d und 2. %d und 3. %d\n", distance[0], distance[1], distance[2]);
		//printf("%d\n", distance[1]);
		//printf("%d\n", distance[2]);
        while(distance[0]>=250 && distance[1]>=250 && distance[2]>=250)
        {
		    motorLeftSetDirection(MOTOR_FORWARD);
		    motorRightSetDirection(MOTOR_FORWARD);
		    motorLeftSetSpeed(80);
		    motorRightSetSpeed(80);
		    delay(50);
        }
		printf("1. %d und 2. %d und 3. %d\n", distance[0], distance[1], distance[2]);
		if(distance[1] < distance[2])
		{	
            motorRightSetDirection(MOTOR_BACKWARD);
            motorLeftSetDirection(MOTOR_FORWARD);
	    motorRightSetSpeed(50);
	    motorLeftSetSpeed(50);
            delay(400);
            motorLeftSetSpeed(0);
            motorRightSetSpeed(0);
            delay(100);
		} else {
			motorLeftSetDirection(MOTOR_BACKWARD);
            motorRightSetDirection(MOTOR_FORWARD);
	    motorLeftSetSpeed(50);
	    motorRightSetSpeed(50);
            delay(400);
            motorLeftSetSpeed(0);
            motorRightSetSpeed(0);
            delay(100);
		}
    }

	return(EXIT_SUCCESS);
}





void *sensorMeasurement(threadArgs *sensor)
{

	int trigger = sensor->trigger;
	int echo = sensor->echo;
	int threadNumber = sensor->threadNumber;
	uint16_t *distanceTemp = &sensor->distanceThread;
    int sensorBefore, sensorAfter;
    struct timespec sensorResult, sleeptime;
    struct timespec measureSensorBefore;
    struct timespec measureSensorAfter;

    pinMode(trigger, OUTPUT);
    pinMode(echo, INPUT);

    while(1)
    {

        sleeptime.tv_sec = 0;
        sleeptime.tv_nsec = 000010000L; // sleep 10µs

        digitalWrite(trigger, 1);
        if(clock_nanosleep(CLOCK_MONOTONIC, 0, &sleeptime, NULL))
        {
            perror("nanosleep failed\n");
            exit(EXIT_FAILURE);
        }
        digitalWrite(trigger, 0);

        while(1) {
            if(digitalRead(echo) == 1) {
                sensorBefore = clock_gettime(CLOCK_MONOTONIC, &measureSensorBefore);
                break;
            }
        }

        while(1) {
            if(digitalRead(echo) == 0) {
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

        if(pthread_mutex_lock(&mutex) != 0)
        {
            perror("mutex unlock failed\n");
            exit(EXIT_FAILURE);
        }
        *distanceTemp = (uint64_t) sensorResult.tv_nsec * 343 / 1000000 / 2;
		distance[threadNumber] = *distanceTemp;
        if(pthread_mutex_unlock(&mutex) != 0)
        {
            perror("mutex unlock failed\n");
            exit(EXIT_FAILURE);
        }
        delay(20);
    }
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
    gpioState = fgetc(fp);

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



/*
 * Setup motor GPIOs as outputs and set them to LOW
 */
void motorGpioSetup() {
    // left motor
    // -----------------------------------------------
    // we configure the soft PWM to have the range 0-100; init value: 0
    if (softPwmCreate(GPIO_MOTOR_LEFT_ENABLE, 0, 100)) {
        perror("ERROR: motorGpioSetup: Could not setup software PWM for left motor");
        exit(EXIT_FAILURE);
    };

    pinMode(GPIO_MOTOR_LEFT_FORWARD, OUTPUT);
    digitalWrite(GPIO_MOTOR_LEFT_FORWARD, LOW);

    pinMode(GPIO_MOTOR_LEFT_BACKWARD, OUTPUT);
    digitalWrite(GPIO_MOTOR_LEFT_BACKWARD, LOW);

    // right motor
    // -----------------------------------------------
    // we configure the soft PWM to have the range 0-100; init value: 0
    if (softPwmCreate(GPIO_MOTOR_RIGHT_ENABLE, 0, 100)) {
        perror("ERROR: motorGpioSetup: Could not setup software PWM for right motor");
        exit(EXIT_FAILURE);
    };

    pinMode(GPIO_MOTOR_RIGHT_FORWARD, OUTPUT);
    digitalWrite(GPIO_MOTOR_RIGHT_FORWARD, LOW);

    pinMode(GPIO_MOTOR_RIGHT_BACKWARD, OUTPUT);
    digitalWrite(GPIO_MOTOR_RIGHT_BACKWARD, LOW);
}


void motorLeftSetSpeed(uint8_t speed) {
    motorSetSpeed(GPIO_MOTOR_LEFT_ENABLE, speed);
}


void motorRightSetSpeed(uint8_t speed) {
    motorSetSpeed(GPIO_MOTOR_RIGHT_ENABLE, speed);
}


void motorSetSpeed(uint8_t pin, uint8_t speed) {
    // we can set the soft PWM from 0-100, however we don't need to check if
    // the speed parameter is in range as this is done by the softPwmWrite
    // function already; if speed > 100, then softPwmWrite will just set the
    // speed to 100 anyway
    softPwmWrite(pin, speed);
}


void motorLeftSetDirection(MotorDirection direction) {
    motorSetDirection(GPIO_MOTOR_RIGHT_FORWARD,
                      GPIO_MOTOR_RIGHT_BACKWARD,
                      direction);
}


void motorRightSetDirection(MotorDirection direction) {
    motorSetDirection(GPIO_MOTOR_LEFT_FORWARD,
                      GPIO_MOTOR_LEFT_BACKWARD,
                      direction);
}


void motorSetDirection(uint8_t pinMotorForward,
                       uint8_t pinMotorBackward,
                       MotorDirection direction) {
    if (direction == MOTOR_FORWARD) {
        // ATTENTION: Set backward LOW before setting forward HIGH to prevent
        // a short circuit!
        digitalWrite(pinMotorBackward, LOW);
        digitalWrite(pinMotorForward, HIGH);
    } else if (direction == MOTOR_BACKWARD) {
        // ATTENTION: Set foward LOW before setting backward HIGH to prevent
        // a short circuit!
        digitalWrite(pinMotorForward, LOW);
        digitalWrite(pinMotorBackward, HIGH);
    } else {
        fprintf(stderr, "ERROR: motorSetDirection: Unknown direction given!\n");
        exit(EXIT_FAILURE);
    }
}
