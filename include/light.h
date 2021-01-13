#ifndef __LIGHT_H__
#define __LIGHT_H__

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
const int led_7segment_length = 21;
const int led_7segment_pins[4] = { 41, 43, 45, 47 };

FireTimer danger_animation_timer;
FireTimer pipes_animation_timer;

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


int speed_delay[4] = { 50, 15, 50, 50 };
FireTimer pipe_timers[4];
void SetupPipes() {
    for (int i = 0; i < 4; i++) {
        pipe_timers[i].begin(speed_delay[i]);
    }
}

void ChangeAnimationSpeed(int n, int speed) {
    pipe_timers[n].begin(speed);
}

void LightSetup() {

    danger_animation_timer.begin(300);
    pipes_animation_timer.begin(100);

    for (int i = 0; i < 4; i++) {
        pipes_led[i].begin();
        pipes_led[i].clear();
        led_7segment[i].begin();
        led_7segment[i].clear();
        pipes_led[i].show();
        led_7segment[i].show();
    }

    speedometer_led.begin();
    speedometer_led.clear();
    speedometer_led.show();

    headlight_led.begin();
    headlight_led.clear();
    headlight_led.show();

    pinMode(headlight_pin, OUTPUT);
    digitalWrite(headlight_pin, LOW);

    SetupPipes();
}


void DisplayNumber(int n, int number) {
    uint32_t currentColor, blackColor;
    blackColor = led_7segment[n].Color(0, 0, 0);
    switch (n)
    {
    case 0:
        currentColor = led_7segment[n].Color(116, 100, 0); // yellow
        break;
    case 1:
        currentColor = led_7segment[n].Color(0, 90, 180); // blue
        break;
    case 2:
        currentColor = led_7segment[n].Color(0, 122, 16); // green
        break;
    case 3:
        currentColor = led_7segment[n].Color(122, 16, 0); // red
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

// disable all lights
void LightWaitStage() {
    for (int i = 0; i < speedometer_led_length; i++) {
        speedometer_led.setPixelColor(i, speedometer_led.Color(0, 0, 0));
    }

    for (int i = 0; i < headlight_led_length; i++) {
        headlight_led.setPixelColor(i, headlight_led.Color(0, 0, 0));
    }
    for (int i = 0; i < 4; i++) {
        pipes_led[i].clear();
        pipes_led[i].show();

        led_7segment[i].clear();
        led_7segment[i].show();
    }
    digitalWrite(headlight_pin, LOW);
    speedometer_led.show();
    headlight_led.show();
}


void LightReadyStage() {
    for (int i = 0; i < speedometer_led_length; i++) {
        speedometer_led.setPixelColor(i, speedometer_led.Color(95, 140, 0));
    }

    for (int i = 0; i < headlight_led_length; i++) {
        headlight_led.setPixelColor(i, headlight_led.Color(122, 122, 122));
    }
    speedometer_led.show();
    headlight_led.show();
    digitalWrite(headlight_pin, HIGH);
}


int light_i[4] = { -1, -1, -1, -1 };
boolean pipe_enable[4] = { false, true, false, false };
void CylonBounce(int n, byte red, byte green, byte blue, int EyeSize) {
    if (pipe_timers[n].fire()) {
        for (int i = 0; i < pipe_led_length; i++) {
            pipes_led[n].setPixelColor(i, pipes_led[n].Color(red / 4, green / 4, blue / 4));
        }
        if (pipe_enable[n]) {
            if (light_i[n] < 0)
                light_i[n] = pipe_led_length - EyeSize - 3;
            pipes_led[n].setPixelColor(light_i[n], pipes_led[n].Color(red / 2, green / 2, blue / 2));
            for(int j = 1; j <= EyeSize; j++) {
                pipes_led[n].setPixelColor(light_i[n] + j, pipes_led[n].Color(red, green, blue));
            }
            pipes_led[n].setPixelColor(light_i[n] + EyeSize + 1, pipes_led[n].Color(red / 2, green / 2, blue / 2));
            light_i[n]--;
        } else {
            light_i[n] = -1;
        }
            pipes_led[n].show();
    }
}

void LightWorkAnimation() {
    CylonBounce(0, 116, 100, 0, 4);
    CylonBounce(1, 0, 61, 122, 4);
    CylonBounce(2, 0, 122, 16, 4);
    CylonBounce(3, 122, 16, 0, 4);
}

int c = 0;
void LightDangerAnimation() {
    if (danger_animation_timer.fire()) {
        if (c != 0) {
            c = 0;
            for (int i = 0; i < speedometer_led_length; i++)
                speedometer_led.setPixelColor(i, speedometer_led.Color(95, 140, 0));
            
            for (int i = 0; i < headlight_led_length; i++) {
                headlight_led.setPixelColor(i, headlight_led.Color(122, 16, 0));
            }
        } else {
            c = 1;
            for (int i = 0; i < speedometer_led_length; i++)
                speedometer_led.setPixelColor(i, speedometer_led.Color(0, 140, 0));
            for (int i = 0; i < headlight_led_length; i++) {
                headlight_led.setPixelColor(i, headlight_led.Color(0, 0, 0));
            }
        }
        
        speedometer_led.show();
        headlight_led.show();
    }
}

void LightDangerStage() {
    for (int i = 0; i < speedometer_led_length; i++) {
        speedometer_led.setPixelColor(i, speedometer_led.Color(0, 122, 0));
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
