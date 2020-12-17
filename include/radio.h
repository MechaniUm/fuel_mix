#ifndef __RADIO_H__
#define __RADIO_H__

#include <Arduino.h>
#include <rm4.h>

const int radio_signal = 20;
const int radio_power = 21;

static const int kData0Pin = A0;
static const int kData1Pin = A1;
static const int kData2Pin = A2;
static const int kData3Pin = A3;

volatile boolean radioFlag = false; 

RM4 remote(kData0Pin, kData1Pin, kData2Pin, kData3Pin);

void RadioButtonEvent1();
void RadioButtonEvent2();

void RadioInterrupt() {
    int button_code = remote.buttonCode();
    switch (button_code)
    {
    case 8:
        RadioButtonEvent1();
        break;
    case 4:
        RadioButtonEvent2();
        break;
    case 2:
        break;
    case 1:
        break;
    default:
        break;
    }
}


void RadioSetup() {
    pinMode(radio_signal, INPUT);
    pinMode(radio_power, OUTPUT);
    digitalWrite(radio_power, HIGH);
    attachInterrupt(digitalPinToInterrupt(radio_signal), RadioInterrupt, RISING);    
}

#endif