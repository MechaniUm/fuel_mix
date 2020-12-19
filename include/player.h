#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <Arduino.h>
#include <DFPlayerMini_Fast.h>

DFPlayerMini_Fast mp3;

void PlayerSetup() {
    Serial2.begin(9600);
    mp3.begin(Serial2);
    mp3.volume(15);
    delay(20);
}

void PlayerSetSound(double power) {
    // TODO: не менять, если индекс остался старым
    if (mp3.isPlaying())
        mp3.stop();
    mp3.loop((int)power / 9);
}

void PlayerSetVolume(int vol) {
    mp3.volume(vol);
}

#endif