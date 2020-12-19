#ifndef __STEPPER_H__
#define __STEPPER_H__

#include <Arduino.h>
#include <AccelStepper.h>
#include <GyverTimers.h>

const int stepper_step = 53;
const int stepper_dir = 51;
const int stepper_en = 49;

const int stepper_tick = 20;

const int stepper_speed[11] = {
    800, 1000, 1250, 1500, 1750,
    2000, 2250, 2500, 2750, 3000,
    3500
};
const int stepper_acceleration = 300;

AccelStepper stepper(AccelStepper::DRIVER, stepper_step, stepper_dir);

void StepperSetup() {
    pinMode(stepper_en, OUTPUT);
    pinMode(stepper_en, HIGH);

    stepper.setMaxSpeed(stepper_speed[0]);
    stepper.setAcceleration(stepper_acceleration);

    Timer5.setPeriod(stepper_tick);
    Timer5.enableISR();
}

void StepperStop() {
    stepper.stop();
}

void StepperRun() {
    stepper.run();
}

void StepperStart() {
    stepper.setCurrentPosition(0);
    stepper.moveTo(999999);
}

void StepperSetSpeed(double power) {
    // TODO: не менять, если индекс остался старым
    stepper.setMaxSpeed(stepper_speed[(int)power / 9]);
}

ISR(TIMER5_A) {
    StepperRun();
}

#endif
