// BLE_Heltec.h
#ifndef BLE_HELTEC_H
#define BLE_HELTEC_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID        "87654321-4321-6789-4321-098765fedcba"
#define CHARACTERISTIC_UUID "fedcba98-4321-6789-4321-098765fedcba"

class BLE_Heltec {
public:
    BLEServer* pServer = nullptr;
    BLECharacteristic* pCharacteristic = nullptr;
    bool deviceConnected = false;

    BLE_Heltec();
    void begin(const char* deviceName = "Heltec LoRa 32 BLE");
    void sendMessageToUser(const String& message);
    void setMessageCallback(void (*callback)(const String&));
    void sendNodeDirectory();
private:
    void (*messageCallback)(const String&);
    class MyServerCallbacks : public BLEServerCallbacks, public BLECharacteristicCallbacks {
    public:
        BLE_Heltec& ble;
        MyServerCallbacks(BLE_Heltec& bleInstance) : ble(bleInstance) {}

        void onConnect(BLEServer* pServer) override;
        void onDisconnect(BLEServer* pServer) override;
        void onWrite(BLECharacteristic* pCharacteristic) override;
    };
};

#endif

