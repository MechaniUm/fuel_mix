#include <Arduino.h>
#include <FireTimer.h>
#include <Wire.h>
#include <advancedSerial.h>

#define I2C_SLAVE1_ADDRESS 11
#define I2C_SLAVE2_ADDRESS 12


const unsigned long setup_delay = 10000;
unsigned long setup_time;
const unsigned long ready_delay = 2000, stop_delay = 3500, danger_delay_1 = 1000, danger_delay_2 = 6500;
unsigned long ready_time = 0, stop_time = 0, danger_time = 0;
unsigned long afk_time;
const unsigned long afk_delay = 10000;

const int gold_combo[4][4] = { 
    {8, 92, 0, 0},
    {8, 92, 0, 80},
    {12, 19, 69, 0},
    {8, 47, 45, 50}
 };

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
    EncodersReset(0);
}

void FuelReset() {
    fuel = 0.0;
    air = 100.0;
    n2o = 0.0;
    boost = 0.0;
    power = 0.0;
    WorkChangeAnimationSpeed();
    DisplayNumber(0, fuel);
    DisplayNumber(1, air);
    DisplayNumber(2, n2o);
    DisplayNumber(3, boost);
}

void SlowingToWait() {
    HardwareReset();
    LightWaitStage();
    power_lock = true;
    stage = SLOWING_TO_WAIT;
    aSerial.l(Level::vvv).pln(F("STAGE: SLOWING_TO_WAIT"));
}

void SlowingToReady() {
    HardwareReset();
    // LightWaitStage();
    power_lock = true;
    stage = SLOWING_TO_READY;
    aSerial.l(Level::vvv).pln(F("STAGE: SLOWING_TO_READY"));
}

const unsigned long gold_delay = 15000;
unsigned long gold_timer = 0;
int gold_stage = 0;

void Wait() {
    HardwareReset();
    LightWaitStage();
    power_lock = true;
    stage = WAIT;
    gold_stage = 0;
    gold_timer = millis();
    aSerial.l(Level::vvv).pln(F("STAGE: WAIT"));
}

void HalfWait() {
    HardwareReset();
    LightWaitStage();
    stage = HALF_WAIT;
    gold_stage = 0;
    gold_timer = millis();
    aSerial.l(Level::vvv).pln(F("STAGE: HALF_WAIT"));
}

void Danger() {
    danger_animation_timer.begin(300);
    HardwareReset();
    LightDangerStage();
    power_lock = true;
    danger_time = millis();
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
    power_lock = true;
    aSerial.l(Level::vvv).pln(F("STAGE: STOP"));
}

void Ready() {
    HardwareReset();
    FuelReset();
    PlayerStop();
    LightReadyStage();
    power_lock = false;
    afk_time = millis();
    ready_time = 0;
    stop_time = 0;
    danger_time = 0;
    stage = READY;
    aSerial.l(Level::vvv).pln(F("STAGE: READY"));
}

void WorkUpdate() {
    UpdatePower();
    StepperSetSpeed(power);
    PlayerSetSound(power);
    ServoSet(power);
}

void VolumeChange() {
    HardwareReset();
    DisableLights();
    power_lock = true;
    aSerial.l(Level::vvv).pln(F("STAGE: VOLUME_CHANGE"));
    stage = VOLUME_CHANGE;
    setup_time = millis();
    prev_vol = encoders[0].read() / volume_step_width; // decrement or increment volume fix
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
    if (stage == READY || stage == WORK || stage == DANGER || stage == SLOWING || stage == HALF_WAIT) {
        SlowingToWait();
    }
}

void RadioButtonEvent2() {
    if (stage == WAIT) {
        ServoReset();
        Ready();
    }
}

void RadioButtonEvent3() {
    if (stage == WAIT) {
        VolumeChange();
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

bool danger_stage = false;

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
    case VOLUME_CHANGE:
        if (labs(millis() - setup_time) > setup_delay) {
            Wait();
            PlayerSetVolume(volume);
        } else {
            EncoderReadVolume();
            DisplayNumber(0, volume);
        }
        break;
    case WAIT:
        if (gold_stage % 2 == 0) {
            LightWaitAnimation();        // wait until RadioButtonEvent2()
        } else {
            LightWaitAnimation2();
        }
        if (labs(millis() - gold_timer) > gold_delay) {
            gold_timer = millis();
            gold_stage++;
            if (gold_stage == 8) {
                gold_stage = 0;
                wait_animation_timer.begin(100);
                for (int i = 0; i < speedometer_led_length; i++) {
                    speedometer_led.setPixelColor(i, speedometer_led.Color(0, 0, 0));
                }
                speedometer_led.show();
                ServoReset();
            } else if (gold_stage == 1) {
                for (int i = 0; i < speedometer_led_length; i++) {
                    speedometer_led.setPixelColor(i, speedometer_led.Color(95, 140, 0));
                }
                speedometer_led.show();
                DisplayNumber(0, gold_combo[0][0]);
                DisplayNumber(1, gold_combo[0][1]);
                DisplayNumber(2, gold_combo[0][2]);
                DisplayNumber(3, gold_combo[0][3]);
                ServoSet(Power(gold_combo[0][0], gold_combo[0][1], gold_combo[0][2], gold_combo[0][3]));
            } else if (gold_stage == 3) {
                for (int i = 0; i < speedometer_led_length; i++) {
                    speedometer_led.setPixelColor(i, speedometer_led.Color(95, 140, 0));
                }
                speedometer_led.show();
                DisplayNumber(0, gold_combo[1][0]);
                DisplayNumber(1, gold_combo[1][1]);
                DisplayNumber(2, gold_combo[1][2]);
                DisplayNumber(3, gold_combo[1][3]);
                ServoSet(Power(gold_combo[1][0], gold_combo[1][1], gold_combo[1][2], gold_combo[1][3]));
            } else if (gold_stage == 5) {
                for (int i = 0; i < speedometer_led_length; i++) {
                    speedometer_led.setPixelColor(i, speedometer_led.Color(95, 140, 0));
                }
                speedometer_led.show();
                DisplayNumber(0, gold_combo[2][0]);
                DisplayNumber(1, gold_combo[2][1]);
                DisplayNumber(2, gold_combo[2][2]);
                DisplayNumber(3, gold_combo[2][3]);
                ServoSet(Power(gold_combo[2][0], gold_combo[2][1], gold_combo[2][2], gold_combo[2][3]));
            } else if (gold_stage == 7) {
                for (int i = 0; i < speedometer_led_length; i++) {
                    speedometer_led.setPixelColor(i, headlight_led.Color(95, 140, 0));
                }
                speedometer_led.show();
                DisplayNumber(0, gold_combo[3][0]);
                DisplayNumber(1, gold_combo[3][1]);
                DisplayNumber(2, gold_combo[3][2]);
                DisplayNumber(3, gold_combo[3][3]);
                ServoSet(Power(gold_combo[3][0], gold_combo[3][1], gold_combo[3][2], gold_combo[3][3]));
            } else {
                wait_animation_timer.begin(100);
                for (int i = 0; i < speedometer_led_length; i++) {
                    speedometer_led.setPixelColor(i, speedometer_led.Color(0, 0, 0));
                }
                speedometer_led.show();
                ServoReset();
            }
        }
        break;
    case HALF_WAIT:
        EncoderReadFuel();
        EncoderReadAir();
        EncoderReadN2O();
        EncoderReadBoost();
        if (air < 100 || fuel > 0 || n2o > 0 || boost > 0) {
            Ready();
        }
        if (gold_stage % 2 == 0) {
            LightWaitAnimation();        // wait until RadioButtonEvent2()
        } else {
            LightWaitAnimation2();
        }
        if (labs(millis() - gold_timer) > gold_delay) {
            gold_timer = millis();
            gold_stage++;
            if (gold_stage == 8) {
                gold_stage = 0;
                wait_animation_timer.begin(100);
                for (int i = 0; i < speedometer_led_length; i++) {
                    speedometer_led.setPixelColor(i, speedometer_led.Color(0, 0, 0));
                }
                speedometer_led.show();
                ServoReset();
            } else if (gold_stage == 1) {
                for (int i = 0; i < speedometer_led_length; i++) {
                    speedometer_led.setPixelColor(i, speedometer_led.Color(95, 140, 0));
                }
                speedometer_led.show();
                DisplayNumber(0, gold_combo[0][0]);
                DisplayNumber(1, gold_combo[0][1]);
                DisplayNumber(2, gold_combo[0][2]);
                DisplayNumber(3, gold_combo[0][3]);
                ServoSet(Power(gold_combo[0][0], gold_combo[0][1], gold_combo[0][2], gold_combo[0][3]));
            } else if (gold_stage == 3) {
                for (int i = 0; i < speedometer_led_length; i++) {
                    speedometer_led.setPixelColor(i, speedometer_led.Color(95, 140, 0));
                }
                speedometer_led.show();
                DisplayNumber(0, gold_combo[1][0]);
                DisplayNumber(1, gold_combo[1][1]);
                DisplayNumber(2, gold_combo[1][2]);
                DisplayNumber(3, gold_combo[1][3]);
                ServoSet(Power(gold_combo[1][0], gold_combo[1][1], gold_combo[1][2], gold_combo[1][3]));
            } else if (gold_stage == 5) {
                for (int i = 0; i < speedometer_led_length; i++) {
                    speedometer_led.setPixelColor(i, speedometer_led.Color(95, 140, 0));
                }
                speedometer_led.show();
                DisplayNumber(0, gold_combo[2][0]);
                DisplayNumber(1, gold_combo[2][1]);
                DisplayNumber(2, gold_combo[2][2]);
                DisplayNumber(3, gold_combo[2][3]);
                ServoSet(Power(gold_combo[2][0], gold_combo[2][1], gold_combo[2][2], gold_combo[2][3]));
            } else if (gold_stage == 7) {
                for (int i = 0; i < speedometer_led_length; i++) {
                    speedometer_led.setPixelColor(i, headlight_led.Color(95, 140, 0));
                }
                speedometer_led.show();
                DisplayNumber(0, gold_combo[3][0]);
                DisplayNumber(1, gold_combo[3][1]);
                DisplayNumber(2, gold_combo[3][2]);
                DisplayNumber(3, gold_combo[3][3]);
                ServoSet(Power(gold_combo[3][0], gold_combo[3][1], gold_combo[3][2], gold_combo[3][3]));
            } else {
                wait_animation_timer.begin(100);
                for (int i = 0; i < speedometer_led_length; i++) {
                    speedometer_led.setPixelColor(i, speedometer_led.Color(0, 0, 0));
                }
                speedometer_led.show();
                ServoReset();
            }
        }
        break;
    case READY:
        UpdatePower();
        LightWorkAnimation();
        if (ready_time != 0 && (labs(millis() - ready_time) > ready_delay)) {
            if (power <= 0.0)
                Stop();
            else if (power > 100.0)
                Danger();
            else {
                StepperStart();
                Work();
            }
        } else if (millis() - afk_time > afk_delay) {
            HalfWait();
        } else {
            if (power > 0.0 && !ready_time) {
                ready_time = millis();
                PlayerTurningOnSound();
            }
        }
        
        break;
    
    case WORK:
        WorkUpdate();
        LightWorkAnimation();
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
        if (!danger_stage && labs(millis() - danger_time) > danger_delay_1) {
            danger_stage = true;
            PlayerAlertSound();
            danger_time = millis();
        } else if (labs(millis() - danger_time) > danger_delay_2) {
            if (!StepperIsRunning()) {
                danger_stage = false;
                Ready();
            }
        }
        break;

    case SLOWING:
        WorkUpdate();
        LightWorkAnimation();
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
        LightWorkAnimation();
        if (!StepperIsRunning()) {
            Ready();
        }
        break;
    case STOP:
        LightWorkAnimation();
        if (stop_time != 0 && (labs(millis() - stop_time) > stop_delay)) {
            Ready();
        }
        break;
    default:
        aSerial.l(Level::vvv).pln(F("DEFAULT CONDITION!!!"));
        break;
    }
}