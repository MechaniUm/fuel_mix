#ifndef __SERVO_H__
#define __SERVO_H__

#include <Arduino.h>

const int servo_pin = 5;

void ServoSetup() {
}

int last_angle = 0;
void ServoSet(double power) {
    int angle = map(power, 0, 100, 0, 170);
    if (last_angle != angle) {
        Wire.beginTransmission(I2C_SLAVE1_ADDRESS);
        Wire.write(angle);
        Wire.endTransmission();
        last_angle = angle;
    }
}

void ServoReset() {
    last_angle = 0;
    Wire.beginTransmission(I2C_SLAVE1_ADDRESS);
    Wire.write(0);
    Wire.endTransmission();
}

#endif