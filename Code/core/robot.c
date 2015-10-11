#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <wiringPi.h>
#include <stdint.h>
#include <sched.h>
#include <unistd.h>
#include <signal.h>
#include "priority.h"
#include "GPIO_defines.h"
#include "gpiosetup.h"
#include "time_utils.h"
#include "logger.h"
#include <errno.h>
#include <string.h>


#define NUMBER_OF_VALUES 2
#define NUMBER_OF_SENSORS 3
#define PERCENTAGE_DEVIATION_VALUE_TO_VALUE 33
#define DISTANCE_TO_WALL 250
#define SENSOR_DEADLINE 250000000
#define MOTOR_DEADLINE 40000000

void initValueMatrix();
void *sensorMeasurement();
void openAndWrite(char *path, char *value);
void sigfunc(int sig);
void initializeMotor();
void uninitializeMotor();


uint16_t distance[NUMBER_OF_SENSORS][NUMBER_OF_VALUES];
pthread_mutex_t sensorMeasurementMutex[NUMBER_OF_SENSORS];
pthread_cond_t sensorMeasurementCond[] = {PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER};
int measurementReady[NUMBER_OF_SENSORS];

// struct for define parameters
typedef struct {
    int trigger;
    int echo;
    int threadNumber;
    uint16_t *distanceThread;
} threadArgs;


/*
 * The main method initializes the gpios and starts a thread and checks the content
 * of the files.
 */
int main() {
    pthread_t sensorThreadFront, sensorThreadRight, sensorThreadLeft;

    int res, before, after;
    long diff;
    int status;
    int running = 0;
    int rotate = 0;

    struct timespec motorBefore, motorAfter, motorResult, sleeptime;

    signal(SIGINT, sigfunc);

    initLogger(21);


    threadArgs sensorFront;
    sensorFront.trigger = GPIO_US_FRONT_TRIG;
    sensorFront.echo = GPIO_US_FRONT_ECHO;
    sensorFront.distanceThread = &distance[0][0];
    sensorFront.threadNumber = 0;

    threadArgs sensorRight;
    sensorRight.trigger = GPIO_US_RIGHT_TRIG;
    sensorRight.echo = GPIO_US_RIGHT_ECHO;
    sensorRight.distanceThread = &distance[1][0];
    sensorRight.threadNumber = 1;

    threadArgs sensorLeft;
    sensorLeft.trigger = GPIO_US_LEFT_TRIG;
    sensorLeft.echo = GPIO_US_LEFT_ECHO;
    sensorLeft.distanceThread = &distance[2][0];
    sensorLeft.threadNumber = 2;


    status = setMaxPriority();
    switch (status) {
        case 0:
            break;
        case 1:
            perror("ERROR: Could not determine the maximum priority available");
            exit(EXIT_FAILURE);
            break;
        case 2:
            perror("ERROR: Could not set real time priority (are you running this as root?)");
            exit(EXIT_FAILURE);
            break;
    }


    wiringPiSetupGpio();

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

    sleeptime.tv_sec = 0;
    sleeptime.tv_nsec = 000200000L;
    if(clock_nanosleep(CLOCK_MONOTONIC, 0, &sleeptime, NULL)) {
        fprintf(stderr, "1 nanosleep failed (errno %d: %s)\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    initializeMotor();

    while(1) {
        logEventStart(6);
        logEventStart(7);
        logEventStart(8);
        logEventStart(9);
        
        before = clock_gettime(CLOCK_MONOTONIC, &motorBefore);
        //printf("FORWARD: FRONT %d RIGHT %d LEFT %d\n", distance[0][0], distance[1][0], distance[2][0]);

        //if(!((distance[0][0] < 400 && distance[1][0] < 400) || (distance[0][0] < 400 && distance[2][0] < 400))) {
        if(distance[0][0] >= DISTANCE_TO_WALL && distance[1][0] >= DISTANCE_TO_WALL && distance[2][0] >= DISTANCE_TO_WALL) {    
            if(rotate == 1) {
				logEventStart(16);
                openAndWrite("/sys/class/gpio/gpio18/value", "0"); //LEFTEN
                openAndWrite("/sys/class/gpio/gpio25/value", "0"); //RIGHTEN
                openAndWrite("/sys/class/gpio/gpio7/value", "0"); //RIGHTBACKWARD
                openAndWrite("/sys/class/gpio/gpio23/value", "0"); //LEFTFORWARD
                logEventEnd(16, "Motor IO Initialize after rotate");
                rotate = 0;
            }
            if (!running) {
				logEventStart(17);
                openAndWrite("/sys/class/gpio/gpio18/value", "1"); //LEFTEN
                openAndWrite("/sys/class/gpio/gpio25/value", "1"); //RIGHTEN
                logEventEnd(17, "Motor IO when not running");
                running = 1;
            }
            logEventStart(18);
            openAndWrite("/sys/class/gpio/gpio23/value", "1"); //LEFTFORWARD
            openAndWrite("/sys/class/gpio/gpio8/value", "1"); //RIGHTFORWARD
            logEventEnd(18, "Motor IO FORWARD");

            after = clock_gettime(CLOCK_MONOTONIC, &motorAfter);
            motorResult = diffTime(motorBefore, motorAfter);
            diff = MOTOR_DEADLINE - motorResult.tv_nsec;
            sleeptime.tv_sec = 0;
            sleeptime.tv_nsec = diff;
            logEventEnd(7, "Execusion time motor forward");
            if(clock_nanosleep(CLOCK_MONOTONIC, 0, &sleeptime, NULL)) {
                fprintf(stderr, "1 nanosleep failed (errno %d: %s)\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
            }
            logEventEnd(6, "Process time motor forward");
        } else {
            if(running == 1) {
	            logEventStart(19);
                openAndWrite("/sys/class/gpio/gpio18/value", "0");
                openAndWrite("/sys/class/gpio/gpio25/value", "0");
                openAndWrite("/sys/class/gpio/gpio23/value", "0"); //LEFTFORWARD
                openAndWrite("/sys/class/gpio/gpio8/value", "0"); //RIGHTFORWARD
                logEventEnd(19, "Motor IO disable forward");
                running = 0;
            }
            if(!rotate) {
                rotate = 1;
            }
            
            logEventStart(20);
            openAndWrite("/sys/class/gpio/gpio7/value", "1");
            openAndWrite("/sys/class/gpio/gpio23/value", "1");
            openAndWrite("/sys/class/gpio/gpio18/value", "1"); //LEFTEN
            openAndWrite("/sys/class/gpio/gpio25/value", "1"); //RIGHTEN
            logEventEnd(20, "Motor IO rotate");
            after = clock_gettime(CLOCK_MONOTONIC, &motorAfter);
            motorResult = diffTime(motorBefore, motorAfter);
            diff = MOTOR_DEADLINE - motorResult.tv_nsec;
            sleeptime.tv_sec = 0;
            sleeptime.tv_nsec = diff;
            logEventEnd(9, "Execusion time motor turn right");
            if(clock_nanosleep(CLOCK_MONOTONIC, 0, &sleeptime, NULL)) {
                fprintf(stderr, " 2nanosleep failed (errno %d: %s)\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
            }
            logEventEnd(8, "Process time motor turn right");
        }
    }
    return(EXIT_SUCCESS);
}


void *sensorMeasurement(threadArgs *sensor)
{
    int check = 0;
    long diff;
    int trigger = sensor->trigger;
    int echo = sensor->echo;
    int threadNumber = sensor->threadNumber;
    uint16_t *distanceTemp = sensor->distanceThread;
    int sensorBefore, sensorAfter, before, after;
    struct timespec sensorResult, sleeptime, req;
    struct timespec measureSensorBefore;
    struct timespec measureSensorAfter;
    struct timespec diffBefore, diffAfter, diffResult;
	

    pinMode(trigger, OUTPUT);
    pinMode(echo, INPUT);


    while(1) {
        logEventStart(threadNumber);
        logEventStart(threadNumber + 3);
        
        before = clock_gettime(CLOCK_MONOTONIC, &diffBefore);
        sleeptime.tv_sec = 0;
        sleeptime.tv_nsec = 000010000L; // sleep 10µs

		logEventStart(10);
        digitalWrite(trigger, 1);
        logEventEnd(10, "digital Write Trigger 1");

        if(clock_nanosleep(CLOCK_MONOTONIC, 0, &sleeptime, &req)) {
            fprintf(stderr, "4 nanosleep failed (errno %d: %s)\n", errno, strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        logEventStart(11);
        digitalWrite(trigger, 0);
        logEventEnd(11, "digital Write Trigger 0");

        while(1) {
			logEventStart(12);
			int echo_res = digitalRead(echo);
			logEventEnd(12, "digital Read 1 echo");
            if(echo_res == 1) {
                sensorBefore = clock_gettime(CLOCK_MONOTONIC, &measureSensorBefore);
                sleeptime.tv_nsec = 000200000L;
                if(clock_nanosleep(CLOCK_MONOTONIC, 0, &sleeptime, &req)) {
                    fprintf(stderr, "4 nanosleep failed (errno %d: %s)\n", errno, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                logEventStart(13);
                echo_res = digitalRead(echo);
                logEventEnd(13, "digital Read 2 echo");
                if(echo_res == 1) {
                    check = 0;
                    break;
                }
            }
            sleeptime.tv_nsec = 000200000L;
            if(clock_nanosleep(CLOCK_MONOTONIC, 0, &sleeptime, &req)) {
                fprintf(stderr, "4 nanosleep failed (errno %d: %s)\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
            }
            after = clock_gettime(CLOCK_MONOTONIC, &diffAfter);
            diffResult = diffTime(diffBefore, diffAfter);
            if(diffResult.tv_nsec >= 20000000L) {
                check = 1;
                //printf("timeout by trigger 0\n");
                break;
            }
        }

        if(check != 1) {
            while(1) {
				logEventStart(14);
				int echo_res = digitalRead(echo);
				logEventEnd(14, "digital Read 3 echo");
                if(echo_res == 0) {
                    sensorAfter = clock_gettime(CLOCK_MONOTONIC, &measureSensorAfter);
                    sleeptime.tv_nsec = 000200000L;
                    if(clock_nanosleep(CLOCK_MONOTONIC, 0, &sleeptime, &req)) {
                        fprintf(stderr, "4 nanosleep failed (errno %d: %s)\n", errno, strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    logEventStart(15);
                    echo_res = digitalRead(echo);
					logEventEnd(15, "digital Read 4 echo");
                    if(echo_res == 0) {
                        check = 0;
                        break;
                    }
                }
                sleeptime.tv_nsec = 000005000L;
                if(clock_nanosleep(CLOCK_MONOTONIC, 0, &sleeptime, &req)) {
                    fprintf(stderr, "4 nanosleep failed (errno %d: %s)\n", errno, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                after = clock_gettime(CLOCK_MONOTONIC, &diffAfter);
                diffResult = diffTime(diffBefore, diffAfter);
                if(diffResult.tv_nsec >= 20000000L) {
                    check = 1;
                    break;
                }
            }
        }

        //TODO: Here you must slepp too because value for difftime is negativ
        if(check == 1) {
            distance[threadNumber][0] = 1000;
            //printf("1000\n");
            check = 0;
            logEventEnd(threadNumber + 3, "Execution time Sensor %d without distance");
        } else {
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

            *distanceTemp = (uint64_t) sensorResult.tv_nsec * 343 / 1000000 / 2;
        }


        after = clock_gettime(CLOCK_MONOTONIC, &diffAfter);
        diffResult = diffTime(diffBefore, diffAfter);
        diff = SENSOR_DEADLINE - diffResult.tv_nsec;
        sleeptime.tv_sec = 0;
        sleeptime.tv_nsec = diff;
        logEventEnd(threadNumber + 3, "Execution time Sensor with distance");

        if(clock_nanosleep(CLOCK_MONOTONIC, 0, &sleeptime, &req)) {
            fprintf(stderr, "[%d] 5 nanosleep failed (%ld) (errno %d: %s)\n", threadNumber, diffResult.tv_nsec, errno, strerror(errno));
            exit(EXIT_FAILURE);
        }

        logEventEnd(threadNumber, "Process time Sensor");
    }
}


void initValueMatrix() {
    int i,j;
    for(i=0; i<NUMBER_OF_SENSORS; i++) {
        for(j=0; j<NUMBER_OF_VALUES; j++) {
            distance[i][j] = 0;
        }
    }
}

//

/*
 * The method opens and writes a value in a given file.
 */
void openAndWrite(char *path, char *value)
{
    FILE *fp;
    fp = fopen(path, "w");
    if(fp == NULL)
    {
        perror(path);
        exit(EXIT_FAILURE);
    }
    if(fputs(value, fp) == EOF)
    {
        perror("Write into file reached EOF");
        exit(EXIT_FAILURE);
    }
    fclose(fp);
}


/*
 * If `ctrl+c` is pressed, the programm will be ended
 * Source: http://openbook.rheinwerk-verlag.de/c_von_a_bis_z/020_c_headerdateien_007.htm - 2015-04-14, 10:34
 */
void sigfunc(int sig) {
    if(sig != SIGINT)
    {
        return;
    }
    else
    {
        saveLogToFile("log.csv");
        printf("\nEND\n");
        uninitializeMotor();
        digitalWrite(GPIO_US_LEFT_TRIG, 0);
        digitalWrite(GPIO_US_FRONT_TRIG, 0);
        digitalWrite(GPIO_US_RIGHT_TRIG, 0);
        exit(EXIT_SUCCESS);
    }
}


void initializeMotor() {
    openAndWrite("/sys/class/gpio/export", "23"); //LEFTFORWAR
    openAndWrite("/sys/class/gpio/export", "8");  //RIGHTFORWARD
    openAndWrite("/sys/class/gpio/gpio23/direction", "out");
    openAndWrite("/sys/class/gpio/gpio8/direction", "out");
    openAndWrite("/sys/class/gpio/gpio23/value", "1");
    openAndWrite("/sys/class/gpio/gpio8/value", "1");
    openAndWrite("/sys/class/gpio/export", "18"); //LEFTEN
    openAndWrite("/sys/class/gpio/export", "25");  //RIGHTEN
    openAndWrite("/sys/class/gpio/gpio18/direction", "out");
    openAndWrite("/sys/class/gpio/gpio25/direction", "out");
    openAndWrite("/sys/class/gpio/export", "24"); //LEFTBACKWARD
    openAndWrite("/sys/class/gpio/export", "7");  //RIGHTBACKWARD
    openAndWrite("/sys/class/gpio/gpio24/direction", "out");
    openAndWrite("/sys/class/gpio/gpio7/direction", "out");
}


void uninitializeMotor() {
    openAndWrite("/sys/class/gpio/unexport", "23"); //LEFTFORWAR
    openAndWrite("/sys/class/gpio/unexport", "8");  //RIGHTFORWARD
    openAndWrite("/sys/class/gpio/unexport", "18"); //LEFTEN
    openAndWrite("/sys/class/gpio/unexport", "25"); //RIGHTEN
    openAndWrite("/sys/class/gpio/export", "24"); //LEFTBACKWARD
    openAndWrite("/sys/class/gpio/export", "7");  //RIGHTBACKWARD
}







