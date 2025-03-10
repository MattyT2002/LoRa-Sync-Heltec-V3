#ifndef BLE_H
#define BLE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>  // Include the BLE2902 descriptor

#define SERVICE_UUID        "87654321-4321-6789-4321-098765fedcba"
#define CHARACTERISTIC_UUID "fedcba98-4321-6789-4321-098765fedcba"

// Class to manage the BLE server and communication
class BLE {
public:
    BLEServer* pServer = nullptr;
    BLECharacteristic* pCharacteristic = nullptr;
    bool deviceConnected = false;
    String receivedMessage = "";

    // Callbacks for server events
    class MyServerCallbacks : public BLEServerCallbacks {
        private:
        BLE& ble;  // Reference to BLEManager

    public:
        // Constructor to accept a reference to BLEManager
        MyServerCallbacks(BLE& ble) : ble(ble) {}

        void onConnect(BLEServer* pServer) override;
        void onDisconnect(BLEServer* pServer) override;
    };

    // Callbacks for characteristic events
    class MyCallbacks : public BLECharacteristicCallbacks {
    private:
        BLE& ble;  // Reference to BLE

    public:
        // Constructor to accept a reference to BLE
        MyCallbacks(BLE& ble) : ble(ble) {}

        void onWrite(BLECharacteristic* pCharacteristic) override;
    };

    // Initialize the BLE server and start advertising
    void begin(const char* deviceName = "Heltec LoRa 32 BLE");

    // Call this method in loop() to maintain the BLE connection
    void loop();
};

#endif
