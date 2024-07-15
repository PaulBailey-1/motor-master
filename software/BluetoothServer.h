#pragma once

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

class BindCharacteristicCallback : public BLECharacteristicCallbacks {
public:
  BindCharacteristicCallback(uint8_t* var, size_t len);

private:

  uint8_t* _var;
  size_t _len;

  void onWrite(BLECharacteristic* characteristic);
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
        characteristic->setCallbacks(new BindCharacteristicCallback((uint8_t*) &var, sizeof(T)));
        characteristic->setValue((uint8_t*) &var, sizeof(T));
    }

private:

    BLEServer* _server;
    BLEService* _service;

    String _service_UUID;
};