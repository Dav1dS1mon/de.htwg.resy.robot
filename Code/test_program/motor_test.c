#include <stdlib.h>
#include <stdio.h>

// include sched.h for setting max priority and a different scheduler
#include <sched.h>

// include stdint.h for types like uint8_t
#include <stdint.h>

// include wiringPi.h for access to GPIOs
#include <wiringPi.h>
// include softPwm.h for using Soft-PWM for the motors
#include <softPwm.h>

// include GPIO defines
#include "../core/GPIO_defines.h"


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

int main() {
    int status;

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

    delay(8000);
    // Setup wiringPi and configure motor GPIOs;
    // It's important to notice that we use wiringPiSetup*Gpio* here as this
    // means, that we've decided addressing the GPIOs using the GPIO names, not
    // the pin numbers nor the internal wiringPi numbers!
    wiringPiSetupGpio();
    motorGpioSetup();

    // here the test sequence starts
    // =============================
    // go forward 2 seconds 50% speed
    motorLeftSetDirection(MOTOR_FORWARD);
    motorRightSetDirection(MOTOR_FORWARD);
    motorLeftSetSpeed(50);
    motorRightSetSpeed(50);
    delay(2000);
    // go forward 3 seconds 100% speed
    motorLeftSetSpeed(100);
    motorRightSetSpeed(100);
    delay(3000);
    // turn around clockwise for 2.5 seconds 100% speed
    motorRightSetDirection(MOTOR_BACKWARD);
    delay(2500);
    motorRightSetDirection(MOTOR_FORWARD);
    // stop for 2 seconds
    motorLeftSetSpeed(0);
    motorRightSetSpeed(0);
    delay(2000);
    // go forward for 3 seconds 10% speed
    motorLeftSetSpeed(10);
    motorRightSetSpeed(10);
    delay(3000);
    // go backward for 2 seconds 100% speed
    motorLeftSetDirection(MOTOR_BACKWARD);
    motorRightSetDirection(MOTOR_BACKWARD);
    motorLeftSetSpeed(100);
    motorRightSetSpeed(100);
    // stop forever
    motorLeftSetSpeed(0);
    motorRightSetSpeed(0);
	softPwmStop(GPIO_MOTOR_LEFT_ENABLE);
	softPwmStop(GPIO_MOTOR_RIGHT_ENABLE);

    /* The summary of what the robot should've done:
     *   - go forward 2 seconds 50% speed
     *   - go forward 3 seconds 100% speed
     *   - turn around clockwise for 2.5 seconds 100% speed
     *   - stop for 2 seconds
     *   - go forward for 3 seconds 10% speed
     *   - go backward for 2 seconds 100% speed
     *   - stop forever
     */

    return(EXIT_SUCCESS);
}


/*
 * Set the maximum available priority and the correct scheduler for this
 * process.
 */
int setMaxPriority() {
    int maxPriority;
    int status;
    struct sched_param sched;

    // get max available priority
    maxPriority = sched_get_priority_max(SCHED_FIFO);
    if (maxPriority == -1) {
        // not able to determine max available priority
        return 1;
    }

    // set scheduler to SCHED_FIFO and priority to max priority
    sched.sched_priority = maxPriority;
    status = sched_setscheduler(0, SCHED_FIFO, &sched);

    if (status) {
        return 2;
    }

    return 0;
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
