#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <Arduino.h>
#include <DFPlayerMini_Fast.h>

DFPlayerMini_Fast mp3;
int volume = 15;

void PlayerSetup() {
    Serial2.begin(9600);
    mp3.begin(Serial2);
    mp3.volume(15);
    delay(20);
}

void PlayerStop() {
    if (mp3.isPlaying())
        mp3.stop();
}

int last_sound_idx = 0;
void PlayerPlay(int i) {
    if (i != last_sound_idx) {
        last_sound_idx = i;
        PlayerStop();
        mp3.loop(i);
    }
}

void PlayerSetSound(double power) {
    int idx = map(power, 0, 100, 2, 12);
    PlayerPlay(idx);
}

void PlayerTurningOnSound() {
    PlayerPlay(1);
}

void PlayerSetVolume(int vol) {
    mp3.volume(vol);
}

#endif