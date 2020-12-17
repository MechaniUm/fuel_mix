#ifndef __SERVO_H__
#define __SERVO_H__

#include <Arduino.h>
#include <Servo.h>

Servo speedometer_servo;
const int servo_pin = 5;

void ServoSetup() {
    speedometer_servo.attach(servo_pin);
}

#endif