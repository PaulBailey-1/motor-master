
#include <ESP32Servo.h>
#include "BluetoothServer.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "4f79cca2-cef2-4137-b4f1-c9cc7fda9ec7"
#define PWM1_RUNNING_CHARACTERISTIC_UUID "a506df5a-48b5-4f57-933c-214612bd77ef"
#define PWM1_CMD_CHARACTERISTIC_UUID "ead5e8a1-e5ea-4223-86e5-7ff501a37929"
#define PWM2_RUNNING_CHARACTERISTIC_UUID "1c9eb4ba-a11f-444b-aeca-0765f9387506"
#define PWM2_CMD_CHARACTERISTIC_UUID "c8bd31cf-6364-4aab-8949-d55cca13618f"

const int PWM_OUTPUT_1 = 22;
const int PWM_OUTPUT_2 = 23;

BluetoothServer* btServer;

Servo pwm1Servo;
Servo pwm2Servo;

bool pwm1Running = false;
int pwm1UserCmd = 50;
int pwm1Cmd = 0;

bool pwm2Running = false;
int pwm2UserCmd = 50;
int pwm2Cmd = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE");

    btServer = new BluetoothServer("MotorMaster", SERVICE_UUID);
    btServer->bindCharacteristic(pwm1Running, PWM1_RUNNING_CHARACTERISTIC_UUID);
    btServer->bindCharacteristic(pwm1UserCmd, PWM1_CMD_CHARACTERISTIC_UUID);
    btServer->bindCharacteristic(pwm2Running, PWM2_RUNNING_CHARACTERISTIC_UUID);
    btServer->bindCharacteristic(pwm2UserCmd, PWM2_CMD_CHARACTERISTIC_UUID);
    btServer->start();

    pwm1Servo.attach(PWM_OUTPUT_1);
    pwm2Servo.attach(PWM_OUTPUT_2);

    Serial.println("Device started");
}

void loop() {
    if (pwm1Running) {
        pwm1Cmd = pwm1UserCmd;
    } else {
        pwm1Cmd = 0;
    }
    if (pwm2Running) {
        pwm2Cmd = pwm2UserCmd;
    } else {
        pwm2Cmd = 0;
    }
    pwm1Servo.write(map(pwm1Cmd, -100, 100, 1, 179));
    pwm2Servo.write(map(pwm2Cmd, -100, 100, 1, 179));

    delay(20);
}