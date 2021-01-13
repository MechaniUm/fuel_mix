#include <Arduino.h>
#include <FireTimer.h>
#include <Wire.h>
#include <advancedSerial.h>

#define I2C_SLAVE1_ADDRESS 11
#define I2C_SLAVE2_ADDRESS 12


const unsigned long setup_delay = 10000;
unsigned long setup_time;
const unsigned long ready_delay = 6000, stop_delay = 3500;
unsigned long ready_time = 0, stop_time = 0;
unsigned long afk_time;
const unsigned long afk_delay = 30000;

#define PAYLOAD_SIZE 2

#include "stage.h"
#include "player.h"
#include "encoder.h"
#include "stepper.h"
#include "light.h"
#include "servo.h"
#include "fuel.h"
#include "radio.h"

void HardwareReset() {
    ServoReset();
    StepperStop();
    LightWaitStage();
    EncodersReset(0);
}

void FuelReset() {
    fuel = 0.0;
    air = 100.0;
    n2o = 0.0;
    boost = 0.0;
    power = 0.0;
    DisplayNumber(0, fuel);
    DisplayNumber(1, air);
    DisplayNumber(2, n2o);
    DisplayNumber(3, boost);
}

void SlowingToWait() {
    HardwareReset();
    stage = SLOWING_TO_WAIT;
    aSerial.l(Level::vvv).pln(F("STAGE: SLOWING_TO_WAIT"));
}

void SlowingToReady() {
    HardwareReset();
    stage = SLOWING_TO_READY;
    aSerial.l(Level::vvv).pln(F("STAGE: SLOWING_TO_READY"));
}

void Wait() {
    HardwareReset();
    stage = WAIT;
    aSerial.l(Level::vvv).pln(F("STAGE: WAIT"));
}

void Danger() {
    HardwareReset();
    LightDangerStage();
    PlayerAlertSound();
    stage = DANGER;
    aSerial.l(Level::vvv).pln(F("STAGE: DANGER"));
}

void Work() {
    stage = WORK;
    aSerial.l(Level::vvv).pln(F("STAGE: WORK"));
}

void Slowing() {
    StepperStop();
    stage = SLOWING;
    aSerial.l(Level::vvv).pln(F("STAGE: SLOWING"));
}

void Stop() {
    HardwareReset();
    stop_time = millis();
    digitalWrite(headlight_pin, HIGH);
    PlayerTurningOffSound();
    stage = STOP;
    aSerial.l(Level::vvv).pln(F("STAGE: STOP"));
}

void Ready() {
    PlayerStop();
    LightReadyStage();
    FuelReset();
    afk_time = millis();
    ready_time = 0;
    stop_time = 0;
    stage = READY;
    aSerial.l(Level::vvv).pln(F("STAGE: READY"));
}

void WorkUpdate() {

    LightWorkAnimation();
    UpdatePower();
    StepperSetSpeed(power);
    PlayerSetSound(power);
    ServoSet(power);
}

void Setup() {
    stage = SETUP;
    aSerial.l(Level::vv).pln(F("STAGE: SETUP"));
    
    RadioSetup();
    aSerial.l(Level::vv).pln(F("Radio setup successfull"));

    PlayerSetup();
    aSerial.l(Level::vv).pln(F("Player setup successfull"));

    StepperSetup();
    aSerial.l(Level::vv).pln(F("Stepper setup successfull"));

    LightSetup();
    aSerial.l(Level::vv).pln(F("Light setup successfull"));

    ServoSetup();
    aSerial.l(Level::vv).pln(F("Servo setup successfull"));

    setup_time = millis();

    aSerial.l(Level::vv).pln(F("Setup successfull"));
}

void RadioButtonEvent1() {
    if (stage == READY || stage == WORK || stage == DANGER || stage == SLOWING) {
        SlowingToWait();
    }
}

void RadioButtonEvent2() {
    if (stage == WAIT) {
        Ready();
    }
}

void setup() {
    Wire.begin();
    delay(1000);

    Serial.begin(9600);
    aSerial.setPrinter(Serial);
    aSerial.setFilter(Level::vvv);
    
    Setup();
    
}

void loop() {
    ReadRadio();
    switch (stage)
    {
    case SETUP:
        // the volume can be changed by rotating the first encoder
        // for setup_delay milliseconds after setup
        if (labs(millis() - setup_time) > setup_delay) {
            Wait();
            PlayerSetVolume(volume);
        } else {
            EncoderReadVolume();
            DisplayNumber(0, volume);
        }
        break;
    case WAIT:
        // wait until RadioButtonEvent2()
        break;
    
    case READY:
        UpdatePower();
        LightWorkAnimation();
        if (ready_time != 0 && (labs(millis() - ready_time) > ready_delay)) {
            StepperStart();
            Work();
        } else if (millis() - afk_time > afk_delay) {
            Wait();
        } else {
            if (power > 0.0 && !ready_time) {
                ready_time = millis();
                PlayerTurningOnSound();
            }
        }
        
        break;
    
    case WORK:
        WorkUpdate();
        if (power > 100.0) {
            Danger();
        } else if (power <= 0.0) {
            Slowing();
        } else if (labs(millis() - afk_time) > afk_delay) {
            SlowingToReady();
        }
        break;

    case DANGER:
        LightDangerAnimation();
        if (!StepperIsRunning()) {
            Ready();
        }
        break;

    case SLOWING:
        WorkUpdate();
        if (!StepperIsRunning()) {
            Stop();
        } else if (power > 0.0) {
            Work();
            // StepperResume();
        }
        break;
    case SLOWING_TO_WAIT:
        if (!StepperIsRunning()) {
            Wait();
        }
        break;
    case SLOWING_TO_READY:
        if (!StepperIsRunning()) {
            Ready();
        }
        break;
    case STOP:
        if (stop_time != 0 && (labs(millis() - stop_time) > stop_delay)) {
            Ready();
        }
        break;
    default:
        aSerial.l(Level::vvv).pln(F("DEFAULT CONDITION!!!"));
        break;
    }
}