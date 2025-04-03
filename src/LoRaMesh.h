#ifndef LORAMESH_H
#define LORAMESH_H

#include <Arduino.h>
#include <RadioLib.h>
#include <vector>
#include "Config.h" // Include the configuration file

extern Module radio;





// Node Entry Structure
struct NodeEntry {
    String nodeID;
    int senderID;
    float snr;
    unsigned long lastSeen;
};

class LoRaMesh {
public:
    // Constructor
    LoRaMesh(String nodeId, int nodeNumber);

    // Public Methods
    void setupLoRa();
    void sendHelloPacket();
    void listenForPackets();
    void updateDirectory(String senderID, int senderNumber, float snr);
    void printDirectory();
    void sendNodeDirectory();
    void sendMessage(String message);
    void sendNodeDirectoryToGUI();

    static const unsigned long BROADCAST_INTERVAL = 10000;  // 10 seconds
    static const unsigned long PRINT_INTERVAL = 20000;      // 20 seconds
private:
    String NODE_ID;
    int NODE_NUMBER;
    std::vector<NodeEntry> nodeDirectory;
    unsigned long lastBroadcast;
    unsigned long lastPrint;

    int state; // current state of the radio

    static void onReceive(uint8_t *buffer, uint16_t size);
};
#endif  // LORAMESH_H
