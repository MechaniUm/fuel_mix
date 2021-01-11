#include <Arduino.h>
#include <AccelStepper.h>
#include <Wire.h>

#define I2C_SLAVE_ADDRESS 12

#define PAYLOAD_SIZE 2

const int stepper_step = 9;
const int stepper_dir = 5;
const int stepper_en = 6;
AccelStepper stepper(1, stepper_step, stepper_dir);

boolean slowing = false;

int n = 0;

int stepper_speed[11];

void CalcStepperSpeed() {
    for (int i = 0; i < 11; i++) {
        stepper_speed[i] = map(i, 0, 10, 800, 2200);
    }
}

int target_speed = 0;

void StepperSetup() {
    pinMode(stepper_en, OUTPUT);
    digitalWrite(stepper_en, LOW);
    stepper.setAcceleration(300);
    target_speed = 0;
}

void StepperStop() {
    stepper.move(stepper.speed() * 4);
    slowing = true;
    target_speed = 0;
}

bool StepperIsRunning() {
    return stepper.isRunning();
}

void StepperRun() {
    if (slowing && stepper.speed() <= target_speed) {
        slowing = false;
        stepper.setMaxSpeed(target_speed);
        if (target_speed != 0) {
            stepper.moveTo(999999);
        } else {
            stepper.stop();
        }
    }
    stepper.run();
}

void StepperResume() {

}

void StepperStart() {
    stepper.setCurrentPosition(0);
    stepper.setMaxSpeed(stepper_speed[0]);
    stepper.moveTo(999999);
}

void StepperSetSpeed(int speed) {
    if (speed > stepper.speed()) {
        if (slowing) {
            slowing = false;
            stepper.moveTo(999999);
        }
        stepper.setMaxSpeed(speed);
    } else {
        stepper.move(speed * 4); // slow down
        slowing = true;
    }
    target_speed = speed;
}

void requestEvents()
{   
    Wire.write(StepperIsRunning() ? 1 : 0);
}

void receiveEvents(int numBytes)
{  
    n = Wire.read();
    if (n == 0) {
        StepperStop();
    } else if (n == 1) {
        StepperStart();
    } else if (n == 2) {
        StepperResume();
    } else if (n >= 3 && n <= 13) {
        StepperSetSpeed(stepper_speed[n - 3]);
    }
}

void setup()
{
    // Serial.begin(9600);
    CalcStepperSpeed();
    StepperSetup();
    Wire.begin(I2C_SLAVE_ADDRESS);
    delay(1000);               
    Wire.onRequest(requestEvents);
    Wire.onReceive(receiveEvents);
}

void loop(){
    StepperRun();
}
