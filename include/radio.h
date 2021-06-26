#ifndef __RADIO_H__
#define __RADIO_H__

#include <Arduino.h>
#include <rm4.h>

const int radio_signal = A8;
const int radio_power = 21;

static const int kData0Pin = A0;
static const int kData1Pin = A1;
static const int kData2Pin = A2;
static const int kData3Pin = A3;

volatile boolean radioFlag = false; 

RM4 remote(kData0Pin, kData1Pin, kData2Pin, kData3Pin);

void RadioButtonEvent1();
void RadioButtonEvent2();
void RadioButtonEvent3();

void RadioInterrupt() {
    int button_code = remote.buttonCode();
    switch (button_code)
    {
    case 8:
        break;
    case 4:
        RadioButtonEvent3();
        break;
    case 2:
        RadioButtonEvent2();
        break;
    case 1:
        RadioButtonEvent1();
        break;
    default:
        break;
    }
}
FireTimer radio_timer;
void ReadRadio() {
    if (radio_timer.fire()) {
        if (digitalRead(radio_signal) == HIGH) {
            RadioInterrupt();
        }
    }
}

void RadioSetup() {
    pinMode(radio_signal, INPUT); 
    radio_timer.begin(500);
}


#endif