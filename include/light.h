#ifndef __LIGHT_H__
#define __LIGHT_H__

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
const int led_7segment_length = 21;
const int led_7segment_pins[4] = { 41, 43, 45, 47 };

FireTimer danger_animation_timer;
FireTimer pipes_animation_timer;
FireTimer wait_animation_timer;

FireTimer fade_timer;

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

int anim_7_c = 0;
int anim_7[10][21];
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

float Gravity = -9.81;
int StartHeight = 1;

const int BallCount = 1;
float Height[BallCount];
float ImpactVelocityStart = sqrt( -2 * Gravity * StartHeight );
float ImpactVelocity[BallCount];
float TimeSinceLastBounce[BallCount];
int   Position[BallCount];
long  ClockTimeSinceLastBounce[BallCount];
float Dampening[BallCount];

void InitWaitAnim() {
    for (int i = 0 ; i < BallCount ; i++) {  
        ClockTimeSinceLastBounce[i] = millis();
        Height[i] = StartHeight;
        Position[i] = 0;
        ImpactVelocity[i] = ImpactVelocityStart;
        TimeSinceLastBounce[i] = 0;
        Dampening[i] = 0.90 - float(i)/pow(BallCount,2);
    }
}
void LightSetup() {

    InitWaitAnim();

    anim_7[0][0] = 1;
    anim_7[1][7] = 1;
    anim_7[2][14] = 1;
    anim_7[3][15] = 1;
    anim_7[4][16] = 1;
    anim_7[5][17] = 1;
    anim_7[6][10] = 1;
    anim_7[7][3] = 1;
    anim_7[8][4] = 1;
    anim_7[9][5] = 1;

    pipes_animation_timer.begin(100);
    wait_animation_timer.begin(100);
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

void DisableLights() {
    for (int i = 0; i < 4; i++) {
        pipes_led[i].clear();
        led_7segment[i].clear();
        pipes_led[i].show();
        led_7segment[i].show();
    }

    speedometer_led.clear();
    speedometer_led.show();

    headlight_led.clear();
    headlight_led.show();

    digitalWrite(headlight_pin, LOW);
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
    fade_timer.begin(3);
    wait_animation_timer.begin(100);
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

int cur_pipe = 0;
void BouncingBalls(int n, byte red, byte green, byte blue) {
    for (int i = 0 ; i < BallCount ; i++) {
        TimeSinceLastBounce[i] =  millis() - ClockTimeSinceLastBounce[i];
        Height[i] = 0.5 * Gravity * pow( TimeSinceLastBounce[i]/1000 , 2.0 ) + ImpactVelocity[i] * TimeSinceLastBounce[i]/1000;

        if ( Height[i] < 0 ) {
            cur_pipe = -1;      
            Height[i] = 0;
            ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
            ClockTimeSinceLastBounce[i] = millis();

            if ( ImpactVelocity[i] < 0.01 ) {
                ImpactVelocity[i] = ImpactVelocityStart;
            }
        }
        Position[i] = round( Height[i] * (pipe_led_length - 1) / StartHeight);
    }

    for (int i = 0 ; i < BallCount ; i++) {
        pipes_led[n].setPixelColor(Position[i],red,green,blue);
    }

    pipes_led[n].show();
    pipes_led[n].clear();
}

int fade_k = 0;
int fade_dir = 0;

void FadeInOut(int n, byte red, byte green, byte blue){
    float r, g, b;
    if (fade_timer.fire()) {
        r = (fade_k/256.0)*red;
        g = (fade_k/256.0)*green;
        b = (fade_k/256.0)*blue;
        for (int i = 0; i < pipe_led_length; i++) {
            if (cur_pipe == n)
                pipes_led[n].setPixelColor(i, pipes_led[n].Color(r, g, b));
            else
                pipes_led[n].setPixelColor(i, pipes_led[n].Color(0, 0, 0));
        }

        for (int i = 0; i < headlight_led_length; i++) {
            headlight_led.setPixelColor(i, headlight_led.Color(fade_k, fade_k, fade_k));
        }
        headlight_led.show();

        pipes_led[n].show();
        if (fade_dir == 0) {
            fade_k++;
        } else if (fade_dir == 1) {
            fade_k--;
        }
        if (fade_k == 255) {
            fade_dir = 1;
        } else if (fade_k == 0) {
            fade_dir = 0;
            cur_pipe++;
        }
    }
}

void LightWaitAnimation2() {
    if (cur_pipe >= 4) {
        cur_pipe = 0;
    }
    FadeInOut(0, 58, 50, 0);
    FadeInOut(1, 0, 30, 61);
    FadeInOut(2, 0, 61, 8);
    FadeInOut(3, 61, 8, 0);
}

void LightWaitAnimation() {
    if (cur_pipe >= 4) {
        cur_pipe = 0;
    }
    FadeInOut(0, 58, 50, 0);
    FadeInOut(1, 0, 30, 61);
    FadeInOut(2, 0, 61, 8);
    FadeInOut(3, 61, 8, 0);


    if (wait_animation_timer.fire()) {

        if (anim_7_c == 10) {
            anim_7_c = 0;
        }
        for (int i = 0; i < 21; i++) {
            led_7segment[0].setPixelColor(i, anim_7[anim_7_c][i] * led_7segment[0].Color(116, 100, 0));
            led_7segment[1].setPixelColor(i, anim_7[anim_7_c][i] * led_7segment[1].Color(0, 90, 180));
            led_7segment[2].setPixelColor(i, anim_7[anim_7_c][i] * led_7segment[2].Color(0, 122, 16));
            led_7segment[3].setPixelColor(i, anim_7[anim_7_c][i] * led_7segment[3].Color(122, 16, 0));
        }

        led_7segment[0].show();
        led_7segment[1].show();
        led_7segment[2].show();
        led_7segment[3].show();
        anim_7_c++;
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
    LightWorkAnimation();
    if (danger_animation_timer.fire()) {
        if (c != 0) {
            c = 0;
            for (int i = 0; i < speedometer_led_length; i++)
                speedometer_led.setPixelColor(i, speedometer_led.Color(95, 140, 0));
            
            for (int i = 0; i < headlight_led_length; i++) {
                headlight_led.setPixelColor(i, headlight_led.Color(122, 16, 0));
            }

            // for (int i = 0; i < pipe_led_length; i++) {
            //     pipes_led[0].setPixelColor(i, pipes_led[0].Color(29, 25, 0));
            //     pipes_led[1].setPixelColor(i, pipes_led[1].Color(0, 61 / 4, 122 / 4));
            //     pipes_led[2].setPixelColor(i, pipes_led[2].Color(0, 122 / 4, 4));
            //     pipes_led[3].setPixelColor(i, pipes_led[3].Color(122 / 4, 4, 0));
            // }
        } else {
            c = 1;
            for (int i = 0; i < speedometer_led_length; i++)
                speedometer_led.setPixelColor(i, speedometer_led.Color(16, 122, 0));
            for (int i = 0; i < headlight_led_length; i++) {
                headlight_led.setPixelColor(i, headlight_led.Color(0, 0, 0));
            }
            // for (int j = 0; j < 4; j++) {
            //     for (int i = 0; i < pipe_led_length; i++) {
            //         pipes_led[j].setPixelColor(i, pipes_led[0].Color(140, 0, 0));
            //     }
            // }
        }
        
        speedometer_led.show();
        headlight_led.show();
        // for (int i = 0; i < 4; i++) {
        //     pipes_led[i].show();
        // }
    }
}

void LightDangerStage() {
    for (int i = 0; i < speedometer_led_length; i++) {
        speedometer_led.setPixelColor(i, speedometer_led.Color(0, 122, 0));
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            pipes_led[i].setPixelColor(j, 0);
        }
        pipes_led[i].show();
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
