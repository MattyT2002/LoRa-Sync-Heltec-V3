#ifndef LORAMESH_H
#define LORAMESH_H


#include "LoRaManager.h"
#include <vector>
#include <String.h>
#include "Packet.h"
#include "NodeDirectory.h"
#include "BLE_Heltec.h" 

#define MAX_FRAGMENT_SIZE 180
// Node structure to hold information about each node
struct Node {
    String nodeID;
    int senderID;
    float snr;
    unsigned long lastSeen;
};



// LoRaMesh class to manage LoRa communication and BLE interactions
class LoRaManager {
public:
    LoRaManager(String nodeId, int nodeNumber, NodeDirectory& nodeDirectory, BLE_Heltec& ble); // Constructor
    
    void setupLoRa(); // Initialize LoRa communication
    void sendHelloPacket(); // Send "Hello" packet to network
    void sendMessage(String message, int destinationNode); // Send a generic message
    void listenForPackets(); // Listen for incoming packets
    void processMessage(const String &str);
    void sendRawMessage(String msg);
    void sendDirectory(); // Send the node directory to network

private:
    String NODE_ID;
    int NODE_NUMBER;
    unsigned long lastBroadcast;
    unsigned long lastPrint;
    NodeDirectory& nodeDirectory; // Reference to the node directory
    BLE_Heltec& ble; // Reference to the BLE instance
    Packet packetManager; // Handles packet management
};

#endif // LORAMESH_H
