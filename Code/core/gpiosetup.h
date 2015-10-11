#ifndef GPIOSETUP_H
#define GPIOSETUP_H

#include "stdint.h"

// define motor directions
enum MotorDirection_{MOTOR_FORWARD, MOTOR_BACKWARD};
typedef enum MotorDirection_ MotorDirection;

void motorGpioSetup();
void motorLeftSetSpeed(uint8_t speed);
void motorRightSetSpeed(uint8_t speed);

void motorLeftSetDirection(MotorDirection direction);
void motorRightSetDirection(MotorDirection direction);

uint8_t readGPIO(uint8_t number);

#endif
