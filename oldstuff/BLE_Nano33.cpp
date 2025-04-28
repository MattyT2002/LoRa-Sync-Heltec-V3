
#include "BLE_Nano33.h"

BLE_Nano33::BLE_Nano33() : messageCallback(nullptr) {}

void BLE_Nano33::begin(const char* deviceName) {
    Serial.println("Starting Arduino Nano 33 BLE...");
    if (!BLE.begin()) {
        Serial.println("Starting BLE failed!");
        while (1);
    }

    BLE.setLocalName(deviceName);
    BLE.setAdvertisedServiceUuid(SERVICE_UUID);

    BLEService service(SERVICE_UUID);
    BLECharacteristic characteristic(CHARACTERISTIC_UUID, 
        BLERead | BLEWrite | BLENotify);

    service.addCharacteristic(characteristic);
    BLE.addService(service);

    characteristic.writeValue("Hello Web App!");

    BLE.advertise();
    Serial.println("BLE is now advertising...");
}

void BLE_Nano33::sendMessageToUser(const String& message) {
    Serial.println("Sending (Nano33BLE): " + message);
    // Add your characteristic reference here and call writeValue() to send data
}

void BLE_Nano33::setMessageCallback(void (*callback)(const String&)) {
    messageCallback = callback;
}
