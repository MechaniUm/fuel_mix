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
    DANGER,
    SLOWING,
    SLOWING_TO_WAIT
};

Stage stage;

const unsigned long setup_delay = 10000;
unsigned long setup_time;
const unsigned long ready_delay = 1000;
unsigned long ready_time = 0;

double Ratio(double fuel, double air, double n2o) {
    if (fuel == 100.0) return 0; // TODO: узнать, что делать в такой ситуации
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
    return nitro / 100.0 + 1.0;
}

double BoostCoef(double boost) {
    return boost * 0.5;
}

double Power(double fuel, double air, double n2o, double boost) {
    double fuel_oxygen_ratio = Ratio(fuel, air, n2o);
    return Fuel(fuel_oxygen_ratio) * NitroCoef(n2o) + BoostCoef(boost);
}

double fuel, air, n2o, boost, power;

void ChangeFuel(double x) {
    fuel += x;
    x = x / 2.0;
    air -= x;
    n2o -= x;
}

void ChangeAir(double x) {
    air += x;
    x = x / 2.0;
    n2o -= x;
    fuel -= x;
}

void ChangeN2O(double x) {
    n2o += x;
    x = x / 2.0;
    air -= x;
    fuel -= x;
}

void ChangeBoost(double x) {
    boost += x;
}

void RadioButtonEvent1() {
    if (stage == READY || stage == WORK || stage == DANGER || stage == SLOWING) {
        stage = SLOWING_TO_WAIT;
        aSerial.l(Level::vvv).pln(F("STAGE: WORKING -> SLOWING_TO_WAIT"));
        fuel = 0.0;
        air = 100.0;
        n2o = 0.0;
        boost = 0.0;
        power = 0.0;
        // TODO: change light mode to wait
    }
}

void RadioButtonEvent2() {
    if (stage == WAIT) {
        stage = READY;
        LightReadyStage();
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
            aSerial.l(Level::vvv).pln(F("STAGE: SETUP -> WAIT"));
        } else {
            int vol = EncoderReadVolume();
            PlayerSetVolume(vol);
            DisplayNumber(0, vol);
        }
        break;
    case WAIT:
        // wait until RadioButtonEvent2()
        break;
    
    case READY:
        if (ready_time && millis() - ready_time > ready_delay) {
            stage = WORK;
            // TODO: play engine's turning on sound
        } else {
            // TODO: maybe only one encoder at time 
            // TODO: вынести дублирующуюся историю с энкодерами в отдельную функцию
            encoder_vals[0] = EncoderRead(0); // fuel
            encoder_vals[1] = EncoderRead(1); // air
            encoder_vals[2] = EncoderRead(2); // n2o
            encoder_vals[3] = EncoderRead(3); // boost
            
             // TODO: use ChangeFuel() etc.
            for (int i = 0; i < 4; i++) {
                encoder_vals[i] = EncoderRead(i);
                if (encoder_vals[i] < 0) {
                    encoder_vals[i] = 0.0;
                } else if (encoder_vals[i] > 100) {
                    encoder_vals[i] = 100.0;
                }

                DisplayNumber(i, encoder_vals[i]);
            }

            power = Power(fuel, air, n2o, boost);
            if (power > 0.0) {
                ready_time = millis();
            } else if (power < 0.0) {
                aSerial.l(Level::vvv).pln(F("Power less then zero!!!"));
            }

        }
        break;
    
    case WORK:
        if (power > 100.0) {
            aSerial.l(Level::vvv).pln(F("STAGE: WORK -> DANGER"));
            stage = DANGER;
            LightDangerStage();
        } else if (power > 0.0) {
            aSerial.l(Level::vvv).pln(F("STAGE: WORK -> SLOWING"));
        } else if (power < 0.0) {
            aSerial.l(Level::vvv).pln(F("Power less then zero!!!"));
        } else {
            // TODO: add pipes animation
            
            // TODO: maybe only one encoder at time 
            encoder_vals[0] = EncoderRead(0); // fuel
            encoder_vals[1] = EncoderRead(1); // air
            encoder_vals[2] = EncoderRead(2); // n2o
            encoder_vals[3] = EncoderRead(3); // boost
            
             // TODO: use ChangeFuel() etc.
            for (int i = 0; i < 4; i++) {
                encoder_vals[i] = EncoderRead(i);
                if (encoder_vals[i] < 0) {
                    encoder_vals[i] = 0.0;
                } else if (encoder_vals[i] > 100) {
                    encoder_vals[i] = 100.0;
                }

                DisplayNumber(i, encoder_vals[i]);
            }

            power = Power(fuel, air, n2o, boost);

            StepperSetSpeed(power);
            PlayerSetSound(power);

        }
        break;

    case DANGER:
        // TODO: add danger animation
        break;

    case SLOWING:
        if (!stepper.isRunning()) {
            stage = READY;
            aSerial.l(Level::vvv).pln(F("STAGE: SLOWING -> READY"));
        }
        if (power > 0.0) {
            stage = WORK;
            aSerial.l(Level::vvv).pln(F("STAGE: SLOWING -> WORK"));
        }  else if (power < 0.0) {
            aSerial.l(Level::vvv).pln(F("Power less then zero!!!"));
        } else {
            // TODO: add pipes animation
            
            // TODO: maybe only one encoder at time 
            encoder_vals[0] = EncoderRead(0); // fuel
            encoder_vals[1] = EncoderRead(1); // air
            encoder_vals[2] = EncoderRead(2); // n2o
            encoder_vals[3] = EncoderRead(3); // boost
            
             // TODO: use ChangeFuel() etc.
            for (int i = 0; i < 4; i++) {
                encoder_vals[i] = EncoderRead(i);
                if (encoder_vals[i] < 0) {
                    encoder_vals[i] = 0.0;
                } else if (encoder_vals[i] > 100) {
                    encoder_vals[i] = 100.0;
                }

                DisplayNumber(i, encoder_vals[i]);
            }

            power = Power(fuel, air, n2o, boost);

            StepperSetSpeed(power);
            PlayerSetSound(power);

        }
        break;

    case SLOWING_TO_WAIT:
        if (!stepper.isRunning()) {
            stage = WAIT;
        }
        break;
    default:
        aSerial.l(Level::vvv).pln(F("DEFAULT CONDITION!!!"));
        break;
    }
}