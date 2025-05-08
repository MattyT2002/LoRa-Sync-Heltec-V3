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

}

void BLE_Heltec::begin(const char* deviceName) {
    Serial.println("Starting Heltec LoRa 32 BLE...");
    BLEDevice::init(deviceName);
    pServer = BLEDevice::createServer();

    static MyServerCallbacks callbacks(*this);  // static to avoid memory leak
    pServer->setCallbacks(&callbacks);

    BLEService* pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setCallbacks(&callbacks);  

    pService->start();
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    BLEDevice::startAdvertising();

    Serial.println("BLE is now advertising...");
}

void BLE_Heltec::sendMessageToUser(const String& message) {
    if (deviceConnected && pCharacteristic) {
        Serial.println("Sending message to Web App: " + message);
        const int chunkSize = 80;
        char buffer[chunkSize + 1];  // +1 for null terminator

        int msgLen = message.length();
        for (int i = 0; i < msgLen; i += chunkSize) {
            // Get substring chunk
            String chunk = message.substring(i, i + chunkSize);

            // Safely copy to buffer
            chunk.toCharArray(buffer, sizeof(buffer));

            // Set and notify
            pCharacteristic->setValue((uint8_t*)buffer, chunk.length());
            pCharacteristic->notify();

            delay(100);  // Allow client to process
        }
    } else {
        Serial.println("No device connected. Cannot send message.");
    }
}


void BLE_Heltec::setMessageCallback(void (*callback)(const String&)) {
    messageCallback = callback;
}


