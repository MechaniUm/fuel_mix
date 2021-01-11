#ifndef __ENCODER_H__
#define __ENCODER_H__

#include <Arduino.h>
#include <Encoder.h>

const int encoder_step_width = 20;
const int volume_step_width = 20;

int encoder_prev_vals[4];
double prev_encoder_vals[4]; // TODO: change var names
double encoder_vals[4];

Encoder encoders[4] = {
    Encoder(2, 14),
    Encoder(3, 15),
    Encoder(18, 12),
    Encoder(19, 13),
};

void EncoderSetup() {
    for (int i = 0; i < 4; i++) {
        encoder_prev_vals[i] = 0;
    }
}


void EncoderReset(int i, int val) {
    encoders[i].reset(val);
    encoder_prev_vals[i] = val;
    encoder_vals[i] = val;
}

void EncodersReset(int val) {
    for (int i = 0; i < 4; i++) {
        EncoderReset(i, val);
    }
}

int EncoderRead(int n) {
    return encoders[n].read() / encoder_step_width;
}

int prev_vol = 0;
void EncoderReadVolume() {
    int vol = encoders[0].read() / volume_step_width;
    if (prev_vol < vol) {
        if (volume > 0) {
            volume--;
        }
    } else if (prev_vol > vol) {
        if (volume < 30) {
            volume++;
        }
    }
    prev_vol = vol;
}


#endif
