// BLE_Nano33.h
#ifndef BLE_NANO33_H
#define BLE_NANO33_H

#include <Arduino.h>
#include <ArduinoBLE.h>


#define SERVICE_UUID        "87654321-4321-6789-4321-098765fedcba"
#define CHARACTERISTIC_UUID "fedcba98-4321-6789-4321-098765fedcba"

class BLE_Nano33 {
public:
    BLE_Nano33();
    void begin(const char* deviceName = "Nano 33 BLE");
    void sendMessageToUser(const String& message);
    void setMessageCallback(void (*callback)(const String&));

private:
    void (*messageCallback)(const String&);
};

#endif
