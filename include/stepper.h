#ifndef __STEPPER_H__
#define __STEPPER_H__

#include <Arduino.h>
#include <AccelStepper.h>

const int stepper_pin = 44;
const int stepper_out_pin = 49;
void StepperSetup() {
    pinMode(stepper_pin, OUTPUT);
    pinMode(stepper_out_pin, INPUT);
    analogWrite(stepper_pin, 0);
}

int last_speed_idx = 0;
void StepperSetSpeed(double power) {
    int idx = map(power, 0, 100, 0, 11);
    if (last_speed_idx != idx) {
        Wire.beginTransmission(I2C_SLAVE2_ADDRESS);
        Wire.write(idx + 2);
        Wire.endTransmission();
        last_speed_idx = idx;
    }
}

void StepperResume() {
    // aSerial.pln("resume");
    Wire.beginTransmission(I2C_SLAVE2_ADDRESS);
    Wire.write(2);
    Wire.endTransmission();
}

void StepperStop() {
    // aSerial.pln("stop");
    Wire.beginTransmission(I2C_SLAVE2_ADDRESS);
    Wire.write(0);
    Wire.endTransmission();
}

void StepperStart() {
    // aSerial.pln("start");
    Wire.beginTransmission(I2C_SLAVE2_ADDRESS);
    Wire.write(1);
    Wire.endTransmission();
}

bool StepperIsRunning() {
    Wire.requestFrom(I2C_SLAVE2_ADDRESS, 1);   
    int res = Wire.read();
    return res;
}

#endif
