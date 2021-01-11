#include <Arduino.h>
#include <ServoSmooth.h>
// #include <Servo.h>
#include <Wire.h>

#define I2C_SLAVE_ADDRESS 11 

#define PAYLOAD_SIZE 2


const int servo_pin = 4;

int angle = 0;
int n = 0;
uint32_t myTimer;

ServoSmooth my_servo;

void requestEvents()
{
}

void receiveEvents(int numBytes)
{  
    n = Wire.read();
    if (angle != n) {
    // if (angle != n && (millis() - myTimer >= 40)) {
        angle = n;
        my_servo.setTargetDeg(180 - angle);
        myTimer = millis();
    }
}

void setup()
{
    Wire.begin(I2C_SLAVE_ADDRESS);
    delay(1000);               
    Wire.onRequest(requestEvents);
    Wire.onReceive(receiveEvents);
    my_servo.attach(servo_pin);
    my_servo.setAutoDetach(true);
    my_servo.setSpeed(18);
    my_servo.setAccel(0.05);
    my_servo.write(180); // 0km/h
}

void loop() {
    my_servo.tick();
}