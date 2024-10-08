#pragma once

#include <functional>
#include <mutex>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

class BindCharacteristicCallback : public BLECharacteristicCallbacks {
public:

    BindCharacteristicCallback(uint8_t* var, size_t len, std::mutex* mutex);
    BindCharacteristicCallback(uint8_t* var, size_t len, std::mutex* mutex, std::function<void()> onReadCallback);

private:

    uint8_t* _var;
    size_t _len;
    std::function<void()> _onReadCallback;
    std::mutex* _mutex;

    void onWrite(BLECharacteristic* characteristic, esp_ble_gatts_cb_param_t* param);
    void onRead(BLECharacteristic* characteristic, esp_ble_gatts_cb_param_t* param);
};

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer);
    void onDisconnect(BLEServer* pServer);
};

class BluetoothServer {
public:
    BluetoothServer(String name, String service_UUID);
    void start();

    template<typename T>
    void bindCharacteristic(T& var, String UUID) {
        BLECharacteristic* characteristic = _service->createCharacteristic(UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
        characteristic->setCallbacks(new BindCharacteristicCallback((uint8_t*) &var, sizeof(T), &bindMutex));
        characteristic->setValue((uint8_t*) &var, sizeof(T));
    }

    template<typename T>
    void bindCharacteristic(T& var, String UUID, std::function<void()> onReadCallback) {
        BLECharacteristic* characteristic = _service->createCharacteristic(UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
        characteristic->setCallbacks(new BindCharacteristicCallback((uint8_t*) &var, sizeof(T), &bindMutex, onReadCallback));
        characteristic->setValue((uint8_t*) &var, sizeof(T));
    }

    std::mutex bindMutex;

private:

    BLEServer* _server;
    BLEService* _service;

    String _service_UUID;
};