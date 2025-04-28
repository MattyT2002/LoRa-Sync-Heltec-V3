#pragma once

#include "NodeDirectory.h"
#include <Arduino.h>

class NodeManager {
public:
    NodeManager();

    void begin(uint16_t nodeId);
    void loop();  // To be called in main loop()

    // Called when new packet is received
    void handleLoRaPacket(const uint8_t* data, size_t len, int snr);
    void handleBLECommand(const String& command);  // BLE -> Node

    // Used to send info to BLE
    String getDirectoryJson() const;

    // Optional accessors
    const NodeDirectory& getDirectory() const;

private:
    uint16_t selfId;
    NodeDirectory directory;

    void processHelloPacket(uint16_t fromId, int snr);
    void processDirectoryPacket(const uint8_t* data, size_t len, uint16_t fromId);
    void broadcastHello();
    void sendDirectoryTo(uint16_t targetNodeId);

    unsigned long lastHelloSent = 0;
    const unsigned long helloInterval = 10000;
};
