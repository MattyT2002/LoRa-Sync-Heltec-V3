// BLE_Heltec.cpp
#include "BLE_Heltec.h"

BLE_Heltec::BLE_Heltec() : messageCallback(nullptr) {}

void BLE_Heltec::MyServerCallbacks::onConnect(BLEServer* pServer) {
    ble.deviceConnected = true;
    Serial.println("Device connected!");
}

void BLE_Heltec::MyServerCallbacks::onDisconnect(BLEServer* pServer) {
    ble.deviceConnected = false;
    Serial.println("Device disconnected!");
    BLEDevice::startAdvertising();
}

void BLE_Heltec::MyServerCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
    String received = String(pCharacteristic->getValue().c_str());
    Serial.println("Received from Web App: " + received);

    if (ble.messageCallback) {
        ble.messageCallback(received);
    }

    pCharacteristic->setValue(received.c_str());
    pCharacteristic->notify();
}

void BLE_Heltec::begin(const char* deviceName) {
    Serial.println("Starting Heltec LoRa 32 BLE...");
    BLEDevice::init(deviceName);
    pServer = BLEDevice::createServer();

    MyServerCallbacks* callbacks = new MyServerCallbacks(*this);
    pServer->setCallbacks(callbacks);

    BLEService* pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setCallbacks(callbacks);

    pService->start();
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    BLEDevice::startAdvertising();

    Serial.println("BLE is now advertising...");
}

void BLE_Heltec::sendMessageToUser(const String& message) {
    if (deviceConnected && pCharacteristic) {
        Serial.println("Sending message to Web App: " + message);
        int chunkSize = 100;
        for (int i = 0; i < message.length(); i += chunkSize) {
            String chunk = message.substring(i, i + chunkSize);
            pCharacteristic->setValue(chunk.c_str());
            pCharacteristic->notify();
            delay(50);
        }
    } else {
        Serial.println("No device connected. Cannot send message.");
    }
}

void BLE_Heltec::setMessageCallback(void (*callback)(const String&)) {
    messageCallback = callback;
}


