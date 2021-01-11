#include <Arduino.h>
#include <FireTimer.h>
#include <Wire.h>
#include <advancedSerial.h>

# define I2C_SLAVE1_ADDRESS 11
# define I2C_SLAVE2_ADDRESS 12

#define PAYLOAD_SIZE 2

#include "radio.h"
#include "player.h"
#include "encoder.h"
#include "stepper.h"
#include "light.h"
#include "servo.h"

enum Stage {
    SETUP,
    WAIT,
    READY,
    WORK,
    DANGER,
    SLOWING,
    SLOWING_TO_WAIT,
    SLOWING_TO_READY
};

Stage stage;

const unsigned long setup_delay = 10000;
unsigned long setup_time;
const unsigned long ready_delay = 3000;
unsigned long ready_time = 0;
unsigned long afk_time;
const unsigned long afk_delay = 30000;

double Ratio(double fuel, double air, double n2o) {
    if (fuel == 100.0) return 0;
    return fuel / (air * 0.21 + n2o * 0.356);
}

double Fuel(double x) {
    if (x > 0.12 && x <= 0.23) {
        return 90.91 * x - 0.91;
    } else if (x > 0.23 && x <= 0.3) {
        return 428.6 * x - 78.49;
    } else if (x > 0.3 && x <= 0.4) {
        return 100.0 * x + 20;
    } else if (x > 0.4 && x <= 0.95) {
        return -90.91 * x + 96.3645;
    }
    return 0;
}

double NitroCoef(double nitro) {
    return 1.0 + nitro / 100.0;
}

double BoostCoef(double boost) {
    return boost * 0.5;
}

double Power(double fuel, double air, double n2o, double boost) {
    double fuel_oxygen_ratio = Ratio(fuel, air, n2o);
    double res = Fuel(fuel_oxygen_ratio) * NitroCoef(n2o);
    if (res)
        res += BoostCoef(boost);
    return res;
}

double fuel, air, n2o, boost, power;

void ChangeFuel(double x) {
    fuel += x;
    if (air > 0) {
        air -= x;

    } else {
        n2o -= x;
    }
    // if (n2o == 0) {
    //     air -= x;
    // } else {
    //     x = x / 2.0;
    //     air -= x;
    //     n2o -= x;
    // }
    if (fuel >= 100.0) {
        n2o = 0.0;
        air = 0.0;
        fuel = 100.0;
    }
}

void ChangeAir(double x) {
    air += x;
    if (n2o == 0) {
        fuel -= x;
    } else if (fuel == 0) {
        n2o -= x;
    } else {
        x = x / 2.0;
        fuel -= x;
        n2o -= x;
    }
    if (air >= 100.0) {
        n2o = 0.0;
        fuel = 0.0;
        air = 100.0;
    }
}

void ChangeN2O(double x) {
    if (x > 0 && air > 0) {
        n2o += x;
        air -= x;
    } else if (x < 0 && air < 100) {
        n2o += x;
        air -= x;
    }
    if (n2o >= 100.0) {
        n2o = 100.0;
        fuel = 0.0;
        air = 0.0;
    }
    // x = x / 2.0;
    // fuel -= x;
    // air -= x;
    // if (n2o >= 100.0) {
    //     fuel = 0.0;
    //     air = 0.0;
    //     n2o = 100.0;
    // } else if (n2o <= 0.0) {
    //     fuel = (int) fuel;
    //     air = 100 - fuel;
    // }
}

void ChangeBoost(double x) {
    boost += x;
    if (boost > 100)
        boost = 100;
    if (boost < 0)
        boost = 0;
}

void RadioButtonEvent1() {
    if (stage == READY || stage == WORK || stage == DANGER || stage == SLOWING) {
        stage = SLOWING_TO_WAIT;
        ServoReset();
        PlayerStop();
        aSerial.l(Level::vvv).pln(F("STAGE: WORKING -> SLOWING_TO_WAIT"));
        StepperStop();
        DisplayNumber(0, 0);
        DisplayNumber(1, 0);
        DisplayNumber(2, 0);
        DisplayNumber(3, 0);
        LightWaitStage();
    }
}

void RadioButtonEvent2() {
    if (stage == WAIT) {
        stage = READY;
        LightReadyStage();
        fuel = 0.0;
        air = 100.0;
        n2o = 0.0;
        boost = 0.0;
        power = 0.0;
        DisplayNumber(0, fuel);
        DisplayNumber(1, air);
        DisplayNumber(2, n2o);
        DisplayNumber(3, boost);
        aSerial.l(Level::vvv).pln(F("STAGE: WAIT -> READY"));
        ready_time = 0;
    }
}


int prev_fuel = 0;
void EncoderReadFuel() {
    int f = encoders[0].read() / encoder_step_width;
    if (prev_fuel < f) {
        if (fuel > 0) {
            ChangeFuel(-1);
            ChangeAnimationSpeed(0, 55 - map(fuel, 0, 100, 5, 40));
            ChangeAnimationSpeed(1, 55 - map(air, 0, 100, 5, 40));
            ChangeAnimationSpeed(2, 55 - map(n2o, 0, 100, 5, 40));
        }
    } else if (prev_fuel > f) {
        if (fuel < 100) {
            ChangeFuel(1);
            ChangeAnimationSpeed(0, 55 - map(fuel, 0, 100, 5, 40));
            ChangeAnimationSpeed(1, 55 - map(air, 0, 100, 5, 40));
            ChangeAnimationSpeed(2, 55 - map(n2o, 0, 100, 5, 40));
        }
    }
    prev_fuel = f;
}

int prev_air = 0;
void EncoderReadAir() {
    int f = encoders[1].read() / encoder_step_width;
    if (prev_air < f) {
        if (air > 0) {
            ChangeAir(-1);
            ChangeAnimationSpeed(0, 55 - map(fuel, 0, 100, 5, 40));
            ChangeAnimationSpeed(1, 55 - map(air, 0, 100, 5, 40));
            ChangeAnimationSpeed(2, 55 - map(n2o, 0, 100, 5, 40));
        }
    } else if (prev_air > f) {
        if (air < 100) {
            ChangeAir(1);
            ChangeAnimationSpeed(0, 55 - map(fuel, 0, 100, 5, 40));
            ChangeAnimationSpeed(1, 55 - map(air, 0, 100, 5, 40));
            ChangeAnimationSpeed(2, 55 - map(n2o, 0, 100, 5, 40));
        }
    }
    prev_air = f;
}


int prev_N2O = 0;
void EncoderReadN2O() {
    int f = encoders[2].read() / encoder_step_width;
    if (prev_N2O < f) {
        if (n2o > 0) {
            ChangeN2O(-1);
            ChangeAnimationSpeed(0, 55 - map(fuel, 0, 100, 5, 40));
            ChangeAnimationSpeed(1, 55 - map(air, 0, 100, 5, 40));
            ChangeAnimationSpeed(2, 55 - map(n2o, 0, 100, 5, 40));
        }
    } else if (prev_N2O > f) {
        if (n2o < 100) {
            ChangeN2O(1);
            ChangeAnimationSpeed(0, 55 - map(fuel, 0, 100, 5, 40));
            ChangeAnimationSpeed(1, 55 - map(air, 0, 100, 5, 40));
            ChangeAnimationSpeed(2, 55 - map(n2o, 0, 100, 5, 40));
        }
    }
    prev_N2O = f;
}

int prev_boost = 0;
void EncoderReadBoost() {
    int f = encoders[3].read() / encoder_step_width;
    if (prev_boost < f) {
        if (boost > 0) {
            ChangeBoost(-1);
            ChangeAnimationSpeed(3, 55 - map(boost, 0, 100, 5, 40));
        }

    } else if (prev_boost > f) {
        if (boost < 100) {
            ChangeBoost(1);
            ChangeAnimationSpeed(3, 55 - map(boost, 0, 100, 5, 40));
        }
    }
    prev_boost = f;
}

double prev_power = 0;
void UpdatePower() {
    EncoderReadFuel();
    EncoderReadAir();
    EncoderReadN2O();
    EncoderReadBoost();

    DisplayNumber(0, fuel);
    DisplayNumber(1, air);
    DisplayNumber(2, n2o);
    DisplayNumber(3, boost);

    if (!fuel) pipe_enable[0] = false; else pipe_enable[0] = true;
    if (!air) pipe_enable[1] = false; else pipe_enable[1] = true;
    if (!n2o) pipe_enable[2] = false; else pipe_enable[2] = true;
    if (!boost) pipe_enable[3] = false; else pipe_enable[3] = true;
    prev_power = power;
    power = Power(fuel, air, n2o, boost);
    if (prev_power != power) {
        afk_time = millis();
    }
}

void setup() {
    Serial.begin(9600);
    Wire.begin();
    delay(1000);
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

    ServoSetup();
    aSerial.l(Level::vv).pln(F("Servo setup successfull"));

    setup_time = millis();

    aSerial.l(Level::vv).pln(F("Setup successfull"));
}

void loop() {
    ReadRadio();
    switch (stage)
    {
    case SETUP:
        // the volume can be changed by rotating the first encoder
        // for $setup_delay milliseconds after setup
        if (millis() - setup_time > setup_delay) {
            stage = WAIT;
            PlayerSetVolume(volume);
            LightWaitStage();
            EncodersReset(0);
            ServoReset();
            PlayerStop();
            aSerial.l(Level::vvv).pln(F("STAGE: SETUP -> WAIT"));
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
        if (ready_time != 0 && (millis() - ready_time > ready_delay)) {
            stage = WORK;
            aSerial.l(Level::vvv).pln(F("STAGE: READY -> WORK"));
            StepperStart();
            PlayerTurningOnSound();
        } else {
            if (power > 0.0 && !ready_time) {
                ready_time = millis();
            }
        }
        break;
    
    case WORK:
        if (millis() - afk_time > afk_delay) {
            stage = SLOWING_TO_READY;
            aSerial.l(Level::vvv).pln(F("STAGE: WORK -> SLOWING_TO_READY"));
            ServoReset();
            PlayerStop();
            StepperStop();
            fuel = 0.0;
            air = 100.0;
            n2o = 0.0;
            boost = 0.0;
            power = 0.0;
            DisplayNumber(0, 0);
            DisplayNumber(1, 0);
            DisplayNumber(2, 0);
            DisplayNumber(3, 0);
        }
        LightWorkAnimation();
        UpdatePower();
        if (power > 100.0) {
            ServoReset();
            PlayerStop();
            aSerial.l(Level::vvv).pln(F("STAGE: WORK -> DANGER"));
            stage = DANGER;
            StepperStop();
            LightDangerStage();
        } else if (power <= 0.0) {
            aSerial.l(Level::vvv).pln(F("STAGE: WORK -> SLOWING"));
            StepperStop();
            stage = SLOWING;
        }else {
            StepperSetSpeed(power);
            // PlayerSetSound(power);
            ServoSet(power);
        }
        break;

    case DANGER:
        LightDangerAnimation();
        if (!StepperIsRunning()) {
            stage = READY;
            LightReadyStage();
            fuel = 0.0;
            air = 100.0;
            n2o = 0.0;
            boost = 0.0;
            power = 0.0;
            DisplayNumber(0, fuel);
            DisplayNumber(1, air);
            DisplayNumber(2, n2o);
            DisplayNumber(3, boost);
            EncodersReset(0);
            ready_time = 0;
            aSerial.l(Level::vvv).pln(F("STAGE: DANGER -> READY"));
        }
        break;

    case SLOWING:
        LightWorkAnimation();
        UpdatePower();
        StepperSetSpeed(power);
        // PlayerSetSound(power);
        ServoSet(power);
        
        if (!StepperIsRunning()) {
            stage = READY;
            LightReadyStage();
            ready_time = 0;
            aSerial.l(Level::vvv).pln(F("STAGE: SLOWING -> READY"));
        } else if (power > 0.0) {
            stage = WORK;
            StepperResume();
            aSerial.l(Level::vvv).pln(F("STAGE: SLOWING -> WORK"));
        }
        break;
    case SLOWING_TO_WAIT:
        if (!StepperIsRunning()) {
            stage = WAIT;
            LightWaitStage();
            EncodersReset(0);
            ServoReset();
            PlayerStop();
            aSerial.l(Level::vvv).pln(F("STAGE: SLOWING_TO_WAIT -> WAIT"));
        }
        break;
    case SLOWING_TO_READY:

        LightWorkAnimation();
        if (!StepperIsRunning()) {
            stage = READY;
            LightReadyStage();
            fuel = 0.0;
            air = 100.0;
            n2o = 0.0;
            boost = 0.0;
            power = 0.0;
            DisplayNumber(0, fuel);
            DisplayNumber(1, air);
            DisplayNumber(2, n2o);
            DisplayNumber(3, boost);
            EncodersReset(0);
            ready_time = 0;
            ServoReset();
            PlayerStop();
            aSerial.l(Level::vvv).pln(F("STAGE: SLOWING_TO_READY -> READY"));
        }
        break;
    default:
        aSerial.l(Level::vvv).pln(F("DEFAULT CONDITION!!!"));
        break;
    }
}