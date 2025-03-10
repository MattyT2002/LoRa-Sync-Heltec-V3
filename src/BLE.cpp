#include "BLE.h"

void BLE::MyServerCallbacks::onConnect(BLEServer* pServer) {
    ble.deviceConnected = true;
    Serial.println("Device connected!");
}

void BLE::MyServerCallbacks::onDisconnect(BLEServer* pServer) {
    ble.deviceConnected = false;
    Serial.println("Device disconnected!");
    BLEDevice::startAdvertising();  // Restart advertising after disconnect
}

void BLE::MyCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
    ble.receivedMessage = String(pCharacteristic->getValue().c_str());
    Serial.println("Received: " + ble.receivedMessage);

    Serial.println("Sending: " + ble.receivedMessage);
    pCharacteristic->setValue(ble.receivedMessage.c_str());
    pCharacteristic->notify();  // Notify connected devices
}

void BLE::begin(const char* deviceName) {
    Serial.begin(115200);
    Serial.println("Starting Heltec LoRa 32 BLE...");

    BLEDevice::init(deviceName);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks(*this));

    BLEService* pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );

    // Add BLE2902 descriptor to enable notifications
    pCharacteristic->addDescriptor(new BLE2902());

    // Pass the instance of BLE to MyCallbacks
    pCharacteristic->setCallbacks(new MyCallbacks(*this));
    pService->start();

    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);

    BLEDevice::startAdvertising();
    Serial.println("BLE is now advertising...");
}

void BLE::loop() {
    // You can add additional logic here if needed, like handling other tasks
    delay(10);
}
