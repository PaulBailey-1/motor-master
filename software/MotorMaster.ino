
#include <map>
#include <ESP32Servo.h>

#include "BluetoothServer.h"
#include "CanConnection.h"
#include "CanMotorController.h"

// Add a postfix to distingish the device
String namePostfix = "";

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID "4f79cca2-cef2-4137-b4f1-c9cc7fda9ec7"
#define PWM1_ENABLE_CHARACTERISTIC_UUID "a506df5a-48b5-4f57-933c-214612bd77ef"
#define PWM1_CMD_CHARACTERISTIC_UUID "ead5e8a1-e5ea-4223-86e5-7ff501a37929"
#define PWM2_ENABLE_CHARACTERISTIC_UUID "1c9eb4ba-a11f-444b-aeca-0765f9387506"
#define PWM2_CMD_CHARACTERISTIC_UUID "c8bd31cf-6364-4aab-8949-d55cca13618f"
#define CAN_ENABLE_CHARACTERISTIC_UUID "73bb062a-be94-4a95-8fd0-8460d7a8af88"
#define CAN_INFO_CHARACTERISTIC_UUID "77723d0f-9668-466e-a3f2-56bc80a0eb9d"
#define CAN_CMD_CHARACTERISTIC_UUID "28332f90-0151-479a-8bab-f627c195533c"

const int PWM_OUTPUT_1_PIN = 21;
const int PWM_OUTPUT_2_PIN = 47;
const int CAN_TX_PIN = 9;
const int CAN_RX_PIN = 10;
const int CAN_LED_PIN = 11;
const int TEST_BTN_PIN = 2;
const int ESTOP_BTN_PIN = 48;
const int EXTERN_PIN = 1;

int testDC = 50;
bool estopped = false;

BluetoothServer* btServer;

Servo pwm1Servo;
Servo pwm2Servo;

bool pwm1Enabled = false;
int pwm1UserCmd = 50; // percent DC
int pwm1Cmd = 0;

bool pwm2Enabled = false;
int pwm2UserCmd = 50;
int pwm2Cmd = 0;

bool canEnabled = false;
char canInfo[256] = "";
CanMotorCmd canCmd = {-1, 0.0};
std::map<int, CanMotorController*> canDevices;

void queryCanDevices() {
    delay(300);

    for (auto i = canDevices.begin(); i != canDevices.end(); i++) {
        delete i->second;
    }
    
    // Dummy code
    canDevices[1] = new TalonFX(1);
    const char* info = "[ { \"id\": 1, \"name\": \"drive_motor\" } ]";
    strcpy(canInfo, info);
}

void sendHeartBeat() {
    twai_message_t message = {
        .identifier = 0x01011840,
        .data_length_code = 8,
        .data = {0},
    };
    message.data[4] = 0b00001000;
    if (canEnabled) message.data[4] |= 0b01000000;
    canSend(message);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting MotorMaster");

    btServer = new BluetoothServer("MotorMaster" + namePostfix, SERVICE_UUID);
    btServer->bindCharacteristic(pwm1Enabled, PWM1_ENABLE_CHARACTERISTIC_UUID);
    btServer->bindCharacteristic(pwm1UserCmd, PWM1_CMD_CHARACTERISTIC_UUID);
    btServer->bindCharacteristic(pwm2Enabled, PWM2_ENABLE_CHARACTERISTIC_UUID);
    btServer->bindCharacteristic(pwm2UserCmd, PWM2_CMD_CHARACTERISTIC_UUID);
    btServer->bindCharacteristic(canEnabled, CAN_ENABLE_CHARACTERISTIC_UUID);
    btServer->bindCharacteristic(canInfo, CAN_INFO_CHARACTERISTIC_UUID, queryCanDevices);
    btServer->bindCharacteristic(canCmd, CAN_CMD_CHARACTERISTIC_UUID);
    btServer->start();

    canStart(CAN_TX_PIN, CAN_RX_PIN);

    pinMode(CAN_LED_PIN, OUTPUT);
    pinMode(TEST_BTN_PIN, INPUT);
    pinMode(ESTOP_BTN_PIN, INPUT);
    // pinMode(EXTERN_PIN, INPUT);

    Serial.println("Started");

}

void loop() {

    // Read BT bound params
    btServer->bindMutex.lock();
    if (pwm1Enabled) {
        pwm1Cmd = pwm1UserCmd;
    } else {
        pwm1Cmd = 0;
    }
    if (pwm2Enabled) {
        pwm2Cmd = pwm2UserCmd;
    } else {
        pwm2Cmd = 0;
    }
    if (canCmd.id != -1) {
        if (canDevices.find(canCmd.id) == canDevices.end()) {
            Serial.printf("ERROR: Failed cmd nonexistant CAN device %i\n", canCmd.id);
        } else {
            canDevices[canCmd.id]->cmd = canCmd;
        }
        canCmd.id = -1;
    }
    btServer->bindMutex.unlock();
    sendHeartBeat();

    // Handle buttons
    bool testBtn = !digitalRead(TEST_BTN_PIN);
    if (testBtn) {
        pwm1Cmd = testDC;
        pwm2Cmd = testDC;
    }
    if (!digitalRead(ESTOP_BTN_PIN)) {
        estopped = true;
        Serial.println("e-stopped");
    }

    // Handle CAN
    digitalWrite(CAN_LED_PIN, canRecieve());
    getCanAlerts();
    // getCanStatus();

    if (estopped) {
        pwm1Enabled = false;
        pwm2Enabled = false;
        testBtn = false;
        canEnabled = false;
    }

    // Command outputs
    // Hopefully, because the bound bools are only being read, this doesn't need to be locked
    if (pwm1Servo.attached()) pwm1Servo.write(map(pwm1Cmd, -100, 100, 1, 179));
    if (!pwm1Enabled && !testBtn && pwm1Servo.attached()) pwm1Servo.detach();
    if ((pwm1Enabled || testBtn) && !pwm1Servo.attached()) pwm1Servo.attach(PWM_OUTPUT_1_PIN);

    if (pwm2Servo.attached()) pwm2Servo.write(map(pwm2Cmd, -100, 100, 1, 179));
    if (!pwm2Enabled && !testBtn && pwm2Servo.attached()) pwm2Servo.detach();
    if (pwm2Enabled || testBtn && !pwm2Servo.attached()) pwm2Servo.attach(PWM_OUTPUT_2_PIN);

    if (canEnabled) {
        for (auto i = canDevices.begin(); i != canDevices.end(); i++) {
            i->second->command();
        }
    }

    delay(20);
}