#include <Arduino.h>
#include <advancedSerial.h>
#include "radio.h"
#include "encoder.h"
#include "stepper.h"
#include "light.h"
#include "player.h"
// #include "servo.h"

enum Stage {
    SETUP,
    WAIT,
    READY,
    WORK,
    DANGER
};

Stage stage;

const unsigned long setup_delay = 10000;
unsigned long setup_time;

void RadioButtonEvent1() {
    if (stage == READY) {
        stage = WAIT;
        aSerial.l(Level::vvv).pln(F("STAGE: READY -> WAIT"));
    }
}

void RadioButtonEvent2() {
    if (stage == WAIT) {
        stage = READY;
        aSerial.l(Level::vvv).pln(F("STAGE: WAIT -> READY"));
    }
}

void setup() {
    Serial.begin(9600);
    aSerial.setPrinter(Serial);
    aSerial.setFilter(Level::vvv);
    
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

    // ServoSetup();
    aSerial.l(Level::vv).pln(F("Servo setup successfull"));

    setup_time = millis();

    aSerial.l(Level::vv).pln(F("Setup succesfull"));
}

void loop() {
    switch (stage)
    {
    case SETUP:
        // the volume can be changed by rotating the first encoder
        // for $setup_delay milliseconds after setup
        if (millis() - setup_time > setup_delay) {
            stage = WAIT;
            LightWaitStage();
            // LightDangerStage();
            aSerial.l(Level::vvv).pln(F("STAGE: SETUP -> WAIT"));
            // StepperStart();
        } else {

        }
        break;
    
    default:
        break;
    }
}