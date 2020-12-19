#ifndef __LIGHT_H__
#define __LIGHT_H__

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

const int led_7segment_length = 21;
const int led_7segment_pins[4] = { 99, 99, 99, 99 };
int numbers[10][7] = {
  {1, 1, 1, 1, 1, 1, 0}, //0
  {0, 1, 1, 0, 0, 0, 0}, //1
  {1, 1, 0, 1, 1, 0, 1}, //2
  {1, 1, 1, 1, 0, 0, 1}, //3
  {0, 1, 1, 0, 0, 1, 1}, //4
  {1, 0, 1, 1, 0, 1, 1}, //5
  {1, 0, 1, 1, 1, 1, 1}, //6
  {1, 1, 1, 0, 0, 0, 0}, //7
  {1, 1, 1, 1, 1, 1, 1}, //8
  {1, 1, 1, 1, 0, 1, 1} //9
};
Adafruit_NeoPixel led_7segment[4] = {
    Adafruit_NeoPixel(led_7segment_length, led_7segment_pins[0], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(led_7segment_length, led_7segment_pins[1], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(led_7segment_length, led_7segment_pins[2], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(led_7segment_length, led_7segment_pins[3], NEO_GRB + NEO_KHZ800)
};


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
        led_7segment[i].begin();
        led_7segment[i].clear();
        pipes_led[i].show();
    }

    speedometer_led.begin();
    speedometer_led.clear();
    speedometer_led.show();

    headlight_led.begin();
    headlight_led.clear();
    headlight_led.show();

    pinMode(headlight_pin, OUTPUT);
    digitalWrite(headlight_pin, LOW);
}


void DisplayNumber(int n, int number) {
    uint32_t currentColor, blackColor;
    blackColor = led_7segment[n].Color(0, 0, 0);
    switch (n)
    {
    case 0:
        currentColor = led_7segment[n].Color(0, 255, 255); // yellow
        break;
    case 1:
        currentColor = led_7segment[n].Color(0, 0, 255); // blue
        break;
    case 2:
        currentColor = led_7segment[n].Color(0, 255, 0); // green
        break;
    case 3:
        currentColor = led_7segment[n].Color(255, 0, 0); // red
        break;
    default:
        break;
    }
    for (int j = 0; j <= 6; j++) {
        if (numbers[number % 10][j] > 0)
            led_7segment[n].setPixelColor(j + 14, currentColor);
        else
            led_7segment[n].setPixelColor(j + 14, blackColor);

        if (numbers[(number % 100) / 10][j] > 0)
            led_7segment[n].setPixelColor(j + 7, currentColor);
        else
            led_7segment[n].setPixelColor(j + 7, blackColor);

        if (numbers[(number % 1000) / 100][j] > 0)
            led_7segment[n].setPixelColor(j, currentColor);
        else
            led_7segment[n].setPixelColor(j, blackColor);
    }
    led_7segment[n].show();
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

void LightReadyStage() {
    for (int i = 0; i < 4; i++) {
        DisplayNumber(i, 0);
    }
    for (int i = 0; i < speedometer_led_length; i++) {
        speedometer_led.setPixelColor(i, speedometer_led.Color(255, 255, 255));
    }

    for (int i = 0; i < headlight_led_length; i++) {
        headlight_led.setPixelColor(i, headlight_led.Color(255, 255, 255));
    }
}

// TODO: add red danger animation ??
void LightDangerStage() {
    for (int i = 0; i < speedometer_led_length; i++) {
        speedometer_led.setPixelColor(i, speedometer_led.Color(255, 0, 0));
    }
    speedometer_led.show();
    headlight_led.show();
    digitalWrite(headlight_pin, HIGH);
}

void Led7SegmentOff() {
    for (int i = 0; i < 4; i++) {
        led_7segment[i].clear();
        led_7segment[i].show();
    }
}

#endif
