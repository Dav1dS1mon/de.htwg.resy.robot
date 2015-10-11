#include "gpiosetup.h"
#include "GPIO_defines.h"
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>

void motorSetSpeed(uint8_t pin, uint8_t speed); // private function
void motorSetDirection(uint8_t pinMotorForward,
                            uint8_t pinMotorBackward,
                            MotorDirection direction); // private function

/*
 * Setup motor GPIOs as outputs and set them to LOW
 */
void motorGpioSetup() {
    // left motor
    // -----------------------------------------------
    // we configure the soft PWM to have the range 0-100; init value: 0
    /*if (softPwmCreate(GPIO_MOTOR_LEFT_ENABLE, 0, 100)) {
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
    digitalWrite(GPIO_MOTOR_RIGHT_BACKWARD, LOW);*/
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
    //softPwmWrite(pin, speed);
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
