#include <functional>

#include "BluetoothServer.h"

BindCharacteristicCallback::BindCharacteristicCallback(uint8_t* var, size_t len, std::mutex* mutex) : BLECharacteristicCallbacks() {
    _var = var;
    _len = len;
    _mutex = mutex;
}

BindCharacteristicCallback::BindCharacteristicCallback(uint8_t* var, size_t len, std::mutex* mutex, std::function<void()> onReadCallback)
    : BindCharacteristicCallback(var, len, mutex) {
    _onReadCallback = onReadCallback;
}

void BindCharacteristicCallback::onWrite(BLECharacteristic* characteristic, esp_ble_gatts_cb_param_t* param) {
    if (param->write.len > _len) {
        Serial.println("Error: incoming data is to long");
    } else {
        _mutex->lock();
        memcpy(_var, param->write.value, param->write.len);
        _mutex->unlock();
        Serial.println("Data written for " + characteristic->getUUID().toString());
        // Serial.prinf("(%i bytes) ", param->write.len);
        // for (int i = 0; i < param->write.len; i++) {
        //     Serial.printf("%02X ", param->write.value[i]);
        // }
        // Serial.println();
    }
}

void BindCharacteristicCallback::onRead(BLECharacteristic* characteristic, esp_ble_gatts_cb_param_t* param) {
    if (_onReadCallback) _onReadCallback();
    characteristic->setValue(_var, _len);
}


void ServerCallbacks::onConnect(BLEServer* pServer) {
    Serial.println("Client Connected");
};

void ServerCallbacks::onDisconnect(BLEServer* pServer) {
    Serial.println("Client Disconnected");
    BLEDevice::startAdvertising();
}

BluetoothServer::BluetoothServer(String name, String service_UUID) {

    Serial.printf("Starting Bluetooth server for %s\n", name.c_str());
    BLEDevice::init(name);
    _server = BLEDevice::createServer();
    _server->setCallbacks(new ServerCallbacks());
    _service = _server->createService(service_UUID);
    _service_UUID = service_UUID;
}

void BluetoothServer::start() {
    _service->start();
    BLEAdvertising* advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(_service_UUID);
    advertising->setScanResponse(true);
    advertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    advertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
}

