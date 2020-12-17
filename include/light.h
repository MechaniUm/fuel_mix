#ifndef __LIGHT_H__
#define __LIGHT_H__

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

const int pipe_led_length = 30;
const int pipe_led_pins[4] = { 7, 8, 9, 10 };
Adafruit_NeoPixel pipes_led[4] = {
    Adafruit_NeoPixel(pipe_led_length, pipe_led_pins[0], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(pipe_led_length, pipe_led_pins[1], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(pipe_led_length, pipe_led_pins[2], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(pipe_led_length, pipe_led_pins[3], NEO_GRB + NEO_KHZ800)
};

const int speedometer_led_pin = 4;
const int speedometer_led_length = 7;
Adafruit_NeoPixel speedometer_led(speedometer_led_length, speedometer_led_pin, NEO_GRB + NEO_KHZ800);

const int headlight_led_length = 20;
const int headlight_led_pin = 6;
Adafruit_NeoPixel headlight_led(headlight_led_length, headlight_led_pin, NEO_GRB + NEO_KHZ800);

const int headlight_pin = 11;

void LightSetup() {
    for (int i = 0; i < 4; i++) {
        pipes_led[i].begin();
        pipes_led[i].clear();
        for (int j = 0; j < pipe_led_length; j++) {
            pipes_led[i].setPixelColor(j, pipes_led[i].Color(0, 0, 0));
        }
        pipes_led[i].show();
    }

    speedometer_led.begin();
    speedometer_led.clear();
    for (int i = 0; i < speedometer_led_length; i++) {
        speedometer_led.setPixelColor(i, speedometer_led.Color(0, 0, 0));
    }
    speedometer_led.show();

    headlight_led.begin();
    headlight_led.clear();
    for (int i = 0; i < headlight_led_length; i++) {
        headlight_led.setPixelColor(i, headlight_led.Color(0, 0, 0));
    }
    headlight_led.show();

    pinMode(headlight_pin, OUTPUT);
    digitalWrite(headlight_pin, LOW);
}

void LightWaitStage() {
    for (int i = 0; i < speedometer_led_length; i++) {
        speedometer_led.setPixelColor(i, speedometer_led.Color(120, 120, 120));
    }

    for (int i = 0; i < headlight_led_length; i++) {
        headlight_led.setPixelColor(i, headlight_led.Color(120, 120, 120));
    }
    speedometer_led.show();
    headlight_led.show();
}

void LightDangerStage() {
    for (int i = 0; i < speedometer_led_length; i++) {
        speedometer_led.setPixelColor(i, speedometer_led.Color(255, 0, 0));
    }

    for (int i = 0; i < headlight_led_length; i++) {
        headlight_led.setPixelColor(i, headlight_led.Color(255, 0, 0));
    }
    speedometer_led.show();
    headlight_led.show();
    digitalWrite(headlight_pin, HIGH);
}

#endif
