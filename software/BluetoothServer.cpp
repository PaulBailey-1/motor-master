
#include "BluetoothServer.h"

BindCharacteristicCallback::BindCharacteristicCallback(uint8_t* var, size_t len) : BLECharacteristicCallbacks() {
    _var = var;
    _len = len;
}

void BindCharacteristicCallback::onWrite(BLECharacteristic* characteristic) {
    size_t dataSize = characteristic->getLength();
    if (dataSize > _len) {
        Serial.println("Error: incoming data is to long");
    } else {
        uint8_t* data = characteristic->getData();
        memcpy(_var, data, dataSize);
        Serial.println("Data written for " + characteristic->getUUID().toString());
    }
}

void ServerCallbacks::onConnect(BLEServer* pServer) {
    Serial.println("Client Connected");
};

void ServerCallbacks::onDisconnect(BLEServer* pServer) {
    Serial.println("Client Disconnected");
    BLEDevice::startAdvertising();
}

BluetoothServer::BluetoothServer(String name, String service_UUID) {

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

