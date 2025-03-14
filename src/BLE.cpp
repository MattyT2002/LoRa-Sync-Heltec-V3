#include "BLE.h"

void BLE::MyServerCallbacks::onConnect(BLEServer* pServer) {
    ble.deviceConnected = true;
    Serial.println("Device connected!");
}

void BLE::MyServerCallbacks::onDisconnect(BLEServer* pServer) {
    ble.deviceConnected = false;
    Serial.println("Device disconnected!");
    BLEDevice::startAdvertising();  // Restart advertising for new connections
}

void BLE::MyServerCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
    ble.receivedMessage = String(pCharacteristic->getValue().c_str());
    Serial.println("Received: " + ble.receivedMessage);

    Serial.println("Sending: " + ble.receivedMessage);
    pCharacteristic->setValue(ble.receivedMessage.c_str());
    pCharacteristic->notify();  // Notify connected client
}

void BLE::begin(const char* deviceName) {
    Serial.begin(115200);
    Serial.println("Starting Heltec LoRa 32 BLE...");

    BLEDevice::init(deviceName);
    pServer = BLEDevice::createServer();

    // Use the unified MyServerCallbacks class
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
    pCharacteristic->setCallbacks(callbacks);  // Same callback for characteristic events
    pService->start();

    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    BLEDevice::startAdvertising();

    Serial.println("BLE is now advertising...");
}

void BLE::loop() {
    delay(1000);
}

void BLE::sendMessageToUser(const String& message) {
    if (deviceConnected) {
        Serial.println("Sending message to web app: " + message);
        pCharacteristic->setValue(message.c_str());
        pCharacteristic->notify();
    } else {
        Serial.println("No device connected, unable to send message.");
    }
}
