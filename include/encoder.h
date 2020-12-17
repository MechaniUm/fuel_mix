#ifndef __ENCODER_H__
#define __ENCODER_H__

#include <Arduino.h>
#include <Encoder.h>

const int step_width = 100;
const int volume_step_width = 500;

Encoder encoders[4] = {
    Encoder(2, 14),
    Encoder(3, 15),
    Encoder(18, 0),
    Encoder(19, 1),
};

int ReadEncoder(int n) {
    return encoders[n].read();
}

void ResetEncoders() {
    for (int i = 0; i < 4; i++) {
        encoders[i].reset(0);
    }
}


#endif
