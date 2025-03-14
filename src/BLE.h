#ifndef BLE_H
#define BLE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>  // Needed for notifications

#define SERVICE_UUID        "87654321-4321-6789-4321-098765fedcba"
#define CHARACTERISTIC_UUID "fedcba98-4321-6789-4321-098765fedcba"

class BLE {
public:
    BLEServer* pServer = nullptr;
    BLECharacteristic* pCharacteristic = nullptr;
    bool deviceConnected = false;  // Tracks connection status
    String receivedMessage = "";  

    // Unified callback class handling both server & characteristic events
    class MyServerCallbacks : public BLEServerCallbacks, public BLECharacteristicCallbacks {
    private:
        BLE& ble;  // Reference to BLE instance

    public:
        MyServerCallbacks(BLE& ble) : ble(ble) {}  // Constructor

        // GATT Server Connection Events
        void onConnect(BLEServer* pServer) override;
        void onDisconnect(BLEServer* pServer) override;

        // GATT Characteristic Write Event
        void onWrite(BLECharacteristic* pCharacteristic) override;
    };

    void begin(const char* deviceName = "Heltec LoRa 32 BLE");
    void loop();
    void sendMessageToUser(const String& message);
};

#endif
