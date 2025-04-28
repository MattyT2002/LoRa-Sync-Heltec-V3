#include "NodeManager.h"

NodeManager::NodeManager() {}

void NodeManager::begin(uint16_t nodeId) {
    selfId = nodeId;
    directory.setSelfId(nodeId);
}

void NodeManager::loop() {
    unsigned long now = millis();
    if (now - lastHelloSent > helloInterval) {
        broadcastHello();
        lastHelloSent = now;
    }
}

void NodeManager::handleLoRaPacket(const uint8_t* data, size_t len, int snr) {
    if (len < 2) return;

    uint8_t packetType = data[0];
    uint16_t fromId = data[1];

    switch (packetType) {
        case 0x01:  // Hello
            processHelloPacket(fromId, snr);
            break;
        case 0x02:  // Directory
            processDirectoryPacket(data + 2, len - 2, fromId);
            break;
        default:
            Serial.println("Unknown packet type");
    }
}

void NodeManager::handleBLECommand(const String& command) {
    if (command == "getDirectory") {
        // handled externally via getDirectoryJson()
    }
    // Add more BLE commands as needed
}

String NodeManager::getDirectoryJson() const {
    return directory.toVisJson().c_str();
}

const NodeDirectory& NodeManager::getDirectory() const {
    return directory;
}

void NodeManager::processHelloPacket(uint16_t fromId, int snr) {
    directory.updateNeighbourNode(fromId, snr, millis());
    sendDirectoryTo(fromId);  // Respond with this node's directory
}

void NodeManager::processDirectoryPacket(const uint8_t* data, size_t len, uint16_t fromId) {
    // Placeholder: decode fromId's directory and merge
    NodeDirectory otherDir;
    // TODO: deserialize from compact binary format
    directory.mergeDirectory(otherDir, fromId);
}

void NodeManager::broadcastHello() {
    // TODO: build and send hello packet over LoRa
    Serial.println("Sending hello packet...");
}

void NodeManager::sendDirectoryTo(uint16_t targetNodeId) {
    // TODO: serialize and send directory over LoRa
    Serial.print("Sending directory to node ");
    Serial.println(targetNodeId);
}
