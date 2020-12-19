#ifndef __ENCODER_H__
#define __ENCODER_H__

#include <Arduino.h>
#include <Encoder.h>

const int encoder_step_width = 100;
const int volume_step_width = 500;

int encoder_prev_vals[4];
double encoder_vals[4];

Encoder encoders[4] = {
    Encoder(2, 14),
    Encoder(3, 15),
    Encoder(18, 0),
    Encoder(19, 1),
};

void EncoderSetup() {
    for (int i = 0; i < 4; i++) {
        encoder_prev_vals[i] = 0;
    }
}

int EncoderRead(int n) {
    return encoders[n].read() / encoder_step_width;
}

int EncoderReadVolume() {
    int vol = encoders[0].read() / volume_step_width;
    if (vol < 0)
        vol = 0;
    else if (vol > 30)
        vol = 30;
    return vol;
}

void EncoderReset() {
    for (int i = 0; i < 4; i++) {
        encoders[i].reset(0);
        encoder_prev_vals[i] = 0;
        encoder_vals[i] = 0;
    }
}


#endif
